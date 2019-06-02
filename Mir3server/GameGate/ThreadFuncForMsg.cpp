#include "stdafx.h"

extern HWND						g_hStatusBar;

extern SOCKET					g_csock;
extern HANDLE					g_hSvrMsgEvnt;

extern CWHQueue					g_SendToUserQ;
extern CWHQueue					g_SendToServerQ;
extern BOOL						g_fTerminated;
extern CWHDynamicArray<CSessionInfo>			g_UserInfoArray;

DWORD WINAPI ThreadFuncForMsg(LPVOID lpParameter)
{
	int							nCount;
	CSessionInfo*				pSessionInfo;
	DWORD						dwBytesSends;
	UINT						nIdentity = 0;
	char *						pData = new char[DATA_BUFSIZE];
	while(TRUE)
	{
		if (g_fTerminated) return 0;
		//Send packet to Game server.
		g_SendToServerQ.Lock();
		nCount = g_SendToServerQ.GetCount();
		if (nCount)
		{
			for (int nLoop = 0; nLoop < nCount; nLoop++)
			{
				_LPTSENDBUFF pSendBuff = (_LPTSENDBUFF)g_SendToServerQ.PopQ();
				if (pSendBuff)
				{
					if (pSessionInfo = g_UserInfoArray.GetData(pSendBuff->nSessionIndex))
					{
						SendSocketMsgS(pSendBuff->nMessage, pSendBuff->nSessionIndex, pSendBuff->sock, pSessionInfo->nServerUserIndex, pSendBuff->nLength, pSendBuff->szData, pData);
					}
					g_memPool.SetEmptyElement(pSendBuff->nIndex, pSendBuff);
				}
			}
		}
		g_SendToServerQ.Unlock();

		//Send packet to Client.
		for (int nLoop = 0; nLoop < _MAX_USER_ARRAY; nLoop++)
		{
			pSessionInfo = g_UserInfoArray.GetData(nLoop);
			if (pSessionInfo)
			{
				pSessionInfo->SendBuffLock.Lock();
				if (pSessionInfo->nSendBufferLen)
				{
					WSABUF Buf;
					Buf.len = pSessionInfo->nSendBufferLen;
					Buf.buf = pSessionInfo->SendBuffer;
					int s = WSASend(pSessionInfo->sock, &Buf, 1, &dwBytesSends, 0, NULL, NULL);
					if (s == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
						print("Send packet to client error");
					pSessionInfo->nSendBufferLen = 0;
				}
				pSessionInfo->SendBuffLock.Unlock();
			}
		}

		SleepEx(1, TRUE);
	}

	return 0;
}
