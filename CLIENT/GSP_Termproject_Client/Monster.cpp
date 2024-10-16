#include "Monster.h"
#include "Define.h"
#include "GamePlayStatic.h"

#include "Manager/GameMgr.h"

CImage Monster::Img;
std::vector<std::vector<std::vector<std::pair<int, int>>>> Monster::Sprites;
int Monster::ImageSpriteWidth;
int Monster::ImageSpriteHeight;

Monster::Monster() :
	Position(100.f, 100.f),
	Direction(ACTOR_DIRECTION::LEFT),
	Frame(0.f),
	Size(1.f),
	Experience(0),
	Level(0),
	State(MONSTER_STATE::IDLE),
	ShowHpBarTimer(3.f),
	IsCanRemove(false)
{
	if (Img.IsNull())
		Img.Load(TEXT("Image/Monster/FireMonster.png"));

	ImageSpriteWidth	= 18;
	ImageSpriteHeight	= 17;

	MaxHp = CurrentHp = 100;

	LoadSprite();
}

Monster::~Monster()
{
}

void Monster::InitUsePacket(SC_ADD_OBJECT_PACKET* SAOP)
{
	Position.X = SAOP->x;
	Position.Y = SAOP->y;
	MaxHp = SAOP->max_hp;
	CurrentHp = SAOP->hp;

	Name = SAOP->name;
}

void Monster::Update(float elapsedTime)
{
	UpdateAnim(elapsedTime);
	if (ShowHpBarTimer > 0.f)
		ShowHpBarTimer -= elapsedTime;
}

void Monster::UpdateAnim(float elapsedTime)
{
	Frame += elapsedTime * 6.f;
	if (Frame > Sprites[(int)State][(int)Direction].size())
	{
		Frame = 0.f;

		switch (State)
		{
		case MONSTER_STATE::IDLE:
			break;
		case MONSTER_STATE::DIE:
			IsCanRemove = true;
			break;
		case MONSTER_STATE::END:
			break;
		default:
			break;
		}
	}
}

void Monster::Draw(HDC& memdc)
{
	int BoardWidthSize = WINWIDTH / BOARDSIZE;
	int BoardHeightSize = WINHEIGHT / BOARDSIZE;

	POSITION OwnActorPos = GamePlayStatic::GetOwnActorPosition();
	int DstPosX = (Position.X - OwnActorPos.X) * BoardWidthSize;
	int DstPosY = (Position.Y - OwnActorPos.Y) * BoardHeightSize;

	RECT ImageDst = { WINWIDTH / 2 + DstPosX, WINHEIGHT / 2 + DstPosY,
			BoardWidthSize + WINWIDTH / 2 + DstPosX, BoardHeightSize + WINHEIGHT / 2 + DstPosY };

	int SrcX = Sprites[(int)State][(int)Direction][(int)Frame].first + 1;
	int SrcY = Sprites[(int)State][(int)Direction][(int)Frame].second + 1;
	RECT ImageSrc{ SrcX, SrcY, SrcX + ImageSpriteWidth, SrcY + ImageSpriteHeight };

	Img.Draw(memdc, ImageDst, ImageSrc);

	if (ShowHpBarTimer > 0.f)
	{
		HBRUSH RedBrush = CreateSolidBrush(RGB(255, 0, 0)); 
		//Rectangle(memdc, ImageDst.left, ImageDst.top - 10, ImageDst.right, ImageDst.top);
		HBRUSH WhiteBrush = CreateSolidBrush(RGB(255, 255, 255));

		RECT HpBackgroundRect = { ImageDst.left, ImageDst.top - 10, ImageDst.right, ImageDst.top };
		RECT HpRect = { ImageDst.left, ImageDst.top - 10,
			ImageDst.left + (ImageDst.right - ImageDst.left) * ((float)CurrentHp/MaxHp), ImageDst.top};

		FillRect(memdc, &HpBackgroundRect, WhiteBrush);
		FillRect(memdc, &HpRect, RedBrush);

		DeleteObject(RedBrush);
		DeleteObject(WhiteBrush);
	}
}

void Monster::LoadSprite()
{
	if (not Sprites.empty()) return;
	std::vector<int> IndexList{ 0,1,2,3 };

	std::vector< std::vector<std::pair<int, int>>> TempVector;
	TempVector.clear();
	for (int j = 0; j < 4; j++)
	{
		std::vector< std::pair<int, int>> TempTempVector;
		for (int i = 0; i < 7; i++)
		{	
			TempTempVector.emplace_back(std::make_pair<int, int>(0 + i * (ImageSpriteWidth + 2), j * (ImageSpriteHeight + 2)));
		}
		TempVector.push_back(TempTempVector);
	}
	Sprites.push_back(TempVector);

	TempVector.clear();
	std::vector< std::pair<int, int>> TempTempVector;
	for (int i = 0; i < 4; i++)
	{	
		TempTempVector.emplace_back(std::make_pair<int, int>(0 + i * (ImageSpriteWidth + 2), 4 * (ImageSpriteHeight + 2)));
	}
	TempVector.push_back(TempTempVector);
	Sprites.push_back(TempVector);
}

void Monster::ApplyDamage(int Damage)
{
	ShowHpBarTimer = 3.f;
	CurrentHp -= Damage;
}

void Monster::ChangeState(MONSTER_STATE state)
{
	if (State == state) return;

	State = state;
	Frame = 0;
}

void Monster::ProcessMove(SC_8DIRECT_MOVE_OBJECT_PACKET* SDMOP)
{
	Position = { SDMOP->x , SDMOP->y};
	Direction = (ACTOR_DIRECTION)SDMOP->direction;
}

void Monster::ProcessChangeStat(SC_STAT_CHANGE_PACKET* SSCP)
{
	if (CurrentHp != SSCP->hp)
	{
		CurrentHp = SSCP->hp;
		ShowHpBarTimer = 3.f;

		if (CurrentHp <= 0)
		{
			ChangeState(MONSTER_STATE::DIE);

			GameMgr::Instance()->OnSystemMessage("Monster [" + std::to_string(SSCP->id) + "] is DIED!");
		}
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
}
