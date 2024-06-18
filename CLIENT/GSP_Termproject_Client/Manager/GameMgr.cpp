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
#include "../GamePlayStatic.h"

void CALLBACK recv_callback(DWORD errors, DWORD r_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag);
void CALLBACK send_callback(DWORD errors, DWORD transfer_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag);

GameMgr::GameMgr() :
	ElapsedTime(0.f),
	SerialNum(-1),
	RemainDataLen(0),
	SystemMessageTimer(10.f),
	ChatBoxTimer(10.f),
	IsChatInputMode(false)
{
	OwnActor = std::make_shared<Actor>();

	PrevTime = std::chrono::system_clock::now();

	RecvOverExp = new OverExpansion;

	MapMgr::Instance()->Init();

	FPS = 30.f;

	ChatBoxImage.Load(TEXT("Image/UI/ChatBox.png"));

	Chats.emplace_back("ABCD");
	Chats.emplace_back("41D");
	Chats.emplace_back("A16151CD");
	Chats.emplace_back("A1231231CD");
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

	DrawUI(memdc);
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

	 std::cout << OwnActor->GetLocation().X << ", " << OwnActor->GetLocation().Y << std::endl;
	for (const auto& i : RemoveMonsterTarget)
	{
		Monsters.erase(i);
	}
	for (const auto& i : RemoveOtherActorsTarget)
	{
		OtherActors.erase(i);
	}

	if (SystemMessageTimer > 0.f)
		SystemMessageTimer -= elapsedTime;
	if (ChatBoxTimer > 0.f)
		ChatBoxTimer -= elapsedTime;
}

void GameMgr::ProcessKeyUpInput(WPARAM wParam)
{	
	if (wParam == VK_RETURN)
	{
		if (IsChatInputMode)  SendChat();

		IsChatInputMode = 1 - IsChatInputMode;
		ChatBoxTimer = 5.f;
		return;
	}

	OwnActor->ProcessUpInput(wParam);
}

void GameMgr::ProcessKeyDownInput(WPARAM wParam)
{
	if (IsChatInputMode)
	{
		InputChat(wParam);
		return;
	}

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
	MapMgr::Instance()->Draw(memdc, pos);
}

void GameMgr::DrawUI(HDC& memdc)
{
	SetBkMode(memdc, TRANSPARENT);
	// Pos
	{
		RECT TextRect = { WINWIDTH-400,0,WINWIDTH-10,20 };
		auto OwnPos = GamePlayStatic::GetOwnActorPosition();
		UINT TextFormat = DT_VCENTER | DT_CENTER | DT_SINGLELINE;
		std::string PosString = "(" + std::to_string(OwnPos.X) + ", " + std::to_string(OwnPos.Y) + ")";
		DrawTextA(memdc, PosString.c_str(), -1, &TextRect, TextFormat);
	}


	// Chat Box
	if (ChatBoxTimer > 0.f)
	{
		ChatBoxImage.Draw(memdc, { 0,WINHEIGHT - 200,300,WINHEIGHT }, { 0,0,320,179 });
		SetBkMode(memdc, TRANSPARENT);
		if(IsChatInputMode)
		{	// Now input Chat
			RECT TextRect = { 0,WINHEIGHT - 20,300 - 10,WINHEIGHT };
			UINT TextFormat = DT_LEFT | DT_SINGLELINE;
			FillRect(memdc, &TextRect, 0);
			DrawTextA(memdc, NowChat.c_str(), -1, &TextRect, TextFormat);
		}
		{	// Server Chat
			for (int i = 0; i < Chats.size(); i++)
			{
				int PosY = WINHEIGHT - 20 * (Chats.size() - i + 1);
				RECT TextRect = { 0,PosY,300,PosY + 20 };
				UINT TextFormat = DT_LEFT | DT_SINGLELINE;
				DrawTextA(memdc, Chats[i].c_str(), -1, &TextRect, TextFormat);
			}
		}
	}


	// System Msg
	SetBkMode(memdc, TRANSPARENT);
	if (SystemMessageTimer > 0.f)
	{
		//HFONT hFont = CreateFont(
		//	24,                         // 높이
		//	0,                          // 너비
		//	0,                          // 기울기 각도
		//	0,                          // 기준선과의 각도
		//	FW_NORMAL,                  // 굵기
		//	FALSE,                      // 이탤릭체
		//	FALSE,                      // 밑줄
		//	FALSE,                      // 취소선
		//	HANGEUL_CHARSET,            // 문자 집합
		//	OUT_DEFAULT_PRECIS,         // 출력 정밀도
		//	CLIP_DEFAULT_PRECIS,        // 클리핑 정밀도
		//	DEFAULT_QUALITY,            // 출력 품질
		//	DEFAULT_PITCH | FF_SWISS,   // 피치와 폰트 패밀리
		//	_T("맑은 고딕"));           // 폰트 이름
		//HFONT hOldFont = (HFONT)SelectObject(memdc, hFont);

		RECT TextRect = { 40, WINHEIGHT / 2 + 100, WINWIDTH, WINHEIGHT / 2 + 150 };
		UINT TextFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
		DrawTextA(memdc, SystemText.c_str(), -1, &TextRect, TextFormat);
	}
}

void GameMgr::OnSystemMessage(const std::string mess)
{
	SystemText.clear();
	SystemText = "[System]: " + mess;
	SystemMessageTimer = 5.f;
}

void GameMgr::InputChat(WPARAM wParam)
{
	if (NowChat.size() <= 240)
	{
		if (('a' <= wParam && wParam <= 'z') || ('A' <= wParam && wParam <= 'Z') || ('0' <= wParam && wParam <= '9'))
			NowChat += wParam;
	}
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

void GameMgr::SendChat()
{
	NowChat += "\0";
	CS_CHAT_PACKET CCP;
	strcpy_s(CCP.mess, NowChat.c_str());
	Send(&CCP);

	NowChat.clear();
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
	else if (OtherActors.find(SSCP->id) != OtherActors.end())
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

void GameMgr::ProcessChat(SC_CHAT_PACKET* SCP)
{
	if (Chats.size() > 8)
	{
		Chats.erase(Chats.begin());
	}
	Chats.emplace_back("[" + std::to_string(SCP->id) + "]: " + SCP->mess);
	ChatBoxTimer = 5.f;
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
		ProcessChat(reinterpret_cast<SC_CHAT_PACKET*>(packet));
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
