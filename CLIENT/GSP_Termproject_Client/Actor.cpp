#include "framework.h"
#include "Define.h"
#include "Actor.h"

#include "GamePlayStatic.h"
#include "Manager/MapMgr.h"

CImage Actor::Img;
CImage Actor::AttackEffectImg;
std::vector<std::vector<std::vector<std::pair<int, int>>>> Actor::Sprites;
std::vector<std::pair<int, int>> Actor::AttackSprite;
std::vector<float> Actor::SprintFrameRate;
int Actor::ImageSpriteWidth;
int Actor::ImageSpriteHeight;

Actor::Actor(bool IsPossess) :
	IsPossessed(IsPossess),
	Position(100.f, 100.f),
	Speed(5.f),
	State(ACTOR_STATE::IDLE),
	Direction(ACTOR_DIRECTION::DOWN),
	KeyInputInfo(0),
	Frame(0.f),
	Size(1.f),
	IsCanDraw(true),
	IsCanRemove(false)
	//ForceX(0),
	//ForceY(0)
{
	if (Img.IsNull())
		Img.Load(TEXT("Image/Player/Actor.png"));

	ImageSpriteWidth = 16;
	ImageSpriteHeight = 32;

	LoadSprite();
}

void Actor::Update(float elapsedTime)
{
	if (!IsCanDraw) return;

	UpdateAnim(elapsedTime);
	Move(elapsedTime);

	if (ShowHpBarTimer > 0.f)
		ShowHpBarTimer -= elapsedTime;
}

void Actor::UpdateAnim(float elapsedTime)
{
	Frame += elapsedTime * SprintFrameRate[(int)State];
	if (Frame > Sprites[(int)State][(int)Direction].size() - FLT_MIN)
	{
		Frame = 0.f;
		switch (State)
		{
		case ACTOR_STATE::IDLE:
			break;
		case ACTOR_STATE::WALK:
			break;
		case ACTOR_STATE::RUN:
			break;
		case ACTOR_STATE::ATTACK:
			ChangeState(ACTOR_STATE::IDLE);
			IsCanMove = true;
			break;
		case ACTOR_STATE::DIE:
			IsCanDraw = false;
			IsCanRemove = true;
			break;
		default:
			break;
		}
	}
}

void Actor::Draw(HDC& memdc)
{
	if (!IsCanDraw) return;

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

		ImageDst = { WINWIDTH / 2 + DstPosX, WINHEIGHT / 2 - (BoardHeightSize)+DstPosY,
			BoardWidthSize + WINWIDTH / 2 + DstPosX, BoardHeightSize + WINHEIGHT / 2 + DstPosY };
	}

	if (Direction == ACTOR_DIRECTION::UP)
	{
		DrawEffect(memdc, ImageDst);
		DrawArm(memdc, ImageDst);
		DrawBody(memdc, ImageDst);
	}
	else
	{
		DrawBody(memdc, ImageDst);
		DrawArm(memdc, ImageDst);
		DrawEffect(memdc, ImageDst);
	}

	if (ShowHpBarTimer > 0.f)
	{
		HBRUSH RedBrush = CreateSolidBrush(RGB(255, 0, 0));
		//Rectangle(memdc, ImageDst.left, ImageDst.top - 10, ImageDst.right, ImageDst.top);
		HBRUSH WhiteBrush = CreateSolidBrush(RGB(255, 255, 255));

		RECT HpBackgroundRect = { ImageDst.left, ImageDst.top - 10, ImageDst.right, ImageDst.top };
		RECT HpRect = { ImageDst.left, ImageDst.top - 10,
			ImageDst.left + (ImageDst.right - ImageDst.left) * ((float)CurrentHp / MaxHp), ImageDst.top };

		FillRect(memdc, &HpBackgroundRect, WhiteBrush);
		FillRect(memdc, &HpRect, RedBrush);

		DeleteObject(RedBrush);
		DeleteObject(WhiteBrush);
	}
}

