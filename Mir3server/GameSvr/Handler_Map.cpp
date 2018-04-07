#include "stdafx.h"

//global function def
void LoadMap(CMapInfo* pMapInfo)
{
	int			nLen = memlen(pMapInfo->szMapFileName);
	int			nLen2 = memlen(pMapInfo->szMapName);
	CMirMap*	pMirMap = new CMirMap;
	if (pMirMap)
	{
		memcpy(pMirMap->m_szMapName, pMapInfo->szMapFileName, nLen);
		memcpy(pMirMap->m_szMapTextName, pMapInfo->szMapName, nLen2);
		pMirMap->m_nLevelIdx = pMapInfo->m_nLevelIdx;
		if (pMirMap->LoadMapData(pMapInfo->szMapFileName))
			g_xMirMapList.AddNewNode(pMirMap);
	}
}

CMirMap* GetMap(char *pszMapName)
{
	PLISTNODE		pListNode;
	CMirMap*		pMirMap = NULL;

	if (g_xMirMapList.GetCount())
	{
		pListNode = g_xMirMapList.GetHead();

		while (pListNode)
		{
			pMirMap = g_xMirMapList.GetData(pListNode);

			if (memcmp(pMirMap->m_szMapName, pszMapName, memlen(pszMapName) - 1) == 0)
				return pMirMap;
			pListNode = g_xMirMapList.GetNext(pListNode);
		}
	}
	return NULL;
}

/* **************************************************************************************
		CRoomInfo Class Members
   **************************************************************************************/
CRoomInfo::CRoomInfo()
{
	memset(m_szName, 0, 20);
	memset(m_szPassword, 0, 8);
	m_nCount = 0;
	m_nGroup1 = m_nGroup2 = m_nHpMax = m_nMaxPlayer = m_nRoomIndex = 0;
	m_nRule = 1;
	m_pMap = NULL;
}

CRoomInfo::~CRoomInfo()
{
}

BOOL CRoomInfo::RemovePlayer(CUserInfo * pRemoveObject)
{
	if (pRemoveObject != NULL)
	{
		m_pUserList.Lock();
		m_pUserList.RemoveNodeByData(pRemoveObject);
		m_pUserList.Unlock();
	}

	return TRUE;
}

void CRoomInfo::CreateRoom(CMirMap * map, int maxPlayer, int hpMax, int roomIdx)
{
	if (m_pMap != NULL)
		return;
	m_pMap = map;
	m_nMaxPlayer = maxPlayer;
	m_nCount = 0;
	m_nRoomIndex = roomIdx;
	m_nHpMax = hpMax;
	strncpy(m_szName, map->m_szMapTextName, min(18, strlen(map->m_szMapTextName)));
	m_szName[18] = 0;
	m_szName[19] = 0;
}

void CRoomInfo::Update()
{
	//处理全部角色间的输入同步.
}

/* **************************************************************************************
		CMirMap	Class Members
   **************************************************************************************/
CMirMap::CMirMap()
{
}

CMirMap::~CMirMap()
{
}

BOOL CMirMap::LoadMapData(char *pszName)
{
	return TRUE;
	//读取cob文件。des文件，gmb文件，主要是障碍物，场景碰撞机关，部分
	/*HANDLE			hFile;
	LPCELLINFO		pstCellInfo;
	TCHAR			szMapName[15];
	TCHAR			szMapFileName[256];
	TCHAR			szMapPath[256];
	MultiByteToWideChar(CP_ACP, 0, pszName, -1, szMapName, sizeof(szMapName)/sizeof(TCHAR));
	MultiByteToWideChar(CP_ACP, 0, g_strClientPath, -1, szMapPath, sizeof(szMapPath) / sizeof(TCHAR));
	lstrcpy(szMapFileName, szMapPath);
	lstrcat(szMapFileName, _TEXT("/"));
	lstrcat(szMapFileName, szMapName);
	lstrcat(szMapFileName, _TEXT("/"));
	lstrcat(szMapFileName, szMapName);
	lstrcat(szMapFileName, _TEXT(".des"));
	hFile = CreateFile(szMapFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DWORD		dwReadLen;
		ReadFile(hFile, &m_stMapFH, sizeof(MAPFILEHEADER), &dwReadLen, NULL);
		int nMapSize = m_stMapFH.shWidth * m_stMapFH.shHeight;
		SetFilePointer(hFile, sizeof(TILEINFO) * (nMapSize) / 4, 0, FILE_CURRENT);
		pstCellInfo = new CELLINFO[nMapSize];
		if (pstCellInfo)
		{
			ReadFile(hFile, pstCellInfo, sizeof(CELLINFO) * (nMapSize), &dwReadLen, NULL);
			CloseHandle(hFile);
			m_pMapCellInfo = new CMapCellInfo[nMapSize];
			if (m_pMapCellInfo)
			{
				for (int i = 0; i < nMapSize; i++)
				{
					m_pMapCellInfo[i].m_chFlag			= pstCellInfo[i].cFlag;
					m_pMapCellInfo[i].m_sLightNEvent	= pstCellInfo[i].shLigntNEvent;
					m_pMapCellInfo[i].m_xpObjectList = NULL;
				}
			}

			int nSize = sizeof(m_pMapCellInfo);
			delete [] pstCellInfo;
			pstCellInfo = NULL;
			InsertLogMsgParam(IDS_LOADMAPFILE_GOOD, szMapFileName, LOGPARAM_STR);
			return TRUE;
		}
	}

	InsertLogMsgParam(IDS_LOADMAPFILE_FAIL, szMapFileName, LOGPARAM_STR);
	return FALSE;*/
}


