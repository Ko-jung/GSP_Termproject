#include "stdafx.h"
#include "Define.h"

#include "../../Common/protocol.h"

#include "IOCPServer.h"

int main()
{
	setlocale(LC_ALL, "");
	std::wcout.imbue(std::locale(""));

	IOCPServer* IocpServer = new IOCPServer();

	IocpServer->Init(std::thread::hardware_concurrency() / 2);
	IocpServer->BindListen(GAMESERVERPORT);

	IocpServer->StartServer();

	IocpServer->ThreadJoin();
}