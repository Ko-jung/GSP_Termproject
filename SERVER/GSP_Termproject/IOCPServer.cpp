#include "IOCPServer.h"
#include "Define.h"
#include "../../Common/EnumDef.h"
#include "../../Common/protocol.h"
#include "../../Common/OverExpansion.h"

#include "LogUtil.h"

#include "Manager/ClientMgr.h"
#include "Client.h"

IOCPServer::IOCPServer()
{
	AcceptExpOver = new OverExpansion;
}

IOCPServer::~IOCPServer()
{
}

bool IOCPServer::Init(const int WNum)
{
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
	int a;
	ClientMgr::Instance()->GetEmptyClient(a);

	for (int i = 0; i < WorkerNum; i++)
	{
		WorkerThreads.emplace_back([this]() { Worker(); });
	}
}

void IOCPServer::Worker()
{
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
				delete Exp;
			continue;
		}

		if (0 == num_byte)
		{
			// if (Exp->_comp_op == COMP_OP::OP_SEND || Exp->_comp_op == COMP_OP::OP_RECV)
			// 	ClientMgr::Instance()->Disconnect(client_id);
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
				delete Exp;
				break;
		default:
			break;
		}
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
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	char	accept_buf[sizeof(SOCKADDR_IN) * 2 + 32 + 100];

	*(reinterpret_cast<SOCKET*>(&AcceptExpOver->_send_buf)) = c_socket;
	ZeroMemory(&AcceptExpOver->_over, sizeof(AcceptExpOver->_over));
	AcceptExpOver->_comp_type = COMP_TYPE::OP_ACCEPT;

	AcceptEx(ListenSocket, c_socket, accept_buf, 0, sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16, NULL, &AcceptExpOver->_over);
}

void IOCPServer::ProcessAccept(OverExpansion* exp)
{
	if (ClientMgr::Instance()->GetClientCount() < MAX_USER)
	{
		int NowClientNum;
		Client* socket = ClientMgr::Instance()->GetEmptyClient(NowClientNum);

		socket->ClientNum = NowClientNum;
		socket->Socket = (*(reinterpret_cast<SOCKET*>(exp->_send_buf)));

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket->Socket), hIocp, NowClientNum, 0);
	 
		socket->SendLoginInfo();
	 	socket->Recv();

		ReadyAccept();
	}
	else
	{
	 	std::cerr << "Client MAX!" << std::endl;
	}
}
 