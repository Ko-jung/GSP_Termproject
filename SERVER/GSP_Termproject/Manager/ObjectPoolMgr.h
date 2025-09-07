#pragma once

#include "../Define.h"
#include <mutex>

class ObjectPoolMgr
{
	SINGLETON(ObjectPoolMgr);

protected:
	std::mutex PoolLock;
};

