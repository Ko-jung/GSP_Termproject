#include "ClientMgr.h"
#include "../Client.h"

#include "../../../Common/OverExpansion.h"

#include "MapMgr.h"

#include <atomic>

ClientMgr::ClientMgr() :
	ClientCount(0)
{
	for (auto& c : Clients)
	{
		c = nullptr;
	}
}

ClientMgr::~ClientMgr()
{
	for (auto& c : Clients)
	{
		delete c;
	}
}

void ClientMgr::RecvProcess(int id, int bytes, OverExpansion* exp)
{
	Clients[id]->RecvProcess(bytes, exp);
}

void ClientMgr::Disconnect(int id)
{
	closesocket(Clients[id]->Socket);
	std::lock_guard<std::mutex> ll(Clients[id]->StateMutex);
	Clients[id]->State = CLIENT_STATE::FREE;
}

Client* ClientMgr::GetEmptyClient(int& ClientNum)
{
	while (true)
	{
		for (int i = 0; i < Clients.size(); i++)
		{
			if (!Clients[i])
			{
				auto NewClient = new Client();

				//std::atomic<Client*>* AtomicClientPtr = Clients[i];
				//auto abc = reinterpret_cast<std::atomic<Client*>*>(&Clients[i]);
				Client* llNullptr = nullptr;
				bool succ = std::atomic_compare_exchange_strong(
					reinterpret_cast<std::atomic<Client*>*>(&Clients[i]),
					&llNullptr,
					NewClient
				);

				//long long llNullptr = reinterpret_cast<long long>(nullptr);
				//bool succ = std::atomic_compare_exchange_strong(
				//	reinterpret_cast<std::atomic_llong*>(&Clients[i]),
				//	&llNullptr,
				//	reinterpret_cast<long long>(NewClient)
				//);


				if (succ)
				{
					ClientNum = i;
					ClientCount++;
					return Clients[i];
				}
				else
				{
					delete NewClient;
					break;
				}
			}
		}
	}
	return nullptr;
}

void ClientMgr::MapCollisionCheck(int id)
{
	POSITION TargetPos = Clients[id]->Position;
	RECT TargetCollisionBox = Clients[id]->GetCollisionBox();
	MapMgr* Manager = MapMgr::Instance();

	if (Manager->GetMapInfo(TargetCollisionBox.left, TargetCollisionBox.top) == (WORD)MAP_INFO::WALLS_BLOCK)
	{
	}
	if (Manager->GetMapInfo(TargetCollisionBox.left, TargetCollisionBox.bottom) == (WORD)MAP_INFO::WALLS_BLOCK)
	{
	}
	if (Manager->GetMapInfo(TargetCollisionBox.right, TargetCollisionBox.top) == (WORD)MAP_INFO::WALLS_BLOCK)
	{
	}
	if (Manager->GetMapInfo(TargetCollisionBox.right, TargetCollisionBox.bottom) == (WORD)MAP_INFO::WALLS_BLOCK)
	{
	}
}

void ClientMgr::ProcessLogin(CS_LOGIN_PACKET* CLP, Client* c)
{
	strcpy_s(c->PlayerName, CLP->name);
	{
		std::lock_guard<std::mutex> ll{ c->StateMutex };
		c->Position.X = rand() % W_WIDTH;
		c->Position.Y = rand() % W_HEIGHT;
		c->State = CLIENT_STATE::INGAME;
	}
	c->SendLoginInfo();

	// ADD SECTOR

	// ==========
}

void ClientMgr::ProcessStressTestMove(CS_MOVE_PACKET* CMP, Client* c)
{
	c->StressTestMove(CMP->direction);
}

void ClientMgr::ProcessMove(CS_8DIRECT_MOVE_PACKET* CMP, Client* c)
{
	c->Move(CMP->bitDirection, CMP->direction);
}
