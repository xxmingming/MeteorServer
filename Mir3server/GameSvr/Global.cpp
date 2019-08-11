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

CWHList<CEvent*>				g_xEventList;
CWHList<CEvent*>				g_xEventCloseList;



CWHList<CUserInfo*>				g_xLoginOutUserInfo;
CWHList<CReadyUserInfo*>		g_xReadyUserInfoList;
CWHList<CReadyUserInfo2*>		g_xReadyUserInfoList2;

BOOL							g_fInitMerchant = FALSE;

CWHList<CUserInfo*>				g_xReadyList;

CWHList<CGateInfo*>				g_xGateList;//网关列表.多网关，与多游戏服之间的互通.
CStaticArray<CUserInfo>			g_xUserInfoArr(_NUM_OF_MAXROOM * _NUM_OF_MAXPLAYER);//48*16
CStaticArray<CPlayerObject>		g_xPlayerObjectArr(_NUM_OF_MAXROOM * _NUM_OF_MAXPLAYER);//48*16
CStaticArray<CRoomInfo>			g_xRoom(_NUM_OF_MAXROOM);//最大96个房间
CWHList<CUserInfo*>				g_xUserInfoList;

int								g_nMirDayTime = 1;


// General Standard Data
CWHList<CMirMap*>			g_xMirMapList;					// Map List
CWHList<CRoomInfo*>			g_xRoomList;
CMoveMapEventInfo*			g_pMoveMapEventInfo = NULL;		// Map Event List
int							g_nNumOfMoveMapEventInfo = 0;
CMagicInfo*					g_pMagicInfo = NULL;			// Magic List
int							g_nNumOfMagicInfo = 0;
CMonsterGenInfo*			g_pMonGenInfo = NULL;			// Monster Gen List
int							g_nNumOfMonGenInfo = 0;
CMonRaceInfo*				g_pMonRaceInfo = NULL;			// Monster List
int							g_nNumOfMonRaceInfo = 0;
CStdItemSpecial*			g_pStdItemSpecial = NULL;		// Standard Item List
int							g_nStdItemSpecial = 0;
CStdItem*					g_pStdItemEtc = NULL;			// Standard General Item list
int							g_nStdItemEtc = 0;
CMerchantInfo*				g_pMerchantInfo = NULL;			// Merchant List
int							g_nNumOfMurchantInfo = 0;
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

CWHList<CScripterObject*>		g_xScripterList;
