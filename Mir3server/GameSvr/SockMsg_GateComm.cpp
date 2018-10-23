#include "stdafx.h"
//global fun
CMirMap * FindMap(int levelIdx);
CRoomInfo * FindRoom(int roomIdx);
BOOL ProcessMessage(CGateInfo * pGate, char * pBytes);
void OnUserJoinRoom(_LPTMSGHEADER msgHead, CGateInfo * pGate, CUserInfo* pUserInfo, CRoomInfo * pRoom, const char * szName);
void OnUserEnterLevel(_LPTMSGHEADER pMsgHeader, CGateInfo * pGate, CUserInfo * pUser, CRoomInfo * pRoom, EnterLevelReq * pEnterLevelReq);
void UpdateStatusBarSession(BOOL fGrow)
{
	static long	nNumOfCurrSession = 0;

	TCHAR	szText[20];

	(fGrow ? InterlockedIncrement(&nNumOfCurrSession) : InterlockedDecrement(&nNumOfCurrSession));
	
	wsprintf(szText, _TEXT("%d Sessions"), nNumOfCurrSession);

	SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(1, 0), (LPARAM)szText);
}

void UpdateStatusBarUsers(BOOL fGrow)
{
	static long	nNumOfUsers = 0;

	TCHAR	szText[20];

	(fGrow ? InterlockedIncrement(&nNumOfUsers) : InterlockedDecrement(&nNumOfUsers));
	
	wsprintf(szText, _TEXT("%d Users"), nNumOfUsers);

	SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(2, 0), (LPARAM)szText);
}

DWORD WINAPI AcceptThread(LPVOID lpParameter)
{
	int					nLen = sizeof(SOCKADDR_IN);

	SOCKET				Accept;
	SOCKADDR_IN			Address;

	TCHAR				szGateIP[16];

	while (TRUE)
	{
		nLen	= sizeof( Address );
		Accept	= WSAAccept( g_ssock, (SOCKADDR *) &Address, &nLen, NULL, 0 );

		if (g_fTerminated) return 0L;

		CGateInfo* pGateInfo = new CGateInfo;

		if (pGateInfo)
		{
			pGateInfo->m_sock			= Accept;

			CreateIoCompletionPort((HANDLE)pGateInfo->m_sock, g_hIOCP, (DWORD)pGateInfo, 0);

			if (g_xGateList.AddNewNode(pGateInfo))
			{
				int zero = 0;
				
				setsockopt(pGateInfo->m_sock, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero) );

				ZeroMemory(&(pGateInfo->OverlappedEx), sizeof(OVERLAPPED) * 2);

				pGateInfo->OverlappedEx[1].nOvFlag		= OVERLAPPED_FLAG::OVERLAPPED_SEND;

				pGateInfo->Recv();

				UpdateStatusBarSession(TRUE);

				_stprintf(szGateIP, _T("%d.%d.%d.%d"), Address.sin_addr.s_net, Address.sin_addr.s_host, 
															Address.sin_addr.s_lh, Address.sin_addr.s_impno);

				InsertLogMsgParam(IDS_ACCEPT_GATESERVER, szGateIP, LOGPARAM_STR);
			}
		}
	}

	return 0;
}

