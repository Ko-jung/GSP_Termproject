#include "stdafx.h"

#include "Client.h"

#include "../../Common/protocol.h"
#include "../../Common/EnumDef.h"
#include "LogUtil.h"

#include "Manager/PacketMgr.h"
#include "Manager/MapMgr.h"
#include "Manager/SectorMgr.h"

int Client::ImageSpriteWidth;
int Client::ImageSpriteHeight;

Client::Client() :
	IsActive(false),
	ClientNum(-1),
	RemainDataLen(0),
	Speed(0.7f),
	Size(1.f),
	Experience(0.f),
	State(CLIENT_STATE::FREE)
{
	ImageSpriteWidth = 16;
	ImageSpriteHeight = 32;

	MaxHP = CurrentHP = 100;
}

#include <cassert>
Client::~Client()
{
	assert(false);
	std::cerr << "~Client() Called" << std::endl;
}

void Client::Init()
{
	ClientNum = -1;
	closesocket(Socket);
	RemainDataLen = 0;
	PlayerName[0] = '\0';
	Position = { 0.f, 0.f };
	Speed = 0;
	Size = 0;
	LastMoveTime = 0;

	Direction = ACTOR_DIRECTION::DOWN;

	std::lock_guard<std::mutex> ll(StateMutex);
	State = CLIENT_STATE::FREE;
}

void Client::Send(PACKET* p)
{
	//if (Socket == INVALID_SOCKET)
	//{
	//	std::cout << "ClientInfo Socket is INVALID_SOCKET" << std::endl;
	//	return;
	//}

	OverExpansion* exp = new OverExpansion{ (char*)p };

	//TODO: ÀÌ°Å THread safe?
	int ret = WSASend(Socket, &exp->_wsabuf, 1, 0, 0, &exp->_over, 0);
	//assert(memcmp(&exp->_over, &tmp, sizeof(WSAOVERLAPPED)) != 0);
	if (0 != ret)
	{
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
		{
			//LogUtil::PrintLog("Client::Send() Error: ClientNum: {}, ", ClientNum);
			LogUtil::error_display(error_num);
		}
	}
}

void Client::Recv()
{
	DWORD recv_flag = 0;

	ZeroMemory(&Exp._over, sizeof(Exp._over));
	Exp._wsabuf.buf = reinterpret_cast<char*>(Exp._send_buf + RemainDataLen);
	Exp._wsabuf.len = sizeof(Exp._send_buf) - RemainDataLen;

	int ret = WSARecv(Socket, &Exp._wsabuf, 1, 0, &recv_flag, &Exp._over, NULL);
	if (SOCKET_ERROR == ret)
	{
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
		{
			std::cout << "[Recv Error] ClientNum: " << ClientNum << "  Error Num: " << error_num << std::endl;
		}
	}
}

void Client::RecvProcess(int byte, OverExpansion* exp)
{
	assert(ClientNum >= 0 && ClientNum < MAX_USER);

	int RemainData = byte + RemainDataLen;
	char* Buf = exp->_send_buf;

	while (RemainData > 0)
	{
		PACKET* packet = reinterpret_cast<PACKET*>(Buf);
		if (RemainData >= packet->size)
		{
			PacketMgr::Instance()->ProcessPacket(packet, shared_from_this());
			Buf += packet->size;
			RemainData -= packet->size;
		}
		else
			break;
	}
	RemainDataLen = RemainData;
	if (RemainData > 0)
		memmove(exp->_send_buf, Buf, RemainData);
	Recv();
}

void Client::StressTestMove(const CS_MOVE_PACKET* const CMP)
{
	LastMoveTime = CMP->move_time;
	char Direction = CMP->direction;
	switch (Direction)
	{
	case 0: if (Position.Y > 0)				Position.Y--; break;
	case 1: if (Position.Y < W_HEIGHT - 1)	Position.Y++; break;
	case 2: if (Position.X > 0)				Position.X--; break;
	case 3: if (Position.X < W_WIDTH - 1)	Position.X++; break;
	}
	SendStressTestMovePos();
}

void Client::Move(POSITION NewPos, char direction)
{
	MapMgr* Instance = MapMgr::Instance();
	bool IsRollBacked = false;

	int PrevSectorXPos = (int)Position.X / SECTORSIZE;
	int PrevSectorYPos = (int)Position.Y / SECTORSIZE;
	int CurrSectorXPos = (int)NewPos.X / SECTORSIZE;
	int CurrSectorYPos = (int)NewPos.Y / SECTORSIZE;

	this->Direction = (ACTOR_DIRECTION)direction;

	float NewPosRight = NewPos.X + (float)ImageSpriteWidth / BOARDSIZE * Size;
	float NewPosBottom = NewPos.Y + (float)ImageSpriteHeight / BOARDSIZE * Size;

	if ((Instance->GetMapInfo((int)NewPosRight, (int)NewPos.Y) == (WORD)MAP_INFO::WALLS_BLOCK) ||
		(Instance->GetMapInfo((int)NewPos.X, (int)NewPos.Y) == (WORD)MAP_INFO::WALLS_BLOCK))
	{
		NewPos.X = Position.X;
		IsRollBacked = true;
	}

	if ((Instance->GetMapInfo((int)NewPos.X, (int)NewPosBottom) == (WORD)MAP_INFO::WALLS_BLOCK) ||
		(Instance->GetMapInfo((int)NewPos.X, (int)NewPos.Y) == (WORD)MAP_INFO::WALLS_BLOCK))
	{
		NewPos.Y = Position.Y;
		IsRollBacked = true;
	}

	Position = NewPos;

	// Client Position Reset
	if (IsRollBacked)
		SendMovePos(shared_from_this());

	if (CurrSectorXPos != PrevSectorXPos || CurrSectorYPos != PrevSectorYPos)
	{
		//SectorMgr::Instance()->MoveSector(shared_from_this(), PrevSectorXPos, PrevSectorYPos);
	}
}

