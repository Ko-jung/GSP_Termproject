#pragma once

#include "../Define.h"
#include "../../../Common/protocol.h"

#include <unordered_set>
#include <mutex>
#include <array>

#include <iostream>

class Client;

struct Sector
{
	std::unordered_set<std::shared_ptr<Client>> SectorClient;
	std::mutex SectorLock;
};

class SectorMgr
{
public:
	SINGLETON(SectorMgr);

	void Insert(std::shared_ptr<Client> Target);
	void UnsafeInsert(std::shared_ptr<Client> Target);
	void Remove(std::shared_ptr<Client> Target);
	void MoveSector(std::shared_ptr<Client> Target, int TargetPrevX, int TargetPrevY);

	//Sector* GetSector(int x, int y) { return &Sectors[y][x]; }
	Sector* GetSector(int x, int y) { 
		try {
			return &Sectors.at(y).at(x);
		}
		catch (const std::out_of_range& e){
			std::cout << "Error!: " << e.what() << std::endl;
		}
	}

	void MakeViewList(std::unordered_set<std::shared_ptr<Client>>& ViewList, std::shared_ptr<Client> Center, bool IncludeNPC = false);

private:
	std::array<std::array<Sector, W_WIDTH / SECTORSIZE + 1>, W_HEIGHT / SECTORSIZE + 1> Sectors;

};

