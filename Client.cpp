#include <iostream> // призван предоставить средства ввода-вывода, для стандартной консоли, т.е. то, что вводит с клавиатуры и читает с экрана пользователь.
#include <WS2tcpip.h> // для работы с IP-адресами
#include <string> //  класс с методами и переменными для организации работы со строками

#pragma comment (lib, "ws2_32.lib") // директива, с помощью которой можно добавить имя библиотеки по умолчанию уже после добавленной библиотеки

using namespace std;

void main()
{
	string ipAddress = "127.0.0.1";			// IP-адрес сервера
	int port = 54000;						// Прослушивание порта # на сервере

											// Инициализация WinSock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		cerr << "Can't start Winsock, Err #" << wsResult << endl;
		return;
	}

	// Создание socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// Заполнение структур подсказок
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Подключение к серверу
	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cerr << "Can't connect to server, Err #" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		system("pause");
		return;
	}

	// Цикл do-while для отправки и получения данных
	char buf[4096];
	string userInput;

	do
	{
		// Подсказка текста пользователю 
		cout << "> ";
		getline(cin, userInput);

		if (userInput.size() > 0)		// Нужно убедиться, что пользователь набрал что-то
		{
			// Отправить текст
			int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
			if (sendResult != SOCKET_ERROR)
			{
				// Подождите ответа
				ZeroMemory(buf, 4096);
				int bytesReceived = recv(sock, buf, 4096, 0);
				if (bytesReceived > 0)
				{
					// Эхо-ответ на консоль
					cout << "SERVER> " << string(buf, 0, bytesReceived) << endl;
				}
			}
		}

	} while (userInput.size() > 0);

	// Закрытие всех окон и программ
	closesocket(sock);
	WSACleanup();
}
