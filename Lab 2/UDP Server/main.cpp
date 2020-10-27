#include "Sockets.h"
#pragma comment(lib, "ws2_32.lib")

int main(int argc, char** argv)
{
	InitSockets();
	int result = 0;
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
	const char* message_to_send = "pong";
	char message_received[MAX_BUFFER_SIZE];

	printf("Welcome to server UDP\n");

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

	while (true)
	{
		// Recieve message from client
		int size = sizeof(userAddress);
		result = recvfrom(s, message_received, MAX_BUFFER_SIZE, 0, (sockaddr*)&userAddress, &size);

		if (result == SOCKET_ERROR)
			printWSErrorAndExit("Error: Message not received");
		else
		{
			message_received[result] = '\0';
			printf_s("%s\n", message_received);
			Sleep(500);
		}

		// Send message to client
		result = sendto(s, message_to_send, sizeof(message_to_send), 0, (sockaddr*)(&userAddress), sizeof(userAddress));
		if (result == SOCKET_ERROR)
			printWSErrorAndExit("Error: Message not sent");
	}


	closesocket(s);
	CleanUpSockets();
	system("pause");
	return 0;
}