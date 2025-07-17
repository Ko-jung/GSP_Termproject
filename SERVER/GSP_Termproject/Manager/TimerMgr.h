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

	TimerEvent() :
		IocpID(-1),
		WakeupTime(),
		EventType(),
		TargetID(0)
	{ }
	TimerEvent(int id,
			std::chrono::system_clock::time_point time,
			EVENT_TYPE type,
			int tId) :
		IocpID(id),
		WakeupTime(time),
		EventType(type),
		TargetID(tId)
	{ }

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
	void Insert(std::shared_ptr<TimerEvent> TE);
	void SetIOCP(HANDLE* hIOCP) { this->phIOCP = hIOCP; }

	HANDLE* phIOCP;
	Concurrency::concurrent_priority_queue<std::shared_ptr<TimerEvent>> TimerQueue;
};

