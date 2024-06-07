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

	Client* GetEmptyClient(int& ClientNum);
	int GetClientCount() { return ClientCount; }

private:
	std::array<Client*, MAX_USER> Clients;
	std::atomic<int> ClientCount;
};

