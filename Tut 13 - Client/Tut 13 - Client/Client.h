#pragma once
#include <WinSock2.h> //For SOCKET
#include <string> //For std::string
#include "PacketManager.h" //For PacketManager class

#include "Player.h"
#include "Enemy.h"



class Game;

class Client
{
public: //Public functions
	Client(const char * ip, const int port);

	void SetGame(Game* t_game);

	bool Connect();
	void Disconnect();
	void SendString(const std::string & str);
	void SendPlayerData(const PlayerData& data);

	void SetPlayer(Player* t_player);

	void setEnemies(Enemy& t_enemy1, Enemy& t_enemy2);

	~Client();
private: //Private functions
	bool CloseConnection();
	bool ProcessPacketType(const PacketType packetType);
	static void ClientThread(Client & client); //Client thread which reads messages from server
	static void PacketSenderThread(Client & client); //Packet sender thread which sends out packets existing in packet manager
	//Sending Funcs
	bool sendall(const char * data, const int totalBytes);
	//Getting Funcs
	bool recvall(char * data, const int totalBytes);
	bool Getint32_t(std::int32_t & int32_t);
	bool GetPacketType(PacketType & packetType);
	bool GetString(std::string & str);

	bool GetPlayerData(PlayerData& data);

	bool GetGameState(GameStateUpdate& state);

	bool GetGameEndMsg(GameEnd& gEnd);

private: //Private variables
	bool m_terminateThreads = false;
	bool m_isConnected = false;
	SOCKET m_connection;//This client's connection to the server
	SOCKADDR_IN m_addr; //Address to be binded to our Connection socket
	PacketManager m_pm; //Packet manager which sends all of client's outgoing packets

	std::thread m_pst; //Create thread to send packets
	std::thread m_ct; //Create thread to listen to server

	Game* m_game = nullptr;

	Player* m_player;

	Enemy* m_enemy1 = nullptr;

	Enemy* m_enemy2 = nullptr;
};