#include "ClientMgr.h"
#include "../Client.h"

#include "../OverExpansion.h"

#include <atomic>

ClientMgr::ClientMgr():
	ClientCount(0)
{
	for (auto& c : Clients)
	{
		c = nullptr;
	}
}

ClientMgr::~ClientMgr()
{
	for (auto& c : Clients)
	{
		delete c;
	}
}

void ClientMgr::RecvProcess(int id, int bytes, OverExpansion* exp)
{
	Clients[id]->RecvProcess(bytes, exp);
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

				//std::atomic<Client*>* AtomicClientPtr = Clients[i];
				//auto abc = reinterpret_cast<std::atomic<Client*>*>(&Clients[i]);
				Client* llNullptr = nullptr;
				bool succ = std::atomic_compare_exchange_strong(
					reinterpret_cast<std::atomic<Client*>*>(&Clients[i]),
					&llNullptr,
					NewClient
				);

				//long long llNullptr = reinterpret_cast<long long>(nullptr);
				//bool succ = std::atomic_compare_exchange_strong(
				//	reinterpret_cast<std::atomic_llong*>(&Clients[i]),
				//	&llNullptr,
				//	reinterpret_cast<long long>(NewClient)
				//);


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
