#pragma once
enum class PacketType
{
	ChatMessage,
	PlayerData,
	GameStateUpdate,
	GameEndMessage,
	Setup,
	SetupEnemies
};