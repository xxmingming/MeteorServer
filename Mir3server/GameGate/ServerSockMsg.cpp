#include "stdafx.h"
#include "../Def/protocol.pb.h"
extern HWND			g_hStatusBar;
	
extern SOCKET		g_ssock;
extern SOCKET		g_csock;

extern HANDLE		g_hIOCP;

CWHDynamicArray<CSessionInfo>	g_UserInfoArray;
CWHQueue						g_SendToServerQ;

void UpdateStatusBar(BOOL fGrow)
{
	static long	nNumOfCurrSession = 0;

	TCHAR	szText[20];

	(fGrow ? InterlockedIncrement(&nNumOfCurrSession) : InterlockedDecrement(&nNumOfCurrSession));
	
	wsprintf(szText, _TEXT("%d Sessions"), nNumOfCurrSession);

	SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(3, 0), (LPARAM)szText);
}

//向游戏服发一个消息，告知游戏客户端怎么了
void SendSocketMsgS (int nIdent, WORD wIndex, int nSocket, WORD wSrvIndex, int nLen, char *pszData)
{
	_TMSGHEADER	msg;
	char		szBuf[DATA_BUFSIZE];

	WSABUF		Buf;
	DWORD		dwSendBytes;

	msg.nSocket			= nSocket;
	msg.wSessionIndex	= wIndex;
	msg.wIdent			= nIdent;
	msg.wUserListIndex	= wSrvIndex;
	msg.nLength			= nLen;
	memmove(szBuf, &msg, sizeof(_TMSGHEADER));

	if (pszData)
		memmove(&szBuf[sizeof(_TMSGHEADER)], pszData, nLen);
	Buf.len = sizeof(_TMSGHEADER) + nLen;
	if (Buf.len > DATA_BUFSIZE)
		print("Buf.len > DATA_BUFSIZE");
	Buf.buf = szBuf;
	WSASend(g_csock, &Buf, 1, &dwSendBytes, 0, NULL, NULL);
}

//接收客户端的链接。
DWORD WINAPI AcceptThread(LPVOID lpParameter)
{
	int							nLen = sizeof(SOCKADDR_IN);
	char						szMsg[64];
	TCHAR						szAddress[24];

	int							nCvtLen;

	SOCKET						Accept;
	SOCKADDR_IN					Address;

//	DWORD						dwRecvBytes;
//	DWORD						dwFlags;

	while (TRUE)
	{
		Accept = accept(g_ssock, (struct sockaddr FAR *)&Address, &nLen);

		if (g_fTerminated)
			return 0;

		int nIndex = g_UserInfoArray.GetAvailablePosition();

		if (nIndex >= 0)
		{
			CSessionInfo* pNewSessionInfo = g_UserInfoArray.GetEmptyElement(nIndex);
			pNewSessionInfo->sock = Accept;
			pNewSessionInfo->nSessionIndex = nIndex;
			pNewSessionInfo->nServerUserIndex = 0;
			// Initializing Session Information
			pNewSessionInfo->nSendBufferLen	= 0;
			pNewSessionInfo->bufLen = 0;

			CreateIoCompletionPort((HANDLE)Accept, g_hIOCP, (DWORD)pNewSessionInfo, 0);
			pNewSessionInfo->Recv();
			//UpdateStatusBar(TRUE);
			// Make packet and send to login server.
			//wsprintf(szAddress, _TEXT("%d.%d.%d.%d"), Address.sin_addr.s_net, Address.sin_addr.s_host,
			//	Address.sin_addr.s_lh, Address.sin_addr.s_impno);
			//nCvtLen = WideCharToMultiByte(CP_ACP, 0, szAddress, -1, szMsg, sizeof(szMsg), NULL, NULL);
			//客户端链接到网关时，在游戏服同步创建一个用户，把用户ID给网关
			SendSocketMsgS(GM_OPEN, pNewSessionInfo->nSessionIndex, (int)pNewSessionInfo->sock, 0, 0, NULL);
		}
	}

	return 0;
}

void CloseSession(CSessionInfo* pSessionInfo)
{
	//对象池要重复使用前需要重置
	if (pSessionInfo != NULL)
		pSessionInfo->Reset();
	g_UserInfoArray.SetEmptyElement(pSessionInfo->nSessionIndex, pSessionInfo);

	closesocket(pSessionInfo->sock);

	UpdateStatusBar(FALSE);
}

//只处理客户端发来的消息.
DWORD WINAPI ServerWorkerThread(LPVOID CompletionPortID)
{
	DWORD					dwBytesTransferred;
//	DWORD					dwFlags;
//	DWORD					dwRecvBytes;
//	DWORD					dwSendBytes;

	CSessionInfo*			pSessionInfo;
	LPOVERLAPPED			lpOverlapped;
//	char					*pszFirst, *pszEnd;

//	CHAR					szBuff[DATA_BUFSIZE + 1];
//	INT						nBuffLen;

	while (TRUE)
	{
		GetQueuedCompletionStatus((HANDLE)CompletionPortID, &dwBytesTransferred, (LPDWORD)&pSessionInfo, 
										(LPOVERLAPPED *)&lpOverlapped, INFINITE);
		if (g_fTerminated) return 0;
		if (dwBytesTransferred == 0)
		{
			SendSocketMsgS(GM_CLOSE, pSessionInfo->nSessionIndex, pSessionInfo->sock, pSessionInfo->nServerUserIndex, 0, NULL);
			CloseSession(pSessionInfo);
			continue;
		}

		if (pSessionInfo->nOvFlag == OVERLAPPED_FLAG::OVERLAPPED_RECV)
		{
			pSessionInfo->bufLen += dwBytesTransferred;
			while (pSessionInfo->HasCompletionPacket())
			{
				int k = g_memPool.GetAvailablePosition();
				if (k < 0)
				{
					print("no more memory");
				}

				_LPTSENDBUFF pSendData = g_memPool.GetEmptyElement(k);
				if (!pSendData)
					break;
				pSendData->nIndex = k;
				pSendData->sock				= pSessionInfo->sock;
				pSendData->nSessionIndex	= pSessionInfo->nSessionIndex;
				pSendData->nLength			= pSessionInfo->ExtractPacket(pSendData->szData, pSendData->nMessage);
				if (pSendData->nLength < 0)
				{
					CloseSession(pSessionInfo);
					continue;
				}
				else
				{
					g_SendToServerQ.Lock();
					if (!g_SendToServerQ.PushQ((BYTE *)pSendData))
					{
						InsertLogMsg(_TEXT("PushQ() failed"));
						g_memPool.SetEmptyElement(pSendData->nIndex, pSendData);
					}
					g_SendToServerQ.Unlock();
				}
			}

			if (pSessionInfo->Recv() == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
			{
				InsertLogMsg(_TEXT("WSARecv() failed"));
				CloseSession(pSessionInfo);
				continue;
			}
		}
	}

	return 0L;
}
