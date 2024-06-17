#include "TimerMgr.h"
#include "../../../Common/OverExpansion.h"
#include <thread>

void TimerMgr::Pop()
{
	TimerEvent evnt;
	auto now = std::chrono::system_clock::now();
	while (TimerQueue.try_pop(evnt))
	{
		// Do Bind Func
		if (evnt.WakeupTime <= std::chrono::system_clock::now())
		{

			// case EV_RANDOM_MOVE
			OverExpansion* over = new OverExpansion;
			over->_comp_type = COMP_TYPE::OP_NPC_MOVE;
			PostQueuedCompletionStatus(*hIOCP, 1, evnt.IocpID, &over->_over);
		}
		else
		{
			// RePush
			TimerQueue.push(evnt);
			std::this_thread::yield();
			return;
		}
	}
	std::this_thread::yield();
}

void TimerMgr::Insert(TimerEvent TE)
{
	TimerQueue.push(TE);
}
