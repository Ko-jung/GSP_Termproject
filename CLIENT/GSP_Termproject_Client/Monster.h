#pragma once
#include "framework.h"
#include "../../Common/GameUtil.h"
#include "../../Common/EnumDef.h"
#include "../../Common/protocol.h"

#include "framework.h"
#include <atlimage.h>

class Monster
{
public:
	Monster();
	~Monster();

	void InitUsePacket(SC_ADD_OBJECT_PACKET* SAOP);
	void Update(float elapsedTime);
	void Draw(HDC& memdc);
	void LoadSprite();

	POSITION Position;
	int	CurrentHp;
	int	MaxHp;
	MONSTER_STATE State;
	ACTOR_DIRECTION Direction;
	float Frame;
	float Size;
	std::string Name;

	static CImage Img;
	static std::vector<std::vector<std::vector<std::pair<int, int>>>> Sprites;
	static int ImageSpriteWidth;
	static int ImageSpriteHeight;
};

