#include <iostream> // призван предоставить средства ввода-вывода, дл€ стандартной консоли, т.е. то, что вводит с клавиатуры и читает с экрана пользователь.
#include <WS2tcpip.h> // дл€ работы с IP-адресами
#include <string> //  класс с методами и переменными дл€ организации работы со строками
#include <sstream> // заголовочный файл с классами, функци€ми и переменными дл€ организации работы со строками, через интерфейс потоков

#pragma comment (lib, "ws2_32.lib") // директива, с помощью которой можно добавить им€ библиотеки по умолчанию уже после добавленной библиотеки

using namespace std;

void main()
{
	setlocale(LC_ALL, "Russian");
	// »нициализаци€ windows socket дл€ работы с сервером
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cerr << "Can't Initialize winsock! Quitting" << endl;
		system("pause");
		return;
	}

	// —оздание сокета
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Quitting" << endl;
		system("pause");
		return;
	}

	// ѕрив€зывание IP-адреса и порта с гнездом
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// ”казание, что сокет предназначен дл€ прослушивани€
	listen(listening, SOMAXCONN);

	// —оздание набора дескрипторов главного файла и установление его на нуль
	fd_set master;
	FD_ZERO(&master);

	// ƒобавление нашего первого сокета 
	FD_SET(listening, &master);

	bool running = true;

	while (running)
	{
		fd_set copy = master;

		// ѕросмотр того, кто говорит с нами
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		// ѕросмотр всех текущих соединений (потенциальное соединение)
		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];

			if (sock == listening)
			{
				// ѕрин€тие нового соединени€
				SOCKET client = accept(listening, nullptr, nullptr);

				// ƒобавление нового соединени€ в список подключенных клиентов
				FD_SET(client, &master);

				// Send a welcome message to the connected client
				/*string welcomeMsg = "Welcome to the Awesome Chat Server!\r\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);	*/
			}
			else // Ёто вход€щее сообщение
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				// ѕолучение сообщени€
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					// ”брать своего собеседника
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					if (buf[0] == '\\')
					{
						string cmd = string(buf, bytesIn);
						if (cmd == "\\quit")
						{
							running = false;
							break;
						}
						continue;
					}

					// ќтправка сообщени€ другим собеседникам

					for (int i = 0; i < master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];
						if (outSock != listening && outSock != sock)
						{
							ostringstream ss;
							ss << "SOCKET #" << sock << ": " << buf << "\r\n";
							string strOut = ss.str();

							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
						}
					}
				}
			}
		}
	}


	// ”даление прослушивающего сокета из набора дескрипторов главного файла и его закрытие,чтобы никто не пыталс€ подключитьс€
	FD_CLR(listening, &master);
	closesocket(listening);

	// Ёто сообщение дл€ того, чтобы пользователи знали, что происходит в чате
	string msg = "Server is shutting down. Goodbye\r\n";

	while (master.fd_count > 0)
	{
		// ѕолучение номера сокета
		SOCKET sock = master.fd_array[0];

		// сообщение "до свидани€"
		send(sock, msg.c_str(), msg.size() + 1, 0);

		// ”даление его из списка основных файлов и закрытие сокета
		FD_CLR(sock, &master);
		closesocket(sock);
	}

	// „истка winsock
	WSACleanup();
	system("pause");
}
