#include "GameManager.h"

GameManager::GameManager()
{
}

void GameManager::init()
{
	gameState = GameState::WaitingPlayers;
}

void GameManager::update()
{
	switch (gameState)
	{
	case GameState::WaitingPlayers:
		if (getNumPlayers() >= minPlayers)
		{
			enableInputPlayers(true);
			spawnGemstone({ 0.f, 0.f }, 0.f);
			gameState = GameState::InGame;
		}
		break;
	case GameState::InGame:
		break;
	case GameState::Results:
		break;
	default:
		break;
	}
}

int GameManager::getNumPlayers()
{
	int connectedPlayers = 0;

	for (auto& client : App->modNetServer->clientProxies)
	{
		if (client.connected && client.gameObject != nullptr)
		{
			++connectedPlayers;
		}
	}
	return connectedPlayers;
}

void GameManager::enableInputPlayers(const bool value) 
{
	for (auto& client : App->modNetServer->clientProxies)
	{
		if (client.connected && client.gameObject != nullptr)
		{
			Spaceship* spaceship = (Spaceship*)client.gameObject->behaviour;
			spaceship->enableInput = value;
			NetworkUpdate(client.gameObject);
		}
	}
}

void GameManager::addPlayer(ClientProxy* client)
{
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (&App->modNetServer->clientProxies[i] == client)
		{
			float deg = (360.F / (float)MAX_CLIENTS) * i;
			vec2 pos = 500.f * vec2FromDegrees(180.f + deg);
			client->gameObject = spawnPlayer(client->playerData.spaceshipType, pos, deg);
			client->gameObject->tag = client->clientId;
			break;
		}
	}
}

GameObject* GameManager::spawnPlayer(uint8 spaceshipType, vec2 initialPosition, float initialAngle)
{
	// Create a new game object with the player properties
	GameObject* gameObject = NetworkInstantiate();
	gameObject->position = initialPosition;
	gameObject->size = { 100, 100 };
	gameObject->angle = initialAngle;

	// Create sprite
	gameObject->sprite = App->modRender->addSprite(gameObject);
	gameObject->sprite->order = 5;
	if (spaceshipType == 0) {
		gameObject->sprite->texture = App->modResources->spacecraft1;
	}
	else if (spaceshipType == 1) {
		gameObject->sprite->texture = App->modResources->spacecraft2;
	}
	else {
		gameObject->sprite->texture = App->modResources->spacecraft3;
	}

	// Create collider
	gameObject->collider = App->modCollision->addCollider(ColliderType::Player, gameObject);
	gameObject->collider->isTrigger = true; // NOTE(jesus): This object will receive onCollisionTriggered events

	// Create behaviour
	Spaceship* spaceshipBehaviour = App->modBehaviour->addSpaceship(gameObject);
	gameObject->behaviour = spaceshipBehaviour;
	gameObject->behaviour->isServer = true;

	return gameObject;
}

GameObject* GameManager::spawnGemstone(vec2 initialPosition, float initialAngle)
{
	// Create a new game object with the player properties
	GameObject* gameObject = NetworkInstantiate();
	gameObject->position = initialPosition;
	gameObject->size = { 100, 100 };
	gameObject->angle = initialAngle;
	gameObject->tag = GEMSTONE_TAG;

	// Create sprite
	gameObject->sprite = App->modRender->addSprite(gameObject);
	gameObject->sprite->texture = App->modResources->gemstone;
	gameObject->sprite->order = 5;

	// Create behaviour
	Gemstone* gemstone = App->modBehaviour->addGemstone(gameObject);
	gameObject->behaviour = gemstone;
	gameObject->behaviour->isServer = true;
	return gameObject;
}

GameObject* GameManager::spawnPointer(uint32 clientId, vec2 initialPosition, float initialAngle)
{
	// Create a new game object with the player properties
	GameObject* gameObject = Instantiate();
	gameObject->position = initialPosition;
	gameObject->size = { 50, 50 };
	gameObject->angle = initialAngle;

	// Create sprite
	gameObject->sprite = App->modRender->addSprite(gameObject);
	gameObject->sprite->texture = App->modResources->arrow;
	gameObject->sprite->order = 6;

	// Create behaviour
	Pointer* pointer = App->modBehaviour->addPointer(gameObject);
	pointer->clientId = clientId;
	gameObject->behaviour = pointer;
	gameObject->behaviour->isServer = true;
	return gameObject;
}
