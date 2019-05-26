#include "stdafx.h"

void UpdateStatusBarUsers(BOOL fGrow);

CGateInfo::CGateInfo()
{
	m_fDoSending = FALSE;
	
	memset( &OverlappedEx, 0, sizeof( OverlappedEx ) );
}

void CGateInfo::SendGateCheck()
{
	int k = g_memPool.GetAvailablePosition();
	if (k < 0)
	{
		print("no more memory");
	}
	_LPTSENDBUFF lpSendBuff = g_memPool.GetEmptyElement(k);
	if (lpSendBuff)
	{
		lpSendBuff->nIndex = k;
		_TMSGHEADER		MsgHdr;
		MsgHdr.nSocket			= 0;
		MsgHdr.wSessionIndex	= 0;
		MsgHdr.wIdent			= GM_CHECKSERVER;
		MsgHdr.wUserListIndex	= 0;
		MsgHdr.nLength			= 0;
		lpSendBuff->nLen = sizeof(_TMSGHEADER);
		memmove(lpSendBuff->szData, (char *)&MsgHdr, sizeof(_TMSGHEADER));
		m_xSendBuffQ.PushQ((BYTE *)lpSendBuff);
	}
}

//仅仅是把房间的这个对象剔除。不删除，还可以进入其他房间
//对房间的其他角色，发送玩家离开包。
void CGateInfo::OnLeaveRoom(CUserInfo * pUser)
{
	pUser->m_pRoom->RemovePlayer(pUser);
	if (pUser->m_pRoom->m_pUserList.GetCount() != 0)
	{
		PLISTNODE no = pUser->m_pRoom->m_pUserList.GetHead();
		while (no != NULL)
		{
			CUserInfo * pRoomUser = pUser->m_pRoom->m_pUserList.GetData(no);
			int k = g_memPool.GetAvailablePosition();
			if (k < 0)
			{
				print("no more memory");
			}
			_LPTSENDBUFF lpSendBuff = g_memPool.GetEmptyElement(k);
			OnLeaveRoomRsp pOnLeaveRoomRsp;
			pOnLeaveRoomRsp.set_playerid(pUser->m_nUserServerIndex);
			if (lpSendBuff)
			{
				lpSendBuff->nIndex = k;
				_TMSGHEADER	MsgHdr;
				MsgHdr.nSocket = pRoomUser->m_sock;
				MsgHdr.wSessionIndex = pRoomUser->m_nUserGateIndex;
				MsgHdr.wIdent = MeteorMsg_MsgType_OnLeaveRoomRsp;
				MsgHdr.wUserListIndex = pRoomUser->m_nUserServerIndex;
				MsgHdr.nLength = pOnLeaveRoomRsp.ByteSizeLong();
				lpSendBuff->nLen = sizeof(_TMSGHEADER) + pOnLeaveRoomRsp.ByteSizeLong();
				memmove(lpSendBuff->szData, (char *)&MsgHdr, sizeof(_TMSGHEADER));
				pOnLeaveRoomRsp.SerializeToArray(lpSendBuff->szData + sizeof(_TMSGHEADER), DATA_BUFSIZE - sizeof(_TMSGHEADER));
				m_xSendBuffQ.PushQ((BYTE *)lpSendBuff);
			}
			no = pUser->m_pRoom->m_pUserList.GetNext(no);
		}
	}
	else
	{
		pUser->m_pRoom->OnAllPlayerLeaved();
	}
}

void CGateInfo::OpenNewUser(char *pszPacket)
{
	int					nIndex;
	_TMSGHEADER			MsgHdr;
	_LPTMSGHEADER		lpMsgHeader;
	nIndex = g_xUserInfoArr.GetFreeKey();
	if (nIndex >= 0)
	{
		CUserInfo * pUserInfo = &g_xUserInfoArr[nIndex];

		pUserInfo->Lock();
		
		lpMsgHeader = (_LPTMSGHEADER)pszPacket;

		pUserInfo->m_sock			= lpMsgHeader->nSocket;
		pUserInfo->m_pxPlayerObject	= NULL;

		ZeroMemory(pUserInfo->m_szUserID, sizeof(pUserInfo->m_szUserID));
		ZeroMemory(pUserInfo->m_szCharName, sizeof(pUserInfo->m_szCharName));

		pUserInfo->m_nCertification		= 0;
		pUserInfo->m_nClientVersion		= 0;
		pUserInfo->m_nUserGateIndex		= lpMsgHeader->wSessionIndex;
		pUserInfo->m_nUserServerIndex	= nIndex;
		
		pUserInfo->m_pGateInfo			= this;
		pUserInfo->m_bEmpty = false;
		pUserInfo->Unlock();

		int k = g_memPool.GetAvailablePosition();
		if (k < 0)
		{
			print("no more memory");
		}
		_LPTSENDBUFF lpSendBuff = g_memPool.GetEmptyElement(k);
		MsgHdr.nSocket			= lpMsgHeader->nSocket;
		MsgHdr.wSessionIndex	= lpMsgHeader->wSessionIndex;
		MsgHdr.wIdent			= GM_SERVERUSERINDEX;
		MsgHdr.wUserListIndex	= pUserInfo->m_nUserServerIndex;
		MsgHdr.nLength			= 0;
		if (lpSendBuff != NULL)
		{
			lpSendBuff->nIndex = k;
			lpSendBuff->nLen = sizeof(_TMSGHEADER);
			memmove(lpSendBuff->szData, (char *)&MsgHdr, sizeof(_TMSGHEADER));
			m_xSendBuffQ.PushQ((BYTE *)lpSendBuff);
		}
		g_xUserInfoList.Lock();
		g_xUserInfoList.AddNewNode(pUserInfo);
		g_xUserInfoList.Unlock();
		UpdateStatusBarUsers(TRUE);
	}
}

