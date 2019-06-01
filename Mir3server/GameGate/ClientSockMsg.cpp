#include "stdafx.h"
extern SOCKET					g_csock;

extern HWND						g_hMainWnd;
extern HWND						g_hStatusBar;

extern CWHDynamicArray<CSessionInfo>	g_UserInfoArray;


HANDLE							g_hThreadForComm = NULL;
HANDLE							g_hSvrMsgEvnt = NULL;

char							g_szRemainBuff[DATA_BUFSIZE];
int								g_nRemainBuffLen = 0;

WSAEVENT						g_ClientIoEvent;

#pragma pack(8)
struct CMsg
{
	uint32_t Size;
	uint32_t Message;
};
#pragma pack()

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
			//print("pSessionInfo == null");
			continue;
		}
		pSessionInfo->SendBuffLock.Lock();
		if ((pSessionInfo->nSendBufferLen + lpMsgHeader->nLength + (int)sizeof(CMsg)) > DATA_BUFSIZE)
		{
			//������һ����Ϸ���İ�
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
		//������һ����Ϸ���İ�
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

//������Ϸ����������Ϣ.
bool ProcReceiveBuffer(char *pszPacket, int nRecv)
{
	int limit = nRecv + g_nRemainBuffLen;
	int tid = GetCurrentThreadId();
	if (limit > DATA_BUFSIZE)
	{
		vprint("recv total size:%d > DATA_BUFSIZE:%d !!!!", limit, DATA_BUFSIZE);
		vprint("ProcReceiveBuffer return false reason %d", 0);
		g_nRemainBuffLen = 0;
		return FALSE;
	}
	int				nNext = 0;
	BYTE			*pszData = (BYTE*)&g_szRemainBuff[0];
	_LPTMSGHEADER	lpMsgHeader;
	memmove(pszData + g_nRemainBuffLen, pszPacket, nRecv);

	g_nRemainBuffLen += nRecv;

	while (g_nRemainBuffLen >= (int)sizeof(_TMSGHEADER))
	{
		lpMsgHeader = (_LPTMSGHEADER)pszData;
		//�յ�����ϢID�����л���Ϣ����
		vprint("recv message:%d serialize size:%d, g_nRemainBufflen:%d", lpMsgHeader->wIdent, lpMsgHeader->nLength, g_nRemainBuffLen);
		if (g_nRemainBuffLen < (int)(sizeof(_TMSGHEADER) + lpMsgHeader->nLength))
		{
			vprint("remain len: %d extract packet failed! wait for more info current serialize size:%d", g_nRemainBuffLen, lpMsgHeader->nLength);
			break;
		}
		if ((int)(sizeof(_TMSGHEADER) + lpMsgHeader->nLength) > DATA_BUFSIZE)
		{
			vprint("packet size > DATA_BUFSIZE  ��Ϣid:%d tid:%d, serailize size:%d", lpMsgHeader->wIdent, tid, lpMsgHeader->nLength);
			g_nRemainBuffLen = 0;
			vprint("ProcReceiveBuffer return false reason %d", 1);
			return FALSE;
		}

		switch (lpMsgHeader->wIdent)
		{
			case BOARDCASTS2G:
				ProcessGameSvrBoardCastPacket(pszData);
				break;
			case GM_CHECKSERVER:
				//SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(2, 0), (LPARAM)_TEXT("Activation"));	// Received keep alive check code from game server 
				break;
			case GM_SERVERUSERINDEX://��Ϸ�������ط�������ź���Ϸ����ɫ��Ӧ��ϵ.
			{
				CSessionInfo* pSessionInfo = g_UserInfoArray.GetData(lpMsgHeader->wSessionIndex);
				if (pSessionInfo)
					pSessionInfo->nServerUserIndex = lpMsgHeader->wUserListIndex;
				break;
			}
			default://��Ϸ���������أ������ط����ͻ��˵���Ϣ.
				ProcessGameSvrPacket(pszData);
				break;
		}
		g_nRemainBuffLen -= (sizeof(_TMSGHEADER) + abs(lpMsgHeader->nLength));
		vprint("process message:%d tid:%d left buff len:%d", lpMsgHeader->wIdent, tid, g_nRemainBuffLen);
		pszData += (sizeof(_TMSGHEADER) + abs(lpMsgHeader->nLength));
		
	} // while

	if (g_nRemainBuffLen > 0)
	{
		memmove(g_szRemainBuff, pszData, g_nRemainBuffLen);
		return TRUE;
	}
	else if (g_nRemainBuffLen < 0)
	{
		vprint("the buff remains len:%d", g_nRemainBuffLen);
		vprint("ProcReceiveBuffer return false reason %d", 2);
		return FALSE;
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

LPARAM OnClientSockMsg(WPARAM wParam, LPARAM lParam)
{
	switch (WSAGETSELECTEVENT(lParam))
	{
		case FD_CONNECT:
		{
			if (CheckSocketError(lParam))
			{
				if (InitServerThreadForMsg())
				{
					g_nRemainBuffLen = 0;

					KillTimer(g_hMainWnd, _ID_TIMER_CONNECTSERVER);
					
					SetTimer(g_hMainWnd, _ID_TIMER_KEEPALIVE, 50000, (TIMERPROC)OnTimerProc);
					SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(1, 0), (LPARAM)_TEXT("Connected"));

					//
					UINT			dwThreadIDForMsg = 0;
					unsigned long	hThreadForMsg = 0;

					g_ClientIoEvent = WSACreateEvent();

					//hThreadForMsg = _beginthreadex(NULL, 0, ClientWorkerThread, NULL, 0, &dwThreadIDForMsg);
				}
			}
			else
			{
				closesocket(g_csock);
				g_csock = INVALID_SOCKET;
				if (!g_fTerminated)
					SetTimer(g_hMainWnd, _ID_TIMER_CONNECTSERVER, 10000, (TIMERPROC)OnTimerProc);
			}

			break;
		}
		case FD_CLOSE:
		{
			closesocket(g_csock);
			g_csock = INVALID_SOCKET;
			KillTimer(g_hMainWnd, _ID_TIMER_KEEPALIVE);
			SetTimer(g_hMainWnd, _ID_TIMER_CONNECTSERVER, 10000, (TIMERPROC)OnTimerProc);
			SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(1, 0), (LPARAM)_TEXT("Not Connected"));
			break;
		}
		case FD_READ:
		{
			char szPacket[1024];
			int nRecv = recv((SOCKET)wParam, szPacket, sizeof(szPacket), 0);
			if (nRecv <= 0)
				break;
			BOOL processed = ProcReceiveBuffer(szPacket, nRecv);
			if (!processed)
			{
				MessageBoxW(NULL, _T("�޷���ȷ����������Ϸ����������Ϣ"), _T("Error"), MB_OK);
				int zero = 0;
				setsockopt((SOCKET)wParam, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero));
				ClearSocket((SOCKET)wParam);
			}
			break;
		}
	}

	return 0L;
}

