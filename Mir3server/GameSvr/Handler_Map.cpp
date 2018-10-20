#include "stdafx.h"

//global function def
void LoadMap(CMapInfo* pMapInfo)
{
	int			nLen = memlen(pMapInfo->szMapFileName);
	int			nLen2 = memlen(pMapInfo->szMapName);
	CMirMap*	pMirMap = new CMirMap;
	if (pMirMap)
	{
		memcpy(pMirMap->m_szMapName, pMapInfo->szMapFileName, nLen);
		memcpy(pMirMap->m_szMapTextName, pMapInfo->szMapName, nLen2);
		pMirMap->m_nLevelIdx = pMapInfo->m_nLevelIdx;
		if (pMirMap->LoadMapData(pMapInfo->szMapFileName))
			g_xMirMapList.AddNewNode(pMirMap);
	}
}

CMirMap* GetMap(char *pszMapName)
{
	PLISTNODE		pListNode;
	CMirMap*		pMirMap = NULL;

	if (g_xMirMapList.GetCount())
	{
		pListNode = g_xMirMapList.GetHead();

		while (pListNode)
		{
			pMirMap = g_xMirMapList.GetData(pListNode);

			if (memcmp(pMirMap->m_szMapName, pszMapName, memlen(pszMapName) - 1) == 0)
				return pMirMap;
			pListNode = g_xMirMapList.GetNext(pListNode);
		}
	}
	return NULL;
}

/* **************************************************************************************
		CRoomInfo Class Members
   **************************************************************************************/
CRoomInfo::CRoomInfo()
{
	memset(m_szName, 0, 20);
	memset(m_szPassword, 0, 8);
	m_nCount = 0;
	m_nGroup1 = m_nGroup2 = m_nHpMax = m_nMaxPlayer = m_nRoomIndex = 0;
	m_nRule = 1;
	m_pMap = NULL;
}

CRoomInfo::~CRoomInfo()
{
}

BOOL CRoomInfo::RemovePlayer(CUserInfo * pRemoveObject)
{
	if (pRemoveObject != NULL)
	{
		m_pUserList.Lock();
		m_pUserList.RemoveNodeByData(pRemoveObject);
		int count = m_pUserList.GetCount();
		if (count == 0)
			m_totalTime = 0;
		m_nCount = count;
		m_pUserList.Unlock();
	}

	return TRUE;
}

void CRoomInfo::OnAllPlayerLeaved()
{
	m_bTurnStart = false;
}

void CRoomInfo::CreateRoom(CMirMap * map, int maxPlayer, int hpMax, int turnTime, int roomIdx)
{
	if (m_pMap != NULL)
		return;
	m_pMap = map;
	m_nMaxPlayer = maxPlayer;
	m_nCount = 0;
	m_nRoomIndex = roomIdx;
	m_nHpMax = hpMax;
	strncpy(m_szName, map->m_szMapTextName, min(18, strlen(map->m_szMapTextName)));
	m_szName[18] = 0;
	m_szName[19] = 0;
	m_currentTick = ::GetTickCount();
	m_turnTime = turnTime;
	m_totalTime = turnTime;
	m_delta = 0;
}

void CRoomInfo::OnNewTurn()
{
	m_currentTick = ::GetTickCount();
	m_totalTime = m_turnTime;
	m_delta = 0;
	m_bTurnStart = true;
}

