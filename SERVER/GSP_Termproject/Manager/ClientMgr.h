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

	// From Process Packet
	void ProcessLogin(CS_LOGIN_PACKET* CLP, Client* c);
	void ProcessStressTestMove(CS_MOVE_PACKET* CMP, Client* c);
	void ProcessMove(CS_8DIRECT_MOVE_PACKET* CMP, Client* c);

private:
	std::array<Client*, MAX_USER> Clients;
	std::atomic<int> ClientCount;
};

