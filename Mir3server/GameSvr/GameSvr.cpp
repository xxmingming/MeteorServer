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

BOOL	InitApplication(HANDLE hInstance);
BOOL	InitInstance(HANDLE hInstance, int nCmdShow);
LPARAM	APIENTRY MainWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);


BOOL	CALLBACK ConfigDlgFunc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

static WSADATA	g_wsd;

WNDPROC			OrgLogMsgWndProc;

// **************************************************************************************

void __cbDBMsg( char *pState, int nNativeCode, char *pDesc )
{
	static TCHAR szState[256], szDesc[2048];
	static TCHAR szMsg[2048];

	MultiByteToWideChar( CP_ACP, 0, pState, -1, szState, sizeof( szState ) / sizeof( TCHAR ) );
	MultiByteToWideChar( CP_ACP, 0, pDesc, -1, szDesc, sizeof( szDesc ) / sizeof( TCHAR ) );
	
	wsprintf( szMsg, _T("ODBC MsgID: %s(%d)"), szState, nNativeCode );
	InsertLogMsg( szMsg );

	wsprintf( szMsg, _T("%s"), szDesc );
	InsertLogMsg( szMsg );
}

LPARAM APIENTRY LogMsgWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	if (nMsg == WM_ERASEBKGND)
	{
		RECT rc;

		GetClientRect(hWnd, &rc);

		FillRect((HDC)wParam, &rc, CreateSolidBrush(RGB(255, 0, 0)));
	}

	return CallWindowProc(OrgLogMsgWndProc, hWnd, nMsg, wParam, lParam);
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
				hDumpFile, MiniDumpWithDataSegs, (pExceptionPointers ? &ExpParam : NULL), NULL, NULL);

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

	return GenerateMiniDump(NULL, lpExceptionInfo, L"GameSvr");
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
	SetUnhandledExceptionFilter(ExceptionFilter);
	BOOL bRet = PreventSetUnhandledExceptionFilter();

	//SetUnhandledExceptionFilter(ExceptionFilter);
	//int* p = 0;
	//*p = 0;
	LoadConfig();
//	if (CheckAvailableIOCP())
//	{
		if (!InitApplication(hInstance))
			return (FALSE);

		if (!InitInstance(hInstance, nCmdShow))
			return (FALSE);

		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
/*	}
	else
	{
		TCHAR szMsg[1024];

		LoadString(hInstance, IDS_NOTWINNT, szMsg, sizeof(szMsg));
		MessageBox(NULL, szMsg, _LOGINGATE_SERVER_TITLE, MB_OK|MB_ICONINFORMATION);
		
		return -1;
	}
*/
	delete g_set;
    return (msg.wParam);
}

// **************************************************************************************
//
//			
//
// **************************************************************************************

BOOL InitApplication(HANDLE hInstance)
{
    WNDCLASS  wc;

    wc.style            = CS_GLOBALCLASS|CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc      = (WNDPROC)MainWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hIcon            = LoadIcon((HINSTANCE)hInstance, MAKEINTRESOURCE(IDI_MIR2));
    wc.hInstance        = (HINSTANCE)hInstance;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName     = MAKEINTRESOURCE(IDR_MAINMENU);
    wc.lpszClassName    = _GAME_SERVER_CLASS;

	return RegisterClass(&wc);
}

// **************************************************************************************
//
//			
//
// **************************************************************************************

