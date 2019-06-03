#include "stdafx.h"
//global fun
CRoomInfo * FindRoom(int roomIdx);
bool OnProtocolVerify(ProtocolVerifyReq * pReq, int * p_reason);
void OnUserJoinRoom(_LPTMSGHEADER msgHead, CGateInfo * pGate, CUserInfo* pUserInfo, CRoomInfo * pRoom, const char * szName);
void OnUserEnterLevel(_LPTMSGHEADER pMsgHeader, CGateInfo * pGate, CUserInfo * pUser, CRoomInfo * pRoom, EnterLevelReq * pEnterLevelReq);
//void OnUserReborn(_LPTMSGHEADER pMsgHeader, CGateInfo * pGate, CUserInfo * pUser, CRoomInfo * pRoom, UserId * pRebornReq);
void OnUserAudioChatInRoom(_LPTMSGHEADER msgHead, CGateInfo * pGate, CUserInfo* pUserInfo, CRoomInfo * pRoom, AudioChatMsg * pMsg);
void OnUserChatInRoom(_LPTMSGHEADER msgHead, CGateInfo * pGate, CUserInfo* pUserInfo, CRoomInfo * pRoom, ChatMsg * pMsg);
void ProcessGameGateMsgTail(CGateInfo * pGate, int offset);
int ProcessGameGateMsg(CGateInfo * pGate, int offset);
DWORD WINAPI AcceptThread(LPVOID lpParameter)
{
	int					nLen = sizeof(SOCKADDR_IN);

	SOCKET				Accept;
	SOCKADDR_IN			Address;

	while (TRUE)
	{
		nLen	= sizeof( Address );
		print("wait for gamegate connect");
		Accept	= WSAAccept( g_ssock, (SOCKADDR *) &Address, &nLen, NULL, 0 );
		print("game gate connected");
		if (g_fTerminated) return 0L;

		CGateInfo* pGateInfo = new CGateInfo;

		if (pGateInfo)
		{
			pGateInfo->m_sock			= Accept;

			CreateIoCompletionPort((HANDLE)pGateInfo->m_sock, g_hIOCP, (DWORD)pGateInfo, 0);

			int zero = 0;
			setsockopt(pGateInfo->m_sock, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero));
			ZeroMemory(&(pGateInfo->OverlappedEx), sizeof(OVERLAPPED) * 2);
			pGateInfo->Lock();
			pGateInfo->Recv();
			g_xGateList.AddNewNode(pGateInfo);
			pGateInfo->Unlock();
		}
	}

	return 0;
}

