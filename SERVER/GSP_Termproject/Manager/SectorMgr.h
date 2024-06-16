#pragma once

#include "../Define.h"
#include "../../../Common/protocol.h"

#include <unordered_set>
#include <mutex>
#include <array>

class Client;

struct Sector
{
	std::unordered_set<Client*> SectorClient;
	std::mutex SectorLock;
};

class SectorMgr
{
public:
	SINGLETON(SectorMgr);

	void Insert(Client* Target);
	void UnsafeInsert(Client* Target);
	void Remove(Client* Target);
	void MoveSector(Client* Target, int TargetPrevX, int TargetPrevY);

	Sector* GetSector(int x, int y) { return &Sectors[y][x]; }

private:
	std::array<std::array<Sector, W_WIDTH / SECTORSIZE + 1>, W_HEIGHT / SECTORSIZE + 1> Sectors;

};

