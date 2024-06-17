#pragma once

#include "../stdafx.h"
#include "../Define.h"
#include "../../../Common/protocol.h"
#include "../../../Common/EnumDef.h"
#include "atomic"

#include <array>

class Client;

class ClientMgr
{
	SINGLETON(ClientMgr);

public:
	ClientMgr();
	~ClientMgr();

	void InitNPC();
	void RecvProcess(int id, int bytes, class OverExpansion* exp);
	void Disconnect(int id);

	Client* GetEmptyClient(int& ClientNum);
	int GetClientCount() { return ClientCount; }

	void MapCollisionCheck(int id);

	void SendPosToOtherClientUseSector(Client* c);
	void SendAddPlayerUseSector(Client* c);
	void NPCRandomMove(Client* NPC);
	void WakeUpNPC(int npc_id, int waker);
	
	static bool CanSee(const Client* c1, const Client* c2);
	static bool IsNPC(const Client* Target);

	void ProcessClientDie(Client* Target);

	// From Process Packet
	void ProcessLogin(CS_LOGIN_PACKET* CLP, Client* c);
	void ProcessStressTestMove(CS_MOVE_PACKET* CMP, Client* c);
	void ProcessMove(CS_8DIRECT_MOVE_PACKET* CMP, Client* c);
	void ProcessNPCMove(int id, OverExpansion* exp);
	void ProcessAttack(CS_ATTACK_PACKET* CAP, Client* c);

private:
	std::array<Client*, MAX_USER + MAX_NPC> Clients;
	std::atomic<int> ClientCount;
};

