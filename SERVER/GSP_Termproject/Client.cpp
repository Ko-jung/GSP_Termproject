#include "stdafx.h"

#include "Client.h"

#include "protocol.h"
#include "EnumDef.h"

Client::Client() :
	ClientNum(-1)
{
}

Client::~Client()
{
}

void Client::Send(PACKET* p)
{
	if (Socket == INVALID_SOCKET)
	{
		std::cout << "ClientInfo Socket is INVALID_SOCKET" << std::endl;
		return;
	}

	OverExpansion* exp = new OverExpansion{ (char*)p };

	int ret = WSASend(Socket, &exp->_wsabuf, 1, 0, 0, &exp->_over, 0);
	if (0 != ret) {
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
		{
			std::cout << "Client::Send() ERROR" << std::endl;
		}
	}
}

void Client::Recv()
{
	DWORD recv_flag = 0;

	ZeroMemory(&Exp._over, sizeof(Exp._over));
	Exp._wsabuf.buf = reinterpret_cast<char*>(Exp._send_buf + m_iRemainDataLen);
	Exp._wsabuf.len = sizeof(Exp._send_buf) - m_iRemainDataLen;

	int ret = WSARecv(Socket, &Exp._wsabuf, 1, 0, &recv_flag, &Exp._over, NULL);
	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
		{

		}
	}

	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		LogUtil::error_display("ClientInfo::Recv() ERROR");
	}
}