bool Client::ApplyDamage(std::shared_ptr<Client> Attacker, const int Damage)
{
	CurrentHP.fetch_add(-Damage);

	// float NowHp = CurrentHP - Damage;
	// CurrentHP.compare_exchange_strong(NowHp, CurrentHP - Damage);

	if (CurrentHP <= 0)
	{
		CurrentHP = 0;
		return true;
	}
	return false;
}

RECT Client::GetCollisionBox()
{
	RECT ReturnRect;
	ReturnRect.left = (LONG)Position.X;
	ReturnRect.top = (LONG)Position.Y;
	ReturnRect.right = (LONG)Position.X + (LONG)Size * ImageSpriteWidth;
	ReturnRect.bottom = (LONG)Position.Y + (LONG)Size * ImageSpriteHeight;

	return ReturnRect;
}

RectF Client::GetCollisionFBox()
{
	return RectF(Position.X, Position.Y, Position.X + 1.f, Position.Y + 1.f);
}

void Client::SendLoginInfo()
{
	std::pair<WORD, WORD> Pos = MapMgr::Instance()->GetRandomCanSpawnPos();
	SC_LOGIN_INFO_PACKET SLIP;
	SLIP.id = ClientNum;
	SLIP.x = Pos.first;
	SLIP.y = Pos.second;
	Position.X = Pos.first;
	Position.Y = Pos.second;
	SLIP.visual = 0;
	SLIP.max_hp = SLIP.hp = 100;
	SLIP.exp = 0;
	SLIP.level = 1;
	Send(&SLIP);
}

void Client::SendLoginInfo(SC_LOGIN_INFO_PACKET* SLIP)
{
	Position.X = SLIP->x;
	Position.Y = SLIP->y;
	SLIP->id = ClientNum;
	Send(SLIP);
}

void Client::SendStressTestMovePos()
{
	SC_MOVE_OBJECT_PACKET SMOP;
	SMOP.id = ClientNum;
	SMOP.x = (short)(LONG)Position.X;
	SMOP.y = (short)(LONG)Position.Y;
	SMOP.move_time = LastMoveTime;
	Send(&SMOP);
}

void Client::SendMovePos(std::shared_ptr<Client> c)
{
	// Send Self Pos or Other Pos
	SC_8DIRECT_MOVE_OBJECT_PACKET SDMOP;
	SDMOP.id = c->ClientNum;
	SDMOP.x = c->Position.X;
	SDMOP.y = c->Position.Y;
	SDMOP.direction = (char)c->Direction;
	SDMOP.move_time = c->LastMoveTime;
	Send(&SDMOP);
}

void Client::SendAddPlayer(std::shared_ptr<Client> c)
{
	SC_ADD_OBJECT_PACKET SAOP;
	SAOP.id = c->ClientNum;
	strcpy_s(SAOP.name, NAME_SIZE, c->PlayerName);
	SAOP.size = sizeof(SAOP);
	SAOP.type = SC_ADD_OBJECT;
	SAOP.x = (short)c->Position.X;
	SAOP.y = (short)c->Position.Y;
	SAOP.hp = c->CurrentHP;
	SAOP.max_hp = c->MaxHP;

	//ViewListLock.lock();
	//ViewList.insert(c);
	//ViewListLock.unlock();

	Send(&SAOP);
}

void Client::SendRemovePlayer(std::shared_ptr<Client> c)
{
	//ViewListLock.lock();
	//if (ViewList.count(c))
	//	ViewList.erase(c);
	//else {
	//	ViewListLock.unlock();
	//	return;
	//}
	//ViewListLock.unlock();

	SC_REMOVE_OBJECT_PACKET SROP;
	SROP.id = c->ClientNum;
	Send(&SROP);
}

void Client::SendStatChange(std::shared_ptr<Client> c)
{
	SC_STAT_CHANGE_PACKET SSCP;
	SSCP.id = c->ClientNum;
	SSCP.hp = c->CurrentHP;
	SSCP.max_hp = c->MaxHP;
	SSCP.exp = c->Experience;
	SSCP.level = c->Level;

	Send(&SSCP);
}
