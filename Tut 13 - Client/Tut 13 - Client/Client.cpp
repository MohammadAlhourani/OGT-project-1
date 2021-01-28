#include "Client.h"
#include <Ws2tcpip.h> //for inet_pton
#pragma comment(lib,"ws2_32.lib") //Required for WinSock
#include <iostream> //for std::cout

#include "Game.h"

bool Client::ProcessPacketType(PacketType packetType)
{
	switch (packetType)
	{
	case PacketType::PlayerData:
	{
		PlayerData playerInfo;

		if (!GetPlayerData(playerInfo))
		{
			return false;
		}
				
		
		if (m_enemy1->getId() == playerInfo.k_ID)
		{
			m_enemy1->setPosition(playerInfo.k_position);
			m_enemy1->setColour(playerInfo.k_colour);
		}		

		if (m_enemy2->getId() == playerInfo.k_ID)
		{
			m_enemy2->setPosition(playerInfo.k_position);
			m_enemy2->setColour(playerInfo.k_colour);
		}

		break;
	}
	case PacketType::Setup:
	{
		PlayerData playerInfo;

		if (!GetPlayerData(playerInfo))
		{
			return false;
		}

		m_player->setId(playerInfo.k_ID);

		PlayerData playerstuff;

		playerstuff.k_ID = m_player->getId();

		PS::GameUpdate gameU(playerstuff);

		m_pm.Append(gameU.toPacket(PacketType::Setup));

		break;
	}
	case PacketType::SetupEnemies:
	{
		PlayerData playerInfo;

		if (!GetPlayerData(playerInfo))
		{
			return false;
		}

		if (m_player->getId() == 0)
		{

		}
		else if (m_player->getId() == 1 )
		{
			m_player->setPosition(sf::Vector2f(300,200));
			m_player->setColour(sf::Color::Green);

			m_enemy1->setId(0);
			m_enemy2->setId(2);
		}
		else if (m_player->getId() == 2)
		{
			m_player->setPosition(sf::Vector2f(600,200));
			m_player->setColour(sf::Color::Red);

			m_enemy1->setId(0);
			m_enemy2->setId(1);
		}

		break;
	}
	case PacketType::GameStateUpdate:
	{
		GameStateUpdate GTU;

		if (!GetGameState(GTU))
		{
			return false;
		}

		m_game->ChangeGameState(GTU.k_state);

		break;
	}
	case PacketType::GameEndMessage:
	{
		GameEnd gEnd;

		if (!GetGameEndMsg(gEnd))
		{
			return false;
		}

		m_game->setTimer(gEnd.Time);

		m_game->setGameEndMsg(gEnd.gameEndText);

		break;
	}
	case PacketType::ChatMessage: //If PacketType is a chat message PacketType
	{
		std::string Message; //string to store our message we received
		if (!GetString(Message)) //Get the chat message and store it in variable: Message
			return false; //If we do not properly get the chat message, return false
		std::cout << Message << std::endl; //Display the message to the user
		break;
	}
	default: //If PacketType type is not accounted for
		std::cout << "Unrecognized PacketType: " << (std::int32_t)packetType << std::endl; //Display that PacketType was not found
		break;
	}
	return true;
}

void Client::ClientThread(Client & client)
{
	PacketType PacketType;
	while (true)
	{
		if (client.m_terminateThreads == true)
			break;
		if (!client.GetPacketType(PacketType)) //Get PacketType type
			break; //If there is an issue getting the PacketType type, exit this loop
		if (!client.ProcessPacketType(PacketType)) //Process PacketType (PacketType type)
			break; //If there is an issue processing the PacketType, exit this loop
	}
	std::cout << "Lost connection to the server.\n";
	client.m_terminateThreads = true;
	if (client.CloseConnection()) //Try to close socket connection..., If connection socket was closed properly
	{
		std::cout << "Socket to the server was closed successfully." << std::endl;
	}
	else //If connection socket was not closed properly for some reason from our function
	{
		std::cout << "Socket was not able to be closed." << std::endl;
	}
}

void Client::PacketSenderThread(Client & client) //Thread for all outgoing packets
{
	while (true)
	{
		if (client.m_terminateThreads == true)
			break;
		while (client.m_pm.HasPendingPackets())
		{
			std::shared_ptr<Packet> p = client.m_pm.Retrieve();
			if (!client.sendall((const char*)(&p->m_buffer[0]), p->m_buffer.size()))
			{
				std::cout << "Failed to send packet to server..." << std::endl;
				break;
			}
		}
		Sleep(5);
	}
	std::cout << "Packet thread closing..." << std::endl;
}

void Client::Disconnect()
{
	m_pm.Clear();
	closesocket(m_connection);
	std::cout << "Disconnected from server." << std::endl;
}