#include "stdafx.h"
extern SOCKET					g_csock;

extern CWHDynamicArray<CSessionInfo>	g_UserInfoArray;
WSAEVENT						g_ClientIoEvent;

#pragma pack(8)
struct CMsg
{
	uint32_t Size;
	uint32_t Message;
};
#pragma pack()
void CloseSession(CSessionInfo* pSessionInfo);
void ProcessGameSvrBoardCastPacket(BYTE *lpMsg)
{
	_LPTMSGHEADER	lpMsgHeader = (_LPTMSGHEADER)lpMsg;
	for (int i = 0; i < _NUM_OF_MAXPLAYER; i++)
	{
		if (lpMsgHeader->wUserList[i] == -1)
			continue;
		CSessionInfo* pSessionInfo = g_UserInfoArray.GetData(lpMsgHeader->wUserList[i]);
		if (!pSessionInfo)
		{
			continue;
		}
		pSessionInfo->SendBuffLock.Lock();
		if ((pSessionInfo->nSendBufferLen + lpMsgHeader->nLength + (int)sizeof(CMsg)) > DATA_BUFSIZE)
		{
			vprint("discard packet serialize size:%d", lpMsgHeader->nLength);
			pSessionInfo->SendBuffLock.Unlock();
			continue;
		}

		char * pszData = &pSessionInfo->SendBuffer[pSessionInfo->nSendBufferLen];
		CMsg msg;
		msg.Size = htonl(lpMsgHeader->nLength + (int)sizeof(CMsg));
		msg.Message = htonl(lpMsgHeader->nMessage);
		memmove(pszData, &msg, sizeof(CMsg));
		memmove(pszData + sizeof(CMsg), lpMsg + (int)sizeof(_TMSGHEADER), lpMsgHeader->nLength);
		pSessionInfo->nSendBufferLen += (lpMsgHeader->nLength + (int)sizeof(CMsg));
		pSessionInfo->SendBuffLock.Unlock();
	}
}

void ProcessGameSvrPacket(BYTE *lpMsg)
{
	_LPTMSGHEADER	lpMsgHeader = (_LPTMSGHEADER)lpMsg;

	CSessionInfo* pSessionInfo = g_UserInfoArray.GetData(lpMsgHeader->wSessionIndex);

	if (!pSessionInfo)
	{
		//print("pSessionInfo == null");
		return;
	}

	pSessionInfo->SendBuffLock.Lock();
	if ((pSessionInfo->nSendBufferLen + lpMsgHeader->nLength + (int)sizeof(CMsg)) > DATA_BUFSIZE)
	{
		vprint("discard packet serialize size:%d", lpMsgHeader->nLength);
		pSessionInfo->SendBuffLock.Unlock();
		return;
	}

	char * pszData = &pSessionInfo->SendBuffer[pSessionInfo->nSendBufferLen];
	CMsg msg;
	msg.Size = htonl(lpMsgHeader->nLength + (int)sizeof(CMsg));
	msg.Message = htonl(lpMsgHeader->wIdent);
	memmove(pszData, &msg, sizeof(CMsg));
	memmove(pszData + sizeof(CMsg), lpMsg + (int)sizeof(_TMSGHEADER), lpMsgHeader->nLength);
	pSessionInfo->nSendBufferLen += (lpMsgHeader->nLength + (int)sizeof(CMsg));
	pSessionInfo->SendBuffLock.Unlock();
}

