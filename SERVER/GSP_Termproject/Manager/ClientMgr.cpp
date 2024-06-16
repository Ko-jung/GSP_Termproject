#include "ClientMgr.h"
#include "../Client.h"

#include "../../../Common/OverExpansion.h"

#include "MapMgr.h"
#include "SectorMgr.h"

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

void ClientMgr::InitNPC()
{
	std::cout << "InitNPC begin.\n";
	for (int i = MAX_USER; i < Clients.size(); i++)
	{
		Clients[i] = new Client();
		Clients[i]->ClientNum = i;
		sprintf_s(Clients[i]->PlayerName, "NPC%d", i);
		Clients[i]->Position = { (float)(rand() % 200), (float)(rand() % 200) };
		Clients[i]->Direction = ACTOR_DIRECTION::DOWN;
		Clients[i]->State = CLIENT_STATE::INGAME;

		SectorMgr::Instance()->UnsafeInsert(Clients[i]);
	}
	std::cout << "InitNPC end.\n";
}

void ClientMgr::RecvProcess(int id, int bytes, OverExpansion* exp)
{
	Clients[id]->RecvProcess(bytes, exp);
}

void ClientMgr::Disconnect(int id)
{
	Clients[id]->Init();
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

void ClientMgr::SendPosToOtherClientUseSector(Client* c)
{
	int	CurrSectorXPos = c->Position.X / SECTORSIZE;
	int CurrSectorYPos = c->Position.Y / SECTORSIZE;
	std::unordered_set<Client*> new_vl;
	// 0, 0 -> 2, 2 까지 9섹터 검색 8,9
	for (int i = 0; i < 9; i++)
	{
		int Y = CurrSectorYPos + i / 3 - 1;
		int X = CurrSectorXPos + i % 3 - 1;

		if (X < 0 || X >= W_WIDTH || Y < 0 || Y >= W_HEIGHT) continue;

		SectorMgr* SectorInstance = SectorMgr::Instance();
		Sector* sector = SectorMgr::Instance()->GetSector(X, Y);
		sector->SectorLock.lock();
		for (auto& cl : sector->SectorClient)
		{
			if (cl->State != CLIENT_STATE::INGAME) continue;
			if (cl->ClientNum == c->ClientNum) continue;
			if (CanSee(cl, c))
			{
				new_vl.insert(cl);
			}
		}
		sector->SectorLock.unlock();
	}

	c->ViewListLock.lock();
	std::unordered_set<Client*> OldTargetViewList = c->ViewList;
	c->ViewListLock.unlock();

	//c->send_move_packet(c_id);

	for (auto& pClient : new_vl)
	{
		if (!IsNPC(pClient))
		{
			pClient->ViewListLock.lock();
			if (pClient->ViewList.count(c)) {
				pClient->ViewListLock.unlock();
				pClient->SendMovePos(c);
			}
			else {
				pClient->ViewListLock.unlock();
				pClient->SendAddPlayer(c);
			}
		}
		//else WakeUpNPC(pl, c_id);

		if (OldTargetViewList.count(pClient) == 0)
			c->SendAddPlayer(pClient);
	}

	for (auto& pClient : OldTargetViewList)
		if (0 == new_vl.count(pClient)) {
			c->SendRemovePlayer(pClient);
			if (!IsNPC(pClient))
				pClient->SendRemovePlayer(c);
		}
}

void ClientMgr::SendAddPlayerUseSector(Client* c)
{
	int	CurrSectorXPos = c->Position.X / SECTORSIZE;
	int CurrSectorYPos = c->Position.Y / SECTORSIZE;
	for (int i = 0; i < 9; i++)
	{
		int Y = CurrSectorYPos + i / 3 - 1;
		int X = CurrSectorXPos + i % 3 - 1;

		if (X < 0 || X >= W_WIDTH || Y < 0 || Y >= W_HEIGHT) continue;

		Sector* sector = SectorMgr::Instance()->GetSector(X, Y);
		sector->SectorLock.lock();
		for (auto& pClient : sector->SectorClient)
		{
			{
				std::lock_guard<std::mutex> ll(pClient->StateMutex);
				if (CLIENT_STATE::INGAME != pClient->State) continue;
			}
			if (pClient->ClientNum == c->ClientNum) continue;
			if (false == CanSee(c, pClient))
				continue;
			if (IsNPC(pClient))
				pClient->SendAddPlayer(c);
			//else WakeUpNPC(pClient->ClientNum, c->ClientNum);
			c->SendAddPlayer(pClient);
		}
		sector->SectorLock.unlock();
	}
}

bool ClientMgr::CanSee(const Client* c1, const Client* c2)
{
	if (abs(c1->Position.X - c2->Position.X) > VIEW_RANGE) return false;
	return abs(c1->Position.Y - c2->Position.Y) <= VIEW_RANGE;
}

bool ClientMgr::IsNPC(const Client* Target)
{
	return Target->ClientNum >= MAX_USER;
}

void ClientMgr::ProcessLogin(CS_LOGIN_PACKET* CLP, Client* c)
{
	strcpy_s(c->PlayerName, CLP->name);
	{
		std::lock_guard<std::mutex> ll{ c->StateMutex };
		c->Position.X = 100.f;//rand() % W_WIDTH;
		c->Position.Y = 100.f;//rand() % W_HEIGHT;
		c->State = CLIENT_STATE::INGAME;
	}
	c->SendLoginInfo();

	// ADD SECTOR
	SectorMgr::Instance()->Insert(c);
	// ==========

	SendAddPlayerUseSector(c);
}

void ClientMgr::ProcessStressTestMove(CS_MOVE_PACKET* CMP, Client* c)
{
	c->StressTestMove(CMP->direction);
	SendPosToOtherClientUseSector(c);
}

void ClientMgr::ProcessMove(CS_8DIRECT_MOVE_PACKET* CMP, Client* c)
{
	c->Move(CMP->Position, CMP->direction);
	SendPosToOtherClientUseSector(c);
}
