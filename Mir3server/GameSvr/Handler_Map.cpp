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

void CRoomInfo::OnUserKeyFrame()
{
	//收到玩家的帧同步信息.
	//PLISTNODE pListNode = NULL;
	//if (m_pUserList.GetCount())
	//{
	//	//第一次遍历填充所有角色的输入信息到整个消息
	//	pListNode = m_pUserList.GetHead();
	//	while (pListNode)
	//	{
	//		CUserInfo *pUserInfo = m_pUserList.GetData(pListNode);
	//		if (pUserInfo != NULL)
	//		{
	//			pUserInfo->Lock();
	//			if (pUserInfo->m_pxPlayerObject != NULL)
	//			{
	//				if (pUserInfo->m_nUserServerIndex == pk->mutable_players(0)->id())
	//				{
	//					//print("pUserInfo->Update");
	//					pUserInfo->Update(pk->mutable_players(0));
	//					pUserInfo->Unlock();
	//					break;
	//				}
	//				else
	//				{
	//					//print("pUserInfo->m_nUserServerIndex != pk->mutable_players(0)->id()");
	//				}
	//			}
	//			pUserInfo->Unlock();
	//		}
	//		pListNode = m_pUserList.GetNext(pListNode);
	//	}
	//}
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
	m_currentTick = t;
	if (m_totalTime <= 0)
	{
		NewTurn();
		return;
	}

	_TMSGHEADER MsgHeader;
	//处理死亡消息，更新每个角色
	PLISTNODE pListNode = NULL;
	if (m_pUserList.GetCount())
	{
		UserId id;
		//第一次遍历填充所有角色的输入信息到整个消息
		pListNode = m_pUserList.GetHead();
		while (pListNode)
		{
			CUserInfo *pUserInfo = m_pUserList.GetData(pListNode);
			if (pUserInfo)
			{
				pUserInfo->Lock();
				//若玩家还未进场/或玩家在房间的角色注销.
				if (!pUserInfo->m_bDirty)
				{
					if (pUserInfo->m_pxPlayerObject != NULL)
					{
						if (pUserInfo->NeedReborn(m_delta))
						{
							//print("some one need reborn");
							if (pUserInfo->m_pxPlayerObject->m_bNeedSend)
							{
								id.add_player(pUserInfo->m_nUserServerIndex);
								pUserInfo->m_pxPlayerObject->m_bNeedSend = FALSE;
							}
						}
					}
					else
					{
						//print("pUserInfo->m_pxPlayerObject == NULL");
					}
				}
				else
				{
					//////print("pUserInfo->m_bDirty");
				}
				pUserInfo->Unlock();
			}
			pListNode = m_pUserList.GetNext(pListNode);
		}

		WORD user[_NUM_OF_MAXPLAYER];
		for (int i = 0; i < _NUM_OF_MAXPLAYER; i++)
			user[i] = -1;
		CGateInfo * gate = NULL;
		int userIndexOffset = 0;
		if (id.player_size() != 0)
		{
			pListNode = m_pUserList.GetHead();
			while (pListNode)
			{
				CUserInfo *pUserInfo = m_pUserList.GetData(pListNode);
				if (pUserInfo)
				{
					pUserInfo->Lock();
					//若玩家还未进场/或玩家在房间的角色注销.
					if (!pUserInfo->m_bDirty)
					{
						if (pUserInfo->m_pxPlayerObject != NULL)
						{
							user[userIndexOffset++] = pUserInfo->m_nUserGateIndex;
							if (gate == NULL)
								gate = pUserInfo->m_pGateInfo;
						}
						else
						{
							//print("pUserInfo->m_pxPlayerObject == NULL");
						}
					}
					else
					{
						//////print("pUserInfo->m_bDirty");
					}
					pUserInfo->Unlock();
				}
				pListNode = m_pUserList.GetNext(pListNode);
			}

			if (gate != NULL)
			{
				int k = g_memPool.GetAvailablePosition();
				if (k < 0)
					print("no more memory");
				_LPTSENDBUFF lpSendBuff = g_memPool.GetEmptyElement(k);
				if (lpSendBuff != NULL)
				{
					lpSendBuff->nIndex = k;
					id.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
					lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + id.ByteSizeLong();
					MsgHeader.nMessage = MeteorMsg_MsgType_UserDeadSB2C;
					MsgHeader.wIdent = BOARDCASTS2G;//清理死亡角色/清理自己的角色，并向服务端发送申请复活消息.
					MsgHeader.nLength = id.ByteSizeLong();
					MsgHeader.nSocket = 0;
					MsgHeader.wSessionIndex = 0;
					MsgHeader.wUserListIndex = 0;
					memmove(&MsgHeader.wUserList, user, sizeof(user));
					memmove(lpSendBuff->szData, &MsgHeader, sizeof(tag_TMSGHEADER));
					gate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
				}
			}
		}
	}

	if (m_delta > syncDelta)
	{
		//收到玩家的帧同步信息.
		//KeyFrame rsp;
		//rsp.Clear();
		//
		//rsp.set_frameindex(m_totalTime);
		//ZeroMemory(&MsgHeader, sizeof(MsgHeader));
		//PLISTNODE pListNode = NULL;
		//if (m_pUserList.GetCount())
		//{
		//	WORD user[_NUM_OF_MAXPLAYER];
		//	for (int i = 0; i < _NUM_OF_MAXPLAYER; i++)
		//		user[i] = -1;
		//	CGateInfo * gate = NULL;
		//	int userIndexOffset = 0;
		//	//第一次遍历填充所有角色的输入信息到整个消息
		//	pListNode = m_pUserList.GetHead();
		//	while (pListNode)
		//	{
		//		CUserInfo *pUserInfo = m_pUserList.GetData(pListNode);
		//		if (pUserInfo)
		//		{
		//			pUserInfo->Lock();
		//			//若玩家还未进场/或玩家在房间的角色注销.
		//			if (!pUserInfo->m_bDirty)
		//			{
		//				if (pUserInfo->m_pxPlayerObject != NULL)
		//				{
		//					Player_* p = rsp.add_players();
		//					pUserInfo->CopyTo(p);
		//				}
		//				else
		//				{
		//					
		//				}
		//			}
		//			else
		//			{
		//				
		//			}
		//			pUserInfo->Unlock();
		//		}
		//		pListNode = m_pUserList.GetNext(pListNode);
		//	}

		//	//要同步的角色数量不为0,
		//	if (rsp.players_size() != 0)
		//	{
		//		pListNode = m_pUserList.GetHead();
		//		while (pListNode)
		//		{
		//			CUserInfo *pUserInfo = m_pUserList.GetData(pListNode);
		//			if (pUserInfo != NULL)
		//			{
		//				pUserInfo->Lock();
		//				if (!pUserInfo->IsEmpty() && pUserInfo->m_pxPlayerObject != NULL)
		//				{
		//					user[userIndexOffset++] = pUserInfo->m_nUserGateIndex;
		//					if (gate == NULL)
		//						gate = pUserInfo->m_pGateInfo;
		//				}
		//				pUserInfo->Unlock();
		//			}
		//			pListNode = m_pUserList.GetNext(pListNode);
		//		}

		//		if (gate != NULL)
		//		{
		//			int k = g_memPool.GetAvailablePosition();
		//			if (k < 0)
		//				print("no more memory");
		//			_LPTSENDBUFF lpSendBuff = g_memPool.GetEmptyElement(k);
		//			if (lpSendBuff != NULL)
		//			{
		//				lpSendBuff->nIndex = k;
		//				rsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
		//				lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + rsp.ByteSizeLong();
		//				vprint("key frame bytes len:%d, totallen:%d, datamax size:%d, player count:%d roomid:%d r player count:%d", rsp.ByteSizeLong(), lpSendBuff->nLen, DATA_BUFSIZE, rsp.players_size(), m_nRoomIndex, m_nCount);
		//				//缓冲区不足，无法容纳这么多玩家的信息
		//				if (rsp.ByteSizeLong() > (DATA_BUFSIZE - sizeof(tag_TMSGHEADER)))
		//				{
		//					vprint("the key sync packet occur error player count:%d", rsp.players_size());
		//				}
		//				MsgHeader.nMessage = MeteorMsg_MsgType_SyncKeyFrame;
		//				MsgHeader.wIdent = BOARDCASTS2G;
		//				MsgHeader.nLength = rsp.ByteSizeLong();
		//				MsgHeader.nSocket = 0;
		//				MsgHeader.wSessionIndex = 0;
		//				MsgHeader.wUserListIndex = 0;
		//				memmove(&MsgHeader.wUserList, user, sizeof(user));
		//				memmove(lpSendBuff->szData, &MsgHeader, sizeof(tag_TMSGHEADER));
		//				gate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
		//			}
		//		}
		//	}
		//	else
		//	{
		//		//print("rsp.players_size() == 0");
		//	}

		//}

		m_delta = 0;
	}
}

CMirMap::CMirMap()
{
}

CMirMap::~CMirMap()
{
}

BOOL CMirMap::LoadMapData(char *pszName)
{
	return TRUE;
}


