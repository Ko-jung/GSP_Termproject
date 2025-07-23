#include "TimerMgr.h"
#include "../../../Common/OverExpansion.h"
#include <thread>
#include <cassert>

void TimerMgr::Pop()
{
	std::lock_guard<std::mutex> lock(TimerQueueMutex);
	while (!TimerQueue.empty())
	{
		std::shared_ptr<TimerEvent> evnt;
		evnt = TimerQueue.top();
		TimerQueue.pop();
		assert(evnt != nullptr);

		// Do Bind Func
		if (evnt->WakeupTime <= std::chrono::system_clock::now())
		{
			// case EV_RANDOM_MOVE
			switch (evnt->EventType)
			{
			case EVENT_TYPE::EV_RANDOM_MOVE:
			{
				//std::shared_ptr<OverExpansion> over = std::make_shared< OverExpansion>();
				OverExpansion* over = new OverExpansion;
				over->_comp_type = COMP_TYPE::OP_NPC_MOVE;
				PostQueuedCompletionStatus(*phIOCP, 1, evnt->IocpID, &over->_over);
				break;
			}
			case EVENT_TYPE::EV_SPAWN_PLAYER:
			{
				OverExpansion* over = new OverExpansion;
				over->_comp_type = COMP_TYPE::OP_SPAWN_PLAYER;
				PostQueuedCompletionStatus(*phIOCP, 1, evnt->IocpID, &over->_over);
				break;
			}
			default:
				break;
			}
		}
		else
		{
			// RePush
			TimerQueue.push(evnt);
			break;
		}
	}
	std::this_thread::yield();
}

void TimerMgr::Insert(std::shared_ptr<TimerEvent> TE)
{
	std::lock_guard<std::mutex> lock(TimerQueueMutex);
	TimerQueue.push(TE);
}
