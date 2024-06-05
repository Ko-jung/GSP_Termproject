#pragma once

#include <Windows.h>

enum class COMP_TYPE : BYTE
{
	// Default
	OP_RECV,
	OP_SEND,
	OP_ACCEPT,
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