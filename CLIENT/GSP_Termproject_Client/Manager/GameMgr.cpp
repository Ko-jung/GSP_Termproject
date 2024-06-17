#include <WS2tcpip.h>
#pragma comment (lib, "WS2_32.LIB")

#include "GameMgr.h"
#include "../Actor.h"

#include "../framework.h"

#include "../../../Common/protocol.h"
#include "../../../Common/EnumDef.h"
#include "../../../Common/GameUtil.h"
#include "../Define.h"

#include "MapMgr.h"

#include "../Monster.h"

void CALLBACK recv_callback(DWORD errors, DWORD r_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag);
void CALLBACK send_callback(DWORD errors, DWORD transfer_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag);

GameMgr::GameMgr() :
	ElapsedTime(0.f),
	SerialNum(-1),
	RemainDataLen(0)
{
	OwnActor = std::make_shared<Actor>();

	PrevTime = std::chrono::system_clock::now();

	RecvOverExp = new OverExpansion;

	MapMgr::Instance()->Init();

	FPS = 30.f;
}

GameMgr::~GameMgr()
{
	closesocket(ServerSocket);
	WSACleanup();
}

bool GameMgr::InitSocket()
{
	WSADATA WSAData;
	int ret = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (0 != ret)
	{
		return false;
	}
	return true;
}

bool GameMgr::Connect(const char* ip)
{
	ServerSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(GAMESERVERPORT);
	inet_pton(AF_INET, ip, &server_addr.sin_addr);
	int ret = connect(ServerSocket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	if (ret != 0)
	{
		error_display("connect Error: ", WSAGetLastError());
		return false;
	}

	//NetworkThread = std::thread(&GameMgr::Recv, this);
	SendLogin();
	Recv();

	return true;
}

void GameMgr::Draw(HDC& memdc)
{
	DrawBoard(memdc);

	for (const auto& a : Monsters)
	{
		a.second->Draw(memdc);
	}
	for (const auto& a : OtherActors)
	{
		a.second->Draw(memdc);
	}

	//Recv();
	OwnActor->Draw(memdc);
}

void GameMgr::Update()
{
	float elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - PrevTime).count() / 1000.f;
	PrevTime = std::chrono::system_clock::now();

	std::vector<int> RemoveMonsterTarget;
	for (const auto& a : Monsters)
	{
		if (a.second->GetIsCanRemove())
		{
			RemoveMonsterTarget.push_back(a.first);
			continue;
		}
		a.second->Update(elapsedTime);
	}
	std::vector<int> RemoveOtherActorsTarget;
	for (const auto& a : OtherActors)
	{
		if (a.second->IsCanRemove)
		{
			RemoveOtherActorsTarget.push_back(a.first);
			continue;
		}
		a.second->Update(elapsedTime);
	}

	OwnActor->Update(elapsedTime);

	SendPosition();
	SendState();

	// std::cout << OwnActor->GetLocation().X << ", " << OwnActor->GetLocation().Y << std::endl;
	for (const auto& i : RemoveMonsterTarget)
	{
		Monsters.erase(i);
	}
	for (const auto& i : RemoveOtherActorsTarget)
	{
		OtherActors.erase(i);
	}
}

void GameMgr::ProcessKeyUpInput(WPARAM wParam)
{	
	OwnActor->ProcessUpInput(wParam);
}

void GameMgr::ProcessKeyDownInput(WPARAM wParam)
{
	OwnActor->ProcessDownInput(wParam);
}

void GameMgr::ProcessMouseUpInput(LPARAM lParam, MOUSE_TYPE MouseType)
{
	switch (MouseType)
	{
	case MOUSE_TYPE::LEFT_BTN:
		break;
	case MOUSE_TYPE::MIDDLE_BTN:
		break;
	case MOUSE_TYPE::RIGHT_BTN:
		break;
	default:
		break;
	}
}

