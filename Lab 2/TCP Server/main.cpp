#include "../UDP Server/Sockets.h"
#pragma comment(lib, "ws2_32.lib")

int main(int argc, char** argv)
{
	InitSockets();
	int result = 0;
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	SOCKET s2;
	const char* message_to_send = "pong";
	char message_received[MAX_BUFFER_SIZE];

	printf("Welcome to server TCP\n");

	sockaddr_in userAddress;
	sockaddr_in myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_port = htons(PORT);
	myAddress.sin_addr.S_un.S_addr = INADDR_ANY;

	result = bind(s, (const struct sockaddr*)&myAddress, sizeof(myAddress));

	if (result == SOCKET_ERROR)
	{
		printWSErrorAndExit("Error: Socket bind failed");
	}

	result = listen(s, 1);

	if (result == SOCKET_ERROR)
	{
		printWSErrorAndExit("Error: Socket listen failed");
	}
	else 
	{
		int size = sizeof(userAddress);
		s2 = accept(s, (sockaddr*)&userAddress, &size);
	}

	while (true)
	{
		// Recieve message from client
		int size = sizeof(userAddress);
		result = recv(s2, message_received, MAX_BUFFER_SIZE, 0);

		if (result == SOCKET_ERROR) {
			printWSErrorAndExit("Error: Message not recived. Client Disconnected");
			printf_s("Client Disconnected");
			break;
		}
		else
		{
			message_received[result] = '\0';
			printf_s("%s\n", message_received);
			Sleep(500);
		}

		// Send message to client
		result = send(s2, message_to_send, sizeof(message_to_send), 0);
		if (result == SOCKET_ERROR) {
			printWSErrorAndExit("Error: Message not sent. Client Disconnected");
			printf_s("Client Disconnected");
			break;
		}
	}

	printf_s("\nServer Finished\n");

	closesocket(s);
	CleanUpSockets();
	system("pause");
	return 0;
}