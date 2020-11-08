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

		for (auto& chatMessage : chatMessages) {
			ImGui::Text(chatMessage.text.data());
		}

		static char buffer[100];
		ImGui::InputText("Input", buffer, 100);

		if (ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_Enter))
		{
			ChatMessage chatMessage(
				std::string(buffer),
				ChatMessage::Type::Normal,
				playerColor,
				playerName,
				"");


			OutputMemoryStream stream;
			stream << ClientMessage::ChatMessage;
			chatMessage.Write(stream);
			sendPacket(stream, _socket);
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
