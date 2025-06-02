#include "TimerMgr.h"
#include "../../../Common/OverExpansion.h"
#include <thread>

void TimerMgr::Pop()
{
	std::shared_ptr<TimerEvent> evnt;
	auto now = std::chrono::system_clock::now();
	while (TimerQueue.try_pop(evnt))
	{
		assert(evnt != nullptr);

		// Do Bind Func
		if (evnt->WakeupTime <= std::chrono::system_clock::now())
		{
			// case EV_RANDOM_MOVE
			switch (evnt->EventType)
			{
			case EVENT_TYPE::EV_RANDOM_MOVE:
			{
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
			std::this_thread::yield();
			return;
		}
	}
	std::this_thread::yield();
}

void TimerMgr::Insert(std::shared_ptr<TimerEvent> TE)
{
	TimerQueue.push(TE);
}