void GameMgr::ProcessMouseDownInput(LPARAM lParam, MOUSE_TYPE MouseType)
{
	switch (MouseType)
	{
	case MOUSE_TYPE::LEFT_BTN:
		OwnActor->ProcessAttack();
		SendAttack();
		break;
	case MOUSE_TYPE::MIDDLE_BTN:
		break;
	case MOUSE_TYPE::RIGHT_BTN:
		break;
	default:
		break;
	}
}

void GameMgr::DrawBoard(HDC& memdc)
{
	POSITION pos = OwnActor->GetLocation();
	//std::cout << X << ", " << Y << std::endl;

	MapMgr::Instance()->Draw(memdc, pos);
}

void GameMgr::SendLogin()
{
	CS_LOGIN_PACKET CLP;
	strcpy_s(CLP.name, OwnActor->GetPlayerName());
	Send(&CLP);
}

void GameMgr::SendPosition()
{
	// POSITION Pos = OwnActor->GetLocation();

	CS_8DIRECT_MOVE_PACKET CMP;
	CMP.Position = OwnActor->GetLocation();
	CMP.direction = (char)OwnActor->GetDirection();

	Send(&CMP);
}

void GameMgr::SendAttack()
{
	CS_ATTACK_PACKET CAP;
	CAP.WeaponType = (BYTE)WEAPON_TYPE::SWORD;
	Send(&CAP);
}

void GameMgr::SendState()
{
	if (!OwnActor->GetIsChangeState()) return;
	OwnActor->SetIsChangeState(false);

	CS_STATE_CHANGE_PACKET CSCP;
	CSCP.ChangedState = (BYTE)OwnActor->GetState();
	Send(&CSCP);
}

void GameMgr::ProcessAddObject(SC_ADD_OBJECT_PACKET* SAOP)
{
	if (SAOP->id < MAX_USER)
	{
		std::shared_ptr<Actor> NewActor = std::make_shared<Actor>(false);
		NewActor->InitUsePacket(SAOP);

		OtherActors.insert(std::make_pair(SAOP->id, NewActor));
	}
	else
	{
		std::shared_ptr<Monster> NewActor = std::make_shared<Monster>();
		NewActor->InitUsePacket(SAOP);

		Monsters.insert(std::make_pair(SAOP->id, NewActor));
	}
}

void GameMgr::ProcessRemoveObject(SC_REMOVE_OBJECT_PACKET* SROP)
{
	if (SROP->id < MAX_USER)
	{
		OtherActors.erase(SROP->id);
	}
	else
	{
		Monsters.erase(SROP->id);
	}
}

void GameMgr::ProcessMoveObject(SC_8DIRECT_MOVE_OBJECT_PACKET* SDMOP)
{
	if (SDMOP->id == SerialNum)
	{
		//OwnActor->ProcessMove(SDMOP);
		std::cout << "The server adjusted the position" << std::endl;
	}
	else
	{
		Actor* TargetActor = nullptr;
		if (OtherActors.find(SDMOP->id) != OtherActors.end())
		{
			OtherActors[SDMOP->id]->ProcessMove(SDMOP);
		}
		else if (Monsters.find(SDMOP->id) != Monsters.end())
		{
			Monsters[SDMOP->id]->ProcessMove(SDMOP);
		}
		else
		{
			std::cout << "Get SC_8DIRECT_MOVE_OBJECT_PACKET but id:" << SDMOP->id << " is Cant Find!" << std::endl;
			return;
		}
	}
}

void GameMgr::ProcessStatChange(SC_STAT_CHANGE_PACKET* SSCP)
{
	if (SSCP->id == SerialNum)
	{
		OwnActor->ProcessChangeStat(SSCP);
	}
	if (OtherActors.find(SSCP->id) != OtherActors.end())
	{
		OtherActors[SSCP->id]->ProcessChangeStat(SSCP);
	}
	else if (Monsters.find(SSCP->id) != Monsters.end())
	{
		Monsters[SSCP->id]->ProcessChangeStat(SSCP);
	}
	else
	{
		std::cout << "Get SC_STAT_CHANGE_PACKET but id:" << SSCP->id << " is Cant Find!" << std::endl;
		return;
	}
}

