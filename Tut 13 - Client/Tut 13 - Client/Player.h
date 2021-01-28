#pragma once

#include <SFML/Graphics.hpp>
#include "PacketStructs.h"
#include "ServerData.h"
#include "Globals.h"

class Client;

class Player
{
public:
	Player();
	Player(Client* t_client);
	~Player();

	void update(sf::Time t_deltaTime);

	void movement();

	void Boundary();

	void setColour(sf::Color t_colour);

	void setPosition(sf::Vector2f t_position);

	void setId(int t_id);

	int getId();

	sf::Color getColour();

	sf::Vector2f getPosition();

	bool collisionCheck(sf::CircleShape t_enemy);

	float Length(const sf::Vector2f t_vect);

	void draw(sf::RenderWindow& t_window);

	PlayerData getPlayerData();

private:
	Client* m_Myclient;

	sf::Vector2f m_position;

	sf::CircleShape m_shape;

	sf::Color m_playerColour;

	int m_id = 0;

	float m_speed;
};

#include "Client.h"