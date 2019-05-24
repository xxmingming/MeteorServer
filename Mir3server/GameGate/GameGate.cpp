// LoginGate.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <DbgHelp.h>
#include <strsafe.h>
#include "TimerMng.h"
// **************************************************************************************

BOOL	InitApplication(HANDLE hInstance);
BOOL	InitInstance(HANDLE hInstance, int nCmdShow);
LPARAM	APIENTRY MainWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

BOOL	CALLBACK ConfigDlgFunc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// **************************************************************************************
//
//			Global Variables Definition
//
// **************************************************************************************

HINSTANCE		g_hInst = NULL;				// Application instance
HWND			g_hMainWnd = NULL;			// Main window handle
HWND			g_hLogMsgWnd = NULL;
HWND			g_hToolBar = NULL;
HWND			g_hStatusBar = NULL;

BOOL			g_fTerminated = FALSE;

//新增 2016-winson  begin

short			g_localPort = 7200;//对外 接受客户端连接
short			g_GameSvrPort = 5000;//对内 往游戏服转发数据.
string			g_strGameSvrIP;

Setting *		g_set = NULL;
//end

static WSADATA	g_wsd;

TBBUTTON tbButtons[] = 
{
	{ 0, IDM_STARTSERVICE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
	{ 1, IDM_STOPSERVICE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0}
};

// **************************************************************************************
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

	return GenerateMiniDump(NULL, lpExceptionInfo, L"GameGate");
}


#if defined (_LOG4CPP)
log4cpp::Category* main_log = &log4cpp::Category::getInstance("log");
void LogInit()
{
	//1. 初始化Layout对象
	log4cpp::PatternLayout* pLayout = new log4cpp::PatternLayout();
	pLayout->setConversionPattern("%d: %p %c %x: %m%n");
	// 2. 初始化一个appender 对象    
	log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", "./gamegate.log");
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

int main()
{
#if defined(_LOG4CPP)
	LogInit();
	print("gamegate start");
#endif
    MSG msg;
	//加入崩溃dump文件功能
	SetUnhandledExceptionFilter(ExceptionFilter);
	BOOL bRet = PreventSetUnhandledExceptionFilter();
	LoadConfig();

	if (WSAStartup(MAKEWORD(2, 2), &g_wsd) != 0)
		return (FALSE);

	TimerMng timer;
	timer.SetTimer(_ID_TIMER_CONNECTSERVER, 5000);
	DWORD tick = ::GetTickCount();
	
	//主线程主要负责
	while (true)
	{
		int elapsed = ::GetTickCount() - tick;
		tick = ::GetTickCount();
		timer.Update(elapsed);
		//主要定时器1，链接到游戏服，2，发送心跳包到游戏服
		//3启动IOCP线程获取完成端口对应的消息，4启动accept线程监听客户端的链接
		Sleep(1);
		
	}
	delete g_set;
	return 0;
}

void LoadConfig()
{
	if (g_set == NULL)
		g_set = new Setting(CFG_GAMEGATE);

	g_localPort = g_set->GetValueInt(DEFAULTSEC, "LocalPort");
	if (g_localPort == 0)
	{
		g_localPort = 7200;
		g_set->SetValueInt(DEFAULTSEC, "LocalPort", g_localPort);
	}

	g_strGameSvrIP = g_set->GetValueString(DEFAULTSEC, "GameSvrIP");
	if (g_strGameSvrIP == "")
	{
		g_strGameSvrIP = "127.0.0.1";
		g_set->SetValueString(DEFAULTSEC, "GameSvrIP", g_strGameSvrIP);
	}

	g_GameSvrPort = g_set->GetValueInt(DEFAULTSEC, "GameSvrPort");
	if (g_GameSvrPort == 0)
	{
		g_GameSvrPort = 5000;
		g_set->SetValueInt(DEFAULTSEC, "GameSvrPort", g_GameSvrPort);
	}
}

void SaveConfig()
{
	if (g_set != NULL)
	{
		g_set->SetValueInt(DEFAULTSEC, "LocalPort", g_localPort);
		g_set->SetValueString(DEFAULTSEC, "GameSvrIP", g_strGameSvrIP);
		g_set->SetValueInt(DEFAULTSEC, "GameSvrPort", g_GameSvrPort);
	}
}