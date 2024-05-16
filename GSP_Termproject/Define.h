#pragma once

constexpr int GAMESERVERPORT = 9000;
constexpr int BUF_SIZE = 256;
constexpr int MAXPLAYER = 50'000;

#define SINGLETON(classType)\
public:\
static classType* Instance()\
{\
	static classType inst;\
	return &inst;\
}	