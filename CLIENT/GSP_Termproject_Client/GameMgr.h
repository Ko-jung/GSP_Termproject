#pragma once

#include <memory>
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <atlimage.h>
#include "framework.h"

#include "../../Common/protocol.h"
#include "../../Common/OverExpansion.h"

class Actor;

class GameMgr
{
public:
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
	void ProcessUpInput(WPARAM wParam);
	void ProcessDownInput(WPARAM wParam);

	void LoadBoard();
	void DrawBoard(HDC& memdc);

	void SendPosition();

	SOCKET& GetSocket() { return ServerSocket; }

	void SetOwnActorID(const char* ID);

protected:
	//std::vector<bool> KeyInputInfo;
	float ElapsedTime;
	
	std::shared_ptr<Actor> OwnActor;
	int SerialNum;

	std::unordered_map<int, std::shared_ptr<Actor>> OtherActors;

	CImage WorldImageTile;
	std::array<std::array<WORD, 2000>, 2000> WorldMap;

	std::chrono::system_clock::time_point PrevTime;

	// For Network
	SOCKET ServerSocket;
	int RemainDataLen;
};