DWORD WINAPI ServerWorkerThread(LPVOID CompletionPortID)
{
	DWORD				dwBytesTransferred;
	CGateInfo*			pGateInfo;
	LPOVERLAPPEDEX		lpOverlapped;
	while (TRUE)
	{
		GetQueuedCompletionStatus((HANDLE)CompletionPortID, &dwBytesTransferred, (LPDWORD)&pGateInfo, (LPOVERLAPPED *)&lpOverlapped, INFINITE);
		if (g_fTerminated)
			return 0L;

		if (dwBytesTransferred == 0)
		{
			vprint("Gamesvr disconnect with gate !!!  dwBytesTransferred == 0");
			g_xUserInfoList.Lock();
			if (g_xUserInfoList.GetCount())
			{
				PLISTNODE pListNode = g_xUserInfoList.GetHead();
				while (pListNode)
				{
					CUserInfo *pUserInfo = g_xUserInfoList.GetData(pListNode);

					if (pUserInfo->m_pGateInfo == pGateInfo)
					{
						pUserInfo->Lock();
						pUserInfo->m_bEmpty = true;
						pUserInfo->m_pGateInfo = NULL;
						pUserInfo->Unlock();
					}
					pListNode = g_xUserInfoList.GetNext(pListNode);
				}
			}
			g_xUserInfoList.Unlock();
			ClearSocket(pGateInfo->m_sock);
			g_xGateList.Lock();
			g_xGateList.RemoveNodeByData(pGateInfo);
			g_xGateList.Unlock();
			continue;
		}

		if (lpOverlapped->nOvFlag == OVERLAPPED_FLAG::OVERLAPPED_RECV)
		{
			pGateInfo->OverlappedEx[0].bufLen += dwBytesTransferred;
			int offset = 0;
			while (pGateInfo->HasCompletionPacket(offset))
			{
				ProcessGameGateMsg(pGateInfo, offset);
				int nextOffset = pGateInfo->NextPacketOffset(offset);
				pGateInfo->OverlappedEx[0].bufLen -= (nextOffset - offset);
				offset = nextOffset;
			}

			//还有余下的数据，不足以解开包，把剩余数据移到缓冲区首部.
			if (offset != 0)
				ProcessGameGateMsgTail(pGateInfo, offset);

			if (pGateInfo->Recv() == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
				print("pGateInfo->Recv() Failed!!!");
		}
		else if (lpOverlapped->nOvFlag == OVERLAPPED_FLAG::OVERLAPPED_SEND)
		{
			pGateInfo->Lock();
			pGateInfo->OverlappedEx[1].bufLen = 0;
			vprint("send to gate complete size %d", dwBytesTransferred);
			pGateInfo->m_fDoSending = false;
			pGateInfo->Unlock();
		}
	}

	return 0;
}

void CloseUser(CGateInfo * pGate, CUserInfo * pUserInfo, int userIndex)
{
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
		g_xUserInfoList.Lock();
		g_xUserInfoList.RemoveNodeByData(&g_xUserInfoArr[userIndex]);
		g_xUserInfoList.Unlock();
	}
}

//处理无法解包出的尾部，把尾部内存移动到首
void ProcessGameGateMsgTail(CGateInfo * pGate, int offset)
{
	memmove(pGate->OverlappedEx[0].Buffer, &pGate->OverlappedEx[0].Buffer[offset], pGate->OverlappedEx[0].bufLen);
}

