#pragma once

#include "ObjectPoolMgr.h"
#include <concurrent_queue.h>
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

	int InitPoolCount;
	std::unordered_map<std::thread::id, concurrency::concurrent_queue<OverExpansion*>> PoolMap;
	std::unordered_map<std::thread::id, std::atomic<int>> Test;
};
