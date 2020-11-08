#include "Networks.h"
#include "ModuleNetworking.h"
#include <list>
#include <map>
static uint8 NumModulesUsingWinsock = 0;

void ModuleNetworking::reportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	ELOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
}

void ModuleNetworking::disconnectAllSockets()
{
	for (SOCKET socket : sockets)
	{
		shutdown(socket, 2);
		closesocket(socket);
	}

	sockets.clear();
}

bool ModuleNetworking::init()
{
	if (NumModulesUsingWinsock == 0)
	{
		NumModulesUsingWinsock++;

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(version, &data) != 0)
		{
			reportError("ModuleNetworking::init() - WSAStartup");
			return false;
		}
	}

	return true;
}

bool ModuleNetworking::preUpdate()
{
	if (sockets.empty()) return true;

	// Get listened sockets -------------------------------

	// New socket set
	fd_set readfds;
	FD_ZERO(&readfds);

	// Fill the set
	for (auto s : sockets) {
		FD_SET(s, &readfds);
	}

	// Timeout (return immediately)
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// Select (check for readability)
	int res = select(0, &readfds, nullptr, nullptr, &timeout);
	if (res == SOCKET_ERROR) {
		reportError("Select readfds failed");
	}

	// Process listened sockets ----------------------------

	std::list<SOCKET> disconnectedSockets;

	for (int i = 0; i < readfds.fd_count; ++i) {

		SOCKET s = readfds.fd_array[i];

		if (FD_ISSET(s, &readfds)) {
			if (isListenSocket(s)) { // Is the server socket
			    // Accept stuff
				sockaddr_in userAddress;
				int size = sizeof(userAddress);
				SOCKET new_connection = accept(s, (sockaddr*)&userAddress, &size);
				addSocket(new_connection);
				onSocketConnected(new_connection, userAddress);
			}
			else { // Is a client socket
				// Recv stuff
				InputMemoryStream packet;
				int byteRead = recv(s, packet.GetBufferPtr(), packet.GetCapacity(), 0);

				if (byteRead > 0)
				{
					packet.SetSize((uint32)byteRead);
					onSocketReceivedData(s, packet);
				}
				else {
					reportError("Disconnection");
					disconnectedSockets.push_back(s);
				}

			}
		}
	}

	// Delete Disconnected Sockets ----------------------

	for (auto s : disconnectedSockets)
	{
		DisconnectSocket(s, DisconnectionType::Error);

	}

	return true;
}

void ModuleNetworking::DisconnectSocket(SOCKET& s, DisconnectionType t)
{
	onSocketDisconnected(s, t);
	std::vector<SOCKET>::iterator itr = std::find(sockets.begin(), sockets.end(), s);
	sockets.erase(itr);
}

bool ModuleNetworking::cleanUp()
{
	disconnectAllSockets();

	NumModulesUsingWinsock--;
	if (NumModulesUsingWinsock == 0)
	{

		if (WSACleanup() != 0)
		{
			reportError("ModuleNetworking::cleanUp() - WSACleanup");
			return false;
		}
	}

	return true;
}

bool ModuleNetworking::IsValidCommand(std::string _command)
{
	for (auto& command : commands)
	{
		if (command == _command)
		{
			return true;
		}
	}

	return false;
}

void ModuleNetworking::addSocket(SOCKET socket)
{
	sockets.push_back(socket);
}

bool ModuleNetworking::sendPacket(const OutputMemoryStream& packet, SOCKET socket)
{
	int result = send(socket, packet.GetBufferPtr(), packet.GetSize(), 0);
	if (result == SOCKET_ERROR) {
		reportError("Send Packet");
		return false;
	}
	return true;
}

#include <string>
#include <ctime>

std::string ModuleNetworking::GetTime()
{
	const time_t now = time(0);
	tm ltm;
	localtime_s(&ltm ,&now);
	return  std::to_string(ltm.tm_hour) + ":" + std::to_string(ltm.tm_min);
}

