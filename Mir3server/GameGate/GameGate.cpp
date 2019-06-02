// LoginGate.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <DbgHelp.h>
#include <strsafe.h>
#include "TimerMng.h"
CWHDynamicArray<tag_TSENDBUFF> g_memPool;
BOOL			g_fTerminated = FALSE;
short			g_localPort = 7200;//���� ���ܿͻ�������
short			g_GameSvrPort = 5000;//���� ����Ϸ��ת������.
string			g_strGameSvrIP;
Setting *		g_set = NULL;
static WSADATA	g_wsd;

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

int GenerateMiniDump(HANDLE hFile, PEXCEPTION_POINTERS pExceptionPointers, PWCHAR pwAppName)
{
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
			(_tcsrchr(szPath, _T('\\')))[1] = 0;
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
	//1. ��ʼ��Layout����
	log4cpp::PatternLayout* pLayout = new log4cpp::PatternLayout();
	pLayout->setConversionPattern("%d: %p %c %x: %m%n");
	// 2. ��ʼ��һ��appender ����    
	log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", "./gamegate.log");
	// 3. ��layout��������appender������    
	appender->setLayout(pLayout);
	// 5. ����additivityΪfalse���滻���е�appender           
	main_log->setAdditivity(false);
	// 5. ��appender���󸽵�category��    
	main_log->setAppender(appender);
	// 6. ����category�����ȼ������ڴ����ȼ�����־������¼    
	main_log->setPriority(log4cpp::Priority::DEBUG);
}
#endif

int main()
{
#if defined(_LOG4CPP)
	LogInit();
	print("gamegate start");
#endif
	SetUnhandledExceptionFilter(ExceptionFilter);
	BOOL bRet = PreventSetUnhandledExceptionFilter();
	LoadConfig();

	if (WSAStartup(MAKEWORD(2, 2), &g_wsd) != 0)
		return (FALSE);

	TimerMng timer;
	timer.SetTimer(_ID_TIMER_CONNECTSERVER, 5000);
	DWORD tick = ::GetTickCount();
	
	while (true)
	{
		int elapsed = ::GetTickCount() - tick;
		tick = ::GetTickCount();
		timer.Update(elapsed);
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