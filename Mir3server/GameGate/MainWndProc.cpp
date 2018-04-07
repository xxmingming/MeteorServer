#include "stdafx.h"

LPARAM OnServerSockMsg(WPARAM wParam, LPARAM lParam);
LPARAM OnClientSockMsg(WPARAM wParam, LPARAM lParam);

BOOL CALLBACK ConfigDlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern HINSTANCE		g_hInst;

extern HWND				g_hMainWnd;
extern HWND				g_hLogMsgWnd;
extern HWND				g_hToolBar;
extern HWND				g_hStatusBar;

extern HANDLE			g_hThreadForComm;

SOCKET			g_ssock = INVALID_SOCKET;
SOCKADDR_IN		g_saddr;

SOCKET			g_csock = INVALID_SOCKET;
SOCKADDR_IN		g_caddr;

#ifndef _SOCKET_ASYNC_IO
extern HANDLE					g_hIOCP;
#endif

extern CWHAbusive	g_xAbusive;

void SwitchMenuItem(BOOL fFlag)
{
	HMENU hMainMenu = GetMenu(g_hMainWnd);
	HMENU hMenu = GetSubMenu(hMainMenu, 0);

	if (fFlag)
	{
		EnableMenuItem(hMenu, IDM_STARTSERVICE, MF_GRAYED|MF_BYCOMMAND);
		EnableMenuItem(hMenu, IDM_STOPSERVICE, MF_ENABLED|MF_BYCOMMAND);

		SendMessage(g_hToolBar, TB_SETSTATE, (WPARAM)IDM_STARTSERVICE, (LPARAM)MAKELONG(TBSTATE_INDETERMINATE, 0));
		SendMessage(g_hToolBar, TB_SETSTATE, (WPARAM)IDM_STOPSERVICE, (LPARAM)MAKELONG(TBSTATE_ENABLED, 0));

		InsertLogMsg(IDS_STARTSERVICE);
	}
	else
	{
		EnableMenuItem(hMenu, IDM_STARTSERVICE, MF_ENABLED|MF_BYCOMMAND);
		EnableMenuItem(hMenu, IDM_STOPSERVICE, MF_GRAYED|MF_BYCOMMAND);

		SendMessage(g_hToolBar, TB_SETSTATE, (WPARAM)IDM_STARTSERVICE, (LPARAM)MAKELONG(TBSTATE_ENABLED, 0));
		SendMessage(g_hToolBar, TB_SETSTATE, (WPARAM)IDM_STOPSERVICE, (LPARAM)MAKELONG(TBSTATE_INDETERMINATE, 0));

		InsertLogMsg(IDS_STOPSERVICE);
	}
}

void OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
		case IDM_STARTSERVICE:
		{
			g_fTerminated = FALSE;

			InitServerSocket(g_ssock, &g_saddr, _IDM_SERVERSOCK_MSG, g_localPort, 2);
			ConnectToServer(g_csock, &g_caddr, _IDM_CLIENTSOCK_MSG, g_strGameSvrIP.c_str(), NULL, g_GameSvrPort, FD_CONNECT|FD_READ|FD_CLOSE);

			SwitchMenuItem(TRUE);

			SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(0, 0), (LPARAM)_TEXT("Ready"));

			g_xAbusive.LoadAbusiveList();

			return;
		}
		case IDM_STOPSERVICE:
		{
			g_fTerminated = TRUE;

			KillTimer(g_hMainWnd, _ID_TIMER_CONNECTSERVER);
			ClearSocket(g_ssock);
			ClearSocket(g_csock);
			CloseHandle(g_hIOCP);
			SwitchMenuItem(FALSE);

			SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(0, 0), (LPARAM)_TEXT(""));

			return;
		}
		case IDM_CONFIG:
		{
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CONFIGDLG), g_hMainWnd, (DLGPROC)ConfigDlgFunc);

			return;
		}
	}
}

// **************************************************************************************
//
//			
//
// **************************************************************************************

LPARAM APIENTRY MainWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
#ifdef _SOCKET_ASYNC_IO
		case _IDM_SERVERSOCK_MSG:
			return OnServerSockMsg(wParam, lParam);
#endif
		case _IDM_CLIENTSOCK_MSG:
			return OnClientSockMsg(wParam, lParam);
		case WM_COMMAND:
			OnCommand(wParam, lParam);
			break;
		case WM_SIZE:
		{
			if (g_hToolBar && g_hMainWnd && g_hStatusBar) 
			{
				RECT rcToolBar, rcMain, rcStatusBar;

				GetWindowRect(g_hToolBar, &rcToolBar);
				GetClientRect(g_hMainWnd, &rcMain);
				GetWindowRect(g_hStatusBar, &rcStatusBar);

				MoveWindow(g_hToolBar, 0, 0, LOWORD(lParam), (rcToolBar.bottom - rcToolBar.top), TRUE);
				MoveWindow(g_hStatusBar, 0, rcMain.bottom - (rcStatusBar.bottom - rcStatusBar.top), 
								LOWORD(lParam), (rcStatusBar.bottom - rcStatusBar.top), TRUE);
				MoveWindow(g_hLogMsgWnd, 0, (rcToolBar.bottom - rcToolBar.top), (rcMain.right - rcMain.left), 
								(rcMain.bottom - rcMain.top) - (rcToolBar.bottom - rcToolBar.top) - (rcStatusBar.bottom - rcStatusBar.top), 
								TRUE);

				int	nStatusPartsWidths[_NUMOFMAX_STATUS_PARTS];
				int nCnt = 0;

				for (int i = _NUMOFMAX_STATUS_PARTS - 1; i >= 0 ; i--)
					nStatusPartsWidths[nCnt++] = (rcStatusBar.right - rcStatusBar.left) - (90 * i);

				SendMessage(g_hStatusBar, SB_SETPARTS, _NUMOFMAX_STATUS_PARTS, (LPARAM)nStatusPartsWidths);
			}

			break;
		}
		case WM_CLOSE:
		{
			TCHAR	szMsg[128];
			TCHAR	szTitle[128];

			LoadString(g_hInst, IDS_PROGRAM_QUIT, szMsg, sizeof(szMsg));
			LoadString(g_hInst, IDS_PROGRAM_TITLE, szTitle, sizeof(szTitle));

			if (MessageBox(g_hMainWnd, szMsg, szTitle, MB_ICONSTOP|MB_YESNO) == IDYES)
			{
				if (SendMessage(g_hToolBar, TB_GETSTATE, (WPARAM)IDM_STARTSERVICE, (LPARAM)0L) == TBSTATE_INDETERMINATE)
					OnCommand(IDM_STOPSERVICE, 0L);

				WSACleanup();

				PostQuitMessage(0);
			}

			return 0L;
		}
	}

	return (DefWindowProc(hWnd, nMsg, wParam, lParam));
}