DWORD WINAPI ServerWorkerThread(LPVOID CompletionPortID)
{
	DWORD				dwBytesTransferred;
	CGateInfo*			pGateInfo;
	LPOVERLAPPEDEX		lpOverlapped;
	_LPTMSGHEADER		pMsgHeader;
	char				completionPacket[DATA_BUFSIZE];

	while (TRUE)
	{
		GetQueuedCompletionStatus((HANDLE)CompletionPortID, &dwBytesTransferred, (LPDWORD)&pGateInfo, (LPOVERLAPPED *)&lpOverlapped, INFINITE);
		
		if (g_fTerminated) return 0L;
		//��Ϸ���غ���Ϸ���Ͽ���.֪ͨ�ѿ�ʼ��Ϸ�ķ�������
		if (dwBytesTransferred == 0)
		{
			g_xUserInfoList.Lock();
			if (g_xUserInfoList.GetCount())
			{
				PLISTNODE pListNode = g_xUserInfoList.GetHead();
				//�ڵ�ǰ�����ϵ�ȫ���û������ӵ�ͼ���޳�.
				while (pListNode)
				{
					CUserInfo *pUserInfo = g_xUserInfoList.GetData(pListNode);

					if (pUserInfo->m_pGateInfo == pGateInfo)
					{
						pUserInfo->Lock();
						pUserInfo->m_bEmpty = true;
						pUserInfo->m_pGateInfo = NULL;
						//����Ҵ��ڷ�����(��ҿ��Խ������ӷ����������ǲ�������Ϸ)���ҷ����Ѿ���ʼ��Ϸ��.
						//if (pUserInfo->m_pxPlayerObject != NULL && pUserInfo->m_pRoom != NULL && pUserInfo->m_pRoom->m_bReady)
						//	pUserInfo->m_pRoom->RemovePlayer(pUserInfo->m_pxPlayerObject);
						//pUserInfo->m_pxPlayerObject = NULL;
						//pUserInfo->
						//pListNode = g_xUserInfoList.RemoveNode(pListNode);
						pUserInfo->Unlock();
						UpdateStatusBarUsers(FALSE);
					}
					else
						pListNode = g_xUserInfoList.GetNext(pListNode);
				}
			}
			g_xUserInfoList.Unlock();
			closesocket(pGateInfo->m_sock);
			g_xGateList.Lock();
			g_xGateList.RemoveNodeByData(pGateInfo);
			g_xGateList.Unlock();
			if (pGateInfo) delete pGateInfo;
			continue;
		}

		if (lpOverlapped->nOvFlag == OVERLAPPED_FLAG::OVERLAPPED_RECV)
		{
			//static DWORD nLastTick = GetTickCount();
			//static DWORD nBytes = 0;

			//nBytes += dwBytesTransferred;
/*
			if (GetTickCount() - nLastTick >= 1000)
			{
				TCHAR buf[256];
				wsprintf( buf, _T("R: %d bytes/sec"), nBytes );

				nLastTick = GetTickCount();
				nBytes = 0;

				SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(3, 0), (LPARAM)buf);
			}*/	

			pGateInfo->OverlappedEx[0].bufLen += dwBytesTransferred;

			while (pGateInfo->HasCompletionPacket())
			{
				pGateInfo->ExtractPacket(completionPacket);
				if (!ProcessMessage(pGateInfo, completionPacket))
				{
					MessageBox(0, _TEXT("�������"), _TEXT("����"), MB_OK);
					__asm int 3;
				}
					//case GM_OPEN:
					//{
					//	pGateInfo->OpenNewUser( completionPacket );
					//	break;
					//}
					//case GM_CLOSE:
					//{
					//	CUserInfo *pUserInfo = &g_xUserInfoArr[ pMsgHeader->wUserListIndex ];

					//	if (pUserInfo)
					//	{
					//		//pUserInfo->m_btCurrentMode = USERMODE_LOGOFF;
					//		g_xLoginOutUserInfo.AddNewNode(pUserInfo);
					//	}

					//	break;
					//}
					//case GM_CHECKCLIENT:
					//{
					//	pGateInfo->SendGateCheck();
					//	break;
					//}
					//case GM_RECEIVE_OK:
					//{
					//	break;
					//}
					/*case GM_DATA:
					{
						CUserInfo *pUserInfo = &g_xUserInfoArr[ pMsgHeader->wUserListIndex ];

						if ( !pUserInfo->IsEmpty() )
						{
							if (pUserInfo->m_btCurrentMode == USERMODE_PLAYGAME)
							{
								if (pMsgHeader->nSocket == pUserInfo->m_sock )
									pUserInfo->ProcessUserMessage(completionPacket + sizeof( _TMSGHEADER ) );
							}
							else
							{
								pUserInfo->Lock();
								pUserInfo->DoClientCertification( completionPacket + sizeof( _TMSGHEADER ) + sizeof(_TDEFAULTMESSAGE) );
								pUserInfo->Unlock();
							}
						}

						break;*/
					//}
				//}
			}
	
			if (pGateInfo->Recv() == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
				InsertLogMsg( _T("WSARecv() failed") );
		}
		else if (lpOverlapped->nOvFlag == OVERLAPPED_FLAG::OVERLAPPED_SEND)
		{
			/*static DWORD nLastTick = GetTickCount();
			static DWORD nBytes = 0;

			nBytes += dwBytesTransferred;

			if (GetTickCount() - nLastTick >= 1000)
			{
				TCHAR buf[256];
				wsprintf( buf, _T("S: %d bytes/sec"), nBytes );

				nLastTick = GetTickCount();
				nBytes = 0;

				SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(4, 0), (LPARAM)buf);
			}*/
		}
	}

	return 0;
}

//8K��С���ֽ�������ȥһ����Ϣͷ.
BOOL ProcessMessage(CGateInfo * pGate, char * pBytes)
{
	_LPTMSGHEADER pMsgHeader = (_LPTMSGHEADER)pBytes;
	CHAR * data = NULL;
	if (pMsgHeader->nLength != 0)
		data = (pBytes + sizeof(tag_TMSGHEADER));

	switch (pMsgHeader->wIdent)
	{
		case GM_CHECKCLIENT:
			pGate->SendGateCheck();
			break;
		case GM_OPEN://ĳ���ͻ������������أ����ط�����Ϸ������Ϸ����һ����Ӧ����.
			pGate->OpenNewUser(pBytes);
			break;
		case GM_CLOSE://ĳ���ͻ��˶Ͽ������ط�����Ϸ������Ϸ��ע���˶����뿪���䲻ע������
			{
				//�ڷ����ȴӷ���ɾ��.��ͬ���������������뿪��Ϣ
				CUserInfo * pUserInfo = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
				if (pUserInfo != NULL)
				{
					if (pUserInfo->m_pRoom != NULL)
					{
						pGate->OnLeaveRoom(pUserInfo);
						pUserInfo->m_pRoom = NULL;
					}
					if (pUserInfo->m_pxPlayerObject != NULL)
					{
						pUserInfo->CloseUserHuman();
						pUserInfo->Lock();
						pUserInfo->m_bEmpty = TRUE;
						pUserInfo->m_pGateInfo = NULL;
						pUserInfo->Unlock();
					}
					//��ȫ��ɾ��.
					g_xUserInfoList.Lock();
					g_xUserInfoList.RemoveNodeByData(&g_xUserInfoArr[pMsgHeader->wUserListIndex]);
					g_xUserInfoList.Unlock();
					UpdateStatusBarUsers(FALSE);
				}
			}
			break;
		//�޲���������ȡ�÷����б�.
		case MeteorMsg_MsgType_GetRoomReq:
		{
			GetRoomRsp pGetRoomRsp;
			g_xRoomList.Lock();
			PLISTNODE pNode = g_xRoomList.GetHead();
			while (pNode != NULL)
			{
				CRoomInfo * pRoom = g_xRoomList.GetData(pNode);
				if (pRoom != NULL)
				{
					RoomInfo * room = pGetRoomRsp.add_roominlobby();
					room->set_roomid(pRoom->m_nRoomIndex);
					room->set_roomname(GBK2UTF8(string(pRoom->m_szName)).c_str());
					room->set_rule((RoomInfo_RoomRule)pRoom->m_nRule);
					room->set_levelidx(pRoom->m_pMap->m_nLevelIdx);
					room->set_group1(pRoom->m_nGroup1);
					room->set_group2(pRoom->m_nGroup2);
					room->set_maxplayer(pRoom->m_nMaxPlayer);
					room->set_playercount(pRoom->m_nCount);
				}
				pNode = g_xRoomList.GetNext(pNode);
			}
			g_xRoomList.Unlock();
			_LPTSENDBUFF lpSendBuff = new _TSENDBUFF;
			pGetRoomRsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
			lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + pGetRoomRsp.ByteSize();
			pMsgHeader->wIdent = (WORD)MeteorMsg_MsgType_GetRoomRsp;
			pMsgHeader->nLength = pGetRoomRsp.ByteSize();
			memmove(lpSendBuff->szData, pMsgHeader, sizeof(tag_TMSGHEADER));
			pGate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
		}
			break;
		case MeteorMsg_MsgType_CreateRoomReq:
		{
			int RoomIdx = g_xRoom.GetFreeKey();
			if (RoomIdx < 0)
			{
				CreateRoomRsp pCreateRoomRsp;
				pCreateRoomRsp.set_result(0);
				pCreateRoomRsp.set_levelid(0);
				pCreateRoomRsp.set_roomid(0);
				_LPTSENDBUFF lpSendBuff = new _TSENDBUFF;
				pCreateRoomRsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
				lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + pCreateRoomRsp.ByteSize();
				pMsgHeader->wIdent = (WORD)MeteorMsg_MsgType_CreateRoomRsp;
				pMsgHeader->nLength = pCreateRoomRsp.ByteSize();
				memmove(lpSendBuff->szData, pMsgHeader, sizeof(tag_TMSGHEADER));
				pGate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
			}
			else
			{
				CreateRoomReq pCreateRoomReq;
				pCreateRoomReq.ParseFromArray(data, pMsgHeader->nLength);
				CRoomInfo * pRoom = &g_xRoom[RoomIdx];
				pRoom->m_nMaxPlayer = min(pCreateRoomReq.maxplayer(), _NUM_OF_MAXPLAYER);
				pRoom->m_nGroup1 = 0;
				pRoom->m_nGroup2 = 0;
				pRoom->m_nHpMax = pCreateRoomReq.hpmax();
				pRoom->m_nRoomIndex = RoomIdx;
				pRoom->m_nRule = pCreateRoomReq.rule();
				strncpy_s(pRoom->m_szName, pCreateRoomReq.roomname().c_str(), min(strlen(pCreateRoomReq.roomname().c_str()), 18));
				pRoom->m_pMap = FindMap(pCreateRoomReq.levelidx());
				g_xRoomList.Lock();
				g_xRoomList.AddNewNode(pRoom);
				g_xRoomList.Unlock();

				CreateRoomRsp pCreateRoomRsp;
				pCreateRoomRsp.set_result(1);
				pCreateRoomRsp.set_roomid(pRoom->m_nRoomIndex);
				pCreateRoomRsp.set_levelid(pRoom->m_pMap->m_nLevelIdx);
				pMsgHeader->wIdent = MeteorMsg_MsgType_CreateRoomRsp;
				pMsgHeader->nLength = pCreateRoomRsp.ByteSize();
				_LPTSENDBUFF lpSendBuff = new _TSENDBUFF;
				memmove(lpSendBuff->szData, pMsgHeader, sizeof(tag_TMSGHEADER));
				lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + pCreateRoomRsp.ByteSize();
				pCreateRoomRsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
				pGate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
			}
		}
			break;
		case MeteorMsg_MsgType_JoinRoomReq:
		{
			JoinRoomReq pJoinRoomReq;
			pJoinRoomReq.ParseFromArray(data, pMsgHeader->nLength);
			CRoomInfo * pRoom = FindRoom(pJoinRoomReq.roomid());
			JoinRoomRsp pJoinRoomRsp;
			if (pRoom == NULL)
			{
				pJoinRoomRsp.set_result(0);
				pJoinRoomRsp.set_reason(2);//δ�ҵ�����
				pJoinRoomRsp.set_levelidx(0);
				pJoinRoomRsp.set_playerid(0);
				pJoinRoomRsp.set_roomid(0);
			}
			else
			{
				CUserInfo * pUser = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
				if (pUser->m_pRoom)
				{
					pJoinRoomRsp.set_result(0);
					pJoinRoomRsp.set_reason(3);//��ǰ�Ѿ���ĳ���䣬��Ҫ���˳�
					pJoinRoomRsp.set_levelidx(0);
					pJoinRoomRsp.set_playerid(0);
					pJoinRoomRsp.set_roomid(0);
				}
				else
				if (pRoom->m_nMaxPlayer == pRoom->m_nCount)
				{
					pJoinRoomRsp.set_result(0);
					pJoinRoomRsp.set_reason(1);//����������
					pJoinRoomRsp.set_levelidx(0);
					pJoinRoomRsp.set_playerid(0);
					pJoinRoomRsp.set_roomid(0);
				}
				else
				{
					pJoinRoomRsp.set_result(1);
					pJoinRoomRsp.set_reason(0);
					pJoinRoomRsp.set_levelidx(pRoom->m_pMap->m_nLevelIdx);
					pJoinRoomRsp.set_playerid(pMsgHeader->wUserListIndex);
					pJoinRoomRsp.set_roomid(pRoom->m_nRoomIndex);
					//�����������˷�,�������Լ�.
					OnUserJoinRoom(pMsgHeader, pGate, pUser, pRoom, UTF82GBK(pJoinRoomReq.usernick()).c_str());
				}
			}

			pMsgHeader->wIdent = MeteorMsg_MsgType_JoinRoomRsp;
			pMsgHeader->nLength = pJoinRoomRsp.ByteSize();
			_LPTSENDBUFF lpSendBuff = new _TSENDBUFF;
			memmove(lpSendBuff->szData, pMsgHeader, sizeof(tag_TMSGHEADER));
			lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + pJoinRoomRsp.ByteSize();
			pJoinRoomRsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
			pGate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
		}
			break;
		case MeteorMsg_MsgType_EnterLevelReq:
		{
			EnterLevelReq pEnterLevelReq;
			pEnterLevelReq.ParseFromArray(data, pMsgHeader->nLength);
			CUserInfo * pUser = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
			if (pUser == NULL || pUser->m_pxPlayerObject != NULL || pUser->m_pRoom == NULL)
			{
				//�Ѿ����ڽ�ɫ/��δ���뷿��
				return TRUE;
			}
			//�ڷ�������ʼ�������ҵ�������������.��������ֵ��ŭ������ʼ��������Ӫ��ģ�ͱ�š�
			OnUserEnterLevel(pMsgHeader, pGate, pUser, pUser->m_pRoom, &pEnterLevelReq);
		}
			break;
		case MeteorMsg_MsgType_LeaveRoomReq:
			{
				CUserInfo * pUserInfo = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
				if (pUserInfo->m_pRoom != NULL)
				{
					pGate->OnLeaveRoom(pUserInfo);
					pUserInfo->m_pRoom = NULL;
				}
				if (pUserInfo->m_pxPlayerObject != NULL)
					pUserInfo->CloseUserHuman();
			}
			break;
		case MeteorMsg_MsgType_InputReq://MeteorMsg_MsgType_SyncInput��Ϣ�ڷ����̴߳���.

			break;
		case MeteorMsg_MsgType_KeyFrameReq://MeteorMsg_MsgType_SyncKeyFrame��Ϣ�ڷ����̴߳���.
			CUserInfo * pUserInfo = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
			KeyFrame pKeyFrame;
			pKeyFrame.ParseFromArray(data, pMsgHeader->nLength);
			if (pUserInfo->m_pRoom != NULL)
			{
				//�յ���ɫ���ĵ�ǰ����״̬.
				pUserInfo->m_pRoom->OnUserKeyFrame(&pKeyFrame);//���ý�ɫ���µ�״̬������һ�������������·���
			}
			break;
	}

	return TRUE;
}

//���뷿��ɹ�ʱ
void OnUserEnterLevel(_LPTMSGHEADER pMsgHeader, CGateInfo * pGate, CUserInfo * pUser, CRoomInfo * pRoom, EnterLevelReq * pEnterLevelReq)
{
	int spawnPoint = rand() % 16;
	int nIndex = g_xPlayerObjectArr.GetFreeKey();
	if (nIndex >= 0)
	{
		pUser->m_pxPlayerObject = &g_xPlayerObjectArr[nIndex];
		pUser->m_pxPlayerObject->Lock();
		pUser->m_pxPlayerObject->m_bEmpty = false;
		pUser->m_pxPlayerObject->m_nArrIndex = nIndex;
		pUser->m_pxPlayerObject->m_pUserInfo = pUser;
		pUser->m_pxPlayerObject->SetCharName(pUser->m_szCharName);
		pUser->m_pxPlayerObject->Spawn(spawnPoint, pEnterLevelReq->camp(), pEnterLevelReq->model(), pEnterLevelReq->weapon());
		pUser->m_pxPlayerObject->Unlock();
	}
	pRoom->Lock();
	//�����˽��뷿���ڵ�ս������֪�������Ѵ��ڵ�������.���˸ý�ɫ�Լ�.
	OnEnterLevelRsp pOnEnterLevelRsp;
	if (pRoom->m_nCount != 0)
	{
		Player_ * player = pOnEnterLevelRsp.mutable_player();
		pUser->CopyTo(player);
	}

	//�Լ�������Ҳ�����Լ����ڵ������Լ��ظ��ķ����.
	EnterLevelRsp pEnterLevelRsp;
	Player_ * pInsertPlayer = pEnterLevelRsp.mutable_scene()->add_players();
	pUser->CopyTo(pInsertPlayer);

	PLISTNODE no = pRoom->m_pUserList.GetHead();
	while (no != NULL)
	{
		CUserInfo * pUserNode = pRoom->m_pUserList.GetData(no);
		if (pUserNode)
		{
			//����ս���Ľ�ɫ�����Լ���.
			if (pUserNode->m_nUserServerIndex != pUser->m_nUserServerIndex)
			{
				CPlayerObject * pPlayer = pUserNode->m_pxPlayerObject;
				if (pPlayer != NULL)
				{
					//���󷢸���ս����ɫ�ģ�����ս����������ɫ������
					Player_ * pInsertPlayer = pEnterLevelRsp.mutable_scene()->add_players();
					pUserNode->CopyTo(pInsertPlayer);

					tag_TMSGHEADER Msg;
					Msg.nLength = pOnEnterLevelRsp.ByteSize();
					Msg.nSocket = pUserNode->m_sock;
					Msg.wIdent = MeteorMsg_MsgType_OnEnterLevelRsp;//�������˽��볡��.
					Msg.wSessionIndex = pUserNode->m_nUserGateIndex;
					Msg.wUserListIndex = pUserNode->m_nUserServerIndex;

					_LPTSENDBUFF pBuffer = new _TSENDBUFF;
					pBuffer->nLen = sizeof(_TMSGHEADER) + pOnEnterLevelRsp.ByteSize();
					memmove(pBuffer->szData, (char *)&Msg, sizeof(_TMSGHEADER));
					pOnEnterLevelRsp.SerializeToArray(pBuffer->szData + sizeof(_TMSGHEADER), DATA_BUFSIZE - sizeof(_TMSGHEADER));
					pGate->m_xSendBuffQ.PushQ((BYTE *)pBuffer);
				}
			}
		}
		no = pRoom->m_pUserList.GetNext(no);
	}

	pRoom->Unlock();

	//�ѳ�����ͼ�еĳ���ͼ��ȫ�����λ�ã���ת�����ݸ��½��������.
	//ͬʱ���½��������λ����Ϣ����������ɫ��
	pMsgHeader->wIdent = MeteorMsg_MsgType_EnterLevelRsp;
	pMsgHeader->nLength = pEnterLevelRsp.ByteSize();
	_LPTSENDBUFF lpSendBuff = new _TSENDBUFF;
	memmove(lpSendBuff->szData, pMsgHeader, sizeof(tag_TMSGHEADER));
	lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + pEnterLevelRsp.ByteSize();
	pEnterLevelRsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
	pGate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
}

//�ڼ��뷿��ʱ��������ҵ��ǳƣ��Լ�����
//�����뷿�䣬����ѡ���ɫ���������볡��ʱ��������Ϸ��ɫ.
void OnUserJoinRoom(_LPTMSGHEADER msgHead, CGateInfo * pGate,  CUserInfo* pUserInfo, CRoomInfo * pRoom, const char * szName)
{
	pUserInfo->SetName(szName);
	pUserInfo->m_pRoom = pRoom;
	pRoom->Lock();
		
	PLISTNODE no = pRoom->m_pUserList.GetHead();
	while (no != NULL)
	{
		CUserInfo * pUserNode = pRoom->m_pUserList.GetData(no);
		if (pUserNode)
		{
			tag_TMSGHEADER Msg;
			OnEnterRoomRsp rsp;
			rsp.set_playernick(GBK2UTF8(string(szName)).c_str());
			Msg.nLength = rsp.ByteSize();
			Msg.nSocket = pUserNode->m_sock;
			Msg.wIdent = MeteorMsg_MsgType_OnJoinRoomRsp;//�����˽��뷿��
			Msg.wSessionIndex = pUserNode->m_nUserGateIndex;
			Msg.wUserListIndex = pUserNode->m_nUserServerIndex;
			_LPTSENDBUFF pBuffer = new _TSENDBUFF;
			pBuffer->nLen = sizeof(_TMSGHEADER) + rsp.ByteSize();
			memmove(pBuffer->szData, (char *)&Msg, sizeof(_TMSGHEADER));
			rsp.SerializeToArray(pBuffer->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
			pGate->m_xSendBuffQ.PushQ((BYTE*)pBuffer);
			no = pRoom->m_pUserList.GetNext(no);
		}
	}
	//��Է�����ÿһ�����󶼷���һ��֪ͨ����֪���˽�ȥ�˷���.���ǻ�δ����ս��.�Ѿ���ʼ��ʱ��
	if (pRoom->m_nCount == 0)
		pRoom->OnNewTurn();
	pRoom->m_nCount++;
	pRoom->m_pUserList.AddNewNode(pUserInfo);
	pRoom->Unlock();
}

CMirMap * FindMap(int levelIdx)
{
	PLISTNODE node = g_xMirMapList.GetHead();
	while (node != NULL)
	{
		CMirMap * pMap = g_xMirMapList.GetData(node);
		if (pMap->m_nLevelIdx == levelIdx)
			return pMap;
		node = g_xMirMapList.GetNext(node);
	}
	return NULL;
}

CRoomInfo * FindRoom(int roomIdx)
{
	g_xRoomList.Lock();
	CRoomInfo * pRet = NULL;
	PLISTNODE no = g_xRoomList.GetHead();
	while (no != NULL)
	{
		CRoomInfo * pRoom = g_xRoomList.GetData(no);
		if (pRoom->m_nRoomIndex == roomIdx)
		{
			pRet = pRoom;
			break;
		}
		no = g_xRoomList.GetNext(no);
	}
	g_xRoomList.Unlock();
	return pRet;
}