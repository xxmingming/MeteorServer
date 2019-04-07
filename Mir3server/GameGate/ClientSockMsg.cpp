#include "stdafx.h"

DWORD WINAPI	ThreadFuncForMsg(LPVOID lpParameter);
BOOL			CheckSocketError(LPARAM lParam);
VOID WINAPI		OnTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

UINT WINAPI		ClientWorkerThread(LPVOID lpParameter);

extern SOCKET					g_csock;

extern HWND						g_hMainWnd;
extern HWND						g_hStatusBar;

extern CWHDynamicArray<CSessionInfo>	g_UserInfoArray;


HANDLE							g_hThreadForComm = NULL;
HANDLE							g_hSvrMsgEvnt = NULL;

char							g_szRemainBuff[DATA_BUFSIZE];
int								g_nRemainBuffLen = 0;

WSAEVENT						g_ClientIoEvent;

//这种包，是要发送给客户端的，所以必须带一些参数标识是哪个客户端.
#pragma pack(1)
struct CMsg
{
	uint32_t Size;
	uint32_t Message;
};
#pragma pack()

void ProcessGameSvrPacket(BYTE *lpMsg)
{
	//_LPTSENDBUFF	lpSendUserData = new _TSENDBUFF;
	_LPTMSGHEADER	lpMsgHeader = (_LPTMSGHEADER)lpMsg;

	CSessionInfo* pSessionInfo = g_UserInfoArray.GetData(lpMsgHeader->wSessionIndex);

	if (!pSessionInfo)
	{
		//print("pSessionInfo == null");
		return;
	}

	pSessionInfo->SendBuffLock.Lock();
	if (pSessionInfo->nSendBufferLen >= DATA_BUFSIZE)
	{
		//print("pSessionInfo->nSendBufferLen >= DATA_BUFSIZE");
		pSessionInfo->nSendBufferLen = 0;
		pSessionInfo->SendBuffLock.Unlock();
		return;
	}
	
	if ((pSessionInfo->nSendBufferLen + lpMsgHeader->nLength + sizeof(CMsg)) >= DATA_BUFSIZE)
	{
		//print("pSessionInfo->nSendBufferLen + lpMsgHeader->nLength + sizeof(CMsg)) >= DATA_BUFSIZE");
		pSessionInfo->SendBuffLock.Unlock();
		return;
	}

	char * pszData = &pSessionInfo->SendBuffer[pSessionInfo->nSendBufferLen];
	CMsg msg;
	msg.Size = htonl(lpMsgHeader->nLength + sizeof(CMsg));
	msg.Message = htonl(lpMsgHeader->wIdent);
	memmove(pszData, &msg, sizeof(CMsg));
	memmove(pszData + sizeof(CMsg), lpMsg + sizeof(_TMSGHEADER), lpMsgHeader->nLength);
	pSessionInfo->nSendBufferLen += lpMsgHeader->nLength + sizeof(CMsg);
	pSessionInfo->SendBuffLock.Unlock();
}

