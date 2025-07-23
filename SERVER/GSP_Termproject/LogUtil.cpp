#include "stdafx.h"
#include "Define.h"
#include "LogUtil.h"

#include <iostream>

using std::cout;
using std::endl;

void LogUtil::error_display(int err_no)
{
#ifdef _DEBUG
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, 0);
	std::wcout << lpMsgBuf << std::endl;
	//while (true);
	LocalFree(lpMsgBuf);
#endif // _DEBUG
}

void LogUtil::error_display(const char* msg)
{
#ifdef _DEBUG
    WCHAR* lpMsgBuf;
    int ErrorNum = WSAGetLastError();
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, ErrorNum,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    std::cout << msg << std::endl;;
    std::wcout << "[Error Num: " << ErrorNum << "]" << lpMsgBuf << std::endl;
    //while (true);
    // µð¹ö±ë ¿ë
    LocalFree(lpMsgBuf);
#endif // _DEBUG
}