#pragma once
#include "../stdafx.h"
#include "../Define.h"
#include "../../../Common/EnumDef.h"
#include <chrono>
#include <concurrent_priority_queue.h>

struct TimerEvent
{
	int IocpID;
	std::chrono::system_clock::time_point WakeupTime;
	EVENT_TYPE EventType;
	int TargetID;
	constexpr bool operator<(const TimerEvent& t) const
	{
		return WakeupTime > t.WakeupTime;
	}
};

class TimerMgr
{
public:
	SINGLETON(TimerMgr);

public:
	void Pop();
	void Insert(TimerEvent TE);
	void SetIOCP(HANDLE* hIOCP) { this->phIOCP = hIOCP; }

	HANDLE* phIOCP;
	Concurrency::concurrent_priority_queue<TimerEvent> TimerQueue;
};

