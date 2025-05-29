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

	std::shared_ptr<Client> GetEmptyClient(int& ClientNum);
	int GetClientCount() { return ClientCount; }

	void MapCollisionCheck(int id);

	void SendPosToOtherClientUseSector(std::shared_ptr<Client> c);
	void SendAddPlayerUseSector(std::shared_ptr<Client> c);
	void NPCRandomMove(std::shared_ptr<Client> NPC);
	void WakeUpNPC(int npc_id, int waker);
	
	static bool CanSee(const std::shared_ptr<Client> c1, const std::shared_ptr<Client> c2);
	static bool IsNPC(const std::shared_ptr<Client> Target);

	void ProcessClientDie(std::shared_ptr<Client> Target);
	void ProcessClientSpawn(int id);

	// From Process Packet
	void ProcessLogin(CS_LOGIN_PACKET* CLP, std::shared_ptr<Client> c);
	void ProcessStressTestMove(CS_MOVE_PACKET* CMP, std::shared_ptr<Client> c);
	void ProcessMove(CS_8DIRECT_MOVE_PACKET* CMP, std::shared_ptr<Client> c);
	void ProcessNPCMove(int id, OverExpansion* exp);
	void ProcessAttack(CS_ATTACK_PACKET* CAP, std::shared_ptr<Client> c);
	void ProcessStateChange(CS_STATE_CHANGE_PACKET* CSCP, std::shared_ptr<Client> c);
	void ProcessChat(CS_CHAT_PACKET* CCP, std::shared_ptr<Client> c);

private:
	std::array<std::shared_ptr<Client>, MAX_USER + MAX_NPC> Clients;
	std::atomic<int> ClientCount;
};