//处理游戏服发来的消息.
//处理成功-返回True
//处理失败-返回FALSE，上一级断开套接字
BYTE * pszData = NULL;
BYTE * pszDataOffset = NULL;
bool ProcReceiveBuffer(char *pszPacket, int nRecv, byte * pMemory)
{
	int limit = nRecv + g_nRemainBuffLen;
	if (limit > DATA_BUFSIZE)
	{
		print("limit > DATA_BUFSIZE");
		return FALSE;
	}
	int				nLen = nRecv;
	int				nNext = 0;
	BYTE			szBuff[8192];
	if (pMemory != NULL)
		pszData = pMemory;
	else
		pszData = &szBuff[0];
	pszDataOffset = pszData;
	_LPTMSGHEADER	lpMsgHeader;

	if (g_nRemainBuffLen > 0)
		memmove(pszDataOffset, g_szRemainBuff, g_nRemainBuffLen);

	memmove(&pszDataOffset[g_nRemainBuffLen], pszPacket, nLen);

	nLen += g_nRemainBuffLen;

	while (nLen >= (int)sizeof(_TMSGHEADER))
	{
		lpMsgHeader = (_LPTMSGHEADER)pszDataOffset;
		if (nLen < (int)(sizeof(_TMSGHEADER) + lpMsgHeader->nLength))
		{
			//print("nLen < (int)(sizeof(_TMSGHEADER) + lpMsgHeader->nLength)");
			break;
		}
		//if ((int)(sizeof(_TMSGHEADER) + lpMsgHeader->nLength) > DATA_BUFSIZE)
		//{
		//	//极端情况，单个包大于8192;
		//}
		switch (lpMsgHeader->wIdent)
		{
			case GM_CHECKSERVER:
				SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(2, 0), (LPARAM)_TEXT("Activation"));	// Received keep alive check code from game server 
				break;
			case GM_SERVERUSERINDEX://游戏服向网关发来的序号和游戏服角色对应关系.
			{
				CSessionInfo* pSessionInfo = g_UserInfoArray.GetData(lpMsgHeader->wSessionIndex);
				if (pSessionInfo)
					pSessionInfo->nServerUserIndex = lpMsgHeader->wUserListIndex;
				break;
			}
			case GM_CLOSE:
				break;
			default://游戏服发向网关，让网关发给客户端的消息.
				ProcessGameSvrPacket(pszDataOffset);
				break;
		}

		pszDataOffset += sizeof(_TMSGHEADER) + abs(lpMsgHeader->nLength);
		nLen -= sizeof(_TMSGHEADER) + abs(lpMsgHeader->nLength);
		if (nLen < 0)
		{
			char buff[64];
			sprintf(buff, "message id:%d nLen < 0", lpMsgHeader->wIdent);
			print(buff);
		}
	} // while

	if (nLen > 0 && nLen <= DATA_BUFSIZE)
	{
		memmove(g_szRemainBuff, pszDataOffset, nLen);
		g_nRemainBuffLen = nLen;
		return TRUE;
	}
	else if (nLen == 0)
	{
		g_nRemainBuffLen = 0;
		return TRUE;
	}
	else if (nLen < 0)
	{
		g_nRemainBuffLen = 0;
		return FALSE;
	}
}

/*
BOOL InitServerThreadForComm()
{
	DWORD	dwThreadIDForComm = 0;

	if (!g_hSvrMsgEvnt)
		g_hSvrMsgEvnt = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (!g_hThreadForComm)
	{
		g_hThreadForComm	= CreateThread(NULL, 0, ThreadFuncForComm,	NULL, 0, &dwThreadIDForComm);

		if (g_hThreadForComm)
			return TRUE;
	}

	return FALSE;
}
*/
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

//没有处理游戏服发来的消息.
byte * pMemory = NULL;
LPARAM OnClientSockMsg(WPARAM wParam, LPARAM lParam)
{
	if (pMemory == NULL)
		pMemory = new byte[DATA_BUFSIZE + sizeof(_TMSGHEADER)];
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
//					SetTimer(g_hMainWnd, _ID_TIMER_KEEPALIVE, 1000, (TIMERPROC)OnTimerProc);

					InsertLogMsg(IDS_CONNECT_LOGINSERVER);
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
			InsertLogMsg(IDS_DISCONNECT_LOGINSERVER);
			SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(1, 0), (LPARAM)_TEXT("Not Connected"));
			break;
		}
		case FD_READ:
		{
			char szPacket[DATA_BUFSIZE / 256];
			int nRecv = recv((SOCKET)wParam, szPacket, sizeof(szPacket), 0);
			if (nRecv <= 0)
				break;
			BOOL processed = ProcReceiveBuffer(szPacket, nRecv, &pMemory[0]);
			break;
		}
	}

	return 0L;
}

//游戏网关收取游戏服传来的消息.
UINT WINAPI	ClientWorkerThread(LPVOID lpParameter)
{
	_TOVERLAPPEDEX		ClientOverlapped;
	DWORD				dwIndex;
	DWORD				dwBytesTransferred;
	DWORD				dwFlags;
	DWORD				dwRecvBytes;
	
	BYTE*				pMemory = new byte[DATA_BUFSIZE + sizeof(_TMSGHEADER)];
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
			return 0;		
	}

	while (TRUE)
	{
		dwIndex = WSAWaitForMultipleEvents(1, &g_ClientIoEvent, FALSE, WSA_INFINITE, FALSE);

		WSAResetEvent(g_ClientIoEvent);

		WSAGetOverlappedResult(g_csock, &ClientOverlapped.Overlapped, &dwBytesTransferred, FALSE, &dwFlags);

		if (dwBytesTransferred == 0)
			break;

		ProcReceiveBuffer(ClientOverlapped.DataBuf.buf, dwBytesTransferred, pMemory);
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
