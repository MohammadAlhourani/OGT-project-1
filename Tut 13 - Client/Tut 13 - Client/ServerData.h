#pragma once

#include <SFML/Graphics.hpp>

enum class ServerState
{
	None,
	Host,
	Client
};

enum class GameState
{
	Connecting,
	Waiting,
	InitGame,
	Playing,
	Finish
};

struct PlayerData
{
	int k_ID;
	sf::Color k_colour;
	sf::Vector2f k_position;
};

struct GameEnd
{
	float Time;
	std::string gameEndText;
};

struct GameStateUpdate
{
	GameState k_state;
};
