#include "stdafx.h"

#define _RANGE_X			12
#define _RANGE_Y			12

#define _CACHE_TICK			500

#define HEALTHFILLTICK		1500
#define SPELLFILLTICK		800

CMirMap*		GetMap(char *pszMapName);

CCharObject::CCharObject(CUserInfo*	pUserInfo)
{ 
	Reset(pUserInfo);
}

CCharObject::~CCharObject()
{
}

void CCharObject::Die()
{
	//print("some one died");
	m_fIsDead = TRUE;
	m_bNeedSend = TRUE;
	m_fDeadTick = 0;
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