void CRoomInfo::OnUserKeyFrame(KeyFrame * pk)
{
	//收到玩家的帧同步信息.
	static KeyFrame rsp;
	rsp.Clear();
	_TMSGHEADER MsgHeader;
	ZeroMemory(&MsgHeader, sizeof(MsgHeader));
	PLISTNODE pListNode = NULL;
	if (m_pUserList.GetCount())
	{
		//第一次遍历填充所有角色的输入信息到整个消息
		pListNode = m_pUserList.GetHead();
		while (pListNode)
		{
			CUserInfo *pUserInfo = m_pUserList.GetData(pListNode);
			if (pUserInfo != NULL && pUserInfo->m_pxPlayerObject != NULL)
			{
				if (pUserInfo->m_nUserServerIndex == pk->mutable_players(0)->id())
				{
					pUserInfo->Update(pk->mutable_players(0));
					break;
					//Player_* p = rsp.add_players();
					//p->CopyFrom(k.players[0]);
				}
				//else
				//{
				//	Player_ * p = rsp.add_players();
				//	pUserInfo->CopyTo(p);
				//}
			}
			pListNode = g_xUserInfoList.GetNext(pListNode);
		}

		//pListNode = m_pUserList.GetHead();
		//while (pListNode)
		//{
		//	CUserInfo *pUserInfo = m_pUserList.GetData(pListNode);
		//	if (pUserInfo)
		//	{
		//		_LPTSENDBUFF lpSendBuff = new _TSENDBUFF;
		//		rsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
		//		lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + rsp.ByteSize();
		//		MsgHeader.wIdent = (WORD)MeteorMsg_MsgType_SyncKeyFrame;
		//		MsgHeader.nLength = rsp.ByteSize();
		//		MsgHeader.nSocket = pUserInfo->m_sock;
		//		MsgHeader.wSessionIndex = pUserInfo->m_nUserGateIndex;
		//		MsgHeader.wUserListIndex = pUserInfo->m_nUserServerIndex;
		//		memmove(lpSendBuff->szData, &MsgHeader, sizeof(tag_TMSGHEADER));
		//		pUserInfo->m_pGateInfo->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
		//	}
		//	pListNode = g_xUserInfoList.GetNext(pListNode);
		//} // while

	} // if g_xReadyUserInfoList.GetCount()
}

void CRoomInfo::NewTurn()
{
	//给房间所有玩家发送消息，让进入结束界面，退出结束界面后，重新选人和武器，开始新一轮.
	OnNewTurn();
}

void CRoomInfo::Update()
{
	//此轮游戏还未开始.
	if (!m_bTurnStart)
		return;
	//处理全部角色间的输入同步.
	DWORD t = ::GetTickCount();
	m_delta += (t - m_currentTick);
	m_totalTime -= (t - m_currentTick);
	if (m_totalTime <= 0)
	{
		NewTurn();
		return;
	}
	if (m_delta > syncDelta)
	{
		//收到玩家的帧同步信息.
		static KeyFrame rsp;
		rsp.Clear();
		rsp.set_frameindex(m_totalTime);
		_TMSGHEADER MsgHeader;
		ZeroMemory(&MsgHeader, sizeof(MsgHeader));
		PLISTNODE pListNode = NULL;
		if (m_pUserList.GetCount())
		{
			//第一次遍历填充所有角色的输入信息到整个消息
			pListNode = m_pUserList.GetHead();
			while (pListNode)
			{
				CUserInfo *pUserInfo = m_pUserList.GetData(pListNode);
				if (pUserInfo)
				{
					if (!pUserInfo->m_bDirty)
					{
						Player_* p = rsp.add_players();
						pUserInfo->CopyTo(p);
					}
				}
				pListNode = g_xUserInfoList.GetNext(pListNode);
			}

			//要同步的角色数量不为0,
			if (rsp.players_size() != 0)
			{
				pListNode = m_pUserList.GetHead();
				while (pListNode)
				{
					CUserInfo *pUserInfo = m_pUserList.GetData(pListNode);
					if (pUserInfo)
					{
						_LPTSENDBUFF lpSendBuff = new _TSENDBUFF;
						rsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
						lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + rsp.ByteSize();
						MsgHeader.wIdent = (WORD)MeteorMsg_MsgType_SyncKeyFrame;
						MsgHeader.nLength = rsp.ByteSize();
						MsgHeader.nSocket = pUserInfo->m_sock;
						MsgHeader.wSessionIndex = pUserInfo->m_nUserGateIndex;
						MsgHeader.wUserListIndex = pUserInfo->m_nUserServerIndex;
						memmove(lpSendBuff->szData, &MsgHeader, sizeof(tag_TMSGHEADER));
						pUserInfo->m_pGateInfo->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
					}
					pListNode = g_xUserInfoList.GetNext(pListNode);
				} // while
			}

		} // if g_xReadyUserInfoList.GetCount()

		//static InputReq req;
		//req.Clear();
		//_TMSGHEADER MsgHeader;
		//ZeroMemory(&MsgHeader, sizeof(MsgHeader));
		//PLISTNODE pListNode = NULL;
		//if (m_pUserList.GetCount())
		//{
		//	//第一次遍历填充所有角色的输入信息到整个消息
		//	pListNode = m_pUserList.GetHead();
		//	while (pListNode)
		//	{
		//		CUserInfo *pUserInfo = m_pUserList.GetData(pListNode);
		//		if (pUserInfo)
		//		{
		//			Input_ * input = req.mutable_input()->Add();
		//			//向每个玩家广播其他玩家的输入.
		//			pUserInfo->Operate(input);
		//		}
		//		pListNode = g_xUserInfoList.GetNext(pListNode);
		//	} // while

		//	pListNode = m_pUserList.GetHead();
		//	while (pListNode)
		//	{
		//		CUserInfo *pUserInfo = m_pUserList.GetData(pListNode);
		//		if (pUserInfo)
		//		{
		//			_LPTSENDBUFF lpSendBuff = new _TSENDBUFF;
		//			req.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
		//			lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + req.ByteSize();
		//			MsgHeader.wIdent = (WORD)MeteorMsg_MsgType_SyncInput;
		//			MsgHeader.nLength = req.ByteSize();
		//			MsgHeader.nSocket = pUserInfo->m_sock;
		//			MsgHeader.wSessionIndex = pUserInfo->m_nUserGateIndex;
		//			MsgHeader.wUserListIndex = pUserInfo->m_nUserServerIndex;
		//			memmove(lpSendBuff->szData, &MsgHeader, sizeof(tag_TMSGHEADER));
		//			pUserInfo->m_pGateInfo->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
		//		}
		//		pListNode = g_xUserInfoList.GetNext(pListNode);
		//	} // while

		//} // if g_xReadyUserInfoList.GetCount()
		m_delta = 0;
	}
	m_currentTick = GetTickCount();
}

