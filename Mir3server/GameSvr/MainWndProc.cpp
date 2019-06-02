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
	UINT			dwThreadIDForMsg = 0;
	unsigned long	hThreadForMsg = 0;
	hThreadForMsg = _beginthreadex(NULL, 0, ProcessRoom, NULL, 0, &dwThreadIDForMsg);
	InitServerSocket(g_ssock, &g_saddr, _IDM_SERVERSOCK_MSG, g_nLocalPort, 1);
	return 0L;
}