//返回下一个包的偏移.
int ProcessGameGateMsg(CGateInfo * pGate, int offset)
{
	char * pBytes = (char*)&pGate->OverlappedEx[0].Buffer[offset];
	_LPTMSGHEADER pMsgHeader = (_LPTMSGHEADER)(pBytes);
	CHAR * data = NULL;
	if (pMsgHeader->nLength != 0)
		data = (pBytes + sizeof(tag_TMSGHEADER));

	switch (pMsgHeader->wIdent)
	{
		case GM_CHECKCLIENT:
			pGate->SendGateCheck();
			break;
		case GM_OPEN:
			pGate->OpenNewUser(pBytes);
			break;
		case GM_CLOSE:
			{
				CUserInfo * pUserInfo = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
				CloseUser(pGate, pUserInfo, pMsgHeader->wUserListIndex);
			}
			break;
		case MeteorMsg_MsgType_ChatInRoomReq:
		{
			ChatMsg pReq;
			pReq.ParseFromArray(data, pMsgHeader->nLength);
			CUserInfo * pUser = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
			if (pUser->m_pRoom)
				OnUserChatInRoom(pMsgHeader, pGate, pUser, pUser->m_pRoom, &pReq);
		}
			break;
		case MeteorMsg_MsgType_AudioChat:
		{
			AudioChatMsg pReq;
			pReq.ParseFromArray(data, pMsgHeader->nLength);
			CUserInfo * pUser = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
			if (pUser->m_pRoom)
				OnUserAudioChatInRoom(pMsgHeader, pGate, pUser, pUser->m_pRoom, &pReq);
		}
			break;
		case MeteorMsg_MsgType_ProtocolVerify:
		{
			ProtocolVerifyReq pReq;
			pReq.ParseFromArray(data, pMsgHeader->nLength);
			ProtocolVerifyRsp pRsp;
			int reason = 0;
			bool verifyOK = OnProtocolVerify(&pReq, &reason);
			if (verifyOK)
			{
				CUserInfo * pUserInfo = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
				pUserInfo->DoClientCertification(pReq.protocol());
				//pRsp.set_result(1);
				//pRsp.set_message("");
				//pRsp.set_secret("");
			}
			else
			{
				CUserInfo * pUserInfo = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
				CloseUser(pGate, pUserInfo, pMsgHeader->wUserListIndex);
				pGate->DisconnectClient(pMsgHeader->wSessionIndex);
				return TRUE;
			}

			int k = g_memPool.GetAvailablePosition();
			if (k < 0)
			{
				print("no more memory");
			}
			_LPTSENDBUFF lpSendBuff = g_memPool.GetEmptyElement(k);
			if (lpSendBuff != NULL)
				lpSendBuff->nIndex = k;

			pRsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
			lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + pRsp.ByteSize();
			pMsgHeader->wIdent = (WORD)MeteorMsg_MsgType_ProtocolVerify;
			pMsgHeader->nLength = pRsp.ByteSize();
			memmove(lpSendBuff->szData, pMsgHeader, sizeof(tag_TMSGHEADER));
			pGate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
		}
		break;
		case MeteorMsg_MsgType_GetRoomReq:
		{
			CUserInfo * pUserInfo = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
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
					room->set_levelidx(pRoom->m_dwMap);
					room->set_group1(pRoom->m_nGroup1);
					room->set_group2(pRoom->m_nGroup2);
					room->set_maxplayer(pRoom->m_nMaxPlayer);
					room->set_playercount(pRoom->m_nCount);
				}
				pNode = g_xRoomList.GetNext(pNode);
			}
			g_xRoomList.Unlock();
			int k = g_memPool.GetAvailablePosition();
			if (k < 0)
			{
				print("no more memory");
			}
			_LPTSENDBUFF lpSendBuff = g_memPool.GetEmptyElement(k);
			if (lpSendBuff != NULL)
				lpSendBuff->nIndex = k;
			pGetRoomRsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
			lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + pGetRoomRsp.ByteSizeLong();
			pMsgHeader->wIdent = MeteorMsg_MsgType_GetRoomRsp;
			pMsgHeader->nLength = pGetRoomRsp.ByteSizeLong();
			memmove(lpSendBuff->szData, pMsgHeader, sizeof(tag_TMSGHEADER));
			pGate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
		}
		break;
		case MeteorMsg_MsgType_CreateRoomReq:
		{
			CUserInfo * pUser = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
			if (pUser->m_pRoom)
			{
				//当前已经在房间内，需要先退出房间。
				CreateRoomRsp pCreateRoomRsp;
				pCreateRoomRsp.set_result(0);
				pCreateRoomRsp.set_levelid(0);
				pCreateRoomRsp.set_roomid(0);
				int k = g_memPool.GetAvailablePosition();
				if (k < 0)
				{
					print("no more memory");
				}
				_LPTSENDBUFF lpSendBuff = g_memPool.GetEmptyElement(k);
				if (lpSendBuff != NULL)
				{
					lpSendBuff->nIndex = k;
					pCreateRoomRsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
					lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + pCreateRoomRsp.ByteSizeLong();
					pMsgHeader->wIdent = MeteorMsg_MsgType_CreateRoomRsp;
					pMsgHeader->nLength = pCreateRoomRsp.ByteSizeLong();
					memmove(lpSendBuff->szData, pMsgHeader, sizeof(tag_TMSGHEADER));
					pGate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
				}
			}
			else
			{
				int RoomIdx = g_xRoom.GetFreeKey();
				if (RoomIdx == -1)
				{
					CreateRoomRsp pCreateRoomRsp;
					pCreateRoomRsp.set_result(0);
					pCreateRoomRsp.set_levelid(0);
					pCreateRoomRsp.set_roomid(0);
					//can not create more room.
					int k = g_memPool.GetAvailablePosition();
					if (k < 0)
					{
						print("no more memory");
					}
					_LPTSENDBUFF lpSendBuff = g_memPool.GetEmptyElement(k);
					if (lpSendBuff != NULL)
					{
						lpSendBuff->nIndex = k;
						pCreateRoomRsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
						lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + pCreateRoomRsp.ByteSizeLong();
						pMsgHeader->wIdent = MeteorMsg_MsgType_CreateRoomRsp;
						pMsgHeader->nLength = pCreateRoomRsp.ByteSizeLong();
						memmove(lpSendBuff->szData, pMsgHeader, sizeof(tag_TMSGHEADER));
						pGate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
					}
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
					pRoom->m_dwMap = pCreateRoomReq.levelidx();
					pRoom->InitKcpServer();
					g_xRoomList.Lock();
					g_xRoomList.AddNewNode(pRoom);
					g_xRoomList.Unlock();

					CreateRoomRsp pCreateRoomRsp;
					pCreateRoomRsp.set_result(1);
					pCreateRoomRsp.set_roomid(pRoom->m_nRoomIndex);
					pCreateRoomRsp.set_levelid(pRoom->m_dwMap);
					pMsgHeader->wIdent = MeteorMsg_MsgType_CreateRoomRsp;
					pMsgHeader->nLength = pCreateRoomRsp.ByteSizeLong();
					int k = g_memPool.GetAvailablePosition();
					if (k < 0)
					{
						print("no more memory");
					}
					_LPTSENDBUFF lpSendBuff = g_memPool.GetEmptyElement(k);
					if (lpSendBuff != NULL)
					{
						lpSendBuff->nIndex = k;
						memmove(lpSendBuff->szData, pMsgHeader, sizeof(tag_TMSGHEADER));
						lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + pCreateRoomRsp.ByteSizeLong();
						pCreateRoomRsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
						pGate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
					}
				}
			}
		}
		break;
		case MeteorMsg_MsgType_JoinRoomReq:
		{
			/*JoinRoomReq pJoinRoomReq;
			pJoinRoomReq.ParseFromArray(data, pMsgHeader->nLength);
			CRoomInfo * pRoom = FindRoom(pJoinRoomReq.roomid());
			JoinRoomRsp pJoinRoomRsp;
			if (pRoom == NULL)
			{
				pJoinRoomRsp.set_result(0);
				pJoinRoomRsp.set_reason(2);
				pJoinRoomRsp.set_levelidx(0);
				pJoinRoomRsp.set_playerid(0);
				pJoinRoomRsp.set_roomid(0);
				pJoinRoomRsp.set_usernick("");
			}
			else
			{
				CUserInfo * pUser = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
				if (pUser->m_pRoom)
				{
					pJoinRoomRsp.set_result(0);
					pJoinRoomRsp.set_reason(3);
					pJoinRoomRsp.set_levelidx(0);
					pJoinRoomRsp.set_playerid(0);
					pJoinRoomRsp.set_roomid(pRoom->m_nRoomIndex);
					pJoinRoomRsp.set_usernick("");
				}
				else
				if (pRoom->m_nMaxPlayer <= pRoom->m_nCount)
				{
					pJoinRoomRsp.set_result(0);
					pJoinRoomRsp.set_reason(1);
					pJoinRoomRsp.set_levelidx(0);
					pJoinRoomRsp.set_playerid(0);
					pJoinRoomRsp.set_roomid(pRoom->m_nRoomIndex);
					pJoinRoomRsp.set_usernick("");
				}
				else if (pRoom->m_bHasPsd)
				{
					if (pRoom->m_dwOwnerId == pMsgHeader->wUserListIndex || strcmp(pRoom->m_szPassword, pJoinRoomReq.secret().c_str()) == 0)
					{
						pJoinRoomRsp.set_result(1);
						pJoinRoomRsp.set_reason(0);
						pJoinRoomRsp.set_levelidx(pRoom->m_dwMap);
						pJoinRoomRsp.set_playerid(pMsgHeader->wUserListIndex);
						pJoinRoomRsp.set_roomid(pRoom->m_nRoomIndex);
						pJoinRoomRsp.set_usernick(GBK2UTF8(pUser->m_szCharName));
						OnUserJoinRoom(pMsgHeader, pGate, pUser, pRoom, UTF82GBK(pJoinRoomReq.usernick()).c_str());
					}
					else
					{
						pJoinRoomRsp.set_result(0);
						pJoinRoomRsp.set_reason(4);
						pJoinRoomRsp.set_levelidx(0);
						pJoinRoomRsp.set_playerid(0);
						pJoinRoomRsp.set_roomid(pRoom->m_nRoomIndex);
						pJoinRoomRsp.set_usernick("");
					}
				}
				else
				{
					pJoinRoomRsp.set_result(1);
					pJoinRoomRsp.set_reason(0);
					pJoinRoomRsp.set_levelidx(pRoom->m_dwMap);
					pJoinRoomRsp.set_playerid(pMsgHeader->wUserListIndex);
					pJoinRoomRsp.set_roomid(pRoom->m_nRoomIndex);
					pJoinRoomRsp.set_usernick(GBK2UTF8(pUser->m_szCharName));
					OnUserJoinRoom(pMsgHeader, pGate, pUser, pRoom, UTF82GBK(pJoinRoomReq.usernick()).c_str());
				}
			}

			pMsgHeader->wIdent = MeteorMsg_MsgType_JoinRoomRsp;
			pMsgHeader->nLength = pJoinRoomRsp.ByteSizeLong();
			int k = g_memPool.GetAvailablePosition();
			if (k < 0)
			{
				print("no more memory");
			}
			_LPTSENDBUFF lpSendBuff = g_memPool.GetEmptyElement(k);
			if (lpSendBuff != NULL)
				lpSendBuff->nIndex = k;
			memmove(lpSendBuff->szData, pMsgHeader, sizeof(tag_TMSGHEADER));
			lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + pJoinRoomRsp.ByteSizeLong();
			pJoinRoomRsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
			pGate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);*/
		}
			break;
		//case MeteorMsg_MsgType_UserRebornReq:
		//{
		//	UserId pRebornReq;
		//	pRebornReq.ParseFromArray(data, pMsgHeader->nLength);
		//	CUserInfo * pUser = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
		//	if (pUser == NULL || pUser->m_pxPlayerObject == NULL || pUser->m_pRoom == NULL)
		//	{
		//		return TRUE;
		//	}
		//	OnUserReborn(pMsgHeader, pGate, pUser, pUser->m_pRoom, &pRebornReq);
		//}
		//break;	
		/*case MeteorMsg_MsgType_EnterLevelReq:
		{
			EnterLevelReq pEnterLevelReq;
			pEnterLevelReq.ParseFromArray(data, pMsgHeader->nLength);
			CUserInfo * pUser = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
			if (pUser == NULL || pUser->m_pxPlayerObject != NULL || pUser->m_pRoom == NULL)
			{
				return TRUE;
			}
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
			break;*/
		//case MeteorMsg_MsgType_InputReq://MeteorMsg_MsgType_SyncInput

		//	break;
		//case MeteorMsg_MsgType_KeyFrameReq://MeteorMsg_MsgType_SyncKeyFrame
			//CUserInfo * pUserInfo = &g_xUserInfoArr[pMsgHeader->wUserListIndex];
			//KeyFrame pKeyFrame;
			//pKeyFrame.ParseFromArray(data, pMsgHeader->nLength);
			//if (pUserInfo->m_pRoom != NULL)
			//{
			//	//print("user key frame req");
			//	pUserInfo->m_pRoom->OnUserKeyFrame(&pKeyFrame);
			//}
		//	break;
	}

	return TRUE;
}

