#include "stdafx.h"

#include "Client.h"

#include "../../Common/protocol.h"
#include "../../Common/EnumDef.h"

#include "Manager/PacketMgr.h"
#include "Manager/MapMgr.h"
#include "Manager/SectorMgr.h"

int Client::ImageSpriteWidth;
int Client::ImageSpriteHeight;

Client::Client() :
	ClientNum(-1),
	RemainDataLen(0),
	Speed(0.7f),
	Size(1.f)
{
	ImageSpriteWidth = 16;
	ImageSpriteHeight = 33;
}

Client::~Client()
{
}

void Client::Init()
{
	ClientNum = -1;
	closesocket(Socket);
	RemainDataLen = 0;
	PlayerName[0] = '\0';
	Position = {0.f, 0.f};
	Speed = 0;
	Size = 0;
	LastMoveTime = 0;

	Direction = ACTOR_DIRECTION::DOWN;
	ViewListLock;
	ViewList;

	std::lock_guard<std::mutex> ll(StateMutex);
	State = CLIENT_STATE::FREE;
}

void Client::Send(PACKET* p)
{
	if (Socket == INVALID_SOCKET)
	{
		std::cout << "ClientInfo Socket is INVALID_SOCKET" << std::endl;
		return;
	}

	OverExpansion* exp = new OverExpansion{ (char*)p };

	int ret = WSASend(Socket, &exp->_wsabuf, 1, 0, 0, &exp->_over, 0);
	if (0 != ret)
	{
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
		{
			std::cout << "Client::Send() ERROR" << std::endl;
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
	int RemainData = byte + RemainDataLen;
	char* Buf = exp->_send_buf;

	while (RemainData > 0)
	{
		PACKET* packet = reinterpret_cast<PACKET*>(Buf);
		if (RemainData >= packet->size)
		{
			PacketMgr::Instance()->ProcessPacket(packet, this);
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

void Client::StressTestMove(char Direction)
{
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

	float NewX = NewPos.X;
	float NewY = NewPos.Y;

	int PrevSectorXPos = Position.X / SECTORSIZE;
	int PrevSectorYPos = Position.Y / SECTORSIZE;
	int CurrSectorXPos = NewPos.X / SECTORSIZE;
	int CurrSectorYPos = NewPos.Y / SECTORSIZE;

	this->Direction = (ACTOR_DIRECTION)direction;

	if ((Instance->GetMapInfo(NewPos.X + Size * ImageSpriteWidth, NewPos.Y) != (WORD)MAP_INFO::WALLS_BLOCK) ||
		(Instance->GetMapInfo(NewPos.X, NewPos.Y) == (WORD)MAP_INFO::WALLS_BLOCK))
	{
		Position.X = NewPos.X;
	}
	else IsRollBacked = true;

	if ((Instance->GetMapInfo(NewPos.X, NewPos.Y + Size * ImageSpriteHeight) != (WORD)MAP_INFO::WALLS_BLOCK) ||
		(Instance->GetMapInfo(NewPos.X, NewPos.Y) == (WORD)MAP_INFO::WALLS_BLOCK))
	{
		Position.Y = NewPos.Y;
	}
	else IsRollBacked = true;

	// Client Position Reset
	if(IsRollBacked) SendMovePos(this);

	if (CurrSectorXPos != PrevSectorXPos || CurrSectorYPos != PrevSectorYPos)
	{
		SectorMgr::Instance()->MoveSector(this, PrevSectorXPos, PrevSectorYPos);
	}
}

RECT Client::GetCollisionBox()
{
	RECT ReturnRect;
	ReturnRect.left = Position.X;
	ReturnRect.top = Position.Y;
	ReturnRect.right = Position.X + Size * ImageSpriteWidth;
	ReturnRect.bottom = Position.Y + Size * ImageSpriteHeight;

	return ReturnRect;
}

void Client::SendLoginInfo()
{
	SC_LOGIN_INFO_PACKET SLIP;
	SLIP.id = ClientNum;
	SLIP.x = Position.X = 100.f;
	SLIP.y = Position.Y = 100.f;
	SLIP.visual = 0;
	SLIP.max_hp = SLIP.hp = 100;
	SLIP.exp = 0;
	SLIP.level = 1;
	Send(&SLIP);
}

void Client::SendStressTestMovePos()
{
	SC_MOVE_OBJECT_PACKET SMOP;
	SMOP.id = ClientNum;
	SMOP.x = (short)Position.X;
	SMOP.y = (short)Position.Y;
	SMOP.move_time = LastMoveTime;
	Send(&SMOP);
}

void Client::SendMovePos(Client* c)
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

void Client::SendAddPlayer(Client* c)
{
	SC_ADD_OBJECT_PACKET SAOP;
	SAOP.id = c->ClientNum;
	strcpy_s(SAOP.name, c->PlayerName);
	SAOP.size = sizeof(SAOP);
	SAOP.type = SC_ADD_OBJECT;
	SAOP.x = c->Position.X;
	SAOP.y = c->Position.Y;

	ViewListLock.lock();
	ViewList.insert(c);
	ViewListLock.unlock();

	Send(&SAOP);
}

void Client::SendRemovePlayer(Client* c)
{
	ViewListLock.lock();
	if (ViewList.count(c))
		ViewList.erase(c);
	else {
		ViewListLock.unlock();
		return;
	}
	ViewListLock.unlock();

	SC_REMOVE_OBJECT_PACKET SROP;
	SROP.id = c->ClientNum;
	Send(&SROP);
}