bool ProcReceiveBuffer(_LPTOVERLAPPEDEX pOverLapped, int nRecv)
{
	pOverLapped->BytesRECV += nRecv;
	int tid = GetCurrentThreadId();
	int				nNext = 0;
	BYTE			*pszData = (BYTE*)&pOverLapped->Buffer[0];
	_LPTMSGHEADER	lpMsgHeader;
	CSessionInfo * pSessionInfo = NULL;
	while (pOverLapped->BytesRECV >= (int)sizeof(_TMSGHEADER))
	{
		lpMsgHeader = (_LPTMSGHEADER)pszData;
		vprint("recv message:%d serialize size:%d, remain:%d", lpMsgHeader->wIdent, lpMsgHeader->nLength, pOverLapped->BytesRECV);
		if (pOverLapped->BytesRECV < (int)(sizeof(_TMSGHEADER) + lpMsgHeader->nLength))
		{
			vprint("extract packet failed! wait for more info current serialize size:%d", lpMsgHeader->nLength);
			break;
		}

		switch (lpMsgHeader->wIdent)
		{
			case GM_CLOSE:
				pSessionInfo = g_UserInfoArray.GetData(lpMsgHeader->wSessionIndex);
				if (pSessionInfo != NULL)
					CloseSession(pSessionInfo);
				break;
			case BOARDCASTS2G:
				ProcessGameSvrBoardCastPacket(pszData);
				break;
			case GM_CHECKSERVER:
				break;
			case GM_SERVERUSERINDEX:
			{
				CSessionInfo* pSessionInfo = g_UserInfoArray.GetData(lpMsgHeader->wSessionIndex);
				if (pSessionInfo)
					pSessionInfo->nServerUserIndex = lpMsgHeader->wUserListIndex;
				break;
			}
			default:
				ProcessGameSvrPacket(pszData);
				break;
		}
		pOverLapped->BytesRECV -= (sizeof(_TMSGHEADER) + abs(lpMsgHeader->nLength));
		pszData += (sizeof(_TMSGHEADER) + abs(lpMsgHeader->nLength));
		
	} // while

	if (pOverLapped->BytesRECV > 0)
	{
		memmove((BYTE*)&pOverLapped->Buffer[0], pszData, pOverLapped->BytesRECV);
		return TRUE;
	}
	return TRUE;
}

BOOL InitServerThreadForMsg()
{
	DWORD	dwThreadIDForMsg = 0;

	HANDLE hThreadForMsg	= CreateThread(NULL, 0, ThreadFuncForMsg,	NULL, 0, &dwThreadIDForMsg);

	if (hThreadForMsg)
	{
		CloseHandle(hThreadForMsg);
	
		return TRUE;
	}

	return FALSE;
}


UINT WINAPI	ClientWorkerThread(LPVOID lpParameter)
{
	_LPTOVERLAPPEDEX	ClientOverlapped = new _TOVERLAPPEDEX();
	DWORD				dwIndex;
	DWORD				dwBytesTransferred;
	DWORD				dwFlags;
	DWORD				dwRecvBytes;
	ZeroMemory(&ClientOverlapped->Overlapped, sizeof(WSAOVERLAPPED));

	ClientOverlapped->Overlapped.hEvent = g_ClientIoEvent;

	ClientOverlapped->DataBuf.len	= DATA_BUFSIZE;
	ClientOverlapped->DataBuf.buf	= &ClientOverlapped->Buffer[0];

	dwFlags = 0;

	if (WSARecv(g_csock, &(ClientOverlapped->DataBuf), 1, &dwRecvBytes, &dwFlags, &(ClientOverlapped->Overlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			print("disconnect with svr");
			return 0;
		}
	}

	while (TRUE)
	{
		dwIndex = WSAWaitForMultipleEvents(1, &g_ClientIoEvent, FALSE, WSA_INFINITE, FALSE);

		WSAResetEvent(g_ClientIoEvent);

		WSAGetOverlappedResult(g_csock, &ClientOverlapped->Overlapped, &dwBytesTransferred, FALSE, &dwFlags);

		if (dwBytesTransferred == 0)
			break;
		ProcReceiveBuffer(ClientOverlapped, dwBytesTransferred);
		dwFlags = 0;

		ZeroMemory(&(ClientOverlapped->Overlapped), sizeof(OVERLAPPED));

		ClientOverlapped->DataBuf.len = DATA_BUFSIZE - ClientOverlapped->BytesRECV;
		ClientOverlapped->DataBuf.buf = &ClientOverlapped->Buffer[ClientOverlapped->BytesRECV];

		ClientOverlapped->Overlapped.hEvent = g_ClientIoEvent;

		if (WSARecv(g_csock, &(ClientOverlapped->DataBuf), 1, &dwRecvBytes, &dwFlags, &(ClientOverlapped->Overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
				break;
		}
	}

	return 0L;
}
