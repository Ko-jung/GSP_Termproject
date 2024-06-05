#include "framework.h"
#include "Define.h"
#include "Actor.h"

Actor::Actor(bool IsPossess) :
	X(100.f), Y(100.f)
{
	if (IsPossess)
	{
		Img.Load(TEXT("Image/흰색말.png"));
	}
	else
	{
		Img.Load(TEXT("Image/검은말.png"));
	}
}

void Actor::Update(float elapsedTime)
{
}

void Actor::Draw(HDC& memdc)
{
	float BoardWidthSize = WINWIDTH / BOARDSIZE;
	float BoardHeightSize = WINHEIGHT / BOARDSIZE;


	//RECT TargetDrawRect = { Location.x * BoardWidthSize, Location.y * BoardHeightSize,
	//	Location.x * BoardWidthSize + BoardWidthSize, Location.y* BoardHeightSize + BoardHeightSize };
	//RECT ImgRect = { 0,0,Img.GetWidth(), Img.GetHeight() };
	//Img.Draw(memdc, TargetDrawRect, ImgRect);
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