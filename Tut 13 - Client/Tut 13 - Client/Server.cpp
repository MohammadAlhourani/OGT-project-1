#include "Server.h"
#include <iostream>
#include "PacketStructs.h"
#pragma comment(lib,"ws2_32.lib") //Required for WinSock


Server::Server(int port, bool loopBacktoLocalHost) //Port = port to broadcast on. loopBacktoLocalHost = true if loopback to local host (nobody else can connect), false if loop back to any addr (anyone can connect) [assumes port is properly forwarded in router settings)
{
	//Winsock Startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) //If WSAStartup returns anything other than 0, then that means an error has occured in the WinSock Startup.
	{
		MessageBoxA(0, "WinSock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	if (loopBacktoLocalHost == true) //If server is to loopback to local host (others cannot connect)
		inet_pton(AF_INET, "127.0.0.1", &m_addr.sin_addr.s_addr);
	else
		m_addr.sin_addr.s_addr = htonl(INADDR_ANY); //loopback to any addr so others can connect

	m_addr.sin_port = htons(port); //Port
	m_addr.sin_family = AF_INET; //IPv4 Socket

	m_sListen = socket(AF_INET, SOCK_STREAM, 0); //Create socket to listen for new connections
	if (bind(m_sListen, (SOCKADDR*)&m_addr, sizeof(m_addr)) == SOCKET_ERROR) //Bind the address to the socket, if we fail to bind the address..
	{
		std::string ErrorMsg = "Failed to bind the address to our listening socket. Winsock Error:" + std::to_string(WSAGetLastError());
		MessageBoxA(0, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}
	if (listen(m_sListen, SOMAXCONN) == SOCKET_ERROR) //Places sListen socket in a state in which it is listening for an incoming connection. Note:SOMAXCONN = Socket Oustanding Max connections, if we fail to listen on listening socket...
	{
		std::string ErrorMsg = "Failed to listen on listening socket. Winsock Error:" + std::to_string(WSAGetLastError());
		MessageBoxA(0, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}


	m_IDCounter = 0;
	std::thread PST(PacketSenderThread, std::ref(*this));
	PST.detach();
	m_threads.push_back(&PST);
}

bool Server::ListenForNewConnection(Server& t_server)
{
	while (true)
	{
		int addrlen = sizeof(m_addr);
		SOCKET newConnectionSocket = accept(t_server.m_sListen, (SOCKADDR*)&t_server.m_addr, &addrlen); //Accept a new connection
		if (newConnectionSocket == 0) //If accepting the client connection failed
		{
			std::cout << "Failed to accept the client's connection." << std::endl;
		}
		else //If client connection properly accepted
		{
			std::lock_guard<std::shared_mutex> lock(t_server.m_mutex_connectionMgr); //Lock connection manager mutex since we are adding an element to connection vector
			std::shared_ptr<Connection> newConnection(std::make_shared<Connection>(newConnectionSocket));
			t_server.m_connections.push_back(newConnection); //push new connection into vector of connections
			t_server.m_IDCounter += 1; //Increment ID Counter...

			newConnection->m_ID = t_server.m_IDCounter; //Set ID for this connection
		
			std::cout << "Client Connected! ID:" << newConnection->m_ID << std::endl;
			std::thread CHT(ClientHandlerThread, std::ref(t_server), newConnection);
			CHT.detach();
			t_server.m_threads.push_back(&CHT);
		}
	}
}

void Server::setPlayer(Player* t_player)
{
	m_player = t_player;
}

void Server::setEnemies(Enemy& t_enemy1, Enemy& t_enemy2)
{
	m_enemy1 = &t_enemy1;

	m_enemy2 = &t_enemy2;
}



void Server::updatePlayerData()
{
	for (auto conn : m_connections)
	{
		SendPlayerData(conn, m_player->getPlayerData());
	}
}

void Server::settupGame()
{
	for (auto conn : m_connections)
	{
		PlayerData pd;

		pd.k_ID = conn->m_ID;

		PS::GameUpdate gameu(pd);

		conn->m_pm.Append(gameu.toPacket(PacketType::Setup));
	}
}

void Server::EndGame(GameEnd gEnd)
{
	for (int i = 0; i < m_connections.size(); i++)
	{
		GameEnd gameEnding;

		gameEnding = gEnd;

		PS::gameEndMsg gem(gameEnding);

		m_connections[i]->m_pm.Append(gem.toPacket(PacketType::GameEndMessage));
	}
}

void Server::ChangeGameState(GameState t_state)
{
	for (int i = 0; i < m_connections.size(); i++)
	{
		GameStateUpdate GTU;

		GTU.k_state = t_state;

		PS::gameState gstate(GTU);

		m_connections[i]->m_pm.Append(gstate.toPacket(PacketType::GameStateUpdate));
	}
}

int Server::getConnectionAmount()
{
	return m_connections.size();
}


Server::~Server()
{
	m_terminateThreads = true;
	for (std::thread* t : m_threads) //Wait for all created threads to end...
	{
		t->join();
	}
}

void Server::ClientHandlerThread(Server & server, std::shared_ptr<Connection> connection) //ID = the index in the SOCKET connections array
{
	PacketType packettype;
	while (true)
	{
		if (server.m_terminateThreads == true)
			break;
		if (!server.GetPacketType(connection, packettype)) //Get packet type
			break; //If there is an issue getting the packet type, exit this loop
		if (!server.ProcessPacket(connection, packettype)) //Process packet (packet type)
			break; //If there is an issue processing the packet, exit this loop
	}
	std::cout << "Lost connection to client ID: " << connection->m_ID << std::endl;
	server.DisconnectClient(connection); //Disconnect this client and clean up the connection if possible
	return;
}

void Server::PacketSenderThread(Server & server) //Thread for all outgoing packets
{
	while (true)
	{
		if (server.m_terminateThreads == true)
			break;
		std::shared_lock<std::shared_mutex> lock(server.m_mutex_connectionMgr);
		for (auto conn : server.m_connections) //for each connection...
		{
			if (conn->m_pm.HasPendingPackets()) //If there are pending packets for this connection's packet manager
			{
				std::shared_ptr<Packet> p = conn->m_pm.Retrieve(); //Retrieve packet from packet manager
				if (!server.sendall(conn, (const char*)(&p->m_buffer[0]), p->m_buffer.size())) //send packet to connection
				{
					std::cout << "Failed to send packet to ID: " << conn->m_ID << std::endl; //Print out if failed to send packet
				}
			}
		}
		Sleep(5);
	}
	std::cout << "Ending Packet Sender Thread..." << std::endl;
}

void Server::DisconnectClient(std::shared_ptr<Connection> connection) //Disconnects a client and cleans up socket if possible
{
	std::lock_guard<std::shared_mutex> lock(m_mutex_connectionMgr); //Lock connection manager mutex since we are possible removing element(s) from the vector
	connection->m_pm.Clear(); //Clear out all remaining packets in queue for this connection
	closesocket(connection->m_socket); //Close the socket for this connection
	m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), connection)); //Remove connection from vector of connections
	std::cout << "Cleaned up client: " << connection->m_ID << "." << std::endl;
	std::cout << "Total connections: " << m_connections.size() << std::endl;
}

bool Server::recvall(std::shared_ptr<Connection> connection, char* data, int totalbytes)
{
	int bytesReceived = 0; //Holds the total bytes received
	while (bytesReceived < totalbytes) //While we still have more bytes to recv
	{
		int retnCheck = recv(connection->m_socket, data + bytesReceived, totalbytes - bytesReceived, 0); //Try to recv remaining bytes
		if (retnCheck == SOCKET_ERROR || retnCheck == 0) //If there is a socket error while trying to recv bytes or if connection lost
			return false; //Return false - failed to recvall
		bytesReceived += retnCheck; //Add to total bytes received
	}
	return true; //Success!
}

bool Server::sendall(std::shared_ptr<Connection> connection, const char* data, const int totalBytes)
{
	int bytesSent = 0; //Holds the total bytes sent
	while (bytesSent < totalBytes) //While we still have more bytes to send
	{
		int retnCheck = send(connection->m_socket, data + bytesSent, totalBytes - bytesSent, 0); //Try to send remaining bytes
		if (retnCheck == SOCKET_ERROR) //If there is a socket error while trying to send bytes
			return false; //Return false - failed to sendall
		bytesSent += retnCheck; //Add to total bytes sent
	}
	return true; //Success!
}

bool Server::Getint32_t(std::shared_ptr<Connection> connection, std::int32_t& int32_t)
{
	if (!recvall(connection, (char*)&int32_t, sizeof(std::int32_t))) //Try to receive long (4 byte int)... If int fails to be recv'd
		return false; //Return false: Int not successfully received
	int32_t = ntohl(int32_t); //Convert long from Network Byte Order to Host Byte Order
	return true;//Return true if we were successful in retrieving the int
}

bool Server::GetPacketType(std::shared_ptr<Connection> connection, PacketType& packetType)
{
	std::int32_t packettype_int;
	if (!Getint32_t(connection, packettype_int)) //Try to receive packet type... If packet type fails to be recv'd
		return false; //Return false: packet type not successfully received
	packetType = (PacketType)packettype_int;
	return true;//Return true if we were successful in retrieving the packet type
}

void Server::SendString(std::shared_ptr<Connection> connection, const std::string& str)
{
	PS::ChatMessage message(str);
	connection->m_pm.Append(message.toPacket());
}

bool Server::GetString(std::shared_ptr<Connection> connection, std::string& str)
{
	std::int32_t bufferlength; //Holds length of the message
	if (!Getint32_t(connection, bufferlength)) //Get length of buffer and store it in variable: bufferlength
		return false; //If get int fails, return false
	if (bufferlength == 0) return true;
	str.resize(bufferlength); //resize string to fit message
	return recvall(connection, &str[0], bufferlength);
}

void Server::SendPlayerData(std::shared_ptr<Connection> connection, const PlayerData& data)
{
	PS::GameUpdate gameU(data);
	connection->m_pm.Append(gameU.toPacket(PacketType::PlayerData));
}

bool Server::GetPlayerData(std::shared_ptr<Connection> connection, PlayerData& data)
{
	std::int32_t bufferlength; //Holds length of the message

	if (!Getint32_t(connection, bufferlength)) //Get length of buffer and store it in variable: bufferlength
	{
		return false; //If get int fails, return false
	}

	if (bufferlength == 0)
	{
		return true;
	}

	return recvall(connection, (char*)&data, bufferlength);
}

void Server::SendGameState(std::shared_ptr<Connection> connection, const GameStateUpdate& state)
{
	PS::gameState gameState(state);
	connection->m_pm.Append(gameState.toPacket(PacketType::GameStateUpdate));
}

void Server::SendGameEnd(std::shared_ptr<Connection> connection, const GameEnd& gEnd)
{
	PS::gameEndMsg gameEndmessage(gEnd);
	connection->m_pm.Append(gameEndmessage.toPacket(PacketType::GameEndMessage));
}

bool Server::ProcessPacket(std::shared_ptr<Connection> connection, PacketType packetType)
{
	switch (packetType)
	{
	case PacketType::PlayerData:
	{
		PlayerData playerInfo;

		if (!GetPlayerData(connection, playerInfo))
		{
			return false;
		}

		PS::GameUpdate data(playerInfo);

		std::shared_ptr<Packet> playerPacket = std::make_shared<Packet>(data.toPacket(PacketType::PlayerData));
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex_connectionMgr);

			for (auto con : m_connections) //For each connection...
			{
				if (con == connection)
				{
					continue;
				}
				else
				{
					con->m_pm.Append(playerPacket);
				}
			}
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

		//std::cout << "Processed player data packet from user ID: " << connection->m_ID << std::endl;
		break;

	}
	case PacketType::Setup:
	{
		PlayerData playerInfo;

		if (!GetPlayerData(connection, playerInfo))
		{
			return false;
		}

		PS::GameUpdate data(playerInfo);

		std::shared_ptr<Packet> playerPacket = std::make_shared<Packet>(data.toPacket(PacketType::SetupEnemies));
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex_connectionMgr);

			for (auto con : m_connections) //For each connection...
			{
				if (con == connection)
				{
					continue;
				}
				else
				{
					con->m_pm.Append(playerPacket);
				}
			}
		}
		break;
	}
	case PacketType::ChatMessage: //Packet Type: chat message
	{
		std::string message; //string to store our message we received
		if (!GetString(connection, message)) //Get the chat message and store it in variable: Message
			return false; //If we do not properly get the chat message, return false
						  //Next we need to send the message out to each user

		PS::ChatMessage cm(message);
		std::shared_ptr<Packet> msgPacket = std::make_shared<Packet>(cm.toPacket()); //use shared_ptr instead of sending with SendString so we don't have to reallocate packet for each connection
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex_connectionMgr);
			for (auto conn : m_connections) //For each connection...
			{
				if (conn == connection) //If connection is the user who sent the message...
					continue;//Skip to the next user since there is no purpose in sending the message back to the user who sent it.
				conn->m_pm.Append(msgPacket);
			}
		}
		std::cout << "Processed chat message packet from user ID: " << connection->m_ID << std::endl;
		break;
	}
	default: //If packet type is not accounted for
	{
		std::cout << "Unrecognized packet: " << (std::int32_t)packetType << std::endl; //Display that packet was not found
		break;
	}
	}
	return true;
}