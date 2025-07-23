#include "ClientMgr.h"
#include "../Client.h"

#include "../../../Common/OverExpansion.h"

#include "MapMgr.h"
#include "SectorMgr.h"
#include "TimerMgr.h"
#include "DBMgr.h"

#include "../CollisionChecker.h"

#include <cassert>

ClientMgr::ClientMgr() :
	ClientCount(0)
{

	for (auto& c : Clients)
	{
		c = std::make_shared<Client>();
	}
}

ClientMgr::~ClientMgr()
{
	//for (auto& c : Clients)
	//{
	//	delete c;
	//}
}

void ClientMgr::InitNPC()
{
	//std::cout << "InitClient begin.\n";
	//for (int i = 0; i < MAX_USER; i++)
	//{
	//	Clients[i] = std::make_shared<Client>();
	//	Clients[i]->State = CLIENT_STATE::FREE;
	//}
	//std::cout << "InitClient end.\n";

	std::cout << "InitNPC begin.\n";
	for (int i = MAX_USER; i < Clients.size(); i++)
	{
		auto Pos = MapMgr::Instance()->GetRandomCanSpawnPos();
		//Clients[i] = std::make_shared<Client>();
		Clients[i]->ClientNum = i;
		sprintf_s(Clients[i]->PlayerName, "NPC%d", i);
		Clients[i]->Position = { (float)Pos.first, (float)Pos.second };// { (float)(rand() % 200), (float)(rand() % 200) };
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

void ClientMgr::SendProcess(std::shared_ptr<Client> c, PACKET* packet)
{
	if (!IsValid(c)) return;

	c->Send(packet);
}

void ClientMgr::Disconnect(int id)
{
	SectorMgr::Instance()->Remove(Clients[id]);
	Clients[id]->Init();
}

bool ClientMgr::IsValid(std::shared_ptr<Client> c)
{
	if (!c ||
		c->Socket == INVALID_SOCKET ||
		IsNPC(c) || 
		c->ClientNum == -1)
	{
		return false;
	}

	return true;
}

std::shared_ptr<Client> ClientMgr::GetEmptyClient()
{
	for (int i = 0; i < MAX_USER; i++)
	{
		std::lock_guard<std::mutex> ll(Clients[i]->StateMutex);
		if (Clients[i]->State == CLIENT_STATE::FREE)
		{
			Clients[i]->ClientNum = i;
			Clients[i]->State = CLIENT_STATE::ALLOC;
			ClientCount++;
			return Clients[i];
		}

		//if (Clients[i]->State == CLIENT_STATE::FREE)
		//{
		//	Clients[i]->State = CLIENT_STATE::ALLOC;
		//	ClientNum = i;
		//	ClientCount++;
		//	return Clients[i];
		//}
	}
	return nullptr;
	//while (true)
	//{
	//	for (int i = 0; i < Clients.size(); i++)
	//	{
	//		if (!Clients[i])
	//		{
	//			auto NewClient = new Client();
	//
	//			//std::atomic<std::shared_ptr<Client>>* AtomicClientPtr = Clients[i];
	//			//auto abc = reinterpret_cast<std::atomic<std::shared_ptr<Client>>*>(&Clients[i]);
	//			std::shared_ptr<Client> llNullptr = nullptr;
	//			bool succ = std::atomic_compare_exchange_strong(
	//				reinterpret_cast<std::atomic<std::shared_ptr<Client>>*>(&Clients[i]),
	//				&llNullptr,
	//				NewClient
	//			);
	//
	//			//long long llNullptr = reinterpret_cast<long long>(nullptr);
	//			//bool succ = std::atomic_compare_exchange_strong(
	//			//	reinterpret_cast<std::atomic_llong*>(&Clients[i]),
	//			//	&llNullptr,
	//			//	reinterpret_cast<long long>(NewClient)
	//			//);
	//
	//
	//			if (succ)
	//			{
	//				ClientNum = i;
	//				ClientCount++;
	//				return Clients[i];
	//			}
	//			else
	//			{
	//				delete NewClient;
	//				break;
	//			}
	//		}
	//	}
	//}
	//return nullptr;
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

void ClientMgr::SendPosToOtherClient(std::shared_ptr<Client> c)
{
	for (const auto& ClientRfe : Clients)
	{
		if (!ClientRfe || !c) continue;
		if (ClientRfe->State != CLIENT_STATE::INGAME) continue;
		if (ClientRfe->ClientNum == c->ClientNum) continue;
		if (IsNPC(ClientRfe)) continue;

		ClientRfe->SendMovePos(c);
	}
}

void ClientMgr::SendAddPlayer(std::shared_ptr<Client> c)
{
	for (const auto& ClientRfe : Clients)
	{
		if (!ClientRfe || c) continue;
		if (ClientRfe->State != CLIENT_STATE::INGAME) continue;
		if (ClientRfe->ClientNum == c->ClientNum) continue;
		if (IsNPC(ClientRfe))
			WakeUpNPC(ClientRfe->ClientNum, c->ClientNum);

		ClientRfe->SendAddPlayer(c);
	}
}

void ClientMgr::SendPosToOtherClientUseSector(std::shared_ptr<Client> c)
{
	int	CurrSectorXPos = (int)c->Position.X / SECTORSIZE;
	int CurrSectorYPos = (int)c->Position.Y / SECTORSIZE;

	std::unordered_set<std::shared_ptr<Client>> new_vl;
	SectorMgr::Instance()->MakeViewList(new_vl, c, true);

	c->ViewListLock.lock();
	std::unordered_set<std::shared_ptr<Client>> OldTargetViewList = c->ViewList;
	c->ViewListLock.unlock();

	//c->send_move_packet(c_id);

	for (auto& pClient : new_vl)
	{
		if (pClient->ClientNum == -1) continue;

		if (!IsNPC(pClient))
		{
			pClient->ViewListLock.lock();
			if (pClient->ViewList.count(c))
			{
				pClient->ViewListLock.unlock();
				pClient->SendMovePos(c);
			}
			else {
				pClient->ViewListLock.unlock();
				pClient->SendAddPlayer(c);
			}
		}
		else WakeUpNPC(pClient->ClientNum, c->ClientNum);

		if (OldTargetViewList.count(pClient) == 0)
		{
			c->SendAddPlayer(pClient);
		}
	}

	for (auto& pClient : OldTargetViewList)
	{
		if (0 == new_vl.count(pClient))
		{
			c->SendRemovePlayer(pClient);
			if (!IsNPC(pClient))
				pClient->SendRemovePlayer(c);
		}
	}
}

void ClientMgr::SendAddPlayerUseSector(std::shared_ptr<Client> c)
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
			if (pClient->ClientNum == -1) continue;
			if (false == CanSee(c, pClient)) continue;
			if (!IsNPC(pClient))
				pClient->SendAddPlayer(c);
			else WakeUpNPC(pClient->ClientNum, c->ClientNum);
			c->SendAddPlayer(pClient);
		}
		sector->SectorLock.unlock();
	}
}

