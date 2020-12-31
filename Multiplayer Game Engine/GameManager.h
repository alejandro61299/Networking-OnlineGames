#pragma once
#define GAME_TIME 50.F
#define RESPAWN_TIME 5.F
#define WAIT_TIME 5.F

#define RESULTS_TIME 5.F
#define READY_TIME 2.F
#define LETSROCK_TIME 2.F

#define MIN_GAME_PLAYERS 2

struct ClientProxy;

enum class GameState
{
	None, WaitingPlayers, Ready,  InGame, Results
};

struct GameData
{
	uint8 spaceshipType = 0u;
	int points = 0;
	float timeToSpawn = RESPAWN_TIME;
	enum class PlayerState { None, Victory, Defeat, InGame, Respawning, Waiting, Ready, Go}
	playerState = PlayerState::None;
};

class GameManager
{
public:

	GameManager();
	void init();
	void update();
	GameState getState() { return gameState; };

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
	static GameObject* spawnNumberUi();

private:

	int getNumPlayers();
	void enableInputPlayers(const bool);

private:

	GameState gameState = GameState::None;
	GameObject* gemstone = nullptr;
	float currentGameTime = 0.f;
};
