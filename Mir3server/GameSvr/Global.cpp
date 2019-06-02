#include "stdafx.h"


//CDatabase						g_MirDB;
//CConnection						*g_pConnCommon;
//CConnection						*g_pConnGame;

CWHList<char *>					g_xAdminCommandList;
CWHList<char *>					g_xUserCommandList;

CWHList<CUserInfo*>				g_xLoginOutUserInfo;
CWHList<CReadyUserInfo*>		g_xReadyUserInfoList;
CWHList<CReadyUserInfo2*>		g_xReadyUserInfoList2;
CWHList<CUserInfo*>				g_xReadyList;
CWHList<CGateInfo*>				g_xGateList;//网关列表.多网关，与多游戏服之间的互通.
CStaticArray<CUserInfo>			g_xUserInfoArr(_NUM_OF_MAXROOM * _NUM_OF_MAXPLAYER);//48*16
CStaticArray<CPlayerObject>		g_xPlayerObjectArr(_NUM_OF_MAXROOM * _NUM_OF_MAXPLAYER);//48*16
CStaticArray<CRoomInfo>			g_xRoom(_NUM_OF_MAXROOM);//最大16个房间
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
