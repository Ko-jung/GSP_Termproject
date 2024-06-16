#pragma once
#include "../Define.h"

#include "../../../Common/EnumDef.h"
#include "../../../Common/GameUtil.h"
#include "../../../Common/protocol.h"

#include <array>
#include <atlimage.h>

class MapMgr
{
public:
	SINGTON(MapMgr);

public:
	void Init();
	void Draw(HDC& memdc, const POSITION& pos);

	WORD GetMapInfo(const int x, const int y) { return WorldMap[y][x]; }
	std::array<std::array<WORD, 2000>, 2000>* GetMap() { return &WorldMap; }

private:
	CImage WorldImageTile;
	std::array<std::array<WORD, 2000>, 2000> WorldMap;
};

