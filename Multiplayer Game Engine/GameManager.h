#pragma once
#define MAX_GAME_TIME 10.F
#define MAX_RESULT_TIME 10.F

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
	static GameObject* spawnSpaceship(uint8 spaceshipType, vec2 initialPosition, float initialAngle);
	static GameObject* spawnGemstone(vec2 initialPosition, float initialAngle);
	static GameObject* spawnPointer(uint32 clientId, vec2 initialPosition, float initialAngle);

private:

	int getNumPlayers();
	void enableInputPlayers(const bool);

private:

	GameState gameState = GameState::None;
	uint32* gemstoneOwner = nullptr;
	GameObject* gemstone = nullptr;
	float currentGameTime = 0.f;
	int minPlayers = 1;
};
