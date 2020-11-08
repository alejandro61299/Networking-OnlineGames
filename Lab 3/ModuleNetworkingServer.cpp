#include "ModuleNetworkingServer.h"

//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
	int result = 0;
	// TODO(jesus): TCP listen socket stuff
	// - Create the listenSocket
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	// - Set address reuse
	sockaddr_in myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_port = htons(port);
	myAddress.sin_addr.S_un.S_addr = INADDR_ANY;

	// - Bind the socket to a local interface
	result = bind(listenSocket, (const struct sockaddr*)&myAddress, sizeof(myAddress));

	if (result == SOCKET_ERROR) {
		reportError("Error: Socket bind failed");
		return false;
	}

	// - Enter in listen mode
	result = listen(listenSocket, 5);

	if (result == SOCKET_ERROR) {
		reportError("Error: Socket listen failed");
		return false;
	}

	// - Add the listenSocket to the managed list of sockets using addSocket()
	addSocket(listenSocket);

	state = ServerState::Listening;

	return true;
}

bool ModuleNetworkingServer::isRunning() const
{
	return state != ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
	return true;
}

bool ModuleNetworkingServer::gui()
{
	if (state != ServerState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets)
		{
			ImGui::Separator();
			ImGui::Text("Socket ID: %d", connectedSocket.socket);
			ImGui::Text("Address: %d.%d.%d.%d:%d",
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
				ntohs(connectedSocket.address.sin_port));
			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
		}
		ImGui::End();
	}
	return true;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
	return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress)
{
	// Add a new connected socket to the list
	ConnectedSocket connectedSocket;
	connectedSocket.socket = socket;
	connectedSocket.address = socketAddress;
	connectedSockets.push_back(connectedSocket);
}

void ModuleNetworkingServer::onSocketReceivedData(SOCKET s, const InputMemoryStream& packet)
{
	ClientMessage clientMessage;
	packet >> clientMessage;

	switch (clientMessage)
	{
	case ClientMessage::Hello: {
		std::string playerName;
		packet >> playerName;

		for (auto& connectedSocket : connectedSockets) 
		{
			if (connectedSocket.socket == s) {
				connectedSocket.playerName = playerName;

				break;
			}
		}
		break; }

	case ClientMessage::ChatMessage: {
		ChatMessage chatMessage;
		chatMessage.Read(packet);

		for (auto& connectedSocket : connectedSockets)
		{
			OutputMemoryStream stream;
			stream << ServerMessage::ChatMessage;
			chatMessage.Write(stream);
			sendPacket(stream, connectedSocket.socket);
		}
		break; }	

	case ClientMessage::ChatCommand: {

		std::string commandName;
		packet >> commandName;
		int commandWordsSize = 0;
		packet >> commandWordsSize;
		std::vector<std::string> commandWords;
		
		for (int i= 0; i < commandWordsSize; ++i)
		{
			std::string commandWord;
			packet >> commandWord;
			commandWords.push_back(commandWord);
		}

		if (commandName == "list")
		{

		}
		else if (commandName == "kick")
		{

		}

		break; }
	default: {
		break; }
	}
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			connectedSockets.erase(it);
			break;
		}
	}
}



void ModuleNetworkingServer::sendWelcomePacket(SOCKET socket)
{

	// ----------
	std::string welcomeMsg(
		"******************************************\n"
		"			 WELCOME TO THE CHAT	       \n"
		" Type /help to see the available commands \n"
		"******************************************\n");

	// ----------
	std::map<std::string, uint32>::iterator randomColor;
	std::advance(randomColor, rand() % colors.size());

}

