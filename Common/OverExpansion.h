#pragma once

#include <WS2tcpip.h>
#include <MSWSock.h>
#include <WinSock2.h>
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "ws2_32.lib")

#include "EnumDef.h"
#include "protocol.h"

class OverExpansion
{
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[CHAT_SIZE];
	COMP_TYPE _comp_type;
	int _ai_target_obj;

	OverExpansion()
	{
		_wsabuf.len = CHAT_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = COMP_TYPE::OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OverExpansion(char* packet)
	{
		_wsabuf.len = *(unsigned short*)packet;
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = COMP_TYPE::OP_SEND;
		memcpy(_send_buf, packet, _wsabuf.len);
	}
};