#pragma once

#include "../Define.h"
#include "atomic"

#include <array>

class Client;

class ClientMgr
{
	SINGLETON(ClientMgr);

public:
	ClientMgr();
	~ClientMgr();

	Client* GetEmptyClient(int& ClientNum);

private:
	std::array<Client*, MAXPLAYER> Clients;
	std::atomic<int> ClientCount;
};

