#include <iostream> // ������� ������������ �������� �����-������, ��� ����������� �������, �.�. ��, ��� ������ � ���������� � ������ � ������ ������������.
#include <WS2tcpip.h> // ��� ������ � IP-��������
#include <string> //  ����� � �������� � ����������� ��� ����������� ������ �� ��������
#include <sstream> // ������������ ���� � ��������, ��������� � ����������� ��� ����������� ������ �� ��������, ����� ��������� �������

#pragma comment (lib, "ws2_32.lib") // ���������, � ������� ������� ����� �������� ��� ���������� �� ��������� ��� ����� ����������� ����������

using namespace std;

void main()
{
	setlocale(LC_ALL, "Russian");
	// ������������� windows socket ��� ������ � ��������
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cerr << "Can't Initialize winsock! Quitting" << endl;
		system("pause");
		return;
	}

	// �������� ������
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Quitting" << endl;
		system("pause");
		return;
	}

	// ������������ IP-������ � ����� � �������
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// ��������, ��� ����� ������������ ��� �������������
	listen(listening, SOMAXCONN);

	// �������� ������ ������������ �������� ����� � ������������ ��� �� ����
	fd_set master;
	FD_ZERO(&master);

	// ���������� ������ ������� ������ 
	FD_SET(listening, &master);

	bool running = true;

	while (running)
	{
		fd_set copy = master;

		// �������� ����, ��� ������� � ����
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		// �������� ���� ������� ���������� (������������� ����������)
		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];

			if (sock == listening)
			{
				// �������� ������ ����������
				SOCKET client = accept(listening, nullptr, nullptr);

				// ���������� ������ ���������� � ������ ������������ ��������
				FD_SET(client, &master);

				// Send a welcome message to the connected client
				/*string welcomeMsg = "Welcome to the Awesome Chat Server!\r\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);	*/
			}
			else // ��� �������� ���������
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				// ��������� ���������
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					// ������ ������ �����������
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

					// �������� ��������� ������ ������������

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


	// �������� ��������������� ������ �� ������ ������������ �������� ����� � ��� ��������,����� ����� �� ������� ������������
	FD_CLR(listening, &master);
	closesocket(listening);

	// ��� ��������� ��� ����, ����� ������������ �����, ��� ���������� � ����
	string msg = "Server is shutting down. Goodbye\r\n";

	while (master.fd_count > 0)
	{
		// ��������� ������ ������
		SOCKET sock = master.fd_array[0];

		// ��������� "�� ��������"
		send(sock, msg.c_str(), msg.size() + 1, 0);

		// �������� ��� �� ������ �������� ������ � �������� ������
		FD_CLR(sock, &master);
		closesocket(sock);
	}

	// ������ winsock
	WSACleanup();
	system("pause");
}