void ClientMgr::NPCRandomMove(std::shared_ptr<Client> NPC)
{
	// 1. make OldViewList
	std::unordered_set<std::shared_ptr<Client>> OldViewList;
	SectorMgr::Instance()->MakeViewList(OldViewList, NPC);

	// 2. Move Random NPC Pos
	int x = NPC->Position.X;
	int y = NPC->Position.Y;
	int PrevSectorXPos = x / SECTORSIZE;
	int PrevSectorYPos = y / SECTORSIZE;
	int MoveDirect = rand() % 4;
	switch (MoveDirect)
	{
	case 0: if (x < (W_WIDTH - 1)) x++; break;
	case 1: if (x > 0) x--; break;
	case 2: if (y < (W_HEIGHT - 1)) y++; break;
	case 3:if (y > 0) y--; break;
	}
	if ((MAP_INFO)MapMgr::Instance()->GetMapInfo(x, y) == MAP_INFO::WALLS_BLOCK)
	{
		switch (MoveDirect)
		{
		case 0: if (x < (W_WIDTH - 1)) x--; break;
		case 1: if (x > 0) x++; break;
		case 2: if (y < (W_HEIGHT - 1)) y--; break;
		case 3:if (y > 0) y++; break;
		}
	}
	int CurrSectorXPos = x / SECTORSIZE;
	int CurrSectorYPos = y / SECTORSIZE;

	NPC->Position.X = x;
	NPC->Position.Y = y;

	// // moved
	// if (x != NPC->Position.X || x != NPC->Position.Y)
	// {
	// 	NPC->Position.X = x;
	// 	NPC->Position.Y = y;
	// 	// no data race
	// 	if (NPC->_move_count > 0)
	// 	{
	// 		if (--NPC->_move_count == 0)
	// 		{
	// 			NPC->_ll.lock();
	// 			lua_getglobal(NPC->_L, "event_say_bye");
	// 			lua_pushnumber(NPC->_L, NPC->_target_obj);
	// 			lua_pcall(NPC->_L, 1, 0, 0);
	// 			NPC->_ll.unlock();
	// 		}
	// 
	// 	}
	// }

   // 2-2. Checking SECTOR
	SectorMgr::Instance()->MoveSector(NPC, PrevSectorXPos, PrevSectorYPos);

	// 3. Make NewViewList
	std::unordered_set<std::shared_ptr<Client>> NewViewList;
	SectorMgr::Instance()->MakeViewList(NewViewList, NPC);

	// 4. Sending
	for (auto pClient : NewViewList)
	{
		if (0 == OldViewList.count(pClient))
		{
			pClient->SendAddPlayer(NPC);
		}
		else
		{
			pClient->SendMovePos(NPC);
		}
	}

	for (auto pClient : OldViewList)
	{
		if (0 == NewViewList.count(pClient))
		{
			pClient->ViewListLock.lock();
			if (0 != pClient->ViewList.count(NPC))
			{
				pClient->ViewListLock.unlock();
				pClient->SendRemovePlayer(NPC);
			}
			else {
				pClient->ViewListLock.unlock();
			}
		}
	}
}

