#pragma once

#include "ObjectPoolMgr.h"
#include <concurrent_queue.h>
#include <unordered_map>
#include <thread>


class OverExpansion;

// ThreadNum, ExpPool pair�� �۵�
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

	std::unordered_map<std::thread::id, concurrency::concurrent_queue<OverExpansion*>> PoolMap;
};
