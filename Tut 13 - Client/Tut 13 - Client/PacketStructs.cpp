#include "PacketStructs.h"

namespace PS
{
	ChatMessage::ChatMessage(const std::string & msg)
		:m_message(msg)
	{
	}

	std::shared_ptr<Packet> ChatMessage::toPacket()
	{
		std::shared_ptr<Packet> p = std::make_shared<Packet>();
		p->Append(PacketType::ChatMessage);
		p->Append(m_message.size());
		p->Append(m_message);
		return p;
	}

	GameUpdate::GameUpdate(const PlayerData& t_data)
	{
		m_playData = t_data;
	}

	std::shared_ptr<Packet> GameUpdate::toPacket(PacketType t_pt)
	{
		std::shared_ptr<Packet> p = std::make_shared<Packet>();
		p->Append(t_pt);
		p->Append(sizeof(m_playData));
		p->Append(m_playData);
		return p;
	}

	gameState::gameState(const GameStateUpdate& t_data)
	{
		m_state = t_data;
	}

	std::shared_ptr<Packet> gameState::toPacket(PacketType t_pt)
	{
		std::shared_ptr<Packet> p = std::make_shared<Packet>();
		p->Append(t_pt);
		p->Append(sizeof(m_state));
		p->Append(m_state);
		return p;
	}
	gameEndMsg::gameEndMsg(const GameEnd& t_gEnd)
	{
		m_gEnd = t_gEnd;
	}

	std::shared_ptr<Packet> gameEndMsg::toPacket(PacketType t_pt)
	{
		std::shared_ptr<Packet> p = std::make_shared<Packet>();
		p->Append(t_pt);
		p->Append(sizeof(m_gEnd));
		p->Append(m_gEnd);
		return p;
	}
}