void ClientMgr::WakeUpNPC(int NpcID, int WakerID)
{
	//OverExpansion* exover = new OverExpansion;
	//exover->_comp_type = COMP_TYPE::OP_AI_HELLO;
	//exover->_ai_target_obj = waker;

	//if (clients[npc_id].x == clients[waker].x && clients[npc_id].y == clients[waker].y)
	//	PostQueuedCompletionStatus(h_iocp, 1, npc_id, &exover->_over);
	std::shared_ptr<Client> NPC = Clients[NpcID];
	if (NPC->IsActive) return;
	bool old_state = false;
	if (false == std::atomic_compare_exchange_strong(&NPC->IsActive, &old_state, true))
		return;
	//TimerEvent evnt = { NpcID, std::chrono::system_clock::now(), EVENT_TYPE::EV_RANDOM_MOVE, 0 };

	std::shared_ptr<TimerEvent> evnt = std::make_shared<TimerEvent>(NpcID, std::chrono::system_clock::now(), EVENT_TYPE::EV_RANDOM_MOVE, 0);
	TimerMgr::Instance()->Insert(evnt);
}

bool ClientMgr::CanSee(const std::shared_ptr<Client> c1, const std::shared_ptr<Client> c2)
{
	if (abs(c1->Position.X - c2->Position.X) > VIEW_RANGE) return false;
	return abs(c1->Position.Y - c2->Position.Y) <= VIEW_RANGE;
}

// Is not Player !(0 ~ MAX_USER - 1)
bool ClientMgr::IsNPC(const std::shared_ptr<Client> Target)
{
	return !(Target->ClientNum >= 0 && Target->ClientNum < MAX_USER);
}

void ClientMgr::ProcessClientDie(std::shared_ptr<Client> Target)
{
	SectorMgr::Instance()->Remove(Target);
	if (IsNPC(Target))
	{
		std::lock_guard<std::mutex> ll{ Target->StateMutex };
		Target->State = CLIENT_STATE::FREE;
	}
	else
	{
		//TimerEvent evnt = TimerEvent{ Target->ClientNum, std::chrono::system_clock::now() + std::chrono::seconds(5), EVENT_TYPE::EV_SPAWN_PLAYER, 0 };

		std::shared_ptr<TimerEvent> evnt = std::make_shared<TimerEvent>(Target->ClientNum,
			std::chrono::system_clock::now() + std::chrono::seconds(5), EVENT_TYPE::EV_SPAWN_PLAYER, 0);
		TimerMgr::Instance()->Insert(evnt);
	}
}

