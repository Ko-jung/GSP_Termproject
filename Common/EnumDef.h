#pragma once

#include <Windows.h>

enum class COMP_TYPE : BYTE
{
	// Default
	OP_RECV,
	OP_SEND,
	OP_ACCEPT,

	OP_NPC_MOVE,
	OP_SPAWN_PLAYER,
};

enum class KEY_INFO : BYTE
{
	UP = 3,
	DOWN = 2,
	LEFT = 1,
	RIGHT = 0,
};

enum class MAP_INFO : WORD
{
	WALLS_BLOCK = 18 * 1 + 5,
	GROUND_EMPTY = 18 * 1 + 14,
	EMPTY,
};

enum class CLIENT_STATE : BYTE
{
	FREE,			// Not Use
	ALLOC,			// Accpet but not Use in game
	INGAME,			// Playing in Game
};

enum class ACTOR_STATE : BYTE
{
	IDLE,
	WALK,
	RUN,
	ATTACK,
	DIE,
	END,
};

enum class ACTOR_DIRECTION : BYTE
{
	DOWN,
	RIGHT,
	UP,
	LEFT,
};

enum class MONSTER_STATE : BYTE
{
	IDLE,
	DIE,
	END,
};

enum class EVENT_TYPE : BYTE
{
	EV_RANDOM_MOVE,
	EV_SPAWN_PLAYER,
};

enum class MOUSE_TYPE : BYTE
{
	LEFT_BTN,
	MIDDLE_BTN,
	RIGHT_BTN
};

enum class WEAPON_TYPE : BYTE
{
	SWORD,
	PICKAXE
};