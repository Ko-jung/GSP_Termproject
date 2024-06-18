#pragma once

#include <memory>
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <atlimage.h>
#include "../framework.h"

#include "../../../Common/protocol.h"
#include "../../../Common/OverExpansion.h"
#include "../Define.h"

class Actor;

class GameMgr
{
public:
	SINGTON(GameMgr);

	GameMgr();
	virtual ~GameMgr();

public:
	// Network Logic
	bool InitSocket();
	bool Connect(const char* ip);
	void error_display(const char* msg, int err_no);
	void Send(PACKET* p);
	void Recv();
	void ProcessRecv(PACKET* packet);

	void CALLBACK recv_callback(DWORD errors, DWORD r_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag);
	void CALLBACK send_callback(DWORD errors, DWORD transfer_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag);

public:
	//Game Logic
	void Draw(HDC& memdc);
	void Update();
	void ProcessKeyUpInput(WPARAM wParam);
	void ProcessKeyDownInput(WPARAM wParam);
	void ProcessMouseUpInput(LPARAM lParam, MOUSE_TYPE MouseType);
	void ProcessMouseDownInput(LPARAM lParam, MOUSE_TYPE MouseType);

	void DrawBoard(HDC& memdc);
	void DrawUI(HDC& memdc);

	void OnSystemMessage(const std::string mess);
	void InputChat(WPARAM wParam);

	void SendLogin();
	void SendPosition();
	void SendAttack();
	void SendState();
	void SendChat();

	void ProcessAddObject(SC_ADD_OBJECT_PACKET* SAOP);
	void ProcessRemoveObject(SC_REMOVE_OBJECT_PACKET* SROP);
	void ProcessMoveObject(SC_8DIRECT_MOVE_OBJECT_PACKET* SCDMOP);
	void ProcessStatChange(SC_STAT_CHANGE_PACKET* SSCP);
	void ProcessStateChange(SC_STATE_CHANGE_PACKET* SSCP);
	void ProcessChat(SC_CHAT_PACKET* SCP);

	SOCKET& GetSocket() { return ServerSocket; }
	Actor* GetOwnActor() { return OwnActor.get(); }
	float GetFPS() { return FPS; }

	void SetOwnActorID(const char* ID);

protected:
	//std::vector<bool> KeyInputInfo;
	float FPS;
	float ElapsedTime;
	float SystemMessageTimer;
	std::string SystemText;

	float ChatBoxTimer;
	std::vector<std::string> Chats;

	bool IsChatInputMode;
	std::string NowChat;

	std::shared_ptr<Actor> OwnActor;
	int SerialNum;

	std::unordered_map<int, std::shared_ptr<Actor>> OtherActors;
	std::unordered_map<int, std::shared_ptr<class Monster>> Monsters;

	CImage WorldImageTile;
	CImage ChatBoxImage;

	std::chrono::system_clock::time_point PrevTime;

	// For Network
	SOCKET ServerSocket;
	int RemainDataLen;
	OverExpansion* RecvOverExp;
};

