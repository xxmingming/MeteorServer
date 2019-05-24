// GameSvr.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <DbgHelp.h>
#include <strsafe.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#ifndef _M_IX86
#error "The following code only works for x86!"
#endif
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(
	LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	return NULL;
}

BOOL PreventSetUnhandledExceptionFilter()
{
	HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
	if (hKernel32 == NULL)
	{
		::MessageBoxA(0, "Cannot find kernel32.dll", "?", MB_OK);
		return FALSE;
	}
	void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
	if (pOrgEntry == NULL) return FALSE;
	unsigned char newJump[100];
	DWORD dwOrgEntryAddr = (DWORD)pOrgEntry;
	dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far
	void *pNewFunc = &MyDummySetUnhandledExceptionFilter;
	DWORD dwNewEntryAddr = (DWORD)pNewFunc;
	DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;

	newJump[0] = 0xE9;  // JMP absolute
	memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
	SIZE_T bytesWritten;
	BOOL bRet = WriteProcessMemory(GetCurrentProcess(),
		pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
	return bRet;
}

//LONG WINAPI MyUnhandledExceptionFilter(
//	struct _EXCEPTION_POINTERS *lpTopLevelExceptionFilter)
//{
//	// TODO: MiniDumpWriteDump
//	FatalAppExit(0, _T("Unhandled Exception occured!"));
//	return EXCEPTION_CONTINUE_SEARCH;
//}

//int _tmain()
//{
//	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
//	BOOL bRet = PreventSetUnhandledExceptionFilter();
//	_tprintf(_T("Prevented: %d"), bRet);
//
//	abort();  // force Dr.Watson in release!
//}


#define _BMP_CX						16
#define _BMP_CY						16

Setting * g_set;
//LuaMng				g_xLuaMng;
//提供给GameSrv的端口7200
short			g_nLocalPort = 5000;
//提供给角色网关的端口5100
//short			g_nDBSrvPort = 6000;
//int				g_nStartLevel = 0;
//int				g_nStartGold = 1000;
//BOOL			g_fTestServer = FALSE;
//char			g_strDBSvrIP[20];
char			g_strClientPath[MAX_PATH];
//string			g_strDBSource;
//string			g_strDBAccount;
//string			g_strDBPassword;
// **************************************************************************************

//生产DUMP文件
int GenerateMiniDump(HANDLE hFile, PEXCEPTION_POINTERS pExceptionPointers, PWCHAR pwAppName)
{
	//::MessageBoxA(0, "a", "b", MB_OK);
	BOOL bOwnDumpFile = FALSE;
	HANDLE hDumpFile = hFile;
	MINIDUMP_EXCEPTION_INFORMATION ExpParam;

	typedef BOOL(WINAPI * MiniDumpWriteDumpT)(
		HANDLE,
		DWORD,
		HANDLE,
		MINIDUMP_TYPE,
		PMINIDUMP_EXCEPTION_INFORMATION,
		PMINIDUMP_USER_STREAM_INFORMATION,
		PMINIDUMP_CALLBACK_INFORMATION
		);

	MiniDumpWriteDumpT pfnMiniDumpWriteDump = NULL;
	HMODULE hDbgHelp = LoadLibrary(L"DbgHelp.dll");
	if (hDbgHelp)
		pfnMiniDumpWriteDump = (MiniDumpWriteDumpT)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
	//else
	//	::MessageBoxA(0, "a", "c", MB_OK);
	if (pfnMiniDumpWriteDump)
	{
		if (hDumpFile == NULL || hDumpFile == INVALID_HANDLE_VALUE)
		{
			TCHAR szPath[MAX_PATH] = { 0 };
			TCHAR szFileName[MAX_PATH] = { 0 };
			TCHAR* szAppName = pwAppName;
			TCHAR* szVersion = L"v1.0";
			TCHAR dwBufferSize = MAX_PATH;
			SYSTEMTIME stLocalTime;

			GetModuleFileName(NULL, szPath, MAX_PATH);
			(_tcsrchr(szPath, _T('\\')))[1] = 0; // 删除文件名，只获得路径字串
			GetLocalTime(&stLocalTime);
			StringCchPrintf(szFileName, MAX_PATH, L"%s%s_%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp",
				szPath, szAppName, szVersion,
				stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
				stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,
				GetCurrentProcessId(), GetCurrentThreadId());
			hDumpFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

			bOwnDumpFile = TRUE;
			//MessageBoxW(0, szFileName, L"aaa", MB_OK);
			OutputDebugString(szFileName);
		}

		if (hDumpFile != INVALID_HANDLE_VALUE)
		{
			ExpParam.ThreadId = GetCurrentThreadId();
			ExpParam.ExceptionPointers = pExceptionPointers;
			ExpParam.ClientPointers = FALSE;

			pfnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
				hDumpFile, MiniDumpWithFullMemory, (pExceptionPointers ? &ExpParam : NULL), NULL, NULL);

			if (bOwnDumpFile)
				CloseHandle(hDumpFile);
		}
	}

	if (hDbgHelp != NULL)
		FreeLibrary(hDbgHelp);

	return EXCEPTION_EXECUTE_HANDLER;
}


LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo)
{
	if (IsDebuggerPresent())
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}
	ClearSocket(g_ssock);
	WSACleanup();
	return GenerateMiniDump(NULL, lpExceptionInfo, L"GameSvr");
}



#if defined (_LOG4CPP)
log4cpp::Category* main_log = &log4cpp::Category::getInstance("log");
void LogInit()
{
	//1. 初始化Layout对象
	log4cpp::PatternLayout* pLayout = new log4cpp::PatternLayout();
	pLayout->setConversionPattern("%d: %p %c %x: %m%n");
	// 2. 初始化一个appender 对象    
	log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", "./gamesvr.log");
	// 3. 把layout对象附着在appender对象上    
	appender->setLayout(pLayout);
	// 5. 设置additivity为false，替换已有的appender           
	main_log->setAdditivity(false);
	// 5. 把appender对象附到category上    
	main_log->setAppender(appender);
	// 6. 设置category的优先级，低于此优先级的日志不被记录    
	main_log->setPriority(log4cpp::Priority::DEBUG);
}
#endif

CMapInfo* InitMapInfo();

void LoadMap(CMapInfo* pMapInfo);

CMapInfo* InitDataInDatabase()
{
	return InitMapInfo();
}

//加载地图数据，地图信息后期不再放在数据库内
CMapInfo* InitMapInfo()
{
	//string s = "sn";
	g_nNumOfMapInfo = 30;
	CMapInfo* pMapInfo = new CMapInfo[g_nNumOfMapInfo];
	char * s = (char*)malloc(14);
	strcpy(s, "01");

	//s[1] = "钟乳洞";
	//s[2] = "一线天";
	for (int i = 0; i < g_nNumOfMapInfo; i++)
	{
		pMapInfo[i].m_nLevelIdx = i + 1;//地图ID
	}
	return pMapInfo;
}

void InitRoom()
{
	PLISTNODE p = g_xMirMapList.GetHead();
	while (p != NULL)
	{
		CMirMap * pMap = g_xMirMapList.GetData(p);
		int RoomIdx = g_xRoom.GetFreeKey();
		//房间数量受数组限制，可能返回RoomIdx重复
		if (RoomIdx >= 0)
		{
			CRoomInfo * pRoom = &g_xRoom[RoomIdx];
			pRoom->CreateRoom(pMap, 16, 2000, 30 * 60 * 1000, RoomIdx);
			g_xRoomList.AddNewNode(pRoom);
		}
		p = g_xMirMapList.GetNext(p);
	}
}


