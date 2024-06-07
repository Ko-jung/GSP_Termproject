#pragma once

#include "../stdafx.h"

#include "../../../Common/EnumDef.h"
#include "../../../Common/protocol.h"
#include "../Define.h"

#define MEMCPYBUFTOPACKET(packet) memcpy(&packet, p, sizeof(packet))


class PacketMgr
{
	SINGLETON(PacketMgr)

public:
	PacketMgr() {}
	~PacketMgr() {}

public:
	void ProcessPacket(PACKET* p, class ClientInfo* c);

private:
};

