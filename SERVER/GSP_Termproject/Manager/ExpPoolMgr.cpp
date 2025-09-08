#include "ExpPoolMgr.h"
#include "../../../Common/OverExpansion.h"
#include "../LogUtil.h"

ExpPoolMgr::ExpPoolMgr()
{
}

ExpPoolMgr::~ExpPoolMgr()
{
	for (auto& PoolPair : PoolMap)
	{
		auto Pool = PoolPair.second.Pool;
		while (!Pool.empty())
		{
			OverExpansion* NewExp = Pool.front();
			Pool.pop();
		}
	}
}

void ExpPoolMgr::Init(int PoolCount)
{
	std::queue<OverExpansion*> Pool;
	for (int i = 0; i < PoolCount; i++)
	{
		Pool.push(new OverExpansion());
	}
	PoolMap.insert({ std::this_thread::get_id(), ThreadPool(Pool) });
}

OverExpansion* ExpPoolMgr::PopExp()
{
	OverExpansion* NewExp = nullptr;

	std::queue<OverExpansion*>& Pool = PoolMap[std::this_thread::get_id()].Pool;
	if (Pool.empty())
	{
		LogUtil::PrintLog("Pool is Empty!");
		NewExp = new OverExpansion();
	}
	else
	{
		NewExp = Pool.front();
		Pool.pop();
	}

	return NewExp;
}

OverExpansion* ExpPoolMgr::GetExp()
{
	OverExpansion* NewExp = PopExp();
	NewExp->Init();

	return NewExp;
}

OverExpansion* ExpPoolMgr::GetExp(char* packet)
{
	OverExpansion* NewExp = PopExp();
	NewExp->Init(packet);

	return NewExp;
}

void ExpPoolMgr::Release(OverExpansion* ReleaseExp)
{
	std::queue<OverExpansion*>& Pool = PoolMap[std::this_thread::get_id()].Pool;
	Pool.push(ReleaseExp);
}
