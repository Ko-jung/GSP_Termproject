#include "IOCPServer.h"
#include "Define.h"
#include "../../Common/EnumDef.h"
#include "../../Common/protocol.h"

#include "LogUtil.h"

#include "Manager/ClientMgr.h"
#include "Manager/MapMgr.h"
#include "Manager/SectorMgr.h"
#include "Manager/TimerMgr.h"
#include "Manager/DBMgr.h"
#include "Manager/ExpPoolMgr.h"

#include "Client.h"

IOCPServer::IOCPServer()
{
	AcceptExpOver = new OverExpansion;
	AcceptExpOver->_comp_type = COMP_TYPE::OP_ACCEPT;
}

IOCPServer::~IOCPServer()
{
}

bool IOCPServer::Init(const int WNum)
{
	std::cout << "THread Num: " << WNum << std::endl;

	// MapMgr Init
	MapMgr::Instance()->Init();
	
	DBMgr::Instance();
	ExpPoolMgr::Instance();

	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
		return false;

	ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == ListenSocket)
	{
		LogUtil::error_display(WSAGetLastError());
		return false;
	}

	WorkerNum = WNum - 2;
	WorkerThreads.reserve(WorkerNum);

	return true;
}

bool IOCPServer::BindListen(const int PortNum)
{
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PortNum);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int retval = bind(ListenSocket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	if (0 != retval)
	{
		LogUtil::error_display(WSAGetLastError());
		return false;
	}

	retval = listen(ListenSocket, SOMAXCONN);
	if (0 != retval)
	{
		LogUtil::error_display(WSAGetLastError());
		return false;
	}

	hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(ListenSocket), hIocp, 9999, 0);

	ReadyAccept();
	std::cout << "Aceept Called\n";

	return true;
}

void IOCPServer::StartServer()
{
	ClientMgr::Instance()->InitNPC();
	TimerMgr::Instance()->SetIOCP(&hIocp);

	for (int i = 0; i < WorkerNum; i++)
	{
		WorkerThreads.emplace_back([this]() { Worker(); });
	}
	TimerThread = std::thread{ &IOCPServer::Timer, this };
}

void IOCPServer::Worker()
{
	// 객체 풀
	ExpPoolMgr::Instance()->Init(5000);	

	while (true)
	{
		DWORD num_byte;
		LONG64 iocp_key;
		WSAOVERLAPPED* p_over;

		BOOL ret = GetQueuedCompletionStatus(hIocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);

		int client_id = static_cast<int>(iocp_key);
		OverExpansion* Exp = reinterpret_cast<OverExpansion*>(p_over);

		if (FALSE == ret)
		{
			int err_no = WSAGetLastError();
			LogUtil::error_display("GQCS Error : ");
			LogUtil::error_display(err_no);
			Disconnect(client_id);
			if (Exp->_comp_type == COMP_TYPE::OP_SEND)
				ExpPoolMgr::Instance()->Release(Exp);
			continue;
		}

		if (0 == num_byte)
		{
			if (Exp->_comp_type == COMP_TYPE::OP_SEND || Exp->_comp_type == COMP_TYPE::OP_RECV)
			 	ClientMgr::Instance()->Disconnect(client_id);
		}

		switch (Exp->_comp_type)
		{
			case COMP_TYPE::OP_ACCEPT:
				ProcessAccept(Exp);
				break;
			case COMP_TYPE::OP_RECV:
				ClientMgr::Instance()->RecvProcess(client_id, num_byte, Exp);
				break;
			case COMP_TYPE::OP_SEND:
				ExpPoolMgr::Instance()->Release(Exp);
				break;
			case COMP_TYPE::OP_NPC_MOVE:
				ClientMgr::Instance()->ProcessNPCMove(client_id, Exp);
				ExpPoolMgr::Instance()->Release(Exp);
				break;
			case COMP_TYPE::OP_SPAWN_PLAYER:
				ClientMgr::Instance()->ProcessClientSpawn(client_id);
				ExpPoolMgr::Instance()->Release(Exp);
				break;
		default:
			break;
		}
	}
}

void IOCPServer::Timer()
{
	TimerMgr* TimerInstance = TimerMgr::Instance();
	while (true)
	{
		TimerInstance->Pop();
	}
}

void IOCPServer::ThreadJoin()
{
	for (auto& t : WorkerThreads)
	{
		t.join();
	}
}

void IOCPServer::Disconnect(int Id)
{
	ClientMgr::Instance()->Disconnect(Id);
}

void IOCPServer::ReadyAccept()
{
	NextAccpetSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);;
	ZeroMemory(&AcceptExpOver->_over, sizeof(AcceptExpOver->_over));

	AcceptEx(ListenSocket, NextAccpetSocket, AcceptExpOver->_send_buf, 0, sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16, NULL, &AcceptExpOver->_over);
}

void IOCPServer::ProcessAccept(OverExpansion* exp)
{
	if (ClientMgr::Instance()->GetClientCount() < MAX_USER)
	{
		std::shared_ptr<Client> NewClient = ClientMgr::Instance()->GetEmptyClient();
		if (NewClient == nullptr)
		{
			std::cerr << "Client NULL!" << std::endl;
			return;
		}
		NewClient->Socket = NextAccpetSocket;

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(NewClient->Socket), hIocp, NewClient->ClientNum, 0);

		NewClient->Recv();

		ReadyAccept();
	}
	else
	{
		std::cerr << "Client MAX!" << std::endl;
	}
}
 