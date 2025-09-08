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
		auto Pool = PoolPair.second;
		OverExpansion* Temp;
		while (Pool.try_pop(Temp))
		{
			delete Temp;
		}
	}
}

void ExpPoolMgr::Init(int PoolCount)
{
	concurrency::concurrent_queue<OverExpansion*> Pool;
	for (int i = 0; i < PoolCount; i++)
	{
		Pool.push(new OverExpansion());
	}
	PoolMap.insert({std::this_thread::get_id(), Pool});
}

OverExpansion* ExpPoolMgr::PopExp()
{
	OverExpansion* NewExp = nullptr;

	concurrency::concurrent_queue<OverExpansion*>& Pool = PoolMap[std::this_thread::get_id()];
	if (Pool.empty())
	{
		LogUtil::PrintLog("Pool is Empty!");
		NewExp = new OverExpansion();
	}
	else
	{
		Pool.try_pop(NewExp);
	}
	NewExp->ThreadPoolNum = std::this_thread::get_id();

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
	concurrency::concurrent_queue<OverExpansion*>& Pool = PoolMap[ReleaseExp->ThreadPoolNum];
	Pool.push(ReleaseExp);

	if (Pool.unsafe_size() >= 10000)
	{

	}
}
