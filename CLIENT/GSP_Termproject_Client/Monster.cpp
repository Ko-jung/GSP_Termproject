#include "Monster.h"
#include "Define.h"
#include "GamePlayStatic.h"

CImage Monster::Img;
std::vector<std::vector<std::vector<std::pair<int, int>>>> Monster::Sprites;
int Monster::ImageSpriteWidth;
int Monster::ImageSpriteHeight;

Monster::Monster() :
	Position(100.f, 100.f),
	Direction(ACTOR_DIRECTION::LEFT),
	Frame(0.f),
	Size(1.f),
	State(MONSTER_STATE::IDLE)
{
	if (Img.IsNull())
		Img.Load(TEXT("Image/Monster/FireMonster.png"));

	ImageSpriteWidth	= 18;
	ImageSpriteHeight	= 17;

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
	Frame += elapsedTime * 6.f;
	if (Frame > Sprites[(int)State][(int)Direction].size())
		Frame = 0.f;
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
