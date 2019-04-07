#include "stdafx.h"

CPlayerObject* GetUserHuman(char *pszCharName)
{
	if (g_xUserInfoList.GetCount())
	{
		PLISTNODE pListNode = g_xUserInfoList.GetHead();

		while (pListNode)
		{
			CUserInfo *pUserInfo = g_xUserInfoList.GetData(pListNode);

			if (pUserInfo)
			{
				if (pUserInfo->m_pxPlayerObject)
				{
					if (memcmp(pUserInfo->m_szCharName, pszCharName, memlen(pUserInfo->m_szCharName) - 1) == 0)
					{
						if (!pUserInfo->m_pxPlayerObject->m_fIsDead)
							return (CPlayerObject*)pUserInfo->m_pxPlayerObject;
					}
				}
			}

			pListNode = g_xUserInfoList.GetNext(pListNode);
		}
	}

	return NULL;
}

UINT WINAPI ProcessRoom(LPVOID lpParameter)
{
	PLISTNODE pListNode = NULL;
	while (TRUE)
	{
		if (g_fTerminated)
			return 0L;

		if (g_xRoomList.GetCount())
		{
			pListNode = g_xRoomList.GetHead();

			while (pListNode)
			{
				CRoomInfo *pRoomInfo = g_xRoomList.GetData(pListNode);

				if (pRoomInfo)
				{
					if (!pRoomInfo->IsEmpty())
						pRoomInfo->Update();//第一次是0，第二次开始
					else
					{
						pRoomInfo->WaitClose();
						if (pRoomInfo->closed)
						{
							pListNode = g_xRoomList.RemoveNode(pListNode);
							continue;
						}
					}
				}
				pListNode = g_xRoomList.GetNext(pListNode);
			} // while
		} // if g_xReadyUserInfoList.GetCount()

		if (g_xGateList.GetCount())
		{
			pListNode = g_xGateList.GetHead();

			while (pListNode)
			{
				CGateInfo *pGateInfo = g_xGateList.GetData(pListNode);

				if (pGateInfo)
					pGateInfo->xSend();

				pListNode = g_xUserInfoList.GetNext(pListNode);
			} // while
		}

		SleepEx(1, TRUE);
	}
}

UINT WINAPI ProcessUserHuman(LPVOID lpParameter)
{
	PLISTNODE pListNode = NULL;

	while (TRUE)
	{
		if (g_fTerminated) 
		{
			if (g_xUserInfoList.GetCount())
			{
				pListNode = g_xUserInfoList.GetHead();

				while (pListNode)
				{
					CUserInfo *pUserInfo = g_xUserInfoList.GetData(pListNode);
					if (pUserInfo)
						pUserInfo->CloseUserHuman();
					pListNode = g_xUserInfoList.RemoveNode(pListNode);
				}
			}

			return 0L;
		}

		if (g_xGateList.GetCount())
		{
			pListNode = g_xGateList.GetHead();

			while (pListNode)
			{
				CGateInfo *pGateInfo = g_xGateList.GetData(pListNode);

				if (pGateInfo)
					pGateInfo->xSend();

				pListNode = g_xUserInfoList.GetNext(pListNode);
			} // while
		}

		SleepEx(1, TRUE);
	}

//	return 0L;
}