void CGateInfo::xSend()
{
	if (m_xSendBuffQ.GetCount())
	{
		vprint("packet %d wait for xSend()", m_xSendBuffQ.GetCount());
		DWORD	dwBytesSends = 0;
		DWORD   dwSend = 0;
		_LPTSENDBUFF lpSendBuff = (_LPTSENDBUFF)m_xSendBuffQ.PopQ();

		while (lpSendBuff)
		{
			if (dwSend + lpSendBuff->nLen > DATA_BUFSIZE)
			{
				if (lpSendBuff->nLen > DATA_BUFSIZE)
				{
					vprint("send to gate packet is too long:%d", lpSendBuff->nLen);
					g_memPool.SetEmptyElement(lpSendBuff->nIndex, lpSendBuff);
					break;
				}
				vprint("send to gate packet is too long 2:%d", lpSendBuff->nLen);
				m_xSendBuffQ.PushQ((BYTE*)lpSendBuff);
				break;
			}
			memmove(&OverlappedEx[1].Buffer[OverlappedEx[1].bufLen], lpSendBuff->szData, lpSendBuff->nLen);
			dwSend += lpSendBuff->nLen;
			OverlappedEx[1].bufLen += lpSendBuff->nLen;
			g_memPool.SetEmptyElement(lpSendBuff->nIndex, lpSendBuff);
			lpSendBuff = (_LPTSENDBUFF)m_xSendBuffQ.PopQ();
		}

		if (dwSend != 0)
		{
			memset( &OverlappedEx[1].Overlapped, 0, sizeof( OVERLAPPED ) );
			OverlappedEx[1].nOvFlag		= OVERLAPPED_FLAG::OVERLAPPED_SEND;
			OverlappedEx[1].DataBuf.len	= dwSend;
			OverlappedEx[1].DataBuf.buf	= OverlappedEx[1].Buffer;
			vprint("send to gate start size %d", dwSend);
			m_fDoSending = true;
			int hResult = WSASend(
				m_sock, 
				&OverlappedEx[1].DataBuf, 
				1, 
				&dwBytesSends, 
				0, 
				(OVERLAPPED *) &OverlappedEx[1], 
				NULL
				);
			if (hResult == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
					print("WSASend failed");
			}
		}
	}
}

int CGateInfo::Recv()
{
	DWORD nRecvBytes = 0, nFlags = 0;

	OverlappedEx[0].nOvFlag		= OVERLAPPED_FLAG::OVERLAPPED_RECV;
	OverlappedEx[0].DataBuf.len = DATA_BUFSIZE - OverlappedEx[0].bufLen;
	OverlappedEx[0].DataBuf.buf = OverlappedEx[0].Buffer + OverlappedEx[0].bufLen;

	memset( &OverlappedEx[0].Overlapped, 0, sizeof( OverlappedEx[0].Overlapped ) );

	return WSARecv( m_sock, &OverlappedEx[0].DataBuf, 1, &nRecvBytes, &nFlags, &OverlappedEx[0].Overlapped, 0 );
}

bool CGateInfo::HasCompletionPacket()
{
	if (OverlappedEx[0].bufLen < sizeof(tag_TMSGHEADER))
		return false;
	_LPTMSGHEADER p = (_LPTMSGHEADER)OverlappedEx[0].Buffer;
	if (OverlappedEx[0].bufLen < p->nLength + sizeof(tag_TMSGHEADER))
		return FALSE;
	return TRUE;
}

int CGateInfo::ExtractPacket(char *pPacket)
{
	int packetLen = 0;
	_LPTMSGHEADER p = (_LPTMSGHEADER)OverlappedEx[0].Buffer;
	packetLen = p->nLength + sizeof(tag_TMSGHEADER);
	memcpy(pPacket, OverlappedEx[0].Buffer, packetLen);
	memmove(OverlappedEx[0].Buffer, OverlappedEx[0].Buffer + packetLen, DATA_BUFSIZE - packetLen);
	OverlappedEx[0].bufLen -= packetLen;
	return packetLen;
}
