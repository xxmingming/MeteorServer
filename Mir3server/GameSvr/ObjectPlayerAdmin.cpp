#include "stdafx.h"

CMirMap*		GetMap(char *pszMapName);
CMonRaceInfo*	GetMonRaceInfo(char *pszMonName);

void CPlayerObject::CmdChangeItemPrefix(char *pszParam1, char *pszParam2)
{
	/*_LPTUSERITEMRCD lpUserItemRcd = NULL;

	for (int i = U_DRESS; i <= U_RINGR; i++)
	{
		if (lpUserItemRcd = m_pUserInfo->GetAccessory(i))
		{
			if (strcmp(pszParam1, g_pStdItemSpecial[lpUserItemRcd->nStdIndex].szName) == 0)
			{
				strcpy(lpUserItemRcd->szPrefixName, pszParam2);
				
				if (UpdateItemToDB(lpUserItemRcd, _ITEM_ACTION_UPDATE))
					AddProcess(this, RM_ITEMUPDATE, i, 0, 0, 0, lpUserItemRcd->szPrefixName);

				return;
			}
		}
	}*/
}

void CPlayerObject::CmdRandomSpaceMove(char *pszParam1)
{
	/*int nX;
	int nY;
	int nEgdey;

	CMirMap* pMirMap = GetMap(pszParam1);

	if (pMirMap)
	{
		if (pMirMap->m_stMapFH.shHeight < 150)
		{
			if (pMirMap->m_stMapFH.shHeight < 30)
				nEgdey = 2;
			else
				nEgdey = 20;
		}
		else
			nEgdey = 50;

		nX = nEgdey + (rand() % (pMirMap->m_stMapFH.shWidth - nEgdey - 1));
		nY = nEgdey + (rand() % (pMirMap->m_stMapFH.shHeight - nEgdey - 1));

		if (GetAvailablePosition(pMirMap, nX, nY, 20))
			SpaceMove(nX, nY, pMirMap);
		else 
			SysMsg("Command Failed", 0);
	}
	else
		SysMsg("Command Failed : Invalid map file name.", 0);*/
}

void CPlayerObject::CmdFreeSpaceMove(char *pszParam1, char *pszParam2, char *pszParam3)
{
	
}

void CPlayerObject::CmdCallMakeMonster(char *pszParam1, char *pszParam2)
{
	
}

void CPlayerObject::CmdCallMakeMonsterXY(char *pszParam1, char *pszParam2, char *pszParam3, char *pszParam4)
{
	int nMax	= _MIN(500, AnsiStrToVal(pszParam4));
	int nX		= AnsiStrToVal(pszParam1);
	int nY		= AnsiStrToVal(pszParam2);

	CMonRaceInfo* pMonRaceInfo = GetMonRaceInfo(pszParam3);

	if (pMonRaceInfo)
	{
		//for (int i = 0; i < nMax; i++)
		//	AddCreatureSysop(nX, nY, pMonRaceInfo, FALSE);
	}
}

void CPlayerObject::CmdMakeItem(char *pszParam1, char *pszParam2)
{
	
}

BOOL CPlayerObject::CmdChangeJob(char *pszParam1)
{
	
	return FALSE;
}

void CPlayerObject::CmdMakeFullSkill(char *pszParam1, char *pszParam2)
{
	
}

void CPlayerObject::CmdSendMonsterLevelInfos()
{
	
}

void CPlayerObject::CmdDyeingHair(char *pszParam1)
{
	//m_tFeatureEx.dwHairColor = AnsiStrToVal(pszParam1);

	//AddRefMsg(RM_TURN, m_nDirection, m_nCurrX, m_nCurrY, 0, m_pUserInfo->m_szCharName);
}

void CPlayerObject::CmdDyeingWear(char *pszParam1)
{
	//m_tFeatureEx.dwWearColor = AnsiStrToVal(pszParam1);

	//AddRefMsg(RM_TURN, m_nDirection, m_nCurrX, m_nCurrY, 0, m_pUserInfo->m_szCharName);
}

void CPlayerObject::CmdHairStyle(char *pszParam1)
{
	//m_tFeature.btHair = AnsiStrToVal(pszParam1);

	//AddRefMsg(RM_TURN, m_nDirection, m_nCurrX, m_nCurrY, 0, m_pUserInfo->m_szCharName);
}

void CPlayerObject::CmdCallMakeSlaveMonster(char *pszParam1, char *pszParam2)
{
//	int nMax = AnsiStrToVal(pszParam2);
//	int nX, nY;
//
//	CMonRaceInfo* pMonRaceInfo = GetMonRaceInfo(pszParam1);
//
//	if (pMonRaceInfo)
//	{
//		GetFrontPosition(nX, nY);
//
//		for (int i = 0; i < nMax; i++)
//		{
//			CMonsterObject* pMonsterObject = (CMonsterObject*)AddCreatureSysop(nX, nY, pMonRaceInfo, TRUE);
//
//			if (pMonsterObject)
//			{
//				char	szMonName[64];
//
//				pMonsterObject->m_pMasterObject = this;
//
//				pMonsterObject->GetCharName(szMonName);
//
//				AddRefMsg(RM_USERNAME, 0, 0, 0, 0, szMonName);
//				
//				m_xSlaveObjList.AddNewNode(pMonsterObject);
///*				cret.MasterRoyaltyTime := GetTickCount + 24 * 60 * 60 * 1000;
//                  cret.SlaveMakeLevel := 3;
//                  cret.SlaveExpLevel := momlv;
//                  cret.UserNameChanged;
//                  SlaveList.Add (cret); */
//			}
//		}
//	}
}
