#pragma once

#include <SFML/Graphics.hpp>
#include "Globals.h"


class Enemy
{
public:
	Enemy();
	~Enemy();

	void setColour(sf::Color t_colour);

	void setPosition(sf::Vector2f t_position);

	void setId(int t_id);

	int getId();

	sf::Color getColour();

	sf::Vector2f getPosition();

	sf::CircleShape getShape();

	void draw(sf::RenderWindow& t_window);

private:

	sf::Vector2f m_position;

	sf::CircleShape m_shape;

	sf::Color m_EnemyColour;

	int m_ID;
};

