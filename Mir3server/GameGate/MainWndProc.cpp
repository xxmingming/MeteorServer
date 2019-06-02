#include "stdafx.h"

LPARAM OnClientSockMsg(WPARAM wParam, LPARAM lParam);
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
