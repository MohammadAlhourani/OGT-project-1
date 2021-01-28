#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include <list>

#include "Server.h"

#include "Globals.h"

#include "Client.h"

class Game
{
public:
	Game();
	~Game();

	/// <summary>
	/// main method for game
	/// </summary>
	void run();

	void ChangeGameState(GameState t_state);

	void setTimer(float t_time);

	void setGameEndMsg(std::string t_msg);

private:

	void processEvents();
	void processKeys(sf::Event t_event);
	void update(sf::Time t_deltaTime);
	void render();

	bool connectToServer();

	ServerState m_CurrentServerState = ServerState::None;

	GameState m_currentState = GameState::Connecting;

	std::thread* m_listenThread;

	sf::Font GAME_FONT;

	std::vector<char> acceptedChar = { '1', '2', '3' , '4' , '5' , '6' , '7' , '8', '9' , '0' , '.' }; 

	sf::RenderWindow m_window; // main SFML window

	Server* m_server = nullptr;

	Client* m_client = nullptr;

	std::string IP = "127.0.0.1";

	sf::Text IPText;

	sf::Text hostClientText;

	sf::Text GameChangeText;

	sf::Text GameFinishText;

	float timer = 0;

	Player* m_player = nullptr;

	Enemy Enemy1;

	Enemy Enemy2;

	bool m_exitGame; // control exiting game

	bool m_host;

};

#endif // !GAME_HPP

