#include "stdafx.h"

LPARAM OnClientSockMsg(WPARAM wParam, LPARAM lParam);
LPARAM OnLogSvrSockMsg(WPARAM wParam, LPARAM lParam);
UINT WINAPI ProcessUserHuman(LPVOID lpParameter);
UINT WINAPI ProcessRoom(LPVOID lpParameter);
BOOL	CALLBACK ConfigDlgFunc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

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


UINT WINAPI InitializingServer(LPVOID lpParameter)
{
	TCHAR		wszPath[128];
	TCHAR		wszFullPath[256];
	DWORD		dwReadLen;

	UINT			dwThreadIDForMsg = 0;
	unsigned long	hThreadForMsg = 0;
	//if (hThreadForMsg = _beginthreadex(NULL, 0, ProcessLogin, NULL, 0, &dwThreadIDForMsg))
	//{
	//hThreadForMsg = _beginthreadex(NULL, 0, ProcessUserHuman, NULL, 0, &dwThreadIDForMsg);
	hThreadForMsg = _beginthreadex(NULL, 0, ProcessRoom, NULL, 0, &dwThreadIDForMsg);
	//hThreadForMsg = _beginthreadex(NULL, 0, ProcessNPC, NULL, 0, &dwThreadIDForMsg);
	//}

	if (InitServerSocket(g_ssock, &g_saddr, _IDM_SERVERSOCK_MSG, g_nLocalPort, 1))
	{
		InsertLogMsg(IDS_STARTSERVICE);
		SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(0, 0), (LPARAM)_T("Ready"));
	}
	//InitAdminCommandList();
	//连接到数据库服务器 端口6000
	//ConnectToServer(g_csock, &g_caddr, _IDM_CLIENTSOCK_MSG, g_strDBSvrIP, NULL, g_nDBSrvPort, FD_CONNECT|FD_READ|FD_CLOSE);
	return 0L;
}

void OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
		case IDM_STARTSERVICE:
		{
			if (!TestDsn())
			{
				MessageBox(NULL, TEXT("数据源未配置正确,请先配置好数据源"), TEXT("失败"), MB_OK);
				return;
			}

			g_fTerminated = FALSE;

			//CMapInfo* pMapInfo = InitDataInDatabase();

			UINT			dwThreadIDForMsg = 0;
			unsigned long	hThreadForMsg = 0;

			hThreadForMsg = _beginthreadex(NULL, 0, InitializingServer, NULL, 0, &dwThreadIDForMsg);

			SwitchMenuItem(TRUE);

			return;
		}
		case IDM_STOPSERVICE:
		{
			g_fTerminated = TRUE;

			SwitchMenuItem(FALSE);
			return;
		}
		case IDM_CONFIG:
		{
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CONFIGDLG), NULL, (DLGPROC)ConfigDlgFunc);
	
			return;
		}

		case IDM_EXIT:
		{
			PostMessage(g_hMainWnd, WM_CLOSE, 0, 0);
			return;
		}

		default:
			printf("cmd:%d", LOWORD(wParam));
			break;
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

				ClearSocket(g_ssock);

				WSACleanup();

				CoUninitialize();

				PostQuitMessage(0);
			}

			return 0L;
		}
	}

	return (DefWindowProc(hWnd, nMsg, wParam, lParam));
}
