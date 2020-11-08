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
			disconnectAllSockets();
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
	case ServerMessage::Disconnection: {
		chatMessages.clear();
		DisconnectionType type;
		packet >> type;

		DisconnectSocket(s, type);
		switch (type)
		{
		case DisconnectionType::Error:
			ELOG("An error ocurred! your connection with the server is lost");
			break;
		case DisconnectionType::Exit:
			LOG("Yoy leave the chat");
			break;
		case DisconnectionType::NameExist:
			ELOG("This name is not available! Try another one");
			break;
		case DisconnectionType::Kick:
			LOG("You have been kicked :(");
			break;

		default:
			break;
		}
		break; }
	default: {
		break; }
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket, DisconnectionType t)
{
	state = ClientState::Stopped;
}


void ModuleNetworkingClient::processInputText(const std::string &originalMessage )
{
	std::string currentMessage(StrTool::Trim(originalMessage) ); // Trim Message

	if (currentMessage.empty()) { return; }
	

	
	if (currentMessage[0] != '/')
	{
		OutputMemoryStream stream;
		ChatMessage chatMessage(currentMessage, ChatMessage::Type::Normal, playerColor, playerName, GetTime());
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
		std::vector<std::string> commandSplited = StrTool::Split(commandLine, " " , 2);

		if (!commandSplited.empty() && IsValidCommand(commandSplited[0])) {
	
			std::string commandName = commandSplited[0];
			std::string commandParameters("");

			// Get Original Multispaced Params -------

			if (commandSplited.size() > 1) {
				std::string firstParamsWord = commandSplited[1];
				const auto strBegin = originalMessage.find(firstParamsWord);
				const auto strEnd = originalMessage.find_last_not_of(" \t");
				const auto strRange = strEnd - strBegin + 1;
				commandParameters = originalMessage.substr(strBegin, strRange);
			}

			// Execute Command Client ----------------
			executeCommand(commandName, commandParameters);
		
		}
		else
		{
			std::string errorText = "'/" + commandSplited[0] + "' is not a valid command";
			ChatMessage errorMessage(errorText, ChatMessage::Type::Error, "Red");
			chatMessages.push_back(errorMessage);
		}
	}
}



void ModuleNetworkingClient::executeCommand(std::string commandName, std::string commandParameters)
{
	OutputMemoryStream stream;
	stream << ClientMessage::ChatCommand;
	stream << commandName;

	if (commandName == "clear")
	{
		chatMessages.clear();
	}
	else if (commandName == "whisper")
	{
		std::vector<std::string> commandSplited = StrTool::Split(commandParameters, " ", 1);

		if (commandSplited.size() < 2) return; // Command Error Message
		std::string whisperMessage = StrTool::Trim(commandSplited[1]);

		if (whisperMessage.find_first_not_of(' ') == std::string::npos) return; // Command Error Message
		std::string whisperedUser = commandSplited[0];
		ChatMessage commandMessage(whisperMessage, ChatMessage::Type::Command, playerColor, playerName, GetTime(), whisperedUser);
		commandMessage.Write(stream); // Message
	}
	else if (commandName == "kick")
	{
		ChatMessage commandMessage("", ChatMessage::Type::Command, playerColor, playerName, GetTime(), commandParameters);
		commandMessage.Write(stream); // Message
		
	}
	else if (commandName == "help")
	{
		ChatMessage chatMessage;

		chatMessage.type = ChatMessage::Type::Command;
		chatMessage.text.assign(
			"----- Commands List ------\n"
			"/clear\n"
			"/help\n"
			"/kick [username]\n"
			"/list\n"
			"/whisper [username] [message]\n"
			"/changeName [new username]\n"
		);

		chatMessages.push_back(chatMessage);
	}
	else if (commandName == "changeName")
	{
		std::vector<std::string> nameSplited = StrTool::Split(commandParameters, " ", 1);

		if (nameSplited.size() >= 2 || nameSplited[0] == "") return; // Command Error Message
		//std::string whisperMessage = StrTool::Trim(nameSplited[1]);

		stream << playerName;
		playerName = nameSplited[0];
		stream << playerName;

	}
	// Other commands

	// Send Command to Server ----------------
	sendPacket(stream, _socket);
}