UINT WINAPI ProcessRoom(LPVOID lpParameter)
{
	PLISTNODE pListNode = NULL;
	while (TRUE)
	{
		if (g_fTerminated)
			return 0L;

		if (g_xRoomList.GetCount())
		{
			pListNode = g_xRoomList.GetHead();

			while (pListNode)
			{
				CRoomInfo *pRoomInfo = g_xRoomList.GetData(pListNode);

				if (pRoomInfo && !pRoomInfo->IsEmpty())
				{
					pRoomInfo->Update();//第一次是0，第二次开始
				}

				pListNode = g_xRoomList.GetNext(pListNode);
			} // while
		} // if g_xReadyUserInfoList.GetCount()

		if (g_xGateList.GetCount())
		{
			pListNode = g_xGateList.GetHead();

			while (pListNode)
			{
				CGateInfo *pGateInfo = g_xGateList.GetData(pListNode);

				if (pGateInfo)
					pGateInfo->xSend();

				pListNode = g_xUserInfoList.GetNext(pListNode);
			} // while
		}

		SleepEx(1, TRUE);
	}
}

UINT WINAPI InitializingServer(LPVOID lpParameter)
{
	TCHAR		wszPath[128];
	TCHAR		wszFullPath[256];
	DWORD		dwReadLen;
	//MultiByteToWideChar(CP_ACP, 0, g_strClientPath, -1, wszPath, sizeof(wszPath) / sizeof(TCHAR));
	CMapInfo* pMapInfo = (CMapInfo*)lpParameter;

	for (int i = 0; i < g_nNumOfMapInfo; i++)
		LoadMap(&pMapInfo[i]);

	delete[] pMapInfo;
	pMapInfo = NULL;

	InitRoom();

	UINT			dwThreadIDForMsg = 0;
	unsigned long	hThreadForMsg = 0;
	//if (hThreadForMsg = _beginthreadex(NULL, 0, ProcessLogin, NULL, 0, &dwThreadIDForMsg))
	//{
	//hThreadForMsg = _beginthreadex(NULL, 0, ProcessUserHuman, NULL, 0, &dwThreadIDForMsg);
	hThreadForMsg = _beginthreadex(NULL, 0, ProcessRoom, NULL, 0, &dwThreadIDForMsg);
	//hThreadForMsg = _beginthreadex(NULL, 0, ProcessNPC, NULL, 0, &dwThreadIDForMsg);
	//}

	InitServerSocket(g_ssock, &g_saddr, g_nLocalPort);

	//InitAdminCommandList();
	//连接到数据库服务器 端口6000
	//ConnectToServer(g_csock, &g_caddr, _IDM_CLIENTSOCK_MSG, g_strDBSvrIP, NULL, g_nDBSrvPort, FD_CONNECT|FD_READ|FD_CLOSE);
	return 0L;
}

int main()
{
	//初始化日志系统
#if defined(_LOG4CPP)
	LogInit();
#endif

	print(L"game svr start");
	//初始化脚本系统
	//g_xLuaMng.Init();
	SetUnhandledExceptionFilter(ExceptionFilter);
	BOOL bRet = PreventSetUnhandledExceptionFilter();

	//SetUnhandledExceptionFilter(ExceptionFilter);
	//int* p = 0;
	//*p = 0;
	LoadConfig();
	WSADATA	g_wsd;
	if (WSAStartup(MAKEWORD(2, 2), &g_wsd) != 0)
		return (FALSE);

	g_fTerminated = FALSE;

	CMapInfo* pMapInfo = InitDataInDatabase();

	UINT			dwThreadIDForMsg = 0;
	unsigned long	hThreadForMsg = 0;

	hThreadForMsg = _beginthreadex(NULL, 0, InitializingServer, pMapInfo, 0, &dwThreadIDForMsg);

	while (1)
	{
		Sleep(1);
	}
		//if (!InitApplication(hInstance))
		//	return (FALSE);

		//if (!InitInstance(hInstance, nCmdShow))
		//	return (FALSE);

	delete g_set;
    return 0;
}