BOOL InitInstance(HANDLE hInstance, int nCmdShow)
{
	g_hInst = (HINSTANCE)hInstance;
	
	OleInitialize(NULL);

	INITCOMMONCONTROLSEX	icex;

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_INTERNET_CLASSES;

	InitCommonControlsEx(&icex);

    g_hMainWnd = CreateWindowEx(0, _GAME_SERVER_CLASS, _GAME_SERVER_TITLE, 
							WS_OVERLAPPEDWINDOW|WS_VISIBLE,
							CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,                 
							NULL, NULL, (HINSTANCE)hInstance, NULL);

	g_hToolBar = CreateToolbarEx(g_hMainWnd, WS_CHILD|CCS_TOP|WS_VISIBLE|WS_BORDER,
									_IDW_TOOLBAR, sizeof(tbButtons) / sizeof(TBBUTTON), (HINSTANCE)hInstance, IDB_TOOLBAR,
									(LPCTBBUTTON)&tbButtons, sizeof(tbButtons) / sizeof(TBBUTTON),
									_BMP_CX, _BMP_CY, _BMP_CX, _BMP_CY, sizeof(TBBUTTON));

	RECT rcMainWnd, rcToolBar, rcStatusBar;

	GetClientRect(g_hMainWnd, &rcMainWnd);
	GetWindowRect(g_hToolBar, &rcToolBar);

	g_hStatusBar = CreateWindowEx(0L, STATUSCLASSNAME, _TEXT(""), WS_CHILD|WS_BORDER|WS_VISIBLE|SBS_SIZEGRIP,
									0, rcMainWnd.bottom - _STATUS_HEIGHT, (rcMainWnd.right - rcMainWnd.left), _STATUS_HEIGHT, g_hMainWnd, (HMENU)_IDW_STATUSBAR, g_hInst, NULL);

	int	nStatusPartsWidths[_NUMOFMAX_STATUS_PARTS];
	int nCnt = 0;
	int i;
	for (i = _NUMOFMAX_STATUS_PARTS - 1; i >= 0; i--)
		nStatusPartsWidths[nCnt++] = (rcMainWnd.right - rcMainWnd.left) - (90 * i);

	SendMessage(g_hStatusBar, SB_SETPARTS, _NUMOFMAX_STATUS_PARTS, (LPARAM)nStatusPartsWidths);

	GetWindowRect(g_hStatusBar, &rcStatusBar);

    g_hLogMsgWnd = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, _TEXT(""), 
							WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT|LVS_EDITLABELS,
							0, (rcToolBar.bottom - rcToolBar.top), (rcMainWnd.right - rcMainWnd.left), 
							(rcMainWnd.bottom - rcMainWnd.top) - (rcToolBar.bottom - rcToolBar.top) - (rcStatusBar.bottom - rcStatusBar.top),
							g_hMainWnd, NULL, (HINSTANCE)hInstance, NULL);

	ListView_SetExtendedListViewStyleEx(g_hLogMsgWnd, 0, LVS_EX_FULLROWSELECT);

	LV_COLUMN	lvc;
	TCHAR		szText[64];

	lvc.mask	= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt		= LVCFMT_LEFT;
	lvc.cx		= 150;
	lvc.pszText	= szText;

	for (i = 0; i < 3; i++)
	{
		lvc.iSubItem = i;
		LoadString((HINSTANCE)hInstance, IDS_LVS_LABEL1 + i, szText, sizeof(szText)/sizeof(TCHAR));
		
		ListView_InsertColumn(g_hLogMsgWnd, i, &lvc);
	}

	OrgLogMsgWndProc = (WNDPROC)SetWindowLong(g_hLogMsgWnd, GWL_WNDPROC, (LONG)LogMsgWndProc);

	SendMessage(g_hToolBar, TB_SETSTATE, (WPARAM)IDM_STOPSERVICE, (LPARAM)MAKELONG(TBSTATE_INDETERMINATE, 0));

	ShowWindow(g_hMainWnd, SW_SHOW);
	UpdateWindow(g_hMainWnd);

	if (WSAStartup(MAKEWORD(2, 2), &g_wsd) != 0)
		return (FALSE);

	srand((unsigned)time(NULL));
	//g_MirDB.SetDiagRec( __cbDBMsg );
	//g_MirDB.Init();
	
	//g_pConnCommon	= g_MirDB.CreateConnection(g_strDBSource.c_str(), g_strDBAccount.c_str(), g_strDBPassword.c_str());
	//g_pConnGame		= g_MirDB.CreateConnection(g_strDBSource.c_str(), g_strDBAccount.c_str(), g_strDBPassword.c_str());

	return TRUE;
}

// **************************************************************************************
//
//			
//
// **************************************************************************************

int AddNewLogMsg()
{
	LV_ITEM		lvi;
	TCHAR		szText[64];

	int nCount = ListView_GetItemCount(g_hLogMsgWnd);

	if (nCount >= 500)
	{
		ListView_DeleteItem(g_hLogMsgWnd, 0);
		nCount--;
	}

	lvi.mask		= LVIF_TEXT;
	lvi.iItem		= nCount;
	lvi.iSubItem	= 0;
	
	_tstrdate(szText);

	lvi.pszText = szText;
	
	ListView_InsertItem(g_hLogMsgWnd, &lvi);

	_tstrtime(szText);

	ListView_SetItemText(g_hLogMsgWnd, nCount, 1, szText);

	return nCount;
}

void InsertLogMsg(UINT nID)
{
	TCHAR	szText[256];

	int nCount = AddNewLogMsg();

	LoadString(g_hInst, nID, szText, sizeof(szText)/sizeof(TCHAR));

	ListView_SetItemText(g_hLogMsgWnd, nCount, 2, szText);
	ListView_Scroll(g_hLogMsgWnd, 0, 8);
}

void InsertLogMsg(LPTSTR lpszMsg)
{
	int nCount = AddNewLogMsg();

	ListView_SetItemText(g_hLogMsgWnd, nCount, 2, lpszMsg);
	ListView_Scroll(g_hLogMsgWnd, 0, 8);
}

void InsertLogMsgParam(UINT nID, void *pParam, BYTE btFlags)
{
	TCHAR	szText[128];
	TCHAR	szMsg[256];

	int nCount = AddNewLogMsg();

	LoadString(g_hInst, nID, szText, sizeof(szText)/sizeof(TCHAR));
	
	switch (btFlags)
	{
		case LOGPARAM_STR:
			_stprintf(szMsg, szText, (LPTSTR)pParam);
			break;
		case LOGPARAM_INT:
			_stprintf(szMsg, szText, *(int *)pParam);
			break;
	}

	if (lstrlen(szMsg) <= 256)
	{
		ListView_SetItemText(g_hLogMsgWnd, nCount, 2, szMsg);
		ListView_Scroll(g_hLogMsgWnd, 0, 8);
	}
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