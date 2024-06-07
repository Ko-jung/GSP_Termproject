#pragma once

#include <Windows.h>
#include <atlimage.h>
#include <string>
#include <vector>

#include "../../Common/GameUtil.h"
#include "../../Common/protocol.h"
#include "../../Common/EnumDef.h"

class Actor
{
public:
	Actor(bool IsPossess = true);
	virtual ~Actor() {}

	void Update(float elapsedTime);
	void Draw(HDC& memdc);
	void LoadSprite();

	void Move(WPARAM wParam);
	//void SetLocation(POINT location) { Location.x = location.x; Location.y = location.y; }
	//POINT GetLocation(POINT location) { Location.x = location.x; Location.y = location.y; }
	void SetLocation(POSITION pos) { Position = pos; }
	POSITION GetLocation() { return Position; }
	void SetName(const char* name) { Name = name; }

	void ProcessLogin(SC_LOGIN_INFO_PACKET* SLIP);

protected:
	POSITION Position;
	float Speed;
	int	CurrentHp;
	int	MaxHp;
	int	Exp;
	int	Level;
	ACTOR_STATE State;
	static CImage Img;

	std::string Name;

	std::vector<std::vector<RECT*>> Sprites;
};

