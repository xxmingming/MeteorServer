#include "stdafx.h"

#define PACKET_KEEPALIVE		"%--$"

extern HWND				g_hToolBar;
extern HWND				g_hStatusBar;

extern SOCKET			g_csock;
extern SOCKADDR_IN		g_caddr;

BOOL	jRegGetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, LPBYTE pValue);

VOID WINAPI OnTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	switch (idEvent)
	{
		case _ID_TIMER_KEEPALIVE:
		{
			if (g_csock != INVALID_SOCKET)
			{
				SendSocketMsgS(GM_CHECKCLIENT, 0, 0, 0, 0, NULL);
				SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(2, 0), (LPARAM)_TEXT("Check Activity"));
			}

			break;
		}
		case _ID_TIMER_CONNECTSERVER:
		{
			if (g_csock == INVALID_SOCKET)
			{
				InsertLogMsg(IDS_APPLY_RECONNECT);
				ConnectToServer(g_csock, &g_caddr, _IDM_CLIENTSOCK_MSG, g_strGameSvrIP.c_str(), NULL, g_GameSvrPort, FD_CONNECT|FD_READ|FD_CLOSE);
			}

			break;
		}
	}
}