//广播
//void OnUserReborn(_LPTMSGHEADER pMsgHeader, CGateInfo * pGate, CUserInfo * pUser, CRoomInfo * pRoom, UserId * pRebornReq)
//{
//	pRoom->Lock();
//	OnEnterLevelRsp pOnEnterLevelRsp;
//	if (pRoom->m_nCount != 0)
//	{
//		Player_ * player = pOnEnterLevelRsp.mutable_player();
//		pUser->m_pxPlayerObject->Reborn();
//		pUser->CopyTo(player);
//	}
//	PLISTNODE no = pRoom->m_pUserList.GetHead();
//	while (no != NULL)
//	{
//		CUserInfo * pUserNode = pRoom->m_pUserList.GetData(no);
//		if (pUserNode)
//		{
//			CPlayerObject * pPlayer = pUserNode->m_pxPlayerObject;
//			if (pPlayer != NULL)
//			{
//				tag_TMSGHEADER Msg;
//				Msg.nLength = pOnEnterLevelRsp.ByteSizeLong();
//				Msg.nSocket = pUserNode->m_sock;
//				Msg.wIdent = MeteorMsg_MsgType_UserRebornSB2C;
//				Msg.wSessionIndex = pUserNode->m_nUserGateIndex;
//				Msg.wUserListIndex = pUserNode->m_nUserServerIndex;
//				int k = g_memPool.GetAvailablePosition();
//				if (k < 0)
//				{
//					print("no more memory");
//				}
//				_LPTSENDBUFF pBuffer = g_memPool.GetEmptyElement(k);
//				if (pBuffer != NULL)
//					pBuffer->nIndex = k;
//				pBuffer->nLen = sizeof(_TMSGHEADER) + pOnEnterLevelRsp.ByteSizeLong();
//				memmove(pBuffer->szData, (char *)&Msg, sizeof(_TMSGHEADER));
//				pOnEnterLevelRsp.SerializeToArray(pBuffer->szData + sizeof(_TMSGHEADER), DATA_BUFSIZE - sizeof(_TMSGHEADER));
//				pGate->m_xSendBuffQ.PushQ((BYTE *)pBuffer);
//			}
//		}
//		no = pRoom->m_pUserList.GetNext(no);
//	}
//
//	pRoom->Unlock();
//}
//
////广播
//void OnUserEnterLevel(_LPTMSGHEADER pMsgHeader, CGateInfo * pGate, CUserInfo * pUser, CRoomInfo * pRoom, EnterLevelReq * pEnterLevelReq)
//{
//	int spawnPoint = rand() % 16;
//	int nIndex = g_xPlayerObjectArr.GetFreeKey();
//	if (nIndex >= 0)
//	{
//		pUser->m_pxPlayerObject = &g_xPlayerObjectArr[nIndex];
//		pUser->m_pxPlayerObject->Lock();
//		pUser->m_pxPlayerObject->Reset(pUser);
//		pUser->m_pxPlayerObject->m_bEmpty = false;
//		pUser->m_pxPlayerObject->m_nArrIndex = nIndex;
//		pUser->m_pxPlayerObject->SetCharName(pUser->m_szCharName);
//		pUser->m_pxPlayerObject->Spawn(spawnPoint, pEnterLevelReq->camp(), pEnterLevelReq->model(), pEnterLevelReq->weapon());
//		pUser->m_pxPlayerObject->Unlock();
//	}
//	else
//	{
//		//print("nIndex < 0");
//	}
//	pRoom->Lock();
//	OnEnterLevelRsp pOnEnterLevelRsp;
//	if (pRoom->m_nCount != 0)
//	{
//		Player_ * player = pOnEnterLevelRsp.mutable_player();
//		pUser->CopyTo(player);
//	}
//
//	EnterLevelRsp pEnterLevelRsp;
//	Player_ * pInsertPlayer = pEnterLevelRsp.mutable_scene()->add_players();
//	pUser->CopyTo(pInsertPlayer);
//
//	PLISTNODE no = pRoom->m_pUserList.GetHead();
//	while (no != NULL)
//	{
//		CUserInfo * pUserNode = pRoom->m_pUserList.GetData(no);
//		if (pUserNode)
//		{
//			if (pUserNode->m_nUserServerIndex != pUser->m_nUserServerIndex)
//			{
//				CPlayerObject * pPlayer = pUserNode->m_pxPlayerObject;
//				if (pPlayer != NULL)
//				{
//					Player_ * pInsertPlayer = pEnterLevelRsp.mutable_scene()->add_players();
//					pUserNode->CopyTo(pInsertPlayer);
//
//					tag_TMSGHEADER Msg;
//					Msg.nLength = pOnEnterLevelRsp.ByteSizeLong();
//					Msg.nSocket = pUserNode->m_sock;
//					Msg.wIdent = MeteorMsg_MsgType_OnEnterLevelRsp;
//					Msg.wSessionIndex = pUserNode->m_nUserGateIndex;
//					Msg.wUserListIndex = pUserNode->m_nUserServerIndex;
//					int k = g_memPool.GetAvailablePosition();
//					if (k < 0)
//						print("no more memory");
//					_LPTSENDBUFF pBuffer = g_memPool.GetEmptyElement(k);
//					if (pBuffer != NULL)
//					{
//						pBuffer->nIndex = k;
//						pBuffer->nLen = sizeof(_TMSGHEADER) + pOnEnterLevelRsp.ByteSizeLong();
//						memmove(pBuffer->szData, (char *)&Msg, sizeof(_TMSGHEADER));
//						pOnEnterLevelRsp.SerializeToArray(pBuffer->szData + sizeof(_TMSGHEADER), DATA_BUFSIZE - sizeof(_TMSGHEADER));
//						pGate->m_xSendBuffQ.PushQ((BYTE *)pBuffer);
//					}
//				}
//			}
//		}
//		no = pRoom->m_pUserList.GetNext(no);
//	}
//
//	pRoom->Unlock();
//	pMsgHeader->wIdent = MeteorMsg_MsgType_EnterLevelRsp;
//	pMsgHeader->nLength = pEnterLevelRsp.ByteSizeLong();
//	int k = g_memPool.GetAvailablePosition();
//	if (k < 0)
//		print("no more memory");
//	_LPTSENDBUFF lpSendBuff = g_memPool.GetEmptyElement(k);
//	if (lpSendBuff != NULL)
//	{
//		lpSendBuff->nIndex = k;
//		memmove(lpSendBuff->szData, pMsgHeader, sizeof(tag_TMSGHEADER));
//		lpSendBuff->nLen = sizeof(tag_TMSGHEADER) + pEnterLevelRsp.ByteSizeLong();
//		pEnterLevelRsp.SerializeToArray(lpSendBuff->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
//		pGate->m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
//	}
//}

