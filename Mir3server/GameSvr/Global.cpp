#include "stdafx.h"

//CDatabase						g_MirDB;
//CConnection						*g_pConnCommon;
//CConnection						*g_pConnGame;

//CWHList<char *>					g_xAdminCommandList;
//CWHList<char *>					g_xUserCommandList;
//
//CWHList<CEvent*>				g_xEventList;
//CWHList<CEvent*>				g_xEventCloseList;


//排队的所有玩家.
CWHList<CUserInfo*>				g_xEnQueueUserInfo;
//队伍消息回复了准备确认，全部人都回复准备才可继续下一步的配置.
CWHList<CUserInfo*>				g_xReadyList;
//分到一个队伍中的				
CWHList<CGroup*>				g_xGroupList;

CWHList<CGateInfo*>				g_xGateList;
CStaticArray<CUserInfo>			g_xUserInfoArr(_NUM_OF_MAXROOM * _NUM_OF_MAXPLAYER);//48*16
CStaticArray<CPlayerObject>		g_xPlayerObjectArr(_NUM_OF_MAXROOM * _NUM_OF_MAXPLAYER);//48*16
CStaticArray<CRoomInfo>			g_xRoom(_NUM_OF_MAXROOM);
CWHList<CUserInfo*>				g_xUserInfoList;

CWHList<CRoomInfo*>			g_xRoomList;

BOOL						g_fTerminated = FALSE;

SOCKET						g_ssock = INVALID_SOCKET;
SOCKADDR_IN					g_saddr;

SOCKET						g_csock = INVALID_SOCKET;
SOCKADDR_IN					g_caddr;

SOCKET						g_clsock = INVALID_SOCKET;
SOCKADDR_IN					g_claddr;
