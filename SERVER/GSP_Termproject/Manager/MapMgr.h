#pragma once
#include "../stdafx.h"
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
	std::pair<WORD, WORD> GetRandomCanSpawnPos();

private:
	std::array<std::array<WORD, 2000>, 2000> WorldMap;
	std::vector<std::pair<WORD, WORD>> CanSpawnPos;
};

