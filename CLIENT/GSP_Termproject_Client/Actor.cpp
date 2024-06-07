#include "framework.h"
#include "Define.h"
#include "Actor.h"

CImage Actor::Img;

Actor::Actor(bool IsPossess) :
	Position(100.f, 100.f)
{
	if (Img.IsNull())
		Img.Load(TEXT("Image/Player/Actor.png"));

	//LoadSprite();
}

void Actor::Update(float elapsedTime)
{
}

void Actor::Draw(HDC& memdc)
{
	static int ImageSpriteWidth = 16;
	static int ImageSpriteHeight = 34;

	float BoardWidthSize = WINWIDTH / BOARDSIZE;
	float BoardHeightSize = WINHEIGHT / BOARDSIZE;

	// RECT ImageDst{ ImageSpriteWidth * 0, (int)State * ImageSpriteHeight,
	// 	ImageSpriteWidth * 0 + ImageSpriteWidth, (int)State * ImageSpriteHeight + ImageSpriteHeight };
	RECT ImageDst{ WINWIDTH / 2 - BoardWidthSize, WINHEIGHT / 2 - BoardHeightSize,
		BoardWidthSize + WINWIDTH / 2, BoardHeightSize + WINHEIGHT / 2 };
	RECT ImageSrc{ ImageSpriteWidth * 0, (int)0 * ImageSpriteHeight,
		ImageSpriteWidth * 0 + ImageSpriteWidth, (int)0 * ImageSpriteHeight + ImageSpriteHeight };

	Img.Draw(memdc, ImageDst, ImageSrc);
}

void Actor::LoadSprite()
{

	//Sprites.emplace_back();
}

void Actor::Move(WPARAM wParam)
{
	// switch (wParam)
	// {
	// case VK_UP:
	// case 'w':
	// case 'W':
	// 	if (--Location.y < 0)
	// 	{
	// 		Location.y = 0;
	// 	}
	// 	break;
	// case VK_LEFT:
	// case 'a':
	// case 'A':
	// 	if (--Location.x < 0)
	// 	{
	// 		Location.x = 0;
	// 	}
	// 	break;
	// case VK_DOWN:
	// case 's':
	// case 'S':
	// 	if (++Location.y >= BOARDSIZE)
	// 	{
	// 		Location.y = BOARDSIZE - 1;
	// 	}
	// 	break;
	// case VK_RIGHT:
	// case 'd':
	// case 'D':
	// 	if (++Location.x >= BOARDSIZE)
	// 	{
	// 		Location.x = BOARDSIZE - 1;
	// 	}
	// 	break;
	// default:
	// 	break;
	// }
}

void Actor::ProcessLogin(SC_LOGIN_INFO_PACKET* SLIP)
{
	CurrentHp = SLIP->hp;
	MaxHp = SLIP->max_hp;
	Exp = SLIP->exp;
	Level = SLIP->level;
	Position.X = SLIP->x;
	Position.Y = SLIP->y;
}
