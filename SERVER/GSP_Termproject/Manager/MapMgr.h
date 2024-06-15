#pragma once
#include "../Define.h"

#include <array>
#include <atlimage.h>

class MapMgr
{
public:
	SINGLETON(MapMgr);

public:
	void Init();

	WORD GetMapInfo(const int x, const int y) { return WorldMap[y][x]; }

private:
	std::array<std::array<WORD, 2000>, 2000> WorldMap;
};

