#include "SectorMgr.h"

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
	Sector& PrevSectorSet = Sectors[TargetPrevSectorY][TargetPrevSectorX];
	Sector& CurrSectorSet = Sectors[Target->Position.Y / SECTORSIZE][Target->Position.X / SECTORSIZE];

	PrevSectorSet.SectorLock.lock();
	PrevSectorSet.SectorClient.erase(Target);
	PrevSectorSet.SectorLock.unlock();

	CurrSectorSet.SectorLock.lock();
	CurrSectorSet.SectorClient.insert(Target);
	CurrSectorSet.SectorLock.unlock();
}
