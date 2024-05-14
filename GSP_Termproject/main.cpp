#include "stdafx.h"
#include "Define.h"

#include "IOCPServer.h"

int main()
{
	setlocale(LC_ALL, "");
	std::wcout.imbue(std::locale(""));

	IOCPServer* IocpServer = new IOCPServer();

	IocpServer->Init(std::thread::hardware_concurrency());
	IocpServer->BindListen(GAMESERVERPORT);

	IocpServer->StartServer();

	IocpServer->ThreadJoin();
}