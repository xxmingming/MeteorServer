#include "stdafx.h"
CRoomInfo::CRoomInfo()
{
	memset(m_szName, 0, 20);
	memset(m_szPassword, 0, 8);
	m_bHasPsd = false;
	m_nCount = 0;
	m_nGroup1 = m_nGroup2 = m_nHpMax = m_nMaxPlayer = m_nRoomIndex = 0;
	m_nRule = 1;
	m_dwTurnIndex = 0;
	m_totalTime = 0;
	m_turnTime = 0;
	m_bTurnStart = false;
	m_currentTick = 0;
	m_delta = 0;
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
	m_dwWaitClose = ::GetTickCount();
}

void CRoomInfo::OnNewTurn()
{
	m_currentTick = ::GetTickCount();
	m_totalTime = m_turnTime;
	m_delta = 0;
	m_bTurnStart = true;
	closed = false;
}

void CRoomInfo::OnUserKeyFrame(TurnFrames * pk)
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
	//		pListNode = g_xUserInfoList.GetNext(pListNode);
	//	}
	//}
}

void CRoomInfo::NewTurn()
{
	//给房间所有玩家发送消息，让进入结束界面，退出结束界面后，重新选人和武器，开始新一轮.
	OnNewTurn();
}

void CRoomInfo::WaitClose()
{
	DWORD t = ::GetTickCount();
	if (t - m_dwWaitClose >= 30000)
	{
		Close();
	}
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
			pListNode = g_xUserInfoList.GetNext(pListNode);
		}

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
							_LPTSENDBUFF lpSendBuff = new _TSENDBUFF;
							id.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
							lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + id.ByteSize();
							MsgHeader.wIdent = (WORD)MeteorMsg_MsgType_UserDeadSB2C;//清理死亡角色/清理自己的角色，并向服务端发送申请复活消息.
							//print("MeteorMsg_MsgType_UserDeadSB2C");
							MsgHeader.nLength = id.ByteSize();
							MsgHeader.nSocket = pUserInfo->m_sock;
							MsgHeader.wSessionIndex = pUserInfo->m_nUserGateIndex;
							MsgHeader.wUserListIndex = pUserInfo->m_nUserServerIndex;
							memmove(lpSendBuff->szData, &MsgHeader, sizeof(tag_TMSGHEADER));
							pUserInfo->m_pGateInfo->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
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
				pListNode = g_xUserInfoList.GetNext(pListNode);
			}
		}
	}

	if (m_delta > syncDelta)
	{
		//收到玩家的帧同步信息.
		static TurnFrames rsp;
		rsp.Clear();
		rsp.set_turnindex(m_totalTime);
		ZeroMemory(&MsgHeader, sizeof(MsgHeader));
		PLISTNODE pListNode = NULL;
		//if (m_pUserList.GetCount())
		//{
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
		//					//print("pUserInfo->m_pxPlayerObject == NULL");
		//				}
		//			}
		//			else
		//			{
		//				//////print("pUserInfo->m_bDirty");
		//			}
		//			pUserInfo->Unlock();
		//		}
		//		pListNode = g_xUserInfoList.GetNext(pListNode);
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
		//					_LPTSENDBUFF lpSendBuff = new _TSENDBUFF;
		//					rsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
		//					lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + rsp.ByteSize();
		//					MsgHeader.wIdent = (WORD)MeteorMsg_MsgType_SyncKeyFrame;
		//					//print("sync key frame");
		//					MsgHeader.nLength = rsp.ByteSize();
		//					MsgHeader.nSocket = pUserInfo->m_sock;
		//					MsgHeader.wSessionIndex = pUserInfo->m_nUserGateIndex;
		//					MsgHeader.wUserListIndex = pUserInfo->m_nUserServerIndex;
		//					memmove(lpSendBuff->szData, &MsgHeader, sizeof(tag_TMSGHEADER));
		//					pUserInfo->m_pGateInfo->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
		//				}
		//				pUserInfo->Unlock();
		//			}
		//			pListNode = g_xUserInfoList.GetNext(pListNode);
		//		} // while
		//	}
		//	else
		//	{
		//		//print("rsp.players_size() == 0");
		//	}

		//}
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


