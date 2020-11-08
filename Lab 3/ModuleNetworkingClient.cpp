#include "ModuleNetworkingClient.h"

bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;

	// TODO(jesus): TCP connection stuff
	// - Create the socket
	_socket = socket(AF_INET, SOCK_STREAM, 0);

	// - Create the remote address object
	playerName = pplayerName;
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverAddressStr, &serverAddress.sin_addr);

	// - Connect to the remote address
	int result = connect(_socket, (const sockaddr*)&serverAddress, sizeof(serverAddress));

	// - Add the created socket to the managed list of sockets using addSocket()
	addSocket(_socket);

	// If everything was ok... change the state
	state = ClientState::Start;

	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update()
{
	if (state == ClientState::Start)
	{
		OutputMemoryStream packet;
		packet << ClientMessage::Hello;
		packet << playerName;

		if (sendPacket(packet, _socket))
		{
			state = ClientState::Logging;
		}
		else 
		{
			disconnect();
			state = ClientState::Stopped;
		}
	}

	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);
		ImGui::Spacing();

		// Draw all messages 
		for (auto& chatMessage : chatMessages) {
			ImGui::Text(chatMessage.text.data());
		}

		static std::string currentMessage;
		char buffer[100] = "";
		strcpy_s(buffer, currentMessage.c_str());
		ImGui::InputText("Input", buffer, 100);
		currentMessage.assign(buffer);


		if (ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_Enter))
		{
			processInputText(currentMessage);

			// Reset Message Input ---------
			currentMessage.clear();
			ImGui::SetKeyboardFocusHere(0);
		}

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET s, const InputMemoryStream& packet)
{
	ServerMessage serverMessage;
	packet >> serverMessage;

	switch (serverMessage)
	{
	case ServerMessage::Welcome: {
		std::string userColor;
		packet >> userColor;
		playerColor = userColor;
		break; }
	case ServerMessage::ChatMessage: {
		ChatMessage chatMessage;
		chatMessage.Read(packet);
		chatMessages.push_back(chatMessage);
		break; }
	default: {
		break; }
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

void ModuleNetworkingClient::processInputText(const std::string &message )
{
	std::string currentMessage(StrTool::Trim(message) ); // Trim Message

	if (currentMessage.empty()) { return; }
	
	OutputMemoryStream stream;

	if (currentMessage[0] != '/')
	{
		// Send Message ----------------
		ChatMessage chatMessage(currentMessage, ChatMessage::Type::Normal, playerColor, playerName, "");
		chatMessage.type = ChatMessage::Type::Normal;
		stream << ClientMessage::ChatMessage;
		chatMessage.Write(stream);
		sendPacket(stream, _socket);
	}
	else
	{
		// Prepare Command Line
		std::string commandLine(StrTool::DeleteMultiSpacing(currentMessage));
		commandLine.erase(0, 1); // Erase bar /

		// Check Command Name
		std::vector<std::string> commandWords = StrTool::Split(commandLine, " ");

		if (!commandWords.empty() && IsValidCommand(commandWords[0])) {

			// Execute Command Client ----------------
			std::string commandName = commandWords[0];
			commandWords.erase(commandWords.begin());
			executeCommand(commandName, commandWords);

			// Send Command to Server ----------------
			stream << ClientMessage::ChatCommand;
			stream << commandName;
			stream << (int)commandWords.size();

			for (auto& commandWord : commandWords)
			{
				stream << commandWord;
			}

			sendPacket(stream, _socket);
		}
	}
}



void ModuleNetworkingClient::executeCommand(std::string command, const std::vector<std::string>& words)
{
	if (command == "clear")
	{
		chatMessages.clear();
	}
	else if (command == "help")
	{

	}
	// Other commands
}
