#include "SectorMgr.h"

#include "ClientMgr.h"
#include "../Client.h"

void SectorMgr::Insert(Client* Target)
{
	Sector& SectorSet = Sectors[Target->Position.Y / SECTORSIZE][Target->Position.X / SECTORSIZE];

	SectorSet.SectorLock.lock();
	SectorSet.SectorClient.insert(Target);
	SectorSet.SectorLock.unlock();
}

void SectorMgr::UnsafeInsert(Client* Target)
{
	Sector& SectorSet = Sectors[Target->Position.Y / SECTORSIZE][Target->Position.X / SECTORSIZE];
	SectorSet.SectorClient.insert(Target);
}

void SectorMgr::Remove(Client* Target)
{
	Sector& SectorSet = Sectors[Target->Position.Y / SECTORSIZE][Target->Position.X / SECTORSIZE];

	SectorSet.SectorLock.lock();
	SectorSet.SectorClient.erase(Target);
	SectorSet.SectorLock.unlock();
}

void SectorMgr::MoveSector(Client* Target, int TargetPrevSectorX, int TargetPrevSectorY)
{
	if ((Target->Position.X / SECTORSIZE) != TargetPrevSectorX || (Target->Position.Y / SECTORSIZE) != TargetPrevSectorY)
	{
		Sector& PrevSectorSet = Sectors[TargetPrevSectorY][TargetPrevSectorX];
		Sector& CurrSectorSet = Sectors[Target->Position.Y / SECTORSIZE][Target->Position.X / SECTORSIZE];

		PrevSectorSet.SectorLock.lock();
		PrevSectorSet.SectorClient.erase(Target);
		PrevSectorSet.SectorLock.unlock();

		CurrSectorSet.SectorLock.lock();
		CurrSectorSet.SectorClient.insert(Target);
		CurrSectorSet.SectorLock.unlock();
	}
}

void SectorMgr::MakeViewList(std::unordered_set<Client*>& ViewList, Client* Center)
{
	for (int i = 0; i < 9; i++)
	{
		int Y = Center->Position.Y / SECTORSIZE + i / 3 - 1;
		int X = Center->Position.X / SECTORSIZE + i % 3 - 1;

		if (X < 0 || X >= W_WIDTH || Y < 0 || Y >= W_HEIGHT) continue;

		Sector* sector = &Sectors[Y][X];
		sector->SectorLock.lock();
		for (auto& pClient : sector->SectorClient)
		{
			if (pClient == Center) continue;
			if (ClientMgr::IsNPC(pClient)) continue;
			if (ClientMgr::CanSee(pClient, Center))
			{
				ViewList.insert(pClient);
			}
		}
		sector->SectorLock.unlock();
	}
}
