#include "stdafx.h"

// **************************************************************************************
//
//			Global Variables Definition
//
// **************************************************************************************

HINSTANCE		g_hInst			= NULL;			// Application instance
HWND			g_hMainWnd		= NULL;			// Main window handle
HWND			g_hLogMsgWnd	= NULL;
HWND			g_hToolBar		= NULL;

HWND			g_hStatusBar	= NULL;
int				g_nStatusPartsWidths[_NUMOFMAX_STATUS_PARTS] = { 10, 10, 10, -1 };

TBBUTTON tbButtons[] = 
{
	{ 0, IDM_STARTSERVICE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
	{ 1, IDM_STOPSERVICE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
	{ 0, 0,					0,					BTNS_SEP,		0L, 0},	
	{ 2, IDM_SETFONTCOLOR,	TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
	{ 3, IDM_SETBKGCOLOR,	TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0}
};

//CDatabase						g_MirDB;
//CConnection						*g_pConnCommon;
//CConnection						*g_pConnGame;

CWHList<char *>					g_xAdminCommandList;
CWHList<char *>					g_xUserCommandList;

CWHList<CUserInfo*>				g_xLoginOutUserInfo;
CWHList<CReadyUserInfo*>		g_xReadyUserInfoList;
CWHList<CReadyUserInfo2*>		g_xReadyUserInfoList2;
CWHList<CUserInfo*>				g_xReadyList;
CWHList<CGateInfo*>				g_xGateList;//�����б�.�����أ������Ϸ��֮��Ļ�ͨ.
CStaticArray<CUserInfo>			g_xUserInfoArr(_NUM_OF_MAXROOM * _NUM_OF_MAXPLAYER);//48*16
CStaticArray<CPlayerObject>		g_xPlayerObjectArr(_NUM_OF_MAXROOM * _NUM_OF_MAXPLAYER);//48*16
CStaticArray<CRoomInfo>			g_xRoom(_NUM_OF_MAXROOM);//���16������
CWHList<CUserInfo*>				g_xUserInfoList;

int								g_nMirDayTime = 1;


// General Standard Data
CWHList<CMirMap*>			g_xMirMapList;					// Map List
CWHList<CRoomInfo*>			g_xRoomList;

int							g_nNumOfMapInfo;

BOOL						g_fTerminated = FALSE;

SOCKET						g_ssock = INVALID_SOCKET;
SOCKADDR_IN					g_saddr;

SOCKET						g_csock = INVALID_SOCKET;
SOCKADDR_IN					g_caddr;

SOCKET						g_clsock = INVALID_SOCKET;
SOCKADDR_IN					g_claddr;

HANDLE							g_hThreadForComm = NULL;
HANDLE							g_hSvrMsgEvnt = NULL;
