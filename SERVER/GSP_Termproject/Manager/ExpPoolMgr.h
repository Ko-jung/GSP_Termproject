#pragma once

#include "ObjectPoolMgr.h"
#include <queue>
#include <vector>
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
};