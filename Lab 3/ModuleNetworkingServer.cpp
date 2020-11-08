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

		// Check If User Name Exist --------------
		if (!CheckIfNameExist(playerName)) {
			onSocketDisconnected(s, DisconnectionType::NameExist);
			break;
		}

		// Find User by name ---------------------
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == s) {

				connectedSocket.playerName = playerName; //Set Name in the server
				// Send Welcome to New User 
				OutputMemoryStream stream;
				stream << ServerMessage::Welcome;
				// Send random color
				srand(time(NULL));
				std::map<std::string, ImVec4>::iterator randomColor = colors.begin();
				std::advance(randomColor, rand() % colors.size());
				stream << randomColor->first;
				// Send message to New User
				ChatMessage welcomeMessage(
					"------------------------------------------\n"
					"			 WELCOME TO THE CHAT	       \n"
					" Type /help to see the available commands \n"
					"------------------------------------------\n",
					ChatMessage::Type::Server);
				ChatMessage joinMessage("Give the welcome to " + playerName + "!!", ChatMessage::Type::Server);
				welcomeMessage.Write(stream);
				joinMessage.Write(stream);
				sendPacket(stream, connectedSocket.socket);

				// Send User Join  to everyone ---------------------
				OutputMemoryStream stream_2;
				stream_2 << ServerMessage::ChatMessage;
				joinMessage.Write(stream_2);

				for (auto& connectedSocket : connectedSockets) {
					if (connectedSocket.playerName != playerName)
						sendPacket(stream_2, connectedSocket.socket);
				}
				break;
			}
		}
		break; }

	case ClientMessage::ChatMessage: {

		// Read stream -----------
		ChatMessage chatMessage;
		chatMessage.Read(packet);

		// Send stream -----------
		OutputMemoryStream stream;
		stream << ServerMessage::ChatMessage;
		chatMessage.Write(stream);

		for (auto& connectedSocket : connectedSockets)
		{
			sendPacket(stream, connectedSocket.socket);
		}

		break; }	

	case ClientMessage::ChatCommand: {

		std::string commandName;
		packet >> commandName;
		if (commandName == "list")
		{
			std::string listOfUsers = "These are the users connected:\n\n";
			for (auto& connectedSocket : connectedSockets)
			{
				listOfUsers += connectedSocket.playerName + "\n";
			}

			ChatMessage chatMessage(listOfUsers, ChatMessage::Type::Server);

			OutputMemoryStream stream;
			stream << ServerMessage::ChatMessage;
			chatMessage.Write(stream);

			sendPacket(stream, s);
		}
		else if (commandName == "kick")
		{
			ChatMessage commandMessage;
			commandMessage.Read(packet);

			bool found = false;
			for (auto& connectedSocket : connectedSockets)
			{
				if (connectedSocket.playerName == commandMessage.dstUser)
				{
					onSocketDisconnected(connectedSocket.socket, DisconnectionType::Kick);
					found = true;
				}

			}

			if (!found)
			{
				ChatMessage errorMessage(
					std::string("The user " + commandMessage.dstUser + " has not been found! \nPlease, check if you have written it correctly."),
					ChatMessage::Type::Error,
					"Red");
				OutputMemoryStream stream;
				stream << ServerMessage::ChatMessage;
				errorMessage.Write(stream);

				sendPacket(stream, s);

				break;

			}


			std::string kickInformMessage = commandMessage.dstUser + " has been kicked by " + commandMessage.srcUser;
			ChatMessage kickInform(kickInformMessage, ChatMessage::Type::Server);
			OutputMemoryStream stream;
			stream << ServerMessage::ChatMessage;
			kickInform.Write(stream);

			for (auto& connectedSocket : connectedSockets) {
				sendPacket(stream, connectedSocket.socket);
			}

		}
		else if (commandName == "whisper")
		{
			// Read stream -----------
			ChatMessage commandMessage;
			commandMessage.Read(packet);

			// Send stream -----------
			OutputMemoryStream stream;
			stream << ServerMessage::ChatMessage;

			bool error = true;
			for (auto& connectedSocket : connectedSockets)
				if (connectedSocket.playerName == commandMessage.dstUser) {
					error = false;
					commandMessage.Write(stream);
					// To Sourve
					sendPacket(stream, s);
					// To Destination
					sendPacket(stream, connectedSocket.socket);
					break;

				}
			if (error) {
				ChatMessage errorMessage("Whisper failed. User '" + commandMessage.dstUser + "' not found.\n", ChatMessage::Type::Error, "Red");
				errorMessage.Write(stream);
				sendPacket(stream, s);
			}
		}
		else if (commandName == "changeName")
		{
			std::string oldPlayerName;
			packet >> oldPlayerName;

			std::string playerName;
			packet >> playerName;

			ChatMessage informMessage(("The user '" + oldPlayerName + "' now is called '" + playerName + "'!\n"), ChatMessage::Type::Server);
			OutputMemoryStream stream;
			stream << ServerMessage::ChatMessage;
			informMessage.Write(stream);

			for (auto& connectedSocket : connectedSockets)
			{
				if (connectedSocket.socket == s)
					connectedSocket.playerName = playerName;

				sendPacket(stream, connectedSocket.socket);
			}

		}

		else if (commandName == "changeColor")
		{
			std::string playerName;
			packet >> playerName;

			std::string playerColor;
			packet >> playerColor;

			//Send the welcome message to everyone.
			ChatMessage informMessage(("The user '" + playerName + "' now have the " + playerColor + " color!\n"),
				ChatMessage::Type::Server);

			OutputMemoryStream stream;
			stream << ServerMessage::ChatMessage;
			informMessage.Write(stream);

			for (auto& connectedSocket : connectedSockets)
				sendPacket(stream, connectedSocket.socket);
		}
		break; }
	default: {
		break; }
	}
}

bool ModuleNetworkingServer::CheckIfNameExist(std::string& playerName)
{
	for (auto& connectedSocket : connectedSockets)
	{
		if (connectedSocket.playerName == playerName)
			return false;
	}

	return true;
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket, DisconnectionType t)
{

	// Remove the connected socket from the list
	bool exist = false;
	std::string name;
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto& connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			name = connectedSocket.playerName;

			OutputMemoryStream stream;
			stream << ServerMessage::Disconnection;
			stream << t;


			sendPacket(stream, socket);


			connectedSockets.erase(it);
			exist = true;
			break;
		}

	}

	if (t != DisconnectionType::NameExist && t != DisconnectionType::Kick && exist) //If the problem is the name alredy exist, the user is not connected ate the eyes of the rest, so it's not necessary to send a message of disconnection.
	{
		// Send message of disconnection
		for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
		{
			auto& currentSocket = *it;

			ChatMessage chatMessage(
				std::string(name + " se ha desconectado :_(\n"),
				ChatMessage::Type::Server
			);

			OutputMemoryStream stream;
			stream << ServerMessage::ChatMessage;
			chatMessage.Write(stream);
			sendPacket(stream, currentSocket.socket);
		}
	}
}


