// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#if !defined(UNICODE)
#define UNICODE
#endif
#if !defined(_UNICODE)
#define _UNICODE
#endif

#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#include <commctrl.h>
#include <time.h>
#include <process.h>
#include <stdio.h>

#include "resource.h"
#include <map>
#include <list>
using namespace std;
#include "..\Def\Setting.h"
#include "..\Def\Queue.h"
#include "..\Def\EnDecode.h"
#include "..\Def\ServerSockHandler.h"
#include "..\Def\Protocol.h"
#include "..\Def\DynamicArray.h"
#include "..\Def\Misc.h"
#include "..\Def\\protocol.pb.h"
#include "GameGate.h"

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "comctl32.lib")

// **************************************************************************************

#define _BMP_CX						16
#define _BMP_CY						16

#define _STATUS_HEIGHT				10
#define _NUMOFMAX_STATUS_PARTS		6

#define _GAMEGATE_SERVER_CLASS		_TEXT("GameGateServerClass")
#define _GAMEGATE_SERVER_TITLE		_TEXT("Legend of Mir II - Game Gate Server")
#define _GAMEGATE_SERVER_REGISTRY	_TEXT("Software\\LegendOfMir\\GameGate")

#define _IDM_SERVERSOCK_MSG			WM_USER + 1000
#define _IDM_CLIENTSOCK_MSG			_IDM_SERVERSOCK_MSG + 1

#define _IDW_TOOLBAR				_IDM_SERVERSOCK_MSG + 2
#define _IDW_STATUSBAR				_IDM_SERVERSOCK_MSG + 3

#define _ID_TIMER_KEEPALIVE			_IDM_SERVERSOCK_MSG + 4
#define _ID_TIMER_CONNECTSERVER		_IDM_SERVERSOCK_MSG + 5
#define CFG_GAMEGATE   "GameGate.ini"
#define DEFAULTSEC	"GameGate"

#if defined (_LOG4CPP)
#include "log4cpp/Category.hh"   
#include "log4cpp/FileAppender.hh"   
#include "log4cpp/PatternLayout.hh"
//#include "../Def/LuaMng.h"
using namespace log4cpp;
extern log4cpp::Category * main_log;
#define print(s)	main_log->debug("[File:%s/Line:%d]\t%s", __FILE__, __LINE__, s)
#define vprint(s, ...)   main_log->debug("[File:%s/Line:%d]\t"##s, __FILE__, __LINE__, __VA_ARGS__)
#else
#define print(s) wprintf(L"%s", s)
#define vprint(s, ...)
#endif
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
DWORD WINAPI	ThreadFuncForMsg(LPVOID lpParameter);
BOOL			CheckSocketError(LPARAM lParam);
VOID WINAPI		OnTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
BOOL InitServerThreadForMsg();
UINT WINAPI		ClientWorkerThread(LPVOID lpParameter);
extern WSAEVENT	g_ClientIoEvent;