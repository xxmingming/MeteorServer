#include "stdafx.h"

#define _RANGE_X			12
#define _RANGE_Y			12

#define _CACHE_TICK			500

#define HEALTHFILLTICK		1500
#define SPELLFILLTICK		800

CMagicInfo*		GetMagicInfo(int nMagicID);
CMirMap*		GetMap(char *pszMapName);

CCharObject::CCharObject(CUserInfo*	pUserInfo)
{ 
	m_pUserInfo					= pUserInfo;
	//ZeroMemory(m_szName, sizeof(m_szName));
	m_nCharStatusEx				= 0;
	m_nCharStatus				= 0;
	ZeroMemory(m_wStatusArr, sizeof(m_wStatusArr));
	ZeroMemory(m_szName, sizeof(m_szName));
}

CCharObject::~CCharObject()
{
}

UINT CCharObject::GetCharStatus()
{
	UINT s = 0;

	for (int i = 0; i < MAX_STATUS_ATTRIBUTE; i++)
	{
		if (m_wStatusArr[i] > 0)
			s |= (0x80000000 >> i);
	}

	return (s | (m_nCharStatusEx & 0x000FFFFF));
}


void CCharObject::Die()
{
	m_fIsDead		= TRUE;
}

void CCharObject::SendSocket(char *pszPacket)
{
	/*DWORD					dwBytesSends = 0;
	_TMSGHEADER				MsgHdr;
	_LPTSENDBUFF			lpSendBuff = new _TSENDBUFF;

	MsgHdr.wIdent			= GM_DATA;
	MsgHdr.wSessionIndex	= m_pUserInfo->m_nUserGateIndex;
	MsgHdr.wUserListIndex	= m_pUserInfo->m_nUserServerIndex;
	MsgHdr.nSocket			= m_pUserInfo->m_sock;
	int nLen		= (memlen(pszPacket) - 1);
	MsgHdr.nLength	= -(nLen);
	lpSendBuff->nLen = sizeof(_TMSGHEADER) + nLen;
	memmove(lpSendBuff->szData, (char *)&MsgHdr, sizeof(_TMSGHEADER));
	memmove(&lpSendBuff->szData[sizeof(_TMSGHEADER)], pszPacket, nLen);
	lpSendBuff->szData[lpSendBuff->nLen] = '\0';
	m_pUserInfo->m_pGateInfo->m_xSendBuffQ.PushQ((BYTE *)lpSendBuff);*/
}
