#include "framework.h"
#include "Define.h"
#include "Actor.h"

CImage Actor::Img;
std::vector<std::vector<std::vector<std::pair<int, int>>>> Actor::Sprites;
int Actor::ImageSpriteWidth;
int Actor::ImageSpriteHeight;

Actor::Actor(bool IsPossess) :
	Position(100.f, 100.f),
	Frame(0.f),
	State(ACTOR_STATE::WALK),
	Direction(ACTOR_DIRECTION::UP)
{
	if (Img.IsNull())
		Img.Load(TEXT("Image/Player/Actor.png"));

	ImageSpriteWidth = 16;
	ImageSpriteHeight = 33;

	LoadSprite();
}

void Actor::Update(float elapsedTime)
{
	Frame += elapsedTime;
	if (Frame > Sprites[(int)State][(int)Direction].size())
		Frame -= Sprites[(int)State][(int)Direction].size();
}

void Actor::Draw(HDC& memdc)
{
	int BoardWidthSize = WINWIDTH / BOARDSIZE;
	int BoardHeightSize = WINHEIGHT / BOARDSIZE;

	RECT ImageDst{ WINWIDTH / 2 - BoardWidthSize, WINHEIGHT / 2 - (BoardHeightSize * 3),
		BoardWidthSize + WINWIDTH / 2, BoardHeightSize + WINHEIGHT / 2 };

	{	// Draw Body
		int SrcX = Sprites[(int)State][(int)Direction][(int)Frame].first;
		int SrcY = Sprites[(int)State][(int)Direction][(int)Frame].second;
		RECT ImageSrc{ SrcX, SrcY, SrcX + ImageSpriteWidth, SrcY + ImageSpriteHeight };
		Img.Draw(memdc, ImageDst, ImageSrc);
	}

	{	// Draw Arm
		int SrcX = Sprites[(int)State + (int)ACTOR_STATE::END][(int)Direction][(int)Frame].first;
		int SrcY = Sprites[(int)State + (int)ACTOR_STATE::END][(int)Direction][(int)Frame].second;
		RECT ImageSrc{ SrcX, SrcY, SrcX + ImageSpriteWidth, SrcY + ImageSpriteHeight };
		Img.Draw(memdc, ImageDst, ImageSrc);
	}
}

void Actor::LoadSprite()
{
	if (not Sprites.empty()) return;

	std::vector<int> IndexList{ 0,1,2,1 };

	std::vector< std::vector<std::pair<int, int>>> TempVector;

	{	// BODY
		TempVector.clear();
		for(const auto& i : IndexList)
		{	// IDLE
			std::vector< std::pair<int, int>> TempTempVector;
			TempTempVector.emplace_back(std::make_pair<int, int>(0, ImageSpriteHeight * i));
			TempVector.push_back(TempTempVector);
		}
		Sprites.push_back(TempVector);

		TempVector.clear();
		for (const auto& i : IndexList)
		{	// WALK
			std::vector< std::pair<int, int>> TempTempVector;
			for (int j = 0; j < 2; j++)
			{
				TempTempVector.emplace_back(std::make_pair<int, int>(ImageSpriteWidth * (1 + j), ImageSpriteHeight * i));
			}
			TempVector.push_back(TempTempVector);
		}
		Sprites.push_back(TempVector);

		TempVector.clear();
		for (const auto& i : IndexList)
		{	// RUN
			std::vector< std::pair<int, int>> TempTempVector;
			for (int j = 0; j < 2; j++)
			{
				TempTempVector.emplace_back(std::make_pair<int, int>(ImageSpriteWidth * (i * 2 + j), ImageSpriteHeight * 3));
			}
			TempVector.push_back(TempTempVector);
		}
		Sprites.push_back(TempVector);
	}

	{	// ARM
		TempVector.clear();
		for (const auto& i : IndexList)
		{	// IDLE
			std::vector< std::pair<int, int>> TempTempVector;
			TempTempVector.emplace_back(std::make_pair<int, int>(ImageSpriteWidth * 6, ImageSpriteHeight * i));
			TempVector.push_back(TempTempVector);
		}
		Sprites.push_back(TempVector);

		TempVector.clear();
		for (const auto& i : IndexList)
		{	// WALK
			std::vector< std::pair<int, int>> TempTempVector;
			for (int j = 0; j < 2; j++)
			{
				TempTempVector.emplace_back(std::make_pair<int, int>(ImageSpriteWidth * (7 + j), ImageSpriteHeight * i));
			}
			TempVector.push_back(TempTempVector);
		}
		Sprites.push_back(TempVector);

		TempVector.clear();
		for (const auto& i : IndexList)
		{	// RUN
			std::vector< std::pair<int, int>> TempTempVector;
			for (int j = 0; j < 2; j++)
			{
				TempTempVector.emplace_back(std::make_pair<int, int>(ImageSpriteWidth * (i * 2 + j + 6), ImageSpriteHeight * 3));
			}
			TempVector.push_back(TempTempVector);
		}
		Sprites.push_back(TempVector);
	}

	TempVector.clear();
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
