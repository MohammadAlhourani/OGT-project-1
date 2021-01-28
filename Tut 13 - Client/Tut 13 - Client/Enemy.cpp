#include "Enemy.h"

Enemy::Enemy()
{
	m_shape.setRadius(15);
	setColour(sf::Color::White);
	m_shape.setOrigin(m_shape.getRadius() / 2.0f, m_shape.getRadius() / 2.0f);
}

Enemy::~Enemy()
{
}

void Enemy::setColour(sf::Color t_colour)
{
	m_EnemyColour = t_colour;
	m_shape.setFillColor(m_EnemyColour);
}

void Enemy::setPosition(sf::Vector2f t_position)
{
	m_position = t_position;
	m_shape.setPosition(m_position);
}

void Enemy::setId(int t_id)
{
	m_ID = t_id;
}

int Enemy::getId()
{
	return m_ID;
}

sf::Color Enemy::getColour()
{
    return m_EnemyColour;
}

sf::Vector2f Enemy::getPosition()
{
    return m_position;
}

sf::CircleShape Enemy::getShape()
{
	return m_shape;
}

void Enemy::draw(sf::RenderWindow& t_window)
{
	t_window.draw(m_shape);
}
