#pragma once
#include "../stdafx.h"
#include "../Define.h"
#include "../../../Common/protocol.h"

#define UNICODE  
#include <windows.h>
#include <sqlext.h>

class DBMgr
{
public:
	SINGLETON(DBMgr);

public:
	DBMgr();
	~DBMgr();

	bool ExecLogin(const wchar_t* Query, const std::string& TargetID, SC_LOGIN_INFO_PACKET& SLIP);

	void print_err(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
};

