#pragma once

#include "OverExpansion.h"

class Client
{
public:
	Client();
	~Client();

	void Send(struct PACKET* p);
	void Recv();

	int ClientNum;
	SOCKET Socket;
	OverExpansion Exp;
};

