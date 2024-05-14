#pragma once
#include "stdafx.h"

class IOCPServer
{
public:
	IOCPServer();
	virtual ~IOCPServer();

	bool Init(const int);
	bool BindListen(const int);

	void StartServer();
	void Worker();
	void ThreadJoin();

protected:
	SOCKET ListenSocket;
	HANDLE hIocp;
	class EXP_OVER* AcceptExpOver;

	std::vector<std::thread> WorkerThreads;

	int WorkerNum;
};

