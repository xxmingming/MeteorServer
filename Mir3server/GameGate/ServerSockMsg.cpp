#include "stdafx.h"
#include "../Def/protocol.pb.h"
extern HWND			g_hStatusBar;
	
extern SOCKET		g_ssock;
extern SOCKET		g_csock;

extern HANDLE		g_hIOCP;

CWHDynamicArray<CSessionInfo>	g_UserInfoArray;
CWHQueue						g_SendToServerQ;

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

DWORD WINAPI AcceptThread(LPVOID lpParameter)
{
	int							nLen = sizeof(SOCKADDR_IN);
	//char						szMsg[64];
	//TCHAR						szAddress[24];

	//int							nCvtLen;

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
			pNewSessionInfo->nSendBufferLen = 0;
			pNewSessionInfo->bufLen = 0;

			CreateIoCompletionPort((HANDLE)Accept, g_hIOCP, (DWORD)pNewSessionInfo, 0);
			pNewSessionInfo->Recv();
			// Make packet and send to login server.
			SendSocketMsgS(GM_OPEN, pNewSessionInfo->nSessionIndex, (int)pNewSessionInfo->sock, 0, 0, NULL);
		}
	}

	return 0;
}

void CloseSession(CSessionInfo* pSessionInfo)
{
	if (pSessionInfo != NULL)
		pSessionInfo->Reset();
	g_UserInfoArray.SetEmptyElement(pSessionInfo->nSessionIndex, pSessionInfo);
	closesocket(pSessionInfo->sock);
}

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
					if (!g_SendToServerQ.PushQ((BYTE *)pSendData))
					{
						g_memPool.SetEmptyElement(pSendData->nIndex, pSendData);
					}
				}
			}

			if (pSessionInfo->Recv() == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
			{
				print("WSARecv() failed");
				CloseSession(pSessionInfo);
				continue;
			}
		}
	}

	return 0L;
}
