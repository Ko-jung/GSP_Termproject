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
	void Disconnect(int Id);

	void ReadyAccept();
	void ProcessAccept(class OverExpansion* exp);

//protected:
	SOCKET ListenSocket;
	HANDLE hIocp;
	class OverExpansion* AcceptExpOver;

	std::vector<std::thread> WorkerThreads;

	int WorkerNum;
};