void ClientMgr::ProcessClientSpawn(int id)
{
	std::shared_ptr<Client> Target = Clients[id];
	std::pair<WORD, WORD> SpawnPos = MapMgr::Instance()->GetRandomCanSpawnPos();

	SC_LOGIN_INFO_PACKET SLIP;
	SLIP.id = Target->ClientNum;
	SLIP.x = SpawnPos.first;
	SLIP.y = SpawnPos.second;
	SLIP.visual = 0;
	SLIP.max_hp = SLIP.hp = 100;
	SLIP.exp = Target->Experience;
	SLIP.level = Target->Level;
	SendProcess(Target, &SLIP);

	Target->Position.X = SpawnPos.first;
	Target->Position.Y = SpawnPos.second;
	Target->CurrentHP = 100;
	Target->MaxHP = 100;
	SectorMgr::Instance()->Insert(Target);

	SC_ADD_OBJECT_PACKET SAOP;
	SAOP.hp = Target->CurrentHP;
	SAOP.max_hp = Target->MaxHP;
	SAOP.id = Target->ClientNum;
	//SAOP.visual = Target->;
	SAOP.x = Target->Position.X;
	SAOP.y = Target->Position.Y;
	strcpy_s(SAOP.name, Target->PlayerName);

	//for (const auto& pClient : Clients)
	//{
	//	SendProcess(Target, &SAOP);
	//}

	std::unordered_set<std::shared_ptr<Client>> ViewList;
	SectorMgr::Instance()->MakeViewList(ViewList, Target);
	for (const auto& pClient : ViewList)
	{
		SendProcess(pClient, &SAOP);
	}
}

void ClientMgr::ProcessLogin(const CS_LOGIN_PACKET* CLP, std::shared_ptr<Client> c)
{
	WCHAR query[100];
	SC_LOGIN_INFO_PACKET SLIP;

	bool Succ = DBMgr::Instance()->ExecLogin(L"SELECT ID, X, Y, Visual, Level, Hp, MaxHp, Exp FROM [GSP_Termproject].[dbo].[GSP_Termproject_Player]",
		CLP->name, SLIP);

	assert(IsValid(c));
	assert(CLP->type == CS_LOGIN);

	strcpy_s(c->PlayerName, sizeof(c->PlayerName), CLP->name);
	{
		std::lock_guard<std::mutex> ll{ c->StateMutex };
		c->State = CLIENT_STATE::INGAME;
	}
	if (Succ)
		c->SendLoginInfo(&SLIP);
	else
		c->SendLoginInfo();


	// ADD SECTOR
	SectorMgr::Instance()->Insert(c);
	// ==========

	SendAddPlayerUseSector(c);
}

void ClientMgr::ProcessStressTestMove(CS_MOVE_PACKET* CMP, std::shared_ptr<Client> c)
{
	c->StressTestMove(CMP);
	//SendPosToOtherClient(c);
	SendPosToOtherClientUseSector(c);
}

void ClientMgr::ProcessMove(CS_8DIRECT_MOVE_PACKET* CMP, std::shared_ptr<Client> c)
{
	c->Move(CMP->Position, CMP->direction);
	//SendPosToOtherClient(c);
	SendPosToOtherClientUseSector(c);
}

void ClientMgr::ProcessNPCMove(int id, OverExpansion* exp)
{
	std::shared_ptr<Client> NPC = Clients[id];
	bool KeepAlive = false;

	for (int i = 0; i < 9; i++)
	{
		int Y = (NPC->Position.Y / SECTORSIZE) + i / 3 - 1;
		int X = (NPC->Position.X / SECTORSIZE) + i % 3 - 1;

		if (X < 0 || X >= W_WIDTH || Y < 0 || Y >= W_HEIGHT) continue;

		Sector* sector = SectorMgr::Instance()->GetSector(X, Y);
		std::lock_guard<std::mutex> sl(sector->SectorLock);
		for (auto& pClient : sector->SectorClient)
		{
			assert(pClient != nullptr);

			std::lock_guard<std::mutex> ll(pClient->StateMutex);
			if (pClient->State == CLIENT_STATE::INGAME && CanSee(NPC, pClient))
			{
				KeepAlive = true;
				i = 9;
				break;
			}
		}
	}

	if (KeepAlive)
	{
		NPC->IsActive = true;

		NPCRandomMove(NPC);

		std::shared_ptr<TimerEvent> evnt = std::make_shared<TimerEvent>(id,
			std::chrono::system_clock::now() + std::chrono::seconds(1), EVENT_TYPE::EV_RANDOM_MOVE, 0);
		TimerMgr::Instance()->Insert(evnt);
	}
	else
	{
		NPC->IsActive = false;
	}
}

