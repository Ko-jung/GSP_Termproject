#include "DBMgr.h"

DBMgr::DBMgr()
{
    // Allocate environment handle  
    int retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

    // Set the ODBC version environment attribute  
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        std::cout << "SQLAllocHandle OK" << std::endl;
        retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

        // Allocate connection handle  
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
            std::cout << "SQLSetEnvAttr OK" << std::endl;
            retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

            // Set login timeout to 5 seconds  
            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                std::cout << "SQLAllocHandle OK" << std::endl;
                SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

                // Connect to data source  
                retcode = SQLConnect(hdbc, (SQLWCHAR*)L"GSP_Termproject_ODBC", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

                // Allocate statement handle  
                if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                    std::cout << "SQLConnect OK" << std::endl;
                    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
                }
            }
        }
    }
}

DBMgr::~DBMgr()
{
    SQLCancel(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

bool DBMgr::ExecLogin(const wchar_t* Query, const std::string& TargetID, SC_LOGIN_INFO_PACKET& SLIP)
{
    // hstmt는 THread Safe 하지 않으므로 전체를 감싼다
    std::lock_guard<std::mutex> lock(DBMutex);

    SQLRETURN ret;

    ret = SQLExecDirect(hstmt, (SQLWCHAR*)Query, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        print_err(hstmt, SQL_NTS, ret);
        SQLCloseCursor(hstmt);
    }

    SQLSMALLINT PosX, PosY;
    SQLINTEGER Visual;
    SQLWCHAR ID[NAME_SIZE];
    SQLUSMALLINT Level, HP, MaxHP, Exp;

    SQLLEN CBPosX, CBPosY, CBID;
    SQLLEN CBVisual;
    SQLLEN CBLevel, CBHP, CBMaxHP, CBExp;
    ret = SQLBindCol(hstmt, 1, SQL_C_WCHAR,     ID, NAME_SIZE * 2, &CBID);
    if (ret == SQL_ERROR)       print_err(hstmt, SQL_BIND_TYPE, ret);
    ret = SQLBindCol(hstmt, 2, SQL_C_SSHORT,    &PosX, sizeof(SQL_C_SSHORT), &CBPosX);
    if (ret == SQL_ERROR)       print_err(hstmt, SQL_BIND_TYPE, ret);
    ret = SQLBindCol(hstmt, 3, SQL_C_SSHORT,    &PosY, sizeof(SQL_C_SSHORT), &CBPosY);
    if (ret == SQL_ERROR)       print_err(hstmt, SQL_BIND_TYPE, ret);
    ret = SQLBindCol(hstmt, 4, SQL_C_SLONG,     &Visual, sizeof(SQL_C_SLONG), &CBVisual);
    if (ret == SQL_ERROR)       print_err(hstmt, SQL_BIND_TYPE, ret);
    ret = SQLBindCol(hstmt, 5, SQL_C_SSHORT,   &Level, sizeof(SQL_C_SSHORT), &CBLevel);
    if (ret == SQL_ERROR)       print_err(hstmt, SQL_BIND_TYPE, ret);
    ret = SQLBindCol(hstmt, 6, SQL_C_SSHORT,   &HP, sizeof(SQL_C_SSHORT), &CBHP);
    if (ret == SQL_ERROR)       print_err(hstmt, SQL_BIND_TYPE, ret);
    ret = SQLBindCol(hstmt, 7, SQL_C_SSHORT,   &MaxHP, sizeof(SQL_C_SSHORT), &CBMaxHP);
    if (ret == SQL_ERROR)       print_err(hstmt, SQL_BIND_TYPE, ret);
    ret = SQLBindCol(hstmt, 8, SQL_C_SSHORT,   &Exp, sizeof(SQL_C_SSHORT), &CBExp);
    if (ret == SQL_ERROR)       print_err(hstmt, SQL_BIND_TYPE, ret);

    for (int i = 0; ; i++) {
        // hstmt 에 들어있는 실행 결과값
        // 전용 버퍼에 속해있으며 BindCol로 꺼내올 수 있다
        ret = SQLFetch(hstmt);
        if (ret == SQL_ERROR)
        {
            continue;
        }
        if (ret == SQL_NO_DATA) break;
        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
        {
            // 정상 처리
            SLIP.x = PosX;
            SLIP.y = PosY;
            SLIP.visual = Visual;
            SLIP.hp = HP;
            SLIP.max_hp = MaxHP;
            SLIP.exp = Exp;
            SLIP.level = Level;
            SQLCloseCursor(hstmt);
            return true;
        }
        else
        {
            SQLCloseCursor(hstmt);
            return false;
        }
    }
    SQLCloseCursor(hstmt);
    return false;
}

void DBMgr::print_err(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
    SQLSMALLINT iRec = 0;
    SQLINTEGER  iError;
    WCHAR wszMessage[1000];
    WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
    if (RetCode == SQL_INVALID_HANDLE) {
        fwprintf(stderr, L"Invalid handle!\n");
        return;
    }
    while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
        (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
        // Hide data truncated..
        if (wcsncmp(wszState, L"01004", 5)) {
            fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
        }
    }
}