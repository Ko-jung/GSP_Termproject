#pragma once

#include "ObjectPoolMgr.h"
#include <queue>
#include <unordered_map>
#include <thread>

class OverExpansion;

// ThreadNum, ExpPool pair∑Œ ¿€µø
class ExpPoolMgr : public ObjectPoolMgr
{
	SINGLETON(ExpPoolMgr);
public:
	void Init(int PoolCount);

	OverExpansion* GetExp();
	OverExpansion* GetExp(char* packet);
	void Release(OverExpansion*);

private:
	ExpPoolMgr();
	~ExpPoolMgr();

	OverExpansion* PopExp();

	struct alignas(64) ThreadPool {
		std::queue<OverExpansion*> Pool;
	};
	std::unordered_map<std::thread::id, ThreadPool> PoolMap;
};

