#include "framework.h"
#include "Define.h"
#include "Actor.h"

#include "GamePlayStatic.h"
#include "Manager/MapMgr.h"

CImage Actor::Img;
std::vector<std::vector<std::vector<std::pair<int, int>>>> Actor::Sprites;
int Actor::ImageSpriteWidth;
int Actor::ImageSpriteHeight;

Actor::Actor(bool IsPossess) :
	IsPossessed(IsPossess),
	Position(100.f, 100.f),
	Speed(5.f),
	State(ACTOR_STATE::WALK),
	Direction(ACTOR_DIRECTION::LEFT),
	KeyInputInfo(0),
	Frame(0.f),
	Size(1.f)
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
		Frame = 0.f;

	Move(elapsedTime);
}

void Actor::Draw(HDC& memdc)
{
	int BoardWidthSize = WINWIDTH / BOARDSIZE;
	int BoardHeightSize = WINHEIGHT / BOARDSIZE;

	float ImageRatio = (float)ImageSpriteHeight / ImageSpriteWidth;
	RECT ImageDst;
	if (IsPossessed)
	{
		//ImageDst = { WINWIDTH / 2 - BoardWidthSize, WINHEIGHT / 2 - (BoardHeightSize * 3),
		//	BoardWidthSize + WINWIDTH / 2, BoardHeightSize + WINHEIGHT / 2 };
		ImageDst = { WINWIDTH / 2, WINHEIGHT / 2 - (BoardHeightSize),
			BoardWidthSize + WINWIDTH / 2, BoardHeightSize + WINHEIGHT / 2 };
	}
	else
	{
		POSITION OwnActorPos = GamePlayStatic::GetOwnActorPosition();
		int DstPosX = (Position.X - OwnActorPos.X) * BoardWidthSize;
		int DstPosY = (Position.Y - OwnActorPos.Y) * BoardHeightSize;

		ImageDst = { WINWIDTH / 2 + DstPosX, WINHEIGHT / 2 - (BoardHeightSize) +DstPosY,
			BoardWidthSize + WINWIDTH / 2 + DstPosX, BoardHeightSize + WINHEIGHT / 2 + DstPosY };
	}

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

void Actor::InitUsePacket(SC_ADD_OBJECT_PACKET* SAOP)
{
	Position.X = SAOP->x;
	Position.Y = SAOP->y;

	Name = SAOP->name;

	// Processing Visual

	// =================
}

void Actor::LoadSprite()
{
	if (not Sprites.empty()) return;

	std::vector<int> IndexList{ 0,1,2,1 };

	std::vector< std::vector<std::pair<int, int>>> TempVector;

	{	// BODY
		TempVector.clear();
		for (const auto& i : IndexList)
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

RECT Actor::GetRectDstWithPos()
{
	float Size = 1.f;
	int BoardWidthSize = WINWIDTH / BOARDSIZE / 2;
	int BoardHeightSize = WINHEIGHT / BOARDSIZE / 2;

	int DstWidth = BoardWidthSize * Size;
	int DstHeight = BoardHeightSize * Size;

	POSITION OwnActorPos = GamePlayStatic::Instance()->GetOwnActorPosition();

	int DstPosX = (Position.X - OwnActorPos.X) * BoardWidthSize;
	int DstPosY = (Position.Y - OwnActorPos.Y) * BoardHeightSize;

	RECT ReturnRect = {};

	if (IsPossessed)
	{
		ReturnRect = { WINWIDTH / 2 - BoardWidthSize, WINHEIGHT / 2 - (BoardHeightSize * 3),
		BoardWidthSize + WINWIDTH / 2, BoardHeightSize + WINHEIGHT / 2 };
	}
	else
	{
		ReturnRect = { WINWIDTH / 2 - BoardWidthSize + DstPosX, WINHEIGHT / 2 - (BoardHeightSize * 3) + DstPosY,
		BoardWidthSize + WINWIDTH / 2 + DstPosX, BoardHeightSize + WINHEIGHT / 2 + DstPosY };
	}

	return ReturnRect;
}

void Actor::Move(float elapsedTime)
{
	POSITION NewPos = Position;
	MapMgr* MapInstance = MapMgr::Instance();
	std::array<std::array<WORD, 2000>, 2000>* abcd = MapInstance->GetMap();
	//float NewPosRight = NewPos.X + (float)ImageSpriteWidth / BOARDSIZE * Size;
	//float NewPosBottom = NewPos.Y + (float)ImageSpriteHeight/ BOARDSIZE * Size;

	// Checking RIGHT
	if (KeyInputInfo & 0b0001)
	{
		if (NewPos.X < W_WIDTH - 1)
			NewPos.X += Speed * elapsedTime;

		float NewPosRight = NewPos.X + (float)1 * Size;
		float NewPosBottom = NewPos.Y + (float)1 * Size;
		if ((MAP_INFO)MapInstance->GetMapInfo(NewPosRight, NewPos.Y)== MAP_INFO::WALLS_BLOCK || 
			(MAP_INFO)MapInstance->GetMapInfo(NewPosRight, NewPosBottom) == MAP_INFO::WALLS_BLOCK)
		{
			NewPos.X = Position.X;
		}
	}

	// Checking LEFT
	if (KeyInputInfo & 0b0010)
	{
		if (NewPos.X > 0)
			NewPos.X -= Speed * elapsedTime;

		float NewPosRight = NewPos.X + (float)1 * Size;
		float NewPosBottom = NewPos.Y + (float)1 * Size;
		if ((MAP_INFO)MapInstance->GetMapInfo(NewPos.X, NewPos.Y) == MAP_INFO::WALLS_BLOCK ||
			(MAP_INFO)MapInstance->GetMapInfo(NewPos.X, NewPosBottom) == MAP_INFO::WALLS_BLOCK)
		{
			NewPos.X = Position.X;
		}
	}

	// Checking DOWN
	if (KeyInputInfo & 0b0100)
	{
		if (NewPos.Y < W_HEIGHT - 1)
			NewPos.Y += Speed * elapsedTime;

		float NewPosRight = NewPos.X + (float)1 * Size;
		float NewPosBottom = NewPos.Y + (float)1 * Size;
		MAP_INFO aaa = (MAP_INFO)MapInstance->GetMapInfo(NewPosRight, NewPosBottom);
		MAP_INFO bbb = (MAP_INFO)MapInstance->GetMapInfo(NewPos.X, NewPosBottom);

		if ((MAP_INFO)MapInstance->GetMapInfo(NewPosRight, NewPosBottom) == MAP_INFO::WALLS_BLOCK ||
			(MAP_INFO)MapInstance->GetMapInfo(NewPos.X, NewPosBottom) == MAP_INFO::WALLS_BLOCK)
		{
			NewPos.Y = Position.Y;
		}
	}

	// Checking UP
	if (KeyInputInfo & 0b1000)
	{
		if (NewPos.Y > 0)
			NewPos.Y -= Speed * elapsedTime;

		float NewPosRight = NewPos.X + (float)1 * Size;
		float NewPosBottom = NewPos.Y + (float)1 * Size;
		if ((MAP_INFO)MapInstance->GetMapInfo(NewPos.X, NewPos.Y) == MAP_INFO::WALLS_BLOCK ||
			(MAP_INFO)MapInstance->GetMapInfo(NewPosRight, NewPos.Y) == MAP_INFO::WALLS_BLOCK)
		{
			NewPos.Y = Position.Y;
		}
	}

	// 3x3 범위로 충돌체크
	// Checking Collide Object is work on SERVER


	// for (int j = 0; j < 3; j++)
	// {
	// 	for (int i = 0; i < 3; i++)
	// 	{
	// 		int X = i + Position.X - 1;
	// 		int Y = j + Position.Y - 1;
	// 		if (X < 0 || X >= W_WIDTH || Y < 0 || Y >= W_HEIGHT) continue;
	// 
	// 		MAP_INFO MapInfo = (MAP_INFO)MapMgr::Instance()->GetMapInfo(X, Y);
	// 		if (MapInfo == MAP_INFO::WALLS_BLOCK)
	// 		{
	// 			RECT ActorCollisionBox = { Position.X , Position.Y, Position.X + ImageSpriteWidth, Position.Y + ImageSpriteHeight };
	// 			RECT TileCollisionBox = { X * BOARDSIZE, Y * BOARDSIZE, (X + 1) * BOARDSIZE, (Y + 1) * BOARDSIZE }
	// 
	// 			if (KeyInputInfo & 0b0001 || KeyInputInfo & 0b0010)
	// 			{
	// 				NewPos.X = Position.X;
	// 				std::cout << "Collide!" << std::endl;
	// 			}
	// 			if (KeyInputInfo & 0b0100 || KeyInputInfo & 0b1000)
	// 			{
	// 				NewPos.Y = Position.Y;
	// 				std::cout << "Collide!" << std::endl;
	// 			}
	// 		}
	// 	}
	// }

	Position = NewPos;
}

void Actor::InversionImage(HDC& memdc, RECT dstRect, RECT srcImageRect)
{
	HDC memdc2 = CreateCompatibleDC(memdc);
	HBITMAP hBit1 = CreateCompatibleBitmap(memdc, ImageSpriteWidth, ImageSpriteHeight);
	HBITMAP oldBit1 = (HBITMAP)SelectObject(memdc2, hBit1);

	FillRect(memdc2, &srcImageRect, (HBRUSH)(COLOR_WINDOW + 1));

	Img.StretchBlt(memdc2, { 0,0,ImageSpriteWidth, ImageSpriteHeight }, { srcImageRect.right, srcImageRect.top , srcImageRect.left, srcImageRect.bottom });
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