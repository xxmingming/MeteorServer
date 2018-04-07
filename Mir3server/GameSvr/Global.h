#pragma once

extern HANDLE						g_hIOCP;			// Defined in /Def/ServerSockHandler.cpp

extern HINSTANCE					g_hInst;
extern HWND							g_hMainWnd;
extern HWND							g_hLogMsgWnd;
extern HWND							g_hToolBar;

extern HWND							g_hStatusBar;
extern int							g_nStatusPartsWidths[_NUMOFMAX_STATUS_PARTS];

extern TBBUTTON						tbButtons[5]; 

extern CDatabase					g_MirDB;
//extern CConnection					*g_pConnCommon;
extern CConnection					*g_pConnGame;

extern CWHList<char *>				g_xAdminCommandList;
extern CWHList<char *>				g_xUserCommandList;


extern CWHList<CUserInfo*>			g_xLoginOutUserInfo;
extern CWHList<CUserInfo*>			g_xReadyList;
extern CWHList<CUserInfo*>			g_xUserInfoList;
extern CStaticArray<CUserInfo>		g_xUserInfoArr;
extern CStaticArray<CPlayerObject>	g_xPlayerObjectArr;
extern CStaticArray<CRoomInfo>		g_xRoom;
extern CWHList<CReadyUserInfo*>		g_xReadyUserInfoList;
extern CWHList<CReadyUserInfo2*>	g_xReadyUserInfoList2;

extern CWHList<CEvent*>				g_xEventList;
extern CWHList<CEvent*>				g_xEventCloseList;

extern CWHList<CGateInfo*>			g_xGateList;

extern int							g_nMirDayTime;


// General Standard Data
extern CWHList<CMirMap*>			g_xMirMapList;
extern CWHList<CRoomInfo*>			g_xRoomList;
extern CMoveMapEventInfo*			g_pMoveMapEventInfo;
extern int							g_nNumOfMoveMapEventInfo;
extern CMagicInfo*					g_pMagicInfo;
extern int							g_nNumOfMagicInfo;
extern CMonsterGenInfo*				g_pMonGenInfo;
extern int							g_nNumOfMonGenInfo;
extern CMonRaceInfo*				g_pMonRaceInfo;
extern int							g_nNumOfMonRaceInfo;
extern CStdItemSpecial*				g_pStdItemSpecial;
extern int							g_nStdItemSpecial;
extern CStdItem*					g_pStdItemEtc;
extern int							g_nStdItemEtc;
extern CMerchantInfo*				g_pMerchantInfo;
extern int							g_nNumOfMurchantInfo;
extern int							g_nNumOfMapInfo;

extern BOOL							g_fTerminated;

extern SOCKET						g_ssock;
extern SOCKADDR_IN					g_saddr;

extern SOCKET						g_csock;
extern SOCKADDR_IN					g_caddr;

extern SOCKET						g_clsock;
extern SOCKADDR_IN					g_claddr;


extern HANDLE						g_hThreadForComm;
extern HANDLE						g_hSvrMsgEvnt;

extern CWHList<CScripterObject*>	g_xScripterList;