/// <summary>
/// @author Mohammad Alhourani
/// </summary>

#include "Game.h"
#include <iostream>

/// <summary>
/// default constructor
/// setup the window properties
/// </summary>
Game::Game() :
	m_window{ sf::VideoMode{ WINDOW_WIDTH, WINDOW_HEIGHT, 32U }, "OGT project 1" },
	m_exitGame{false} //when true game will exit
{
	if (GAME_FONT.loadFromFile("ASSETS/FONTS/ariblk.ttf") == false)
	{
		std::cout << "could not load font" << std::endl;
	}

	IPText.setFont(GAME_FONT);
	IPText.setCharacterSize(25);
	IPText.setFillColor(sf::Color::White);
	IPText.setString(IP);
	IPText.setPosition(300, 200);

	hostClientText.setFont(GAME_FONT);
	hostClientText.setCharacterSize(25);
	hostClientText.setFillColor(sf::Color::White);
	hostClientText.setString("press 1 to host,\n press 2 to be a client");
	hostClientText.setPosition(300, 200);

	GameChangeText.setFont(GAME_FONT);
	GameChangeText.setCharacterSize(25);
	GameChangeText.setFillColor(sf::Color::White);
	GameChangeText.setString("Waiting for players");
	GameChangeText.setPosition(300, 200);

	GameFinishText.setFont(GAME_FONT);
	GameFinishText.setCharacterSize(25);
	GameFinishText.setFillColor(sf::Color::White);
	GameFinishText.setPosition(300, 200);
}

/// <summary>
/// default destructor we didn't dynamically allocate anything
/// so we don't need to free it, but method needs to be here
/// </summary>
Game::~Game()
{
}


/// <summary>
/// main game loop
/// update 60 times per second,
/// process update as often as possible and at least 60 times per second
/// draw as often as possible but only updates are on time
/// if updates run slow then don't render frames
/// </summary>
void Game::run()
{	
	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;
	const float fps{ 60.0f };
	sf::Time timePerFrame = sf::seconds(1.0f / fps); // 60 fps
	while (m_window.isOpen())
	{
		processEvents(); // as many as possible
		timeSinceLastUpdate += clock.restart();
		while (timeSinceLastUpdate > timePerFrame)
		{
			timeSinceLastUpdate -= timePerFrame;
			processEvents(); // at least 60 fps
			update(timePerFrame); //60 fps
		}
		render(); // as many as possible
	}
}
void Game::ChangeGameState(GameState t_state)
{
	m_currentState = t_state;
}

void Game::setTimer(float t_time)
{
	timer = t_time;
}

void Game::setGameEndMsg(sf::String t_msg)
{
	GameFinishText.setString(t_msg);
}

/// <summary>
/// handle user and system events/ input
/// get key presses/ mouse moves etc. from OS
/// </summary>
void Game::processEvents()
{
	sf::Event newEvent;
	while (m_window.pollEvent(newEvent))
	{
		
		if (m_CurrentServerState == ServerState::Client)
		{
			if (sf::Event::TextEntered == newEvent.type)
			{
				unsigned short unicode = newEvent.text.unicode;

				if (unicode == 8)
				{
					if (IP.size() > 0)
					{
						IP = IP.substr(0, IP.size() - 1);

						IPText.setString(IP);
					}
				}
				else
				{
					auto iter = std::find(std::begin(acceptedChar), std::end(acceptedChar), static_cast<char>(unicode));

					if (iter != std::end(acceptedChar))
					{
						IP += static_cast<char>(unicode);

						IPText.setString(IP);
					}
				}
			}
		}

		if ( sf::Event::Closed == newEvent.type) // window message
		{
			if (m_client != nullptr)
			{
				m_client->Disconnect();
			}

			m_exitGame = true;
		}
		if (sf::Event::KeyPressed == newEvent.type) //user pressed a key
		{
			processKeys(newEvent);
		}
	}
}


