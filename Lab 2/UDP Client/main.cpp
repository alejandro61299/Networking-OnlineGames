#include "../UDP Server/Sockets.h"
#pragma comment(lib, "ws2_32.lib")

int main(int argc, char** argv)
{
	InitSockets();
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
	const char* message_to_send = "ping";
	char message_buffer[MAX_BUFFER_SIZE];
	int result = 0;

	printf("Welcome to client UDP\n");

	sockaddr_in fromAddress;
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	const char* remoteAddrStr = "127.0.0.1";
	inet_pton(AF_INET, remoteAddrStr, &serverAddress.sin_addr);

	while (true)
	{
		// Send message 
		result = sendto(s, message_to_send, sizeof(message_to_send), 0, (sockaddr*)(&serverAddress), sizeof(serverAddress));
		if (result == SOCKET_ERROR)
			printWSErrorAndExit("Error: Message not sent");

		// Recieve message
		int size = sizeof(fromAddress);
		result = recvfrom(s, message_buffer, MAX_BUFFER_SIZE, 0, (sockaddr*)&fromAddress, &size);

		if (result == SOCKET_ERROR)
			printWSErrorAndExit("Error: Message not received");
		else
		{
			message_buffer[result] = '\0';
			printf_s("%s\n", message_buffer);
			Sleep(500);
		}
	}

	closesocket(s);
	CleanUpSockets();
	system("pause");
	return 0;
}
