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
		if (client.connected)
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