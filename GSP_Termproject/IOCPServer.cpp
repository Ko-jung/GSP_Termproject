#include "IOCPServer.h"
#include "Define.h"
#include "EnumDef.h"
#include "LogUtil.h"

IOCPServer::IOCPServer()
{
	AcceptExpOver = new EXP_OVER;
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

	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	char	accept_buf[sizeof(SOCKADDR_IN) * 2 + 32 + 100];

	*(reinterpret_cast<SOCKET*>(&AcceptExpOver->_net_buf)) = c_socket;
	ZeroMemory(&AcceptExpOver->_wsa_over, sizeof(AcceptExpOver->_wsa_over));
	AcceptExpOver->_comp_op = COMP_OP::OP_ACCEPT;

	AcceptEx(ListenSocket, c_socket, accept_buf, 0, sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16, NULL, &AcceptExpOver->_wsa_over);
	std::cout << "Aceept Called\n";

	return true;
}

void IOCPServer::StartServer()
{
	for (int i = 0; i < WorkerNum; i++)
	{
		WorkerThreads.emplace_back([this]() { Worker(); });
	}
}

void IOCPServer::Worker()
{
	while (true)
	{
		// I/O�۾����� ���۵� ����Ʈ ���� �����ϴ� ����
		DWORD num_byte;
		// I/O�۾��� �Ϸ�� ���� �ڵ�� ����� �Ϸ� Ű, ���⼱ Ŭ���̾�Ʈ ���� ��ȣ
		LONG64 iocp_key;
		// I/O �۾��� ���۵� �� ������ OVERLAAPED ����ü �ּ�
		WSAOVERLAPPED* p_over;

		BOOL ret = GetQueuedCompletionStatus(hIocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);

		int client_id = static_cast<int>(iocp_key);
		EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(p_over);

		if (FALSE == ret)
		{
			int err_no = WSAGetLastError();
			LogUtil::error_display("GQCS Error : ");
			LogUtil::error_display(err_no);
			//Disconnect(client_id);
			//if (exp_over->_comp_op == COMP_OP::OP_SEND)
			//	delete exp_over;
			continue;
		}

		if (0 == num_byte)
		{
			// if (exp_over->_comp_op == COMP_OP::OP_SEND || exp_over->_comp_op == COMP_OP::OP_RECV)
			// 	ClientMgr::Instance()->Disconnect(client_id);
		}

		switch (exp_over->_comp_op)
		{
			case COMP_OP::OP_ACCEPT:
				break;
			case COMP_OP::OP_RECV:
				break;
			case COMP_OP::OP_SEND:
				break;
		default:
			break;
		}

		//auto FuncIt = m_IocpFunctionMap.find(exp_over->_comp_op);
		//if (FuncIt != m_IocpFunctionMap.end())
		//{
		//	FuncIt->second(client_id, num_byte, exp_over);
		//}
		//else
		//{
		//	std::cerr << "Cant Find mapping Function! _comp_op is: " << (int)exp_over->_comp_op << endl;
		//}
	}
}

void IOCPServer::ThreadJoin()
{
}
