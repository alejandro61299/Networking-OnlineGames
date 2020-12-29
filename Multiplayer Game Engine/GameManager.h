#pragma once

struct ClientProxy;

enum class GameState
{
	None, WaitingPlayers,  InGame, Results
};

class GameManager
{
public:

	GameManager();
	void init();
	void update();

	void addPlayer(ClientProxy* client);

	// Spawn Network Objects -----------------------------
	static GameObject* spawnPlayer(uint8 spaceshipType, vec2 initialPosition, float initialAngle);
	static GameObject* spawnGemstone(vec2 initialPosition, float initialAngle);
	static GameObject* spawnPointer(uint32 clientId, vec2 initialPosition, float initialAngle);

private:

	int getNumPlayers();
	void enableInputPlayers(const bool);

private:

	GameState gameState = GameState::None;
	ClientProxy* gemstoneOwner = nullptr;
	ClientProxy* winner = nullptr;
	int minPlayers = 1;
};
