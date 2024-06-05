constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;
constexpr int CHAT_SIZE = 100;

constexpr int MAX_USER = 50000;
constexpr int MAX_NPC = 200000;

constexpr int W_WIDTH = 2000;
constexpr int W_HEIGHT = 2000;

// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_CHAT = 2;
constexpr char CS_ATTACK = 3;			// 4 방향 공격
constexpr char CS_TELEPORT = 4;			// RANDOM한 위치로 Teleport, Stress Test할 때 Hot Spot현상을 피하기 위해 구현
constexpr char CS_LOGOUT = 5;			// 클라이언트에서 정상적으로 접속을 종료하는 패킷

constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_OBJECT = 3;
constexpr char SC_REMOVE_OBJECT = 4;
constexpr char SC_MOVE_OBJECT = 5;
constexpr char SC_CHAT = 6;
constexpr char SC_LOGIN_OK = 7;
constexpr char SC_LOGIN_FAIL = 8;
constexpr char SC_STAT_CHANGE = 9;

#pragma pack (push, 1)
struct PACKET
{
	unsigned char size;
	char	type;

	PACKET() { sizeof(PACKET); }
};

struct CS_LOGIN_PACKET : PACKET {
	char	name[NAME_SIZE];

	CS_LOGIN_PACKET() { sizeof(CS_LOGIN_PACKET); }
};

struct CS_MOVE_PACKET : PACKET {
	char	direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned	move_time;

	CS_MOVE_PACKET() { sizeof(CS_MOVE_PACKET); }
};

struct CS_CHAT_PACKET : PACKET {
	char	mess[CHAT_SIZE];

	CS_CHAT_PACKET() { sizeof(CS_CHAT_PACKET); }
};

struct CS_TELEPORT_PACKET : PACKET {

	CS_TELEPORT_PACKET() { sizeof(CS_TELEPORT_PACKET); }
};

struct CS_LOGOUT_PACKET : PACKET {
	unsigned char size;
	char	type;

	CS_LOGOUT_PACKET() { sizeof(CS_LOGOUT_PACKET); }
};

struct SC_LOGIN_INFO_PACKET : PACKET {
	int		id;
	int		hp;
	int		max_hp;
	int		exp;
	int		level;
	short	x, y;

	SC_LOGIN_INFO_PACKET() { sizeof(SC_LOGIN_INFO_PACKET); }
};

struct SC_ADD_OBJECT_PACKET : PACKET {
	int		id;
	short	x, y;
	char	name[NAME_SIZE];

	SC_ADD_OBJECT_PACKET() { sizeof(SC_ADD_OBJECT_PACKET); }
};

struct SC_REMOVE_OBJECT_PACKET : PACKET {
	int		id;

	SC_REMOVE_OBJECT_PACKET() { sizeof(SC_REMOVE_OBJECT_PACKET); }
};

struct SC_MOVE_OBJECT_PACKET : PACKET {
	int		id;
	short	x, y;
	unsigned int move_time;

	SC_MOVE_OBJECT_PACKET() { sizeof(SC_MOVE_OBJECT_PACKET); }
};

struct SC_CHAT_PACKET : PACKET {
	int		id;
	char	mess[CHAT_SIZE];

	SC_CHAT_PACKET() { sizeof(SC_CHAT_PACKET); }
};

struct SC_LOGIN_OK_PACKET : PACKET {

	SC_LOGIN_OK_PACKET() { sizeof(SC_LOGIN_OK_PACKET); }
};

struct SC_LOGIN_FAIL_PACKET : PACKET {

	SC_LOGIN_FAIL_PACKET() { sizeof(SC_LOGIN_FAIL_PACKET); }

};

struct SC_STAT_CHANGEL_PACKET : PACKET {
	int		hp;
	int		max_hp;
	int		exp;
	int		level;

	SC_STAT_CHANGEL_PACKET() { sizeof(SC_STAT_CHANGEL_PACKET); }
};

#pragma pack (pop)