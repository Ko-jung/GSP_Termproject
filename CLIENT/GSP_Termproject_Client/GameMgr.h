#pragma once

#include <memory>
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include "../../Common/protocol.h"
#include "framework.h"
#include <atlimage.h>

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
	//void Recv();
	void ProcessRecv(char* recv_buf);
	void Send(PACKET* p, int packetSize);
	void Recv();

	void CALLBACK recv_callback(DWORD errors, DWORD r_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag);
	void CALLBACK send_callback(DWORD errors, DWORD transfer_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag);

public:
	//Game Logic
	void Draw(HDC& memdc);
	void Update(float elapsedTime);
	void ProcessUpInput(WPARAM wParam);
	void ProcessDownInput(WPARAM wParam);

	void LoadBoard();
	void DrawBoard(HDC& memdc);

	SOCKET& GetSocket() { return s_socket; }

	void SetOwnActorID(const char* ID);

protected:
	std::vector<bool> KeyInputInfo;
	
	std::shared_ptr<Actor> OwnActor;
	int SerialNum;

	std::unordered_map<int, std::shared_ptr<Actor>> OtherActor;

	SOCKET s_socket;
	char buf[CHAT_SIZE];
	WSABUF mybuf[1];
	WSAOVERLAPPED wsaover;

	std::thread NetworkThread;

	char send_buf[CHAT_SIZE];
	char recv_buf[CHAT_SIZE];

	CImage WorldImageTile;
	std::array<std::array<WORD, 2000>, 2000> WorldMap;
};