//广播
void OnUserAudioChatInRoom(_LPTMSGHEADER msgHead, CGateInfo * pGate, CUserInfo* pUserInfo, CRoomInfo * pRoom, AudioChatMsg * pMsg)
{
	pRoom->Lock();
	PLISTNODE no = pRoom->m_pUserList.GetHead();
	while (no != NULL)
	{
		CUserInfo * pUserNode = pRoom->m_pUserList.GetData(no);
		if (pUserNode)
		{
			tag_TMSGHEADER Msg;
			Msg.nLength = pMsg->ByteSize();
			Msg.nSocket = pUserNode->m_sock;
			Msg.wIdent = BOARDCASTS2G;
			Msg.nMessage = MeteorMsg_MsgType_AudioChat;
			Msg.wSessionIndex = pUserNode->m_nUserGateIndex;
			Msg.wUserListIndex = pUserNode->m_nUserServerIndex;
			int k = g_memPool.GetAvailablePosition();
			if (k < 0)
				print("no more memory");
			_LPTSENDBUFF pBuffer = g_memPool.GetEmptyElement(k);
			if (pBuffer != NULL)
			{
				pBuffer->nLen = sizeof(_TMSGHEADER) + pMsg->ByteSize();
				memmove(pBuffer->szData, (char *)&Msg, sizeof(_TMSGHEADER));
				pMsg->SerializeToArray(pBuffer->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
				pGate->m_xSendBuffQ.PushQ((BYTE*)pBuffer);
			}
			no = pRoom->m_pUserList.GetNext(no);
		}
	}
	pRoom->Unlock();
}

//广播
void OnUserChatInRoom(_LPTMSGHEADER msgHead, CGateInfo * pGate, CUserInfo* pUserInfo, CRoomInfo * pRoom, ChatMsg * pMsg)
{
	pRoom->Lock();
	PLISTNODE no = pRoom->m_pUserList.GetHead();
	while (no != NULL)
	{
		CUserInfo * pUserNode = pRoom->m_pUserList.GetData(no);
		if (pUserNode)
		{
			tag_TMSGHEADER Msg;
			Msg.nLength = pMsg->ByteSize();
			Msg.nSocket = pUserNode->m_sock;
			Msg.wIdent = BOARDCASTS2G;
			Msg.nMessage = MeteorMsg_MsgType_ChatInRoomRsp;
			Msg.wSessionIndex = pUserNode->m_nUserGateIndex;
			Msg.wUserListIndex = pUserNode->m_nUserServerIndex;
			_LPTSENDBUFF pBuffer = new _TSENDBUFF;
			pBuffer->nLen = sizeof(_TMSGHEADER) + pMsg->ByteSize();
			memmove(pBuffer->szData, (char *)&Msg, sizeof(_TMSGHEADER));
			pMsg->SerializeToArray(pBuffer->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
			pGate->m_xSendBuffQ.PushQ((BYTE*)pBuffer);
			no = pRoom->m_pUserList.GetNext(no);
		}
	}
	pRoom->Unlock();
}

//广播
void OnUserJoinRoom(_LPTMSGHEADER msgHead, CGateInfo * pGate,  CUserInfo* pUserInfo, CRoomInfo * pRoom, const char * szName)
{
	/*pUserInfo->SetName(szName);
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
			Msg.nLength = rsp.ByteSizeLong();
			Msg.nSocket = pUserNode->m_sock;
			Msg.wIdent = MeteorMsg_MsgType_OnJoinRoomRsp;
			Msg.wSessionIndex = pUserNode->m_nUserGateIndex;
			Msg.wUserListIndex = pUserNode->m_nUserServerIndex;
			int k = g_memPool.GetAvailablePosition();
			if (k < 0)
			{
				print("no more memory");
			}
			_LPTSENDBUFF pBuffer = g_memPool.GetEmptyElement(k);
			if (pBuffer != NULL)
				pBuffer->nIndex = k;
			pBuffer->nLen = sizeof(_TMSGHEADER) + rsp.ByteSizeLong();
			memmove(pBuffer->szData, (char *)&Msg, sizeof(_TMSGHEADER));
			rsp.SerializeToArray(pBuffer->szData + sizeof(tag_TMSGHEADER), DATA_BUFSIZE - sizeof(tag_TMSGHEADER));
			pGate->m_xSendBuffQ.PushQ((BYTE*)pBuffer);
			no = pRoom->m_pUserList.GetNext(no);
		}
	}

	pRoom->m_nCount++;
	if (pRoom->m_nCount != 0)
		pRoom->OnNewTurn();
	pRoom->m_pUserList.AddNewNode(pUserInfo);
	pRoom->Unlock();*/
}

bool OnProtocolVerify(ProtocolVerifyReq * pReq, int * p_reason)
{
	int v = pReq->protocol();
	string s = pReq->data();
	if (v >= 20190404)
		return true;
	*p_reason = 10;
	return false;
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