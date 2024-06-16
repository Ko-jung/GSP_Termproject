#pragma once

#include "../stdafx.h"
#include "../Define.h"
#include "../../../Common/protocol.h"
#include "atomic"

#include <array>

class Client;

class ClientMgr
{
	SINGLETON(ClientMgr);

public:
	ClientMgr();
	~ClientMgr();

	void RecvProcess(int id, int bytes, class OverExpansion* exp);
	void Disconnect(int id);

	Client* GetEmptyClient(int& ClientNum);
	int GetClientCount() { return ClientCount; }

	void MapCollisionCheck(int id);

	void SendPosToOtherClientUseSector(Client* c);
	void SendAddPlayerUseSector(Client* c);
	bool CanSee(const Client* c1, const Client* c2);

	// From Process Packet
	void ProcessLogin(CS_LOGIN_PACKET* CLP, Client* c);
	void ProcessStressTestMove(CS_MOVE_PACKET* CMP, Client* c);
	void ProcessMove(CS_8DIRECT_MOVE_PACKET* CMP, Client* c);

private:
	std::array<Client*, MAX_USER> Clients;
	std::atomic<int> ClientCount;
};

