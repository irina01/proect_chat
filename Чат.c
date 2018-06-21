#include "stdafx.h"
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>


#pragma comment (lib, "ws2_32.lib")

using namespace std;


void main()
{
	// Инициализация windows socket для работы с сервером
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cerr << "Can't Initialize winsock! Quitting" << endl;
		return;
	}

	// Создание
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Quitting" << endl;
		return;
	}

	// Привязывание IP-адрес и порт с гнездом
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// Указание winsocket, что socket для прослушивания
	listen(listening, SOMAXCONN);

	// Создание набора дескрипторов главного файла и установление его на нуль
	fd_set master;
	FD_ZERO(&master);

	// Добавление нашего первого сокета
	FD_SET(listening, &master);

	bool running = true;

	while (running)
	{
		fd_set copy = master;

		// Смотрите, кто говорит с нами
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		// Просмотр всех текущих соединений (потенциальное соединение)
		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];

			if (sock == listening)
			{
				// Принятие нового соединения
				SOCKET client = accept(listening, nullptr, nullptr);

				// Добавление нового соединения в список подключенных клиентов
				FD_SET(client, &master);

				// Отправка приветственного сообщения подключенному клиенту
				string welcomeMsg = "Welcome to the Awesome Chat Server!\r\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);
			}
			else // Это входящее сообщение
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				// Получение сообщения
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					// Убрать своего собеседника
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

					// Отправка сообщения другим собеседникам

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


	// Удаление прослушивающего сокета из набора дескрипторов главного файла и его закрытие,чтобы никто не пытался подключиться
	FD_CLR(listening, &master);
	closesocket(listening);

	// Это сообщение для того, чтобы пользователи знали, что происходит в чате
	string msg = "Server is shutting down. Goodbye\r\n";

	while (master.fd_count > 0)
	{
		// Получение номера сокета
		SOCKET sock = master.fd_array[0];

		// сообщение "до свидания"
		send(sock, msg.c_str(), msg.size() + 1, 0);

		// Удаление его из списка основных файлов и закрытие сокета
		FD_CLR(sock, &master);
		closesocket(sock);
	}

	// Чистка winsock
	WSACleanup();
	system("pause");
}