/* **************************************************************************************
		CMirMap	Class Members
   **************************************************************************************/
CMirMap::CMirMap()
{
}

CMirMap::~CMirMap()
{
}

BOOL CMirMap::LoadMapData(char *pszName)
{
	return TRUE;
	//读取cob文件。des文件，gmb文件，主要是障碍物，场景碰撞机关，部分
	/*HANDLE			hFile;
	LPCELLINFO		pstCellInfo;
	TCHAR			szMapName[15];
	TCHAR			szMapFileName[256];
	TCHAR			szMapPath[256];
	MultiByteToWideChar(CP_ACP, 0, pszName, -1, szMapName, sizeof(szMapName)/sizeof(TCHAR));
	MultiByteToWideChar(CP_ACP, 0, g_strClientPath, -1, szMapPath, sizeof(szMapPath) / sizeof(TCHAR));
	lstrcpy(szMapFileName, szMapPath);
	lstrcat(szMapFileName, _TEXT("/"));
	lstrcat(szMapFileName, szMapName);
	lstrcat(szMapFileName, _TEXT("/"));
	lstrcat(szMapFileName, szMapName);
	lstrcat(szMapFileName, _TEXT(".des"));
	hFile = CreateFile(szMapFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DWORD		dwReadLen;
		ReadFile(hFile, &m_stMapFH, sizeof(MAPFILEHEADER), &dwReadLen, NULL);
		int nMapSize = m_stMapFH.shWidth * m_stMapFH.shHeight;
		SetFilePointer(hFile, sizeof(TILEINFO) * (nMapSize) / 4, 0, FILE_CURRENT);
		pstCellInfo = new CELLINFO[nMapSize];
		if (pstCellInfo)
		{
			ReadFile(hFile, pstCellInfo, sizeof(CELLINFO) * (nMapSize), &dwReadLen, NULL);
			CloseHandle(hFile);
			m_pMapCellInfo = new CMapCellInfo[nMapSize];
			if (m_pMapCellInfo)
			{
				for (int i = 0; i < nMapSize; i++)
				{
					m_pMapCellInfo[i].m_chFlag			= pstCellInfo[i].cFlag;
					m_pMapCellInfo[i].m_sLightNEvent	= pstCellInfo[i].shLigntNEvent;
					m_pMapCellInfo[i].m_xpObjectList = NULL;
				}
			}

			int nSize = sizeof(m_pMapCellInfo);
			delete [] pstCellInfo;
			pstCellInfo = NULL;
			InsertLogMsgParam(IDS_LOADMAPFILE_GOOD, szMapFileName, LOGPARAM_STR);
			return TRUE;
		}
	}

	InsertLogMsgParam(IDS_LOADMAPFILE_FAIL, szMapFileName, LOGPARAM_STR);
	return FALSE;*/
}