void GameMgr::ProcessStateChange(SC_STATE_CHANGE_PACKET* SSCP)
{
	if (OtherActors.find(SSCP->id) != OtherActors.end())
	{
		OtherActors[SSCP->id]->ChangeStateByPacket((ACTOR_STATE)SSCP->ChangedState);
	}
	else
	{
		std::cout << "Get SC_STATE_CHANGE_PACKET but id:" << SSCP->id << " is Cant Find!" << std::endl;
		return;
		}
}

void GameMgr::SetOwnActorID(const char* ID)
{
	OwnActor->SetName(ID);
}

void GameMgr::error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L" 에러 " << lpMsgBuf << std::endl;
	//while (true);
	// 디버깅 용
	LocalFree(lpMsgBuf);
}

void GameMgr::ProcessRecv(PACKET* packet)
{
	switch (packet->type)
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* SLIP = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(packet);
		SerialNum = SLIP->id;

		OwnActor->ProcessLogin(SLIP);
		break;
	}
	case SC_LOGIN_FAIL:
		break;
	case SC_ADD_OBJECT:
	{
		ProcessAddObject(reinterpret_cast<SC_ADD_OBJECT_PACKET*>(packet));
		break;
	}
	case SC_REMOVE_OBJECT:
	{
		ProcessRemoveObject(reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(packet));
		break;
	}
	case SC_MOVE_OBJECT:
		break;
	case SC_CHAT:
		break;
	case SC_STAT_CHANGE:
		ProcessStatChange(reinterpret_cast<SC_STAT_CHANGE_PACKET*>(packet));
		break;
	case SC_8DIRECT_MOVE_OBJECT:
	{
		ProcessMoveObject(reinterpret_cast<SC_8DIRECT_MOVE_OBJECT_PACKET*>(packet));
		break;
	}
	case SC_STATE_CHANGE:
	{
		ProcessStateChange(reinterpret_cast<SC_STATE_CHANGE_PACKET*>(packet));
		break;
	}
	default:
		break;
	}
}

void GameMgr::Send(PACKET* p)
{
	OverExpansion* exp = new OverExpansion((char*)p);

	int ret = WSASend(ServerSocket, &exp->_wsabuf, 1, nullptr, 0, &exp->_over, ::send_callback);
	if (ret != 0 && ret != WSA_IO_PENDING)
	{
		error_display("WSASend Error: ", WSAGetLastError());
		delete exp;
	}
}

void GameMgr::Recv()
{
	DWORD RecvFlag = 0;

	ZeroMemory(&RecvOverExp->_over, sizeof(RecvOverExp->_over));
	RecvOverExp->_wsabuf.buf = reinterpret_cast<char*>(RecvOverExp->_send_buf + RemainDataLen);
	RecvOverExp->_wsabuf.len = sizeof(RecvOverExp->_send_buf) - RemainDataLen;

	int ret = WSARecv(ServerSocket, &RecvOverExp->_wsabuf, 1, nullptr, &RecvFlag, &RecvOverExp->_over, ::recv_callback);
	if (ret != 0)
	{
		if(WSAGetLastError() != WSA_IO_PENDING)
		error_display("WSARecv Error: ", WSAGetLastError());
	}
}

void GameMgr::recv_callback(DWORD errors, DWORD r_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag)
{
	if (r_size == 0)
	{
		std::cout << "Maybe Server CLOSE" << std::endl;
		exit(0);
		return;
	}

	OverExpansion* exp = reinterpret_cast<OverExpansion*>(p_wsaover);

	int RemainData = r_size + RemainDataLen;
	char* Buf = exp->_send_buf;

	while (RemainData > 0)
	{
		PACKET* packet = reinterpret_cast<PACKET*>(Buf);
		if (RemainData >= packet->size)
		{
			ProcessRecv(packet);
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
	//Send();
}

void GameMgr::send_callback(DWORD errors, DWORD transfer_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag)
{
	delete p_wsaover;
}
