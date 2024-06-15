#include "framework.h"
#include "Define.h"
#include "Actor.h"

CImage Actor::Img;
std::vector<std::vector<std::vector<std::pair<int, int>>>> Actor::Sprites;
int Actor::ImageSpriteWidth;
int Actor::ImageSpriteHeight;

Actor::Actor(bool IsPossess) :
	Position(100.f, 100.f),
	Speed(5.f),
	State(ACTOR_STATE::WALK),
	Direction(ACTOR_DIRECTION::LEFT),
	KeyInputInfo(0),
	Frame(0.f)
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

	Move(elapsedTime);
}

void Actor::Draw(HDC& memdc)
{
	int BoardWidthSize = WINWIDTH / BOARDSIZE;
	int BoardHeightSize = WINHEIGHT / BOARDSIZE;

	RECT ImageDst{ WINWIDTH / 2 - BoardWidthSize, WINHEIGHT / 2 - (BoardHeightSize * 3),
		BoardWidthSize + WINWIDTH / 2, BoardHeightSize + WINHEIGHT / 2 };
	//Img.SetTransparentColor((COLORREF)(132));

	{	// Draw Body

		int SrcX = Sprites[(int)State][(int)Direction][(int)Frame].first;
		int SrcY = Sprites[(int)State][(int)Direction][(int)Frame].second;

		RECT ImageSrc{ SrcX, SrcY, SrcX + ImageSpriteWidth, SrcY + ImageSpriteHeight };
		if (Direction != ACTOR_DIRECTION::LEFT)
		{
			Img.Draw(memdc, ImageDst, ImageSrc);
		}
		else
		{	// Inversion
			InversionImage(memdc, ImageDst, ImageSrc);
		}
	}

	{	// Draw Arm
		int SrcX = Sprites[(int)State + (int)ACTOR_STATE::END][(int)Direction][(int)Frame].first;
		int SrcY = Sprites[(int)State + (int)ACTOR_STATE::END][(int)Direction][(int)Frame].second;

		RECT ImageSrc{ SrcX, SrcY, SrcX + ImageSpriteWidth, SrcY + ImageSpriteHeight };
		if (Direction != ACTOR_DIRECTION::LEFT)
		{
			Img.Draw(memdc, ImageDst, ImageSrc);
		}
		else
		{	// Inversion
			InversionImage(memdc, ImageDst, ImageSrc);
		}
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

void Actor::Move(float elapsedTime)
{
	// Checking RIGHT
	if (KeyInputInfo & 0b0001)
	{
		if (Position.X < W_WIDTH - 1)
			Position.X += Speed * elapsedTime;
	}

	// Checking LEFT
	if (KeyInputInfo & 0b0010)
	{
		if (Position.X > 0)
			Position.X -= Speed * elapsedTime;
	}

	// Checking DOWN
	if (KeyInputInfo & 0b0100)
	{
		if (Position.Y < W_HEIGHT - 1)
			Position.Y += Speed * elapsedTime;
	}

	// Checking UP
	if (KeyInputInfo & 0b1000)
	{
		if (Position.Y > 0)
			Position.Y -= Speed * elapsedTime;
	}
}

void Actor::InversionImage(HDC& memdc, RECT dstRect, RECT srcImageRect)
{
	HDC memdc2 = CreateCompatibleDC(memdc);
	HBITMAP hBit1 = CreateCompatibleBitmap(memdc, ImageSpriteWidth, ImageSpriteHeight);
	HBITMAP oldBit1 = (HBITMAP)SelectObject(memdc2, hBit1);

	FillRect(memdc2, &srcImageRect, (HBRUSH)(COLOR_WINDOW + 1));

	Img.StretchBlt(memdc2, { 0,0,ImageSpriteWidth, ImageSpriteHeight }, { srcImageRect.right, srcImageRect.top , srcImageRect.left, srcImageRect.bottom});
	TransparentBlt(memdc, dstRect.left, dstRect.top, dstRect.right - dstRect.left, dstRect.bottom - dstRect.top,
		memdc2, 0, 0, ImageSpriteWidth, ImageSpriteHeight, 0);

	SelectObject(memdc2, oldBit1);
	DeleteObject(hBit1);
	DeleteDC(memdc2);
}


void Actor::ProcessUpInput(WPARAM wParam)
{
	switch (wParam)
	{
	case 'w':
	case 'W':
	case VK_UP:
	{
		KeyInputInfo = KeyInputInfo & 0b0111;
		break;
	}
	case 'a':
	case 'A':
	case VK_LEFT:
	{
		KeyInputInfo = KeyInputInfo & 0b1101;
		break;
	}
	case 's':
	case 'S':
	case VK_DOWN:
	{
		KeyInputInfo = KeyInputInfo & 0b1011;
		break;
	}
	case 'd':
	case 'D':
	case VK_RIGHT:
	{
		KeyInputInfo = KeyInputInfo & 0b1110;
		break;
	}
	default:
		break;
	}
}

void Actor::ProcessDownInput(WPARAM wParam)
{
	switch (wParam)
	{
	case 'w':
	case 'W':
	case VK_UP:
	{
		KeyInputInfo = KeyInputInfo | 0b1000;
		Direction = ACTOR_DIRECTION::UP;
		break;
	}
	case 'a':
	case 'A':
	case VK_LEFT:
	{
		KeyInputInfo = KeyInputInfo | 0b0010;
		Direction = ACTOR_DIRECTION::LEFT;
		break;
	}
	case 's':
	case 'S':
	case VK_DOWN:
	{
		KeyInputInfo = KeyInputInfo | 0b0100;
		Direction = ACTOR_DIRECTION::DOWN;
		break;
	}
	case 'd':
	case 'D':
	case VK_RIGHT:
	{
		KeyInputInfo = KeyInputInfo | 0b0001;
		Direction = ACTOR_DIRECTION::RIGHT;
		break;
	}
	default:
		break;
	}
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

void Actor::ProcessMove(SC_MOVE_OBJECT_PACKET* SLIP)
{
	Position.X = SLIP->x;
	Position.Y = SLIP->y;
	Direction = ACTOR_DIRECTION::DOWN;
}

void Actor::ProcessMove(SC_8DIRECT_MOVE_OBJECT_PACKET* SDMOP)
{
	SetLocation({ SDMOP->x, SDMOP->y });
	SetDirection((ACTOR_DIRECTION)SDMOP->direction);
}