/// <summary>
/// deal with key presses from the user
/// </summary>
/// <param name="t_event">key press event</param>
void Game::processKeys(sf::Event t_event)
{
	if (sf::Keyboard::Escape == t_event.key.code)
	{
		if (m_client != nullptr)
		{
			m_client->Disconnect();
		}

		m_exitGame = true;
	}

	if (m_CurrentServerState == ServerState::None)
	{
		if (sf::Keyboard::Num1 == t_event.key.code)
		{
			m_CurrentServerState = ServerState::Host;
			m_currentState = GameState::Waiting;

			m_server = new Server(1111, true);

			m_player = new Player();
			m_player->setPosition(sf::Vector2f(350, 550));

			m_player->setId(0);

			m_player->setColour(sf::Color::Blue);

			m_server->setPlayer(m_player);

			m_server->setEnemies(Enemy1, Enemy2);

			std::thread lc(m_server->ListenForNewConnection, std::ref(*m_server));
			lc.detach();
			m_listenThread = &lc;
		}

		if (sf::Keyboard::Num2 == t_event.key.code)
		{
			m_CurrentServerState = ServerState::Client;
		}
	}


	if (m_CurrentServerState == ServerState::Client && m_currentState == GameState::Connecting)
	{
		if (sf::Keyboard::Enter == t_event.key.code)
		{
			if (connectToServer() == false)
			{
				std::cout << "could not connect" << std::endl;
			}
			else
			{
				m_currentState = GameState::Waiting;

				m_player = new Player(m_client);

				m_client->SetGame(this);
				
				m_client->SetPlayer(m_player);

				m_client->setEnemies(Enemy1 , Enemy2);				
			}
		}
	}
}

/// <summary>
/// Update the game world
/// </summary>
/// <param name="t_deltaTime">time interval per frame</param>
void Game::update(sf::Time t_deltaTime)
{
	if (m_exitGame)
	{
		m_window.close();
	}	

	if (m_CurrentServerState == ServerState::Host)
	{
		if (m_currentState == GameState::Waiting && m_server->getConnectionAmount() == 2)
		{
			m_currentState = GameState::InitGame;
		}
		else if (m_currentState == GameState::InitGame && m_server->getConnectionAmount() == 2)
		{
			m_server->settupGame();

			m_currentState = GameState::Playing;

			m_server->ChangeGameState(m_currentState);		
		}
	}

	if (m_currentState == GameState::Playing)
	{
		m_player->update(t_deltaTime);
			

		if (m_CurrentServerState == ServerState::Host)
		{
			m_server->updatePlayerData();

			timer += t_deltaTime.asSeconds();

			if (m_player->collisionCheck(Enemy1.getShape()) == true)
			{
				GameFinishText.setString("Game Finished.Green Player caught blue This game took" + std::to_string(timer));
				m_currentState = GameState::Finish;
				m_server->ChangeGameState(m_currentState);
			}
			else if (m_player->collisionCheck(Enemy2.getShape()) == true)
			{
				GameFinishText.setString("Game Finished.Red Player caught blue  This game took" + std::to_string(timer));
				m_currentState = GameState::Finish;
				m_server->ChangeGameState(m_currentState);
			}
		}
	}
}

/// <summary>
/// draw the frame and then switch buffers
/// </summary>
void Game::render()
{
	m_window.clear(sf::Color::Black);

	if (m_CurrentServerState == ServerState::None)
	{
		m_window.draw(hostClientText);
	}

	if (m_currentState == GameState::Playing)
	{
		m_player->draw(m_window);
		Enemy1.draw(m_window);
		Enemy2.draw(m_window);
	}

	if (m_currentState == GameState::Waiting)
	{
		m_window.draw(GameChangeText);
	}

	if (m_CurrentServerState == ServerState::Client && m_currentState == GameState::Connecting)
	{
		m_window.draw(IPText);
	}

	if (m_currentState == GameState::Finish)
	{
		m_window.draw(GameFinishText);
	}

	m_window.display();
}

bool Game::connectToServer()
{
	m_client = new Client(IP.c_str(), 1111);

	if (!m_client->Connect()) //If client fails to connect...
	{
		delete(m_client);
		return false;
	}
	else
	{
		return true;
	}
}

