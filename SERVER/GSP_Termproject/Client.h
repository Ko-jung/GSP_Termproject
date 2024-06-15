#pragma once

#include "../../Common/OverExpansion.h"
#include "../../Common/GameUtil.h"
#include "../../Common/EnumDef.h"
#include <mutex>
#include <chrono>

class Client
{
public:
	Client();
	~Client();

	void Send(struct PACKET* p);
	void Recv();
	void RecvProcess(int byte, OverExpansion* exp);

	void StressTestMove(char Direction);
	void Move(char bitDirection, char Direction);

	RECT GetCollisionBox();

	void SendLoginInfo();
	void SendStressTestMovePos();
	void SendMovePos();

	int ClientNum;
	SOCKET Socket;
	OverExpansion Exp;

	int RemainDataLen;

	char PlayerName[NAME_SIZE];
	POSITION Position;
	ACTOR_DIRECTION Direction;
	float Speed;
	CLIENT_STATE State;
	float Size;

	std::mutex StateMutex;
	int LastMoveTime;

	static int ImageSpriteWidth;
	static int ImageSpriteHeight;
};

