#include "stdafx.h"

extern HWND						g_hStatusBar;

extern SOCKET					g_csock;
extern HANDLE					g_hSvrMsgEvnt;

extern CWHQueue					g_SendToUserQ;
extern CWHQueue					g_SendToServerQ;
extern BOOL						g_fTerminated;
extern CWHDynamicArray<CSessionInfo>			g_UserInfoArray;

CWHAbusive						g_xAbusive;

//����Ϸ������Ϣ���߳�.
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
	//MsgHdr.wIdent	= GM_DATA;

//	g_xAbusive.LoadAbusiveList();
	
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
			}// for nLoop
		}
		g_SendToServerQ.Unlock();

		//Send Packet to Client,��Ϸ���������أ��������·�����Ϣ��������SendBuffer�����pSessionûlockӦ������ͬ�������.
		for (int nLoop = 0; nLoop < _MAX_USER_ARRAY; nLoop++)
		{
			pSessionInfo = g_UserInfoArray.GetData(nLoop);
			if (pSessionInfo)
			{
				pSessionInfo->SendBuffLock.Lock();
				if (pSessionInfo->nSendBufferLen)
				{
					WSABUF	Buf;
					Buf.len = pSessionInfo->nSendBufferLen;
					Buf.buf = pSessionInfo->SendBuffer;
					WSASend(pSessionInfo->sock, &Buf, 1, &dwBytesSends, 0, NULL, NULL);
					//if (dwBytesSends < Buf.len)
					//	_RPT2(_CRT_WARN, "%d:%s\n", memlen(pSessionInfo->SendBuffer) - 1, pSessionInfo->SendBuffer);
					pSessionInfo->nSendBufferLen = 0;
				}
				pSessionInfo->SendBuffLock.Unlock();
			}
		}

		SleepEx(1, TRUE);
	}

	return 0;
}
