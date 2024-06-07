#pragma once

#include "OverExpansion.h"

class Client
{
public:
	Client();
	~Client();

	void Send(struct PACKET* p);
	void Recv();
	void RecvProcess(int byte, OverExpansion* exp);

	int ClientNum;
	SOCKET Socket;
	OverExpansion Exp;

	int RemainDataLen;
};

