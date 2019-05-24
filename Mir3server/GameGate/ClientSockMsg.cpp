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



void ProcessGameSvrPacket(BYTE *lpMsg)
{
	_LPTMSGHEADER lpMsgHeader = (_LPTMSGHEADER)lpMsg;
	CSessionInfo* pSessionInfo = g_UserInfoArray.GetData(lpMsgHeader->wSessionIndex);
	CMsg * msg = new CMsg();
	msg->Size = htonl(lpMsgHeader->nLength + sizeof(CMsg));
	msg->Message = htonl(lpMsgHeader->wIdent);
	memmove(msg->Data, lpMsg + sizeof(_TMSGHEADER), lpMsgHeader->nLength);
	pSessionInfo->m_xSendBuffQ.PushQ((BYTE*)msg);
}

//处理游戏服发来的消息.
bool ProcReceiveBuffer(char *pszPacket, int nRecv)
{
	int limit = nRecv + g_nRemainBuffLen;
	if (limit > DATA_BUFSIZE)
	{
		print("limit > 2 * DATA_BUFSIZE");
		return FALSE;
	}
	int				nLen = nRecv;
	int				nNext = 0;
	BYTE			szBuff[2 * DATA_BUFSIZE];
	BYTE			*pszData = &szBuff[0];
	_LPTMSGHEADER	lpMsgHeader;

	if (g_nRemainBuffLen > 0)
		memmove(szBuff, g_szRemainBuff, g_nRemainBuffLen);

	memmove(&szBuff[g_nRemainBuffLen], pszPacket, nLen);

	nLen += g_nRemainBuffLen;

	while (nLen >= (int)sizeof(_TMSGHEADER))
	{
		lpMsgHeader = (_LPTMSGHEADER)pszData;
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
			default://游戏服发向网关，让网关发给客户端的消息.
				ProcessGameSvrPacket(pszData);
				break;
		}

		pszData += sizeof(_TMSGHEADER) + abs(lpMsgHeader->nLength);
		nLen -= sizeof(_TMSGHEADER) + abs(lpMsgHeader->nLength);
		if (nLen < 0)
		{
			char buff[64];
			sprintf(buff, "message id:%d nLen < 0", lpMsgHeader->wIdent);
			print(buff);
		}
	} // while

	if (nLen > 0)
	{
		memmove(g_szRemainBuff, pszData, nLen);
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

//游戏网关收取游戏服传来的消息.
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
