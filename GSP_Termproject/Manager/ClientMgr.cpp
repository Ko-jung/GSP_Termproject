#include "ClientMgr.h"
#include "../Client.h"

#include <atomic>

ClientMgr::ClientMgr():
	ClientCount(0)
{
	for (auto& c : Clients)
	{
		c = new Client();
	}
}

ClientMgr::~ClientMgr()
{
	for (auto& c : Clients)
	{
		delete c;
	}
}

Client* ClientMgr::GetEmptyClient(int& ClientNum)
{
	while (true)
	{
		for (int i = 0; i < Clients.size(); i++)
		{
			if (!Clients[i])
			{
				auto NewClient = new Client();
				std::atomic<Client*> AtomicClientPtr = std::atomic<Client*>(Clients[i]);
				bool succ = std::atomic_compare_exchange_strong(
					&AtomicClientPtr,
					nullptr,
					NewClient
				);

				if (succ) 
				{
					ClientNum = i;
					ClientCount++;
					return Clients[i];
				}
				else
				{
					delete NewClient;
					break;
				}
			}
		}
	}
	return nullptr;
}
