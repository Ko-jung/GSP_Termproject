#pragma once

#include <Windows.h>
#include <atlimage.h>
#include <string>
#include <vector>
#include <utility>

#include "../../Common/GameUtil.h"
#include "../../Common/protocol.h"
#include "../../Common/EnumDef.h"

class Actor
{
public:
	Actor(bool IsPossess = true);
	virtual ~Actor() {}

	void InitUsePacket(SC_ADD_OBJECT_PACKET* SAOP);
	void Update(float elapsedTime);
	void UpdateAnim(float elapsedTime);
	void Draw(HDC& memdc);
	void LoadSprite();

	RECT GetRectDstWithPos();
	void Move(float elapsedTime);
	void InversionImage(HDC& memdc,RECT dstRect, RECT srcRect);
	void ChangeState(ACTOR_STATE state);
	void ChangeStateByPacket(ACTOR_STATE state);

	void ProcessUpInput(WPARAM wParam);
	void ProcessDownInput(WPARAM wParam);
	
	void SetLocation(POSITION pos) { Position = pos; }
	void SetName(const char* name) { Name = name; }
	void SetDirection(ACTOR_DIRECTION direction) { Direction = direction; }
	void SetIsChangeState(bool b) { IsChangeState = b; }

	POSITION GetLocation() { return Position; }
	ACTOR_DIRECTION GetDirection() { return Direction; }
	//char GetKeyInputInfo() { return KeyInputInfo; }
	const char* GetPlayerName() { return Name.c_str(); }
	ACTOR_STATE GetState() { return State; }
	bool GetIsChangeState() { return IsChangeState; }

	void ProcessAttack();

	void ProcessLogin(SC_LOGIN_INFO_PACKET* SLIP);
	void ProcessMove(SC_MOVE_OBJECT_PACKET* SLIP);
	void ProcessMove(SC_8DIRECT_MOVE_OBJECT_PACKET* SDMOP);
	void ProcessChangeStat(SC_STAT_CHANGE_PACKET* SSCP);

	bool IsCanRemove;
	int SerialNum;
protected:
	void DrawBody(HDC& memdc, const RECT& ImageDst);
	void DrawArm(HDC& memdc, const RECT& ImageDst);
	void DrawEffect(HDC& memdc, const RECT& ImageDst);

	bool IsCanDraw;
	bool IsPossessed;
	bool IsCanMove;
	bool IsChangeState;
	float ShowHpBarTimer;

	POSITION Position;
	float Speed;
	int	CurrentHp;
	int	MaxHp;
	int	Exp;
	int	Level;
	int Experience;
	ACTOR_STATE State;
	ACTOR_DIRECTION Direction;
	float Frame;

	char KeyInputInfo;

	// No Use NOW
	float Size;

	std::string Name;

	static CImage Img;
	static CImage AttackEffectImg;
	static std::vector<std::vector<std::vector<std::pair<int, int>>>> Sprites;
	static std::vector<std::pair<int, int>> AttackSprite;
	static std::vector<float> SprintFrameRate;
	static int ImageSpriteWidth;
	static int ImageSpriteHeight;
};

