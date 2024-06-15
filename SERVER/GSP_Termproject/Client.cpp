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

	ZeroMemory(&Exp, sizeof(Exp));
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

void Client::Move(char BitDirection, char Direction)
{
	MapMgr* Instance = MapMgr::Instance();
	bool IsRollBacked = false;


	float X = Position.X;
	float Y = Position.Y;
	int PrevSectorXPos = Position.X / SECTORSIZE;
	int PrevSectorYPos = Position.Y / SECTORSIZE;

	{	// Checking Move
		// Checking RIGHT
		if (BitDirection & 0b0001)
		{
			if (X < W_WIDTH - 1)
				X += Speed;

			if (Instance->GetMapInfo(X + Size * ImageSpriteWidth, Y) == (WORD)MAP_INFO::WALLS_BLOCK)
			{
				X -= Speed;
				IsRollBacked = true;
			}
		}
		// Checking LEFT
		if (BitDirection & 0b0010)
		{
			if (X > 0)
				X -= Speed;

			if (Instance->GetMapInfo(X, Y) == (WORD)MAP_INFO::WALLS_BLOCK)
			{
				X += Speed;
				IsRollBacked = true;
			}
		}
		// Checking DOWN
		if (BitDirection & 0b0100)
		{
			if (Y < W_HEIGHT - 1)
				Y += Speed;

			if (Instance->GetMapInfo(X , Y + Size * ImageSpriteHeight) == (WORD)MAP_INFO::WALLS_BLOCK)
			{
				Y -= Speed;
				IsRollBacked = true;
			}
		}
		// Checking UP
		if (BitDirection & 0b1000)
		{
			if (Y > 0)
				Y -= Speed;

			if (Instance->GetMapInfo(X, Y) == (WORD)MAP_INFO::WALLS_BLOCK)
			{
				Y += Speed;
				IsRollBacked = true;
			}
		}
	}
	this->Direction = (ACTOR_DIRECTION)Direction;

	int CurrSectorXPos = Position.X / SECTORSIZE;
	int CurrSectorYPos = Position.Y / SECTORSIZE;

	if (CurrSectorXPos != PrevSectorXPos || CurrSectorYPos != PrevSectorYPos)
	{
		SectorMgr::Instance()->MoveSector(this, PrevSectorXPos, PrevSectorYPos);
	}

	SendMovePos();
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

void Client::SendMovePos()
{
	SC_8DIRECT_MOVE_OBJECT_PACKET SDMOP;
	SDMOP.id = ClientNum;
	SDMOP.x = Position.X;
	SDMOP.y = Position.Y;
	SDMOP.direction = (char)Direction;
	SDMOP.move_time = LastMoveTime;
	Send(&SDMOP);
}
