#include "ExpPoolMgr.h"
#include "../../../Common/OverExpansion.h"
#include "../LogUtil.h"

struct ThreadPool
{
	std::queue<OverExpansion*> Pool;

	~ThreadPool()
	{
		while (!Pool.empty())
		{
			delete Pool.front();
			Pool.pop();
		}
	}
};

thread_local ThreadPool TLPool;

ExpPoolMgr::ExpPoolMgr()
{
}

ExpPoolMgr::~ExpPoolMgr()
{
}

void ExpPoolMgr::Init(int PoolCount)
{
	for (int i = 0; i < PoolCount; i++)
	{
		TLPool.Pool.push(new OverExpansion());
	}
}

OverExpansion* ExpPoolMgr::PopExp()
{
	OverExpansion* NewExp = nullptr;

	if (TLPool.Pool.empty())
	{
		LogUtil::PrintLog("Pool is Empty!");
		NewExp = new OverExpansion();
	}
	else
	{
		NewExp = TLPool.Pool.front();
		TLPool.Pool.pop();
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
	TLPool.Pool.push(ReleaseExp);
}
