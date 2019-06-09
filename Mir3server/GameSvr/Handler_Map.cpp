#include "stdafx.h"
#include "KcpServer.h"
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
	m_bGameStart = false;
	m_currentTick = 0;
	m_dwWaitClose = 0;
	m_delta = 0;
	running = false;
}

int CRoomInfo::InitKcpServer()
{
	print("InitKcpServer");
	//只要是非重播的就建立KCP服务
	if (m_pKcpServer == NULL && m_nPattern != RoomInfo_RoomPattern__Replay)
	{
		m_pKcpServer = new KcpServer(m_nRoomIndex);
		m_pKcpServer->InitKcp();
		return m_pKcpServer->port;
	}
	return -1;
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

void CRoomInfo::OnPlayerAllLeaved()
{
	m_bGameStart = false;
	m_dwWaitClose = ::GetTickCount();
}

void CRoomInfo::OnNewTurn()
{
	m_dwWaitClose = 0;
	m_currentTick = ::GetTickCount();
	m_totalTime = m_turnTime;
	m_delta = 0;
	m_bGameStart = true;
	running = true;
}

void CRoomInfo::NewTurn()
{
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

void CRoomInfo::Close() 
{
	this->m_bGameStart = false;
	this->running = false;
}

void CRoomInfo::Update()
{
	if (m_dwWaitClose != 0)
		WaitClose();
	if (!m_bGameStart)
		return;
	DWORD t = ::GetTickCount();
	m_delta += (t - m_currentTick);
	m_totalTime -= (t - m_currentTick);
	m_currentTick = t;
	if (m_totalTime <= 0)
	{
		NewTurn();
		return;
	}

	//_TMSGHEADER MsgHeader;
	//PLISTNODE pListNode = NULL;
	//if (m_pUserList.GetCount())
	//{
	//	UserId id;
	//	pListNode = m_pUserList.GetHead();
	//	while (pListNode)
	//	{
	//		CUserInfo *pUserInfo = m_pUserList.GetData(pListNode);
	//		if (pUserInfo)
	//		{
	//			pUserInfo->Lock();
	//			if (!pUserInfo->m_bDirty)
	//			{
	//				if (pUserInfo->m_pxPlayerObject != NULL)
	//				{
	//					if (pUserInfo->NeedReborn(m_delta))
	//					{
	//						//print("some one need reborn");
	//						if (pUserInfo->m_pxPlayerObject->m_bNeedSend)
	//						{
	//							id.add_player(pUserInfo->m_nUserServerIndex);
	//							pUserInfo->m_pxPlayerObject->m_bNeedSend = FALSE;
	//						}
	//					}
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
	//		pListNode = m_pUserList.GetNext(pListNode);
	//	}

	//	WORD user[_NUM_OF_MAXPLAYER];
	//	for (int i = 0; i < _NUM_OF_MAXPLAYER; i++)
	//		user[i] = -1;
	//	CGateInfo * gate = NULL;
	//	int userIndexOffset = 0;
	//	if (id.player_size() != 0)
	//	{
	//		pListNode = m_pUserList.GetHead();
	//		while (pListNode)
	//		{
	//			CUserInfo *pUserInfo = m_pUserList.GetData(pListNode);
	//			if (pUserInfo)
	//			{
	//				pUserInfo->Lock();
	//				if (!pUserInfo->m_bDirty)
	//				{
	//					if (pUserInfo->m_pxPlayerObject != NULL)
	//					{
	//						user[userIndexOffset++] = pUserInfo->m_nUserGateIndex;
	//						if (gate == NULL)
	//							gate = pUserInfo->m_pGateInfo;
	//					}
	//					else
	//					{
	//						//print("pUserInfo->m_pxPlayerObject == NULL");
	//					}
	//				}
	//				else
	//				{
	//					//////print("pUserInfo->m_bDirty");
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
	//				id.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
	//				lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + id.ByteSizeLong();
	//				MsgHeader.nMessage = MeteorMsg_MsgType_UserDeadSB2C;
	//				MsgHeader.wIdent = BOARDCASTS2G;
	//				MsgHeader.nLength = id.ByteSizeLong();
	//				MsgHeader.nSocket = 0;
	//				MsgHeader.wSessionIndex = 0;
	//				MsgHeader.wUserListIndex = 0;
	//				memmove(&MsgHeader.wUserList, user, sizeof(user));
	//				memmove(lpSendBuff->szData, &MsgHeader, sizeof(tag_TMSGHEADER));
	//				gate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
	//			}
	//		}
	//	}
	//}

	if (m_delta > syncDelta)
	{
		if (m_pKcpServer != NULL)
			m_pKcpServer->Update();
		//�յ���ҵ�֡ͬ����Ϣ.
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
		//	//��һ�α���������н�ɫ��������Ϣ��������Ϣ
		//	pListNode = m_pUserList.GetHead();
		//	while (pListNode)
		//	{
		//		CUserInfo *pUserInfo = m_pUserList.GetData(pListNode);
		//		if (pUserInfo)
		//		{
		//			pUserInfo->Lock();
		//			//����һ�δ����/������ڷ���Ľ�ɫע��.
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

		//	//Ҫͬ���Ľ�ɫ������Ϊ0,
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
		//				//���������㣬�޷�������ô����ҵ���Ϣ
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


