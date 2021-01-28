#include "Player.h"


Player::Player()
{
	m_shape.setRadius(15);
	setColour(sf::Color::White);
	m_shape.setOrigin(m_shape.getRadius() / 2.0f, m_shape.getRadius() / 2.0f);

	m_position = sf::Vector2f(400, 300);

	m_speed = 10;

	m_id = 0;
}

Player::Player(Client* t_client)
{
	m_Myclient = t_client;
	m_shape.setRadius(15);
	setColour(sf::Color::White);
	m_shape.setOrigin(m_shape.getRadius() / 2.0f, m_shape.getRadius() / 2.0f);

	m_position = sf::Vector2f(400, 300);

	m_speed = 10;
}

Player::~Player()
{
}

void Player::update(sf::Time t_deltaTime)
{
	movement();
	Boundary();

	if (m_Myclient != nullptr)
	{
		m_Myclient->SendPlayerData(getPlayerData());
	}
}

void Player::movement()
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		m_position.y -= m_speed;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		m_position.y += m_speed;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		m_position.x += m_speed;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		m_position.x -= m_speed;
	}

	m_shape.setPosition(m_position);
}

void Player::Boundary()
{
	if (m_position.x < 0 - m_shape.getRadius())
	{
		m_position.x = WINDOW_WIDTH;
	}
	else if (m_position.x > WINDOW_WIDTH + m_shape.getRadius())
	{
		m_position.x = 0;
	}

	if (m_position.y < 0 - m_shape.getRadius())
	{
		m_position.y = WINDOW_HEIGHT;
	}
	else if (m_position.y > WINDOW_HEIGHT + m_shape.getRadius())
	{
		m_position.y = 0;
	}
}

void Player::setColour(sf::Color t_colour)
{
    m_playerColour = t_colour;
    m_shape.setFillColor(m_playerColour);
}

void Player::setPosition(sf::Vector2f t_position)
{
	m_position = t_position;
	m_shape.setPosition(m_position);
}

void Player::setId(int t_id)
{
	m_id = t_id;
}

int Player::getId()
{
	return m_id;
}

sf::Color Player::getColour()
{
	return m_playerColour;
}

sf::Vector2f Player::getPosition()
{
    return m_position;
}

bool Player::collisionCheck(sf::CircleShape t_enemy)
{
	sf::Vector2f vectorToEnemy = t_enemy.getPosition() - m_position;

	float distance = Length(vectorToEnemy);

	if (distance < m_shape.getRadius() + t_enemy.getRadius())
	{
		return true;
	}

	return false;
}

void Player::draw(sf::RenderWindow& t_window)
{
    t_window.draw(m_shape);
}

PlayerData Player::getPlayerData()
{
	return PlayerData({ m_id ,m_playerColour, m_position });
}

float Player::Length(const sf::Vector2f t_vect)
{
	float length = sqrt((t_vect.x * t_vect.x) + (t_vect.y * t_vect.y));
	return length;
}