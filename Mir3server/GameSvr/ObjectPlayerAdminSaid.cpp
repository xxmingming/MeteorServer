#include "stdafx.h"

#define _MAX_ADMINCOMMAND_LIST	(IDS_COMMAND_HAIRSTYLE - IDS_COMMAND_MONGEN + 1)

void InitAdminCommandList()
{
	/*TCHAR	szCommand[64];
	int		nLen;

	HINSTANCE hInstance = LoadLibrary(_T("AdminCmd.DLL"));

	if (hInstance)
	{
		int i;
		for (i = 0; i < _MAX_ADMINCOMMAND_LIST; i++)
		{
			LoadString(hInstance, IDS_COMMAND_MONGEN + i, szCommand, sizeof(szCommand)/sizeof(TCHAR));

			nLen = lstrlen(szCommand) * sizeof(TCHAR) + 1;

			char *pszAdminCommand = new char[nLen];

			if (pszAdminCommand)
			{
				WideCharToMultiByte(CP_ACP, 0, szCommand, -1, pszAdminCommand, nLen, NULL, NULL);
				g_xAdminCommandList.AddNewNode(pszAdminCommand);
			}
		}

		InsertLogMsgParam(IDS_LOAD_ADMINCOMMAND, &i, LOGPARAM_INT);

		FreeLibrary(hInstance);
	}*/
}

void UnInitAdminCommandList()
{
	PLISTNODE pListNode = g_xAdminCommandList.GetHead();

	while (pListNode)
	{
		delete [] g_xAdminCommandList.GetData(pListNode);

		pListNode = g_xAdminCommandList.RemoveNode(pListNode);
	}
}

BOOL CPlayerObject::ProcessForAdminSaid(char *pszMsg)
{

	return FALSE;
}
