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
	{
		setPlayersState(GameData::PlayerState::Waiting);
		setPlayersPoints(0);
		enableInputPlayers(false);

		if (getNumPlayers() >= MIN_GAME_PLAYERS)
		{
			enableInputPlayers(true);
			gemstone = spawnGemstone({ 0.f, 0.f }, 0.f);
			currentGameTime = 0.f;
			setPlayersState(GameData::PlayerState::InGame);
			gameState = GameState::InGame;
		}

		break;
	} 
	case GameState::InGame: 
	{
		currentGameTime += Time.deltaTime;
		if (currentGameTime >= MAX_GAME_TIME)
		{
			despawnAllPlayers();
			NetworkDestroy(gemstone);
			setPlayersResults();
			currentGameTime = 0.f;
			gameState = GameState::Results;
		}
		break;
	} 
	case GameState::Results:
	{
		currentGameTime += Time.deltaTime;
		if (currentGameTime >= MAX_RESULT_TIME)
		{
			spawnAllPlayers();
			currentGameTime = 0.f;
			gameState = GameState::WaitingPlayers;
		}
		break;
	}
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

void GameManager::spawnPlayer(const uint32 clientId)
{
	ClientProxy* client = App->modNetServer->getClientProxyById(clientId);
	int clientIndex = 0;

	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (App->modNetServer->clientProxies[i].clientId == clientId)
		{
			clientIndex = i;
			break;
		}
	}

	float deg = (360.F / (float)MAX_CLIENTS) * clientIndex;
	vec2 pos = 500.f * vec2FromDegrees(180.f + deg);
	client->gameObject = spawnSpaceship(client->gameData.spaceshipType, pos, deg);
	client->gameObject->tag = client->clientId;
}

void GameManager::despawnPlayer(const uint32 clientId)
{
	ClientProxy* client = App->modNetServer->getClientProxyById(clientId);
	if (client->gameObject != nullptr)
	{
		NetworkDestroy(client->gameObject);
		client->gameObject = nullptr;
	}
}

void GameManager::spawnAllPlayers()
{
	for (auto& client : App->modNetServer->clientProxies)
	{
		if (client.connected && client.gameObject == nullptr)
		{
			spawnPlayer(client.clientId);
		}
	}
}

void GameManager::despawnAllPlayers()
{
	for (auto& client : App->modNetServer->clientProxies)
	{
		if (client.connected && client.gameObject != nullptr)
		{
			GameManager::spawnExplosion(true, client.gameObject->position, 0.f);
			despawnPlayer(client.clientId);
		}
	}
}

void GameManager::setPlayersPoints(int points)
{
	for (auto& client : App->modNetServer->clientProxies)
	{
		if (client.connected)
		{
			client.gameData.points = points;
		}
	}
}

void GameManager::setPlayersState(GameData::PlayerState state)
{
	for (auto& client : App->modNetServer->clientProxies)
	{
		if (client.connected)
		{
			client.gameData.playerState = state;
		}
	}
}

void GameManager::setPlayersResults()
{
	int maxPoints = 0;
	ClientProxy* clientMaxPoints = nullptr;

	for (auto& client : App->modNetServer->clientProxies)
	{
		if (maxPoints < client.gameData.points)
		{
			maxPoints = client.gameData.points;
			clientMaxPoints = &client;
		}
	}

	setPlayersState(GameData::PlayerState::Defeat);

	if (clientMaxPoints != nullptr)
	{
		clientMaxPoints->gameData.playerState = GameData::PlayerState::Victory;
	}

}

GameObject* GameManager::spawnSpaceship(uint8 spaceshipType, vec2 initialPosition, float initialAngle)
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

	// Create collider
	gameObject->collider = App->modCollision->addCollider(ColliderType::Gemstone, gameObject);
	gameObject->collider->isTrigger = true; // NOTE(jesus): This object will receive onCollisionTriggered events

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
	pointer->ownerTag = clientId;
	gameObject->behaviour = pointer;
	return gameObject;
}

GameObject* GameManager::spawnExplosion(bool bigExplosion, vec2 initialPosition, float initialAngle)
{
	float size;
	vec2  position;

	if (bigExplosion)
	{
		// Centered big explosion
		size = 250.0f + 100.0f * Random.next();
		position = initialPosition;
	}
	else
	{
		// Little Random explosion
		size = 30 + 50.0f * Random.next();
		position = initialPosition + 50.0f * vec2{ Random.next() - 0.5f, Random.next() - 0.5f };
	}

	GameObject* explosion = NetworkInstantiate();
	explosion->position = position;
	explosion->size = vec2{ size, size };
	explosion->angle = 365.0f * Random.next();
	explosion->tag = 4;
	explosion->sprite = App->modRender->addSprite(explosion);
	explosion->sprite->texture = App->modResources->explosion1;
	explosion->sprite->order = 100;
	explosion->animation = App->modRender->addAnimation(explosion);
	explosion->animation->clip = App->modResources->explosionClip;
	NetworkDestroy(explosion, 2.0f);

	return explosion;
}

GameObject* GameManager::spawnGameStateUi()
{
	GameObject* gameObject = Instantiate();
	gameObject->position = {0.f, 0.f };
	gameObject->size = { 600, 50 };
	gameObject->angle = 0.f;

	// Create sprite
	gameObject->sprite = App->modRender->addSprite(gameObject);
	gameObject->sprite->texture = nullptr;
	gameObject->sprite->order = -10;
	return gameObject;
}

GameObject* GameManager::spawnPointNumberUi()
{
	GameObject* gameObject = Instantiate();
	gameObject->position = { 0.f, 0.f };
	gameObject->size = { 20.f, 20.f };
	gameObject->angle = 0.f;

	// Create sprite
	gameObject->sprite = App->modRender->addSprite(gameObject);
	gameObject->sprite->texture = nullptr;
	gameObject->sprite->order = -10;
	return gameObject;
}
