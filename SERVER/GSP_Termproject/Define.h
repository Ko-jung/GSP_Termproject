#pragma once

#define SINGLETON(classType)\
public:\
static classType* Instance()\
{\
	static classType inst;\
	return &inst;\
}	