void LoadConfig()
{
	if (g_set == NULL)
		g_set = new Setting(CFG_GameSvr);
	g_nLocalPort = g_set->GetValueInt(DEFAULTSECT, "LocalPort");
	if (g_nLocalPort == 0)
		g_nLocalPort = 5000;
	//g_nDBSrvPort = g_set->GetValueInt(DEFAULTSECT, "DBPort");
	//if (g_nDBSrvPort == 0)
		//g_nDBSrvPort = 6000;

	g_set->SetValueInt(DEFAULTSECT, "LocalPort", g_nLocalPort);

	string strRemoteIP;
	strRemoteIP = g_set->GetValueString(DEFAULTSECT, "DBSvrIP");
	/*if (strRemoteIP != "")
		sprintf(g_strDBSvrIP, "%s", strRemoteIP.c_str());
	else
	{
		strRemoteIP = "127.0.0.1";
		sprintf(g_strDBSvrIP, "%s", strRemoteIP.c_str());
		g_set->SetValueString(DEFAULTSECT, "DBSvrIP", strRemoteIP);
	}*/

	string strPath;
	strPath = g_set->GetValueString(DEFAULTSECT, "ClientPath");
	if (strPath != "")
		sprintf(g_strClientPath, "%s", strPath.c_str());
	else
	{
		strPath = "D:/Meteor33";
		sprintf(g_strClientPath, "%s", strPath.c_str());
		g_set->SetValueString(DEFAULTSECT, "MapPath", strPath);
	}

	//g_strDBSource = g_set->GetValueString(DEFAULTSECT, "DBSource");
	//g_strDBAccount = g_set->GetValueString(DEFAULTSECT, "DBAccount");
	//g_strDBPassword = g_set->GetValueString(DEFAULTSECT, "DBPassword");
	/*if (g_strDBSource == "")
		g_strDBSource = "Mysql5.1";
	if (g_strDBAccount == "")
		g_strDBAccount = "sure";
	if (g_strDBPassword == "")
		g_strDBPassword = "sure1013";
	g_set->SetValueString(DEFAULTSECT, "DBSource", g_strDBSource);
	g_set->SetValueString(DEFAULTSECT, "DBAccount", g_strDBAccount);
	g_set->SetValueString(DEFAULTSECT, "DBPassword", g_strDBPassword);*/

	//g_nStartLevel = g_set->GetValueInt(DEFAULTSECT, "StartLevel");
	//g_nStartGold = g_set->GetValueInt(DEFAULTSECT, "StartGold");
}

void SaveConfig()
{
	if (g_set != NULL)
	{
		g_set->SetValueInt(DEFAULTSECT, "LocalPort", g_nLocalPort);
		//g_set->SetValueInt(DEFAULTSECT, "DBPort", g_nDBSrvPort);
		
		//g_set->SetValueString(DEFAULTSECT, "DBSvrIP", g_strDBSvrIP);
		g_set->SetValueString(DEFAULTSECT, "ClientPath", g_strClientPath);

		/*g_set->SetValueString(DEFAULTSECT, "DBSource", g_strDBSource);
		g_set->SetValueString(DEFAULTSECT, "DBAccount", g_strDBAccount);
		g_set->SetValueString(DEFAULTSECT, "DBPassword", g_strDBPassword);

		g_set->SetValueInt(DEFAULTSECT, "StartLevel", g_nStartLevel);*/
		//g_set->SetValueInt(DEFAULTSECT, "StartGold", g_nStartGold);
	}
}

bool TestDsn()
{
	//CConnection *pConn = NULL;
	////pConn = g_MirDB.CreateConnection(g_strDBSource.c_str(), g_strDBAccount.c_str(), g_strDBPassword.c_str());
	//if (!pConn)
	//	return false;
	//else
	//{
	//	SaveConfig();
	//	g_MirDB.DestroyConnection(pConn);
	//}
	return true;
}