void ClientMgr::ProcessAttack(CS_ATTACK_PACKET* CAP, std::shared_ptr<Client> c)
{
	POSITION ClientPos = c->Position;
	ACTOR_DIRECTION ClientDirection = c->Direction;

	// TODO: Weapon Variable
	float WeaponDamage = 10.f;

	// Create Collision Box
	RectF AttackCollisionBox;
	if (CAP->WeaponType == (BYTE)WEAPON_TYPE::SWORD)
	{
		WeaponDamage = 40.f;
		switch (ClientDirection)
		{
		case ACTOR_DIRECTION::DOWN:
			AttackCollisionBox = { ClientPos.X - 0.5f, ClientPos.Y + 1.f, ClientPos.X + 1.5f, ClientPos.Y + 2.f };
			break;
		case ACTOR_DIRECTION::RIGHT:
			AttackCollisionBox = { ClientPos.X + 1.f, ClientPos.Y - 0.5f, ClientPos.X + 1.f + 1.f, ClientPos.Y + 1.5f };
			break;
		case ACTOR_DIRECTION::UP:
			AttackCollisionBox = { ClientPos.X - 0.5f, ClientPos.Y - 1.f, ClientPos.X + 1.5f, ClientPos.Y };
			break;
		case ACTOR_DIRECTION::LEFT:
			AttackCollisionBox = { ClientPos.X - 1.f, ClientPos.Y - 0.5f, ClientPos.X, ClientPos.Y + 1.5f };
			break;
		default:
			break;
		}
	}
	else if (CAP->WeaponType == (BYTE)WEAPON_TYPE::PICKAXE)
	{
		WeaponDamage = 75.f;

	}
	else
	{
		return; // Invalid Weapon Type
	}

	std::unordered_set<std::shared_ptr<Client>> SectorClient;
	std::unordered_set<std::shared_ptr<Client>> CollideClient;
	SectorMgr::Instance()->MakeViewList(SectorClient, c, true);
	// Apply Damage
	for (auto& pClient : SectorClient)
	{
		RectF TargetCollisionBox = pClient->GetCollisionFBox();
		if (CollisionChecker::CollisionCheck(TargetCollisionBox, AttackCollisionBox))
		{
			if (pClient->CurrentHP <= 0) continue;

			bool IsDead = pClient->ApplyDamage(c, WeaponDamage);
			CollideClient.insert(pClient);
		}
	}

	// Send Result
	//SectorClient.insert(c);
	for (auto& pClient : SectorClient)
	{
		if (IsNPC(pClient)) continue;

		for (auto& pCollideClient : CollideClient)
		{
			pClient->SendStatChange(pCollideClient);
		}
	}

	for (auto& pCollideClient : CollideClient)
	{
		if (pCollideClient->CurrentHP <= 0) ProcessClientDie(pCollideClient);
	}
}

void ClientMgr::ProcessStateChange(CS_STATE_CHANGE_PACKET* CSCP, std::shared_ptr<Client> c)
{
	//SC_STATE_CHANGE_PACKET SSCP;
	//SSCP.ChangedState = CSCP->ChangedState;
	//SSCP.id = c->ClientNum;
	//for (const auto& pClient : Clients)
	//{
	//	SendProcess(pClient, &SSCP);
	//}

	std::unordered_set<std::shared_ptr<Client>> ViewList;
	SectorMgr::Instance()->MakeViewList(ViewList, c);

	SC_STATE_CHANGE_PACKET SSCP;
	SSCP.ChangedState = CSCP->ChangedState;
	SSCP.id = c->ClientNum;
	for (const auto& pClient : ViewList)
	{
		SendProcess(pClient, &SSCP);
	}
}

void ClientMgr::ProcessChat(CS_CHAT_PACKET* CCP, std::shared_ptr<Client> c)
{
	//SC_CHAT_PACKET SCP;
	//SCP.id = c->ClientNum;
	//strcpy_s(SCP.mess, CCP->mess);
	//for (const auto& pClient : Clients)
	//{
	//	SendProcess(pClient, &SCP);
	//}

	std::unordered_set<std::shared_ptr<Client>> ViewList;
	SectorMgr::Instance()->MakeViewList(ViewList, c);

	SC_CHAT_PACKET SCP;
	SCP.id = c->ClientNum;
	strcpy_s(SCP.mess, CCP->mess);
	ViewList.insert(c);
	for (const auto& pClient : ViewList)
	{
		SendProcess(pClient, &SCP);
	}
}