void Actor::InitUsePacket(SC_ADD_OBJECT_PACKET* SAOP)
{
	IsCanRemove = false;
	Position.X = SAOP->x;
	Position.Y = SAOP->y;
	MaxHp = SAOP->max_hp;
	CurrentHp = SAOP->hp;

	Name = SAOP->name;

	// Processing Visual

	// =================
}

void Actor::LoadSprite()
{
	if (not Sprites.empty()) return;

	SprintFrameRate = { 1.f, 3.f, 3.f, 20.f, 1.f };

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

		TempVector.clear();
		for (const auto& i : IndexList)
		{	// ATTACK
			std::vector< std::pair<int, int>> TempTempVector;
			for (int j = 0; j < 6; j++)
			{
				TempTempVector.emplace_back(std::make_pair<int, int>(ImageSpriteWidth * (j), ImageSpriteHeight * (4 + i)));
			}
			TempVector.push_back(TempTempVector);
		}
		Sprites.push_back(TempVector);

		TempVector.clear();
		for (const auto& i : IndexList)
		{	// DIE
			std::vector< std::pair<int, int>> TempTempVector;
			for (int j = 0; j < 2; j++)
			{
				TempTempVector.emplace_back(std::make_pair<int, int>(ImageSpriteWidth * (j + 4), ImageSpriteHeight * 21));
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

		TempVector.clear();
		for (const auto& i : IndexList)
		{	// ATTACK
			std::vector< std::pair<int, int>> TempTempVector;
			for (int j = 0; j < 6; j++)
			{
				TempTempVector.emplace_back(std::make_pair<int, int>(ImageSpriteWidth * (j + 12), ImageSpriteHeight * (4 + i)));
			}
			TempVector.push_back(TempTempVector);
		}
		Sprites.push_back(TempVector);

		TempVector.clear();
		for (const auto& i : IndexList)
		{	// DIE
			std::vector< std::pair<int, int>> TempTempVector;
			for (int j = 0; j < 2; j++)
			{
				TempTempVector.emplace_back(std::make_pair<int, int>(ImageSpriteWidth * (j + 10), ImageSpriteHeight * 21));
			}
			TempVector.push_back(TempTempVector);
		}
		Sprites.push_back(TempVector);
	}

	TempVector.clear();
	AttackEffectImg.Load(TEXT("Image/Player/AttackEffect.png"));
	for (int i = 0; i < 4; i++)
	{
		AttackSprite.emplace_back(std::make_pair(15 * i, 0));
	}
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
	// ATTACK etc ....
	if (!IsCanMove) return;
	if (!IsPossessed) return;

	POSITION NewPos = Position;
	MapMgr* MapInstance = MapMgr::Instance();

	int ForceX = 0;
	int ForceY = 0;
	if (KeyInputInfo & 0b0001)
	{
		ForceX++;
	}
	if (KeyInputInfo & 0b0010)
	{
		ForceX--;
	}
	if (KeyInputInfo & 0b0100)
	{
		ForceY++;
	}
	if (KeyInputInfo & 0b1000)
	{
		ForceY--;
	}

	// Checking RIGHT
	if (ForceX > 0)
	{
		Direction = ACTOR_DIRECTION::RIGHT;

		if (NewPos.X < W_WIDTH - 1)
			NewPos.X += Speed * elapsedTime;

		float NewPosRight = NewPos.X + (float)1 * Size;
		float NewPosBottom = NewPos.Y + (float)1 * Size;
		if ((MAP_INFO)MapInstance->GetMapInfo(NewPosRight, NewPos.Y) == MAP_INFO::WALLS_BLOCK ||
			(MAP_INFO)MapInstance->GetMapInfo(NewPosRight, NewPosBottom) == MAP_INFO::WALLS_BLOCK)
		{
			NewPos.X = Position.X;
		}
	}

	// Checking LEFT
	else if (ForceX < 0)
	{
		Direction = ACTOR_DIRECTION::LEFT;

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
	if (ForceY > 0)
	{
		Direction = ACTOR_DIRECTION::DOWN;

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
	else if (ForceY < 0)
	{
		Direction = ACTOR_DIRECTION::UP;

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

	Position = NewPos;

	if (ForceX == 0 && ForceY == 0) ChangeState(ACTOR_STATE::IDLE);
	else							ChangeState(ACTOR_STATE::WALK);
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

void Actor::ChangeState(ACTOR_STATE state)
{
	if (State == state) return;

	IsChangeState = true;
	State = state;
	Frame = 0;
}

void Actor::ChangeStateByPacket(ACTOR_STATE state)
{
	State = state;
	Frame = 0;

	//if (State == ACTOR_STATE::ATTACK) ;
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
		break;
	}
	case 'a':
	case 'A':
	case VK_LEFT:
	{
		KeyInputInfo = KeyInputInfo | 0b0010;
		break;
	}
	case 's':
	case 'S':
	case VK_DOWN:
	{
		KeyInputInfo = KeyInputInfo | 0b0100;
		break;
	}
	case 'd':
	case 'D':
	case VK_RIGHT:
	{
		KeyInputInfo = KeyInputInfo | 0b0001;
		break;
	}
	default:
		break;
	}
}

void Actor::ProcessAttack()
{
	ChangeState(ACTOR_STATE::ATTACK);
	IsCanMove = false;
}

void Actor::ProcessLogin(SC_LOGIN_INFO_PACKET* SLIP)
{
	IsCanDraw = true;
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

void Actor::ProcessChangeStat(SC_STAT_CHANGE_PACKET* SSCP)
{
	if (CurrentHp != SSCP->hp)
	{
		CurrentHp = SSCP->hp;
		ShowHpBarTimer = 3.f;

		if (CurrentHp <= 0)
			ChangeState(ACTOR_STATE::DIE);
	}

	if (MaxHp != SSCP->max_hp)
	{
		ShowHpBarTimer = 3.f;
		if (MaxHp > SSCP->max_hp)
		{
			MaxHp = SSCP->max_hp;
			if (CurrentHp > MaxHp) CurrentHp = MaxHp;
		}
		else
		{
			CurrentHp += (SSCP->max_hp - MaxHp);
		}
	}

	{
		Experience = SSCP->exp;
		Level = SSCP->level;
	}
}

void Actor::DrawBody(HDC& memdc, const RECT& ImageDst)
{
	// Draw Body
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

void Actor::DrawArm(HDC& memdc, const RECT& ImageDst)
{
	// Draw Arm
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

void Actor::DrawEffect(HDC& memdc, const RECT& ImageDst)
{
	int BoardWidthSize = WINWIDTH / BOARDSIZE;
	int BoardHeightSize = WINHEIGHT / BOARDSIZE;
	if (State == ACTOR_STATE::ATTACK && Frame > 3.f)
	{
		int SrcX = AttackSprite[(int)Direction].first;
		int SrcY = AttackSprite[(int)Direction].second;
		static RectF AttackBoxDiff[] = {
			RectF{-0.5f,  1.f,1.5f,2.f},
			RectF{  1.f, -.5f, 2.f,1.5f},
			RectF{ -.5f, -1.f,1.5f,0.f},
			RectF{ -1.f,-0.5f, 0.f,1.5f}
		};

		POSITION OwnActorPos = GamePlayStatic::GetOwnActorPosition();
		int DstPosX = (Position.X - OwnActorPos.X) * BoardWidthSize;
		int DstPosY = (Position.Y - OwnActorPos.Y) * BoardHeightSize;

		RECT a = { WINWIDTH / 2 + AttackBoxDiff[(int)Direction].left * BoardWidthSize	  + DstPosX,
					WINHEIGHT / 2 + AttackBoxDiff[(int)Direction].top * BoardWidthSize	  + DstPosY,
					WINWIDTH / 2 + AttackBoxDiff[(int)Direction].right * BoardWidthSize	  + DstPosX,
					WINHEIGHT / 2 + AttackBoxDiff[(int)Direction].bottom * BoardWidthSize + DstPosY };
		RECT b = { SrcX, SrcY, SrcX + 15, SrcY + 15 };
		AttackEffectImg.Draw(memdc, a, b);
	}
}
