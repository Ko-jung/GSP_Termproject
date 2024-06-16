#include "PacketMgr.h"
#include "ClientMgr.h"
//#include "TimerMgr.h"
//#include "RoomMgr.h"

#include "../Client.h"
//#include "../TimerEvent.h"

void PacketMgr::ProcessPacket(PACKET* packet, Client* c)
{
	switch (packet->type)
	{
	case CS_LOGIN:
	{
		ClientMgr::Instance()->ProcessLogin(reinterpret_cast<CS_LOGIN_PACKET*>(packet), c);
		break;
	}
	case CS_MOVE:
	{
		ClientMgr::Instance()->ProcessStressTestMove(reinterpret_cast<CS_MOVE_PACKET*>(packet), c);
		break;
	}
	case CS_8DIRECT_MOVE:
	{
		ClientMgr::Instance()->ProcessMove(reinterpret_cast<CS_8DIRECT_MOVE_PACKET*>(packet), c);
		break;
	}
	case CS_CHAT:
	{
		break;
	}
	case CS_ATTACK:
	{
		break;
	}
	case CS_TELEPORT:
	{
		break;
	}
	case CS_LOGOUT:
	{
		break;
	}
	default:
		break;
	}
}