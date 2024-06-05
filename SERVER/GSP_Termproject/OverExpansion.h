#pragma once

#include "stdafx.h"
#include "Define.h"
#include "EnumDef.h"

class OverExpansion
{
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;

	OverExpansion()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = COMP_TYPE::OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OverExpansion(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = COMP_TYPE::OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}
};