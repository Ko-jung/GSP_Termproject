#pragma once
#include "GameUtil.h"

constexpr int PORT_NUM = 4000;
constexpr int NAME_SIZE = 20;
constexpr int CHAT_SIZE = 300;

constexpr int MAX_USER = 30000;
constexpr int MAX_NPC = 2000;

constexpr int W_WIDTH = 2000;
constexpr int W_HEIGHT = 2000;

constexpr int SECTORSIZE = 8;
constexpr int VIEW_RANGE = 5;

constexpr int BOARDSIZE = 20;

// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_CHAT = 2;
constexpr char CS_ATTACK = 3;			// 4 방향 공격
constexpr char CS_TELEPORT = 4;			// RANDOM한 위치로 Teleport, Stress Test할 때 Hot Spot현상을 피하기 위해 구현
constexpr char CS_LOGOUT = 5;			// 클라이언트에서 정상적으로 접속을 종료하는 패킷
constexpr char CS_8DIRECT_MOVE = 6;
constexpr char CS_STATE_CHANGE = 7;

constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_LOGIN_FAIL = 3;
constexpr char SC_ADD_OBJECT = 4;
constexpr char SC_REMOVE_OBJECT = 5;
constexpr char SC_MOVE_OBJECT = 6;
constexpr char SC_CHAT = 7;
constexpr char SC_STAT_CHANGE = 8;
constexpr char SC_8DIRECT_MOVE_OBJECT = 9;
constexpr char SC_STATE_CHANGE = 10;

constexpr int GAMESERVERPORT = 9000;

#pragma pack (push, 1)
struct PACKET
{
	unsigned short size;
	char	type;

	PACKET(char type) : type(type) { size = sizeof(PACKET); }
};

struct CS_LOGIN_PACKET : PACKET {
	char	name[NAME_SIZE];

	CS_LOGIN_PACKET() : PACKET(CS_LOGIN) { size = sizeof(CS_LOGIN_PACKET); }
};

struct CS_MOVE_PACKET : PACKET {
	char	direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned	move_time;

	CS_MOVE_PACKET() : PACKET(CS_MOVE) { size = sizeof(CS_MOVE_PACKET); }
};

struct CS_8DIRECT_MOVE_PACKET : PACKET {
	POSITION		Position;
	char			direction;		// See Direction
	unsigned int	move_time;

	CS_8DIRECT_MOVE_PACKET() : PACKET(CS_8DIRECT_MOVE) { size = sizeof(CS_8DIRECT_MOVE_PACKET); }
};

struct CS_CHAT_PACKET : PACKET {
	char	mess[CHAT_SIZE - 50];

	CS_CHAT_PACKET() : PACKET(CS_CHAT) { size = sizeof(CS_CHAT_PACKET); }
};

struct CS_TELEPORT_PACKET : PACKET {			// 랜덤으로 텔레포트 하는 패킷, 동접 테스트에 필요

	CS_TELEPORT_PACKET() : PACKET(CS_TELEPORT) { size = sizeof(CS_TELEPORT_PACKET); }
};

struct CS_LOGOUT_PACKET : PACKET {

	CS_LOGOUT_PACKET() : PACKET(CS_LOGOUT) { size = sizeof(CS_LOGOUT_PACKET); }
};

struct CS_ATTACK_PACKET : PACKET
{
	unsigned char WeaponType;
	CS_ATTACK_PACKET() : PACKET(CS_ATTACK) { size = sizeof(CS_ATTACK_PACKET); }
};

struct CS_STATE_CHANGE_PACKET : PACKET
{
	unsigned char ChangedState;
	CS_STATE_CHANGE_PACKET() : PACKET(CS_STATE_CHANGE) { size = sizeof(CS_STATE_CHANGE_PACKET); }
};
struct SC_STATE_CHANGE_PACKET : PACKET
{
	int id;
	unsigned char ChangedState;
	SC_STATE_CHANGE_PACKET() : PACKET(SC_STATE_CHANGE) { size = sizeof(SC_STATE_CHANGE_PACKET); }
};

struct SC_LOGIN_INFO_PACKET : PACKET {
	int		visual;				// 종족, 성별등을 구분할 때 사용
	int		id;
	int		hp;
	int		max_hp;
	int		exp;
	int		level;
	short	x, y;

	SC_LOGIN_INFO_PACKET() : PACKET(SC_LOGIN_INFO) { size = sizeof(SC_LOGIN_INFO_PACKET); }
};

struct SC_ADD_OBJECT_PACKET : PACKET {
	int		id;
	int		visual;				// 어떻게 생긴 OBJECT인가를 지시
	short	x, y;
	int		hp;
	int		max_hp;
	char	name[NAME_SIZE];

	SC_ADD_OBJECT_PACKET() : PACKET(SC_ADD_OBJECT) { size = sizeof(SC_ADD_OBJECT_PACKET); }
};

struct SC_REMOVE_OBJECT_PACKET : PACKET {
	int		id;

	SC_REMOVE_OBJECT_PACKET() : PACKET(SC_REMOVE_OBJECT) { size = sizeof(SC_REMOVE_OBJECT_PACKET); }
};

struct SC_MOVE_OBJECT_PACKET : PACKET {
	int		id;
	short	x, y;
	unsigned int move_time;

	SC_MOVE_OBJECT_PACKET() : PACKET(SC_MOVE_OBJECT) { size = sizeof(SC_MOVE_OBJECT_PACKET); }
};

struct SC_8DIRECT_MOVE_OBJECT_PACKET : PACKET {
	int		id;
	float	x, y;
	char	direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned int move_time;

	SC_8DIRECT_MOVE_OBJECT_PACKET() : PACKET(SC_8DIRECT_MOVE_OBJECT) { size = sizeof(SC_8DIRECT_MOVE_OBJECT_PACKET); }
};

struct SC_CHAT_PACKET : PACKET {
	int		id;
	char	mess[CHAT_SIZE - 50];

	SC_CHAT_PACKET() : PACKET(SC_CHAT) { size = sizeof(SC_CHAT_PACKET); }
};

struct SC_LOGIN_FAIL_PACKET : PACKET {

	SC_LOGIN_FAIL_PACKET() : PACKET(SC_LOGIN_FAIL) { size = sizeof(SC_LOGIN_FAIL_PACKET); }
};

struct SC_STAT_CHANGE_PACKET : PACKET {
	int		id;
	int		hp;
	int		max_hp;
	int		exp;
	int		level;

	SC_STAT_CHANGE_PACKET() : PACKET(SC_STAT_CHANGE) { size = sizeof(SC_STAT_CHANGE_PACKET); }
};

#pragma pack (pop)