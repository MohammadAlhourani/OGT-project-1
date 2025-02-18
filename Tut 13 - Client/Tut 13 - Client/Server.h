#pragma once
#include <WinSock2.h> //For win sockets
#include <string> //For std::string
#include "PacketManager.h" //for PacketManager class
#include "ServerData.h"
#include <WS2tcpip.h>
#include <vector> //for std::vector
#include <shared_mutex> //for shared_mutex

#include "Player.h"
#include "Enemy.h"

class Connection
{
public:
	Connection(SOCKET socket) 
		:m_socket(socket)
	{
	}
	SOCKET m_socket;
	//file transfer data
	PacketManager m_pm; //Packet Manager for outgoing data for this connection
	int m_ID = 0;
};

class Server
{
public: //Public functions
	Server(int port, bool loopBacktoLocalHost = true);
	~Server();
	static bool ListenForNewConnection(Server & t_server);

	void setPlayer(Player* t_player);

	void setEnemies(Enemy& t_enemy1, Enemy& t_enemy2);

	void updatePlayerData();

	void settupGame();

	void EndGame(GameEnd gEnd);

	void ChangeGameState(GameState t_state);

	int getConnectionAmount();

private: //Private functions
	bool sendall(std::shared_ptr<Connection> connection, const char * data, const int totalBytes);
	bool recvall(std::shared_ptr<Connection> connection, char * data, int totalBytes);
	bool Getint32_t(std::shared_ptr<Connection> connection, std::int32_t & int32_t);
	bool GetPacketType(std::shared_ptr<Connection> connection, PacketType & packetType);
	void SendString(std::shared_ptr<Connection> connection, const std::string & str);
	bool GetString(std::shared_ptr<Connection> connection, std::string & str);

	void SendPlayerData(std::shared_ptr<Connection> connection, const PlayerData& data);
	bool GetPlayerData(std::shared_ptr<Connection> connection, PlayerData& data);

	void SendGameState(std::shared_ptr<Connection> connection, const GameStateUpdate& state);

	void SendGameEnd(std::shared_ptr<Connection> connection, const GameEnd& gEnd);

	bool ProcessPacket(std::shared_ptr<Connection> connection, PacketType packetType);
	static void ClientHandlerThread(Server & server, std::shared_ptr<Connection> connection);
	static void PacketSenderThread(Server & server); 
	void DisconnectClient(std::shared_ptr<Connection> connection); //Called to properly disconnect and clean up a client (if possible)
private: //Private Variables
	std::vector<std::shared_ptr<Connection>> m_connections;
	std::shared_mutex m_mutex_connectionMgr; //mutex for managing connections (used when a client disconnects)
	int m_IDCounter = 0;
	SOCKADDR_IN m_addr; //Address that we will bind our listening socket to
	SOCKET m_sListen;
	bool m_terminateThreads = false;
	std::vector<std::thread*> m_threads; //so destructor can wait on created threads to end...

	Player* m_player = nullptr;

	Enemy* m_enemy1 = nullptr;

	Enemy* m_enemy2 = nullptr;
};
