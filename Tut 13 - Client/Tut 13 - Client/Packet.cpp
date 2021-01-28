#include "Packet.h"
#include <cstdint> //Required to use std::int32_t
#include <WinSock2.h> //for htonl
Packet::Packet()
{
}

Packet::Packet(const std::shared_ptr<Packet> t_p)
{
	Append(t_p);
}

Packet::Packet(const char * dataBuffer, const int size)
{
	Append(dataBuffer, size);
}

Packet::Packet(PacketType pt)
{
	Append(pt);
}

void Packet::Append(const char * dataBuffer, const int size)
{
	m_buffer.insert(m_buffer.end(), dataBuffer, dataBuffer + size);
}

void Packet::Append(PlayerData t_data)
{
	Append((const char*)&t_data, sizeof(t_data));
}

void Packet::Append(GameStateUpdate t_state)
{
	Append((const char*)&t_state, sizeof(t_state));
}

void Packet::Append(GameEnd t_gEnd)
{
	Append((const char*)&t_gEnd, sizeof(t_gEnd));
}

void Packet::Append(const std::shared_ptr<Packet> t_p)
{
	Append((const char*)&(t_p->m_buffer[0]), t_p->m_buffer.size());
}

void Packet::Append(const std::string & str)
{
	Append(str.c_str(), str.size());
}

void Packet::Append(const Packet & p) //Allocate new block for buffer
{
	Append((const char*)&p.m_buffer, p.m_buffer.size());
}

void Packet::Append(std::int32_t int32)
{
	std::int32_t val = htonl((std::int32_t)int32);
	Append((const char*)&val, sizeof(std::int32_t));
}

void Packet::Append(PacketType pt)
{
	Append((std::int32_t)pt);
}

void Packet::Append(std::size_t s)
{
	Append((std::int32_t)s);
}
