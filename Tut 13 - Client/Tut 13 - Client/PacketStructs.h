#pragma once
#include "PacketType.h"
#include "Packet.h"
#include "ServerData.h"
#include <string> //for std::string
#include <memory> //for std::shared_ptr

namespace PS //Packet Structures Namespace
{
	class ChatMessage
	{
	public:
		ChatMessage(const std::string & str);
		std::shared_ptr<Packet> toPacket(); //Converts ChatMessage to packet
	private:
		std::string m_message;
	};

	class GameUpdate
	{
	public:
		GameUpdate(const PlayerData& t_data);
		std::shared_ptr<Packet> toPacket(PacketType t_pt); 
	private:
		PlayerData m_playData;
	};

	class gameState
	{
	public:
		gameState(const GameStateUpdate& t_state);
		std::shared_ptr<Packet> toPacket(PacketType t_pt);
	private:
		GameStateUpdate m_state;
	};

	class gameEndMsg
	{
	public:
		gameEndMsg(const GameEnd& t_state);
		std::shared_ptr<Packet> toPacket(PacketType t_pt);

	private:
		GameEnd m_gEnd;
	};
}