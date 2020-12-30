#pragma once
#define MAX_GAME_TIME 10.F
#define MAX_RESULT_TIME 10.F

#define MIN_GAME_PLAYERS 2

struct ClientProxy;

enum class GameState
{
	None, WaitingPlayers,  InGame, Results
};

struct GameData
{
	uint8 spaceshipType = 0u;
	bool spawned = false;
	int points = 0;
	float timeToSpawn = 0.f;
	enum class PlayerState { None, Victory, Defeat, InGame, Spawning, Waiting}
	playerState = PlayerState::None;
};

class GameManager
{
public:

	GameManager();
	void init();
	void update();

	void spawnPlayer(const uint32 clientId );
	void despawnPlayer(const uint32 clientId );

	void spawnAllPlayers();
	void despawnAllPlayers();
	void setPlayersPoints(int points);
	void setPlayersState(GameData::PlayerState state);
	void setPlayersResults();

	// Spawn Network Objects -----------------------------
	static GameObject* spawnSpaceship(uint8 spaceshipType, vec2 initialPosition, float initialAngle);
	static GameObject* spawnGemstone(vec2 initialPosition, float initialAngle);
	static GameObject* spawnPointer(uint32 clientId, vec2 initialPosition, float initialAngle);
	static GameObject* spawnExplosion(bool bigExplosion , vec2 initialPosition, float initialAngle);
	static GameObject* spawnGameStateUi();
	static GameObject* spawnPointNumberUi();

private:

	int getNumPlayers();
	void enableInputPlayers(const bool);

private:

	GameState gameState = GameState::None;
	GameObject* gemstone = nullptr;
	float currentGameTime = 0.f;
};
