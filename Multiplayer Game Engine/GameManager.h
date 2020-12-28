#pragma once

struct ClientProxy;

enum class GameState
{
	None, WaitingPlayers, Start,  InGame, Results
};

class GameManager
{
public:

	GameManager();
	void init();
	void update();

private:
	int getNumPlayers();
	void enableInputPlayers(const bool);

	GameState gameState = GameState::None;
	ClientProxy* gemstoneOwner = nullptr;
	ClientProxy* winner = nullptr;
	int minPlayers = 2;
};