//��Ϸ������ȡ��Ϸ����������Ϣ.
UINT WINAPI	ClientWorkerThread(LPVOID lpParameter)
{
	_TOVERLAPPEDEX		ClientOverlapped;
	DWORD				dwIndex;
	DWORD				dwBytesTransferred;
	DWORD				dwFlags;
	DWORD				dwRecvBytes;
		
//	char				*pszPos;
//	int					nSocket;

	ZeroMemory(&ClientOverlapped.Overlapped, sizeof(WSAOVERLAPPED));

	ClientOverlapped.Overlapped.hEvent = g_ClientIoEvent;

	ClientOverlapped.DataBuf.len	= DATA_BUFSIZE;
	ClientOverlapped.DataBuf.buf	= &ClientOverlapped.Buffer[0];

	dwFlags = 0;

	if (WSARecv(g_csock, &(ClientOverlapped.DataBuf), 1, &dwRecvBytes, &dwFlags, &(ClientOverlapped.Overlapped), NULL) == SOCKET_ERROR)
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

		WSAGetOverlappedResult(g_csock, &ClientOverlapped.Overlapped, &dwBytesTransferred, FALSE, &dwFlags);

		if (dwBytesTransferred == 0)
			break;

		ProcReceiveBuffer(ClientOverlapped.DataBuf.buf, dwBytesTransferred);
		dwFlags = 0;

		ZeroMemory(&(ClientOverlapped.Overlapped), sizeof(OVERLAPPED));

		ClientOverlapped.DataBuf.len = DATA_BUFSIZE;
		ClientOverlapped.DataBuf.buf = &ClientOverlapped.Buffer[0];

		ClientOverlapped.Overlapped.hEvent = g_ClientIoEvent;

		if (WSARecv(g_csock, &(ClientOverlapped.DataBuf), 1, &dwRecvBytes, &dwFlags, &(ClientOverlapped.Overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
				break;
		}
	}

	return 0L;
}
