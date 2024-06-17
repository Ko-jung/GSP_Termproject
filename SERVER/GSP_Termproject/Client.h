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

	void Init();
	void Send(struct PACKET* p);
	void Recv();
	void RecvProcess(int byte, OverExpansion* exp);

	void StressTestMove(char Direction);
	void Move(POSITION NewPos, char direction);

	RECT GetCollisionBox();

	void SendLoginInfo();
	void SendStressTestMovePos();
	void SendMovePos(Client* c);
	void SendAddPlayer(Client* c);
	void SendRemovePlayer(Client* c);

	std::atomic_bool IsActive;
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
	std::mutex	ViewListLock;
	std::unordered_set<Client*> ViewList;

	static int ImageSpriteWidth;
	static int ImageSpriteHeight;

	static int MonsterImageSpriteWidth;
	static int MonsterImageSpriteHeight;
};

