#include "ModuleNetworkingClient.h"


//////////////////////////////////////////////////////////////////////
// ModuleNetworkingClient public methods
//////////////////////////////////////////////////////////////////////


void ModuleNetworkingClient::setServerAddress(const char * pServerAddress, uint16 pServerPort)
{
	serverAddressStr = pServerAddress;
	serverPort = pServerPort;
}

void ModuleNetworkingClient::setPlayerInfo(const char * pPlayerName, uint8 pSpaceshipType)
{
	playerName = pPlayerName;
	spaceshipType = pSpaceshipType;
}

void ModuleNetworkingClient::setPlayerGameObjectNetId(uint32 netId)
{
	if (netId != 0)
	{
		spaceshipNetId = netId;
	}
}

uint32 ModuleNetworkingClient::getPlayerGameObjectId()
{
	return spaceshipNetId;
}

uint32 ModuleNetworkingClient::GetClientId()
{
	return playerId;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingClient::onStart()
{
	if (!createSocket()) return;

	if (!bindSocketToPort(0)) {
		disconnect();
		return;
	}

	// Create remote address
	serverAddress = {};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	int res = inet_pton(AF_INET, serverAddressStr.c_str(), &serverAddress.sin_addr);
	if (res == SOCKET_ERROR) {
		reportError("ModuleNetworkingClient::startClient() - inet_pton");
		disconnect();
		return;
	}

	state = ClientState::Connecting;

	inputDataFront = 0;
	inputDataBack = 0;

	secondsSinceLastHello = 9999.0f;
	secondsSinceLastInputDelivery = 0.0f;

	for (auto& point : pointsUI)
	{
		point = nullptr;
	}

	for (auto& time : gameTimeUI)
	{
		time = nullptr;
	}
}

void ModuleNetworkingClient::onGui()
{
	if (state == ClientState::Stopped) return;

	if (ImGui::CollapsingHeader("ModuleNetworkingClient", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (state == ClientState::Connecting)
		{
			ImGui::Text("Connecting to server...");
		}
		else if (state == ClientState::Connected)
		{
			ImGui::Text("Connected to server");

			ImGui::Separator();

			ImGui::Text("Player info:");
			ImGui::Text(" - Id: %u", playerId);
			ImGui::Text(" - Name: %s", playerName.c_str());

			ImGui::Separator();

			ImGui::Text("Spaceship info:");
			ImGui::Text(" - Type: %u", spaceshipType);
			ImGui::Text(" - Network id: %u", spaceshipNetId);

			vec2 playerPosition = {};
			GameObject *playerGameObject = App->modLinkingContext->getNetworkGameObject(spaceshipNetId);
			if (playerGameObject != nullptr) {
				playerPosition = playerGameObject->position;
			}
			ImGui::Text(" - Coordinates: (%f, %f)", playerPosition.x, playerPosition.y);

			ImGui::Separator();

			ImGui::Text("Connection checking info:");
			ImGui::Text(" - Ping interval (s): %f", PING_INTERVAL_SECONDS);
			ImGui::Text(" - Disconnection timeout (s): %f", DISCONNECT_TIMEOUT_SECONDS);

			ImGui::Separator();

			ImGui::Text("Input:");
			ImGui::InputFloat("Delivery interval (s)", &inputDeliveryIntervalSeconds, 0.01f, 0.1f, 4);
		}
	}
}

void ModuleNetworkingClient::onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress)
{
	// TODO(done): UDP virtual connection lab session
	lastPacketRecivedTime = 0.0f;
	// ---------------------------------------------

	uint32 protoId;
	packet >> protoId;
	if (protoId != PROTOCOL_ID) return;

	ServerMessage message;
	packet >> message;

	if (state == ClientState::Connecting)
	{
		if (message == ServerMessage::Welcome)
		{
			packet >> playerId;
			packet >> spaceshipNetId;

			LOG("ModuleNetworkingClient::onPacketReceived() - Welcome from server");
			state = ClientState::Connected;
			pointer = GameManager::spawnPointer(playerId, { 0.f, 0.f }, 0.f);
			gameStateUI = GameManager::spawnGameStateUi();
			respawnTimeUI = GameManager::spawnNumberUi();;
			respawnTimeUI->size = { 50.f, 50.f };

			for (auto& point : pointsUI)
			{
				point = GameManager::spawnNumberUi();
			}

			//for (auto& time : gameTimeUI)
			//{
			//	time = GameManager::spawnNumberUi();
			//}
		}
		else if (message == ServerMessage::Unwelcome)
		{
			WLOG("ModuleNetworkingClient::onPacketReceived() - Unwelcome from server :-(");
			disconnect();
		}
	}
	else if (state == ClientState::Connected)
	{
		// TODO(done): World state replication lab session
		if (message == ServerMessage::Replication)
		{
			uint32 lastInputRecived = 0u;

			replicationClient.read(packet, lastInputRecived, deliveryManager);

			// TODO(you): Reliability on top of UDP lab session
			inputDataFront = lastInputRecived; //Update last input processed.
		}
		
	}
}

void ModuleNetworkingClient::onUpdate()
{
	if (state == ClientState::Stopped) return;


	// TODO(done): UDP virtual connection lab session

	lastPacketRecivedTime += Time.deltaTime;

	if (lastPacketRecivedTime > DISCONNECT_TIMEOUT_SECONDS)
	{
		disconnect();
		return;
	}

	// ----------------------------------------------

	if (state == ClientState::Connecting)
	{
		secondsSinceLastHello += Time.deltaTime;

		if (secondsSinceLastHello > 0.1f)
		{
			secondsSinceLastHello = 0.0f;

			OutputMemoryStream packet;
			packet << PROTOCOL_ID;
			packet << ClientMessage::Hello;
			packet << playerName;
			packet << spaceshipType;

			sendPacket(packet, serverAddress);
		}
	}
	else if (state == ClientState::Connected)
	{
		// TODO(done): UDP virtual connection lab session
		secondsSinceLastPingDelivery += Time.deltaTime;

		if (secondsSinceLastPingDelivery > PING_INTERVAL_SECONDS)
		{
			OutputMemoryStream packet;
			packet << PROTOCOL_ID;
			packet << ClientMessage::Ping;
			sendPacket(packet, serverAddress);
			secondsSinceLastPingDelivery = 0.f;
		}

		// Process more inputs if there's space
		if (inputDataBack - inputDataFront < ArrayCount(inputData))
		{
			// Pack current input
			uint32 currentInputData = inputDataBack++;
			InputPacketData &inputPacketData = inputData[currentInputData % ArrayCount(inputData)];
			inputPacketData.sequenceNumber = currentInputData;
			inputPacketData.horizontalAxis = Input.horizontalAxis;
			inputPacketData.verticalAxis = Input.verticalAxis;
			inputPacketData.buttonBits = packInputControllerButtons(Input);
		}

		secondsSinceLastInputDelivery += Time.deltaTime;

		// Input delivery interval timed out: create a new input packet
		if (secondsSinceLastInputDelivery > inputDeliveryIntervalSeconds)
		{
			secondsSinceLastInputDelivery = 0.0f;

			OutputMemoryStream packet;
			packet << PROTOCOL_ID;
			packet << ClientMessage::Input;

			

			for (uint32 i = inputDataFront; i < inputDataBack; ++i)
			{
				InputPacketData &inputPacketData = inputData[i % ArrayCount(inputData)];
				packet << inputPacketData.sequenceNumber;
				packet << inputPacketData.horizontalAxis;
				packet << inputPacketData.verticalAxis;
				packet << inputPacketData.buttonBits;
			}

			// TODO(you): Reliability on top of UDP lab session (Coment the clear queue DONE)

			// Clear the queue
			//inputDataFront = inputDataBack;

			sendPacket(packet, serverAddress);
		}

		secondsSinceLastConfirmationDelivery += Time.deltaTime;

		if (secondsSinceLastConfirmationDelivery > CONFIRMATION_INTERVAL_SECONDS)
		{
			if (deliveryManager.hasSequenceNumbersPendingAck())
			{
				OutputMemoryStream packet;
				deliveryManager.writeSequenceNumbersPendingAck(packet);

				sendPacket(packet, serverAddress);
			}

			secondsSinceLastConfirmationDelivery = 0.0f;
		}


		// TODO(you): Latency management lab session

		// Update camera for player
		GameObject *playerGameObject = App->modLinkingContext->getNetworkGameObject(spaceshipNetId);
		if (playerGameObject != nullptr)
		{
			App->modRender->cameraPosition = lerp(App->modRender->cameraPosition, playerGameObject->position, 10.f * Time.deltaTime);
		}
		else
		{
			// This means that the player has been destroyed (e.g. killed)
		}

		// Update UI  ----------------------------------

		// Game State
		vec2 offset = { 0.f, -100.f };
		gameStateUI->position = App->modRender->cameraPosition + offset;
		gameStateUI->sprite->order = 200;
		respawnTimeUI->sprite->order = -10;

		if (gameData.playerState == GameData::PlayerState::Waiting)
		{
			gameStateUI->sprite->texture = App->modResources->waitingText;
		}
		else if (gameData.playerState == GameData::PlayerState::Ready)
		{
			gameStateUI->sprite->texture = App->modResources->readyText;
		}
		else if (gameData.playerState == GameData::PlayerState::Go)
		{
			gameStateUI->sprite->texture = App->modResources->letsrockText;
		}
		else if (gameData.playerState == GameData::PlayerState::Respawning)
		{
			gameStateUI->sprite->texture = App->modResources->respawnText;
			respawnTimeUI->position = App->modRender->cameraPosition;
			respawnTimeUI->sprite->texture = App->modResources->getTextureNumber(gameData.timeToSpawn + 1.f);
			respawnTimeUI->sprite->order = 200;
		}
		else if (gameData.playerState == GameData::PlayerState::Victory)
		{
			gameStateUI->sprite->texture = App->modResources->victoryText;
		}
		else if (gameData.playerState == GameData::PlayerState::Defeat)
		{
			gameStateUI->sprite->texture = App->modResources->defeatText;
		}
		else
		{
			gameStateUI->sprite->texture = nullptr;
			gameStateUI->sprite->order = -10;
		}


		// Game Time


		// Points
		int points = gameData.points;
		float width = 0.f, height = 0.f;
		App->modRender->getViewportSize(width, height);
		vec2 position = { width * 0.5f - width * 0.1f, -height * 0.5f + height * 0.1f };


		for (int i = 0; i < 4; ++i)
		{
			pointsUI[3 - i]->position = App->modRender->cameraPosition + position;
			pointsUI[3 - i]->sprite->texture = App->modResources->getTextureNumber(points % 10);
			pointsUI[3 - i]->sprite->order = 10;

			position += { -23.f, 0 };
			points /= 10;
		}

	}
}

void ModuleNetworkingClient::onConnectionReset(const sockaddr_in & fromAddress)
{
	disconnect();
}

void ModuleNetworkingClient::onDisconnect()
{
	state = ClientState::Stopped;

	GameObject *networkGameObjects[MAX_NETWORK_OBJECTS] = {};
	uint16 networkGameObjectsCount;
	App->modLinkingContext->getNetworkGameObjects(networkGameObjects, &networkGameObjectsCount);
	App->modLinkingContext->clear();

	for (uint32 i = 0; i < networkGameObjectsCount; ++i)
	{
		Destroy(networkGameObjects[i]);
	}

	App->modRender->cameraPosition = {};

	if ( pointer != nullptr)
		Destroy(pointer);

	if (gameStateUI != nullptr)
		Destroy(gameStateUI);

	for (auto& point : pointsUI)
	{
		if (point != nullptr)
		{
			Destroy(point);

		}
	}

	if (respawnTimeUI != nullptr)
	{
		Destroy(respawnTimeUI);
	}

	for (auto& time : gameTimeUI)
	{
		if (time != nullptr)
		{
			Destroy(time);
		}
	}
	
}
