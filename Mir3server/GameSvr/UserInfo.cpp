#include "stdafx.h"
CUserInfo::CUserInfo()
{
	m_bEmpty					= true;
	m_bDirty					= true;
	m_pxPlayerObject			= NULL;
	m_pGateInfo					= NULL;
	m_pRoom = NULL;
	m_nCertification = 0;
}

bool CUserInfo::IsEmpty()
{
	return m_bEmpty;
}

void CUserInfo::SetName(const char * pszName)
{
	int namelen = min(strlen(pszName), 18);
	memcpy(m_szCharName, pszName, namelen);
	for (int i = namelen; i < 20; i++)
		m_szCharName[i] = 0x00;
}

void CUserInfo::CloseUserHuman()
{
	Lock();
	if (m_pxPlayerObject)
	{
		m_pxPlayerObject->Lock();
		m_pxPlayerObject->m_pUserInfo = NULL;
		m_pxPlayerObject->m_bEmpty = TRUE;
		m_pxPlayerObject->Unlock();
		m_pxPlayerObject = NULL;
	}
	m_bEmpty = TRUE;
	Unlock();
}

void CUserInfo::DoClientCertification(UINT32 clientV)
{
	m_nCertification = 1;
	m_nClientVersion = clientV;
}