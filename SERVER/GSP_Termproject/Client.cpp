#include "stdafx.h"

#include "Client.h"

#include "../../Common/protocol.h"
#include "../../Common/EnumDef.h"

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
	Exp._wsabuf.buf = reinterpret_cast<char*>(Exp._send_buf + RemainDataLen);
	Exp._wsabuf.len = sizeof(Exp._send_buf) - RemainDataLen;

	int ret = WSARecv(Socket, &Exp._wsabuf, 1, 0, &recv_flag, &Exp._over, NULL);
	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
		{
			std::cout << "[Recv Error] ClientNum: " << ClientNum << std::endl;
		}
	}

	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		std::cout << "ClientInfo::Recv() ERROR" << std::endl;
		//LogUtil::error_display();
	}
}

void Client::RecvProcess(int byte, OverExpansion* exp)
{
	int RemainData = byte + RemainDataLen;
	char* Buf = exp->_send_buf;

	while (RemainData > 0)
	{
		PACKET* packet = reinterpret_cast<PACKET*>(Buf);
		if (RemainData >= packet->size)
		{
			// ProcessPacket();
			Buf += packet->size;
			RemainData -= packet->size;
		}
		else
			break;
	}
	RemainDataLen = RemainData;

	if (RemainData > 0)
		memmove(exp->_send_buf, Buf, RemainData);
	Recv();
}
