#include "../UDP Server/Sockets.h"
#pragma comment(lib, "ws2_32.lib")

int main(int argc, char** argv)
{
	InitSockets();
	int times = 5;
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	const char* message_to_send = "ping";
	char message_buffer[MAX_BUFFER_SIZE];
	int result = 0;

	printf("Welcome to client TCP\n");

	sockaddr_in fromAddress;
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	const char* remoteAddrStr = "127.0.0.1";
	inet_pton(AF_INET, remoteAddrStr, &serverAddress.sin_addr);

	result = connect(s, (const sockaddr*)&serverAddress, sizeof(serverAddress));

	if (result == SOCKET_ERROR)
		printWSErrorAndExit("Error: Connection to server failed");

	while (times > 0)
	{
		// Send message 
		result = send(s, message_to_send, sizeof(message_to_send), 0);
		if (result == SOCKET_ERROR) {
			printWSErrorAndExit("Error: Message not sent. Server Disconnected");
			printf_s("Client Disconnected");
			break;
		}
		// Recieve message
		int size = sizeof(fromAddress);
		result = recv(s, message_buffer, MAX_BUFFER_SIZE, 0);

		if (result == SOCKET_ERROR) {
			printWSErrorAndExit("Error: Message not received. Server Disconnected");
			printf_s("Client Disconnected");
			break;
		}
		else
		{
			message_buffer[result] = '\0';
			printf_s("%s\n", message_buffer);
			Sleep(500);
		}

		--times;
	}

	printf_s("\nClient Finished\n");

	closesocket(s);
	CleanUpSockets();
	system("pause");
	return 0;
}
