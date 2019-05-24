#include "stdafx.h"

extern HWND						g_hStatusBar;

extern SOCKET					g_csock;
extern HANDLE					g_hSvrMsgEvnt;

extern CWHQueue					g_SendToUserQ;
extern CWHQueue					g_SendToServerQ;
extern BOOL						g_fTerminated;
extern CWHDynamicArray<CSessionInfo>			g_UserInfoArray;

//向游戏服发消息的线程.
DWORD WINAPI ThreadFuncForMsg(LPVOID lpParameter)
{
	int							nCount;
	int							nBodyLen;
	_TMSGHEADER					MsgHdr;
	char						szDecodeSay[256];
	char						szEncodeSay[256];
	char						*pData;
	CSessionInfo*				pSessionInfo;
	DWORD						dwBytesSends;
	
	int							nPos;

	UINT						nIdentity = 0;
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
						SendSocketMsgS(pSendBuff->nMessage, pSendBuff->nSessionIndex, pSendBuff->sock, pSessionInfo->nServerUserIndex, pSendBuff->nLength, pSendBuff->szData);
					}
					delete pSendBuff;
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
				CMsg * Buf = (CMsg*)pSessionInfo->m_xSendBuffQ.PopQ();
				while (Buf != NULL)
				{
					WSASend(pSessionInfo->sock, Buf, 1, &dwBytesSends, 0, NULL, NULL);
					Buf = (WSABUF*)pSessionInfo->m_xSendBuffQ.PopQ();
				}
			}
		}

		SleepEx(1, TRUE);
	}

	return 0;
}
