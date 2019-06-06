#include "stdafx.h"

void CPlayerObject::Constructor()
{
	m_bEmpty				= true;
	m_fAdmin				= FALSE;
}

bool CPlayerObject::IsEmpty()
{
	return m_bEmpty;
}

CPlayerObject::CPlayerObject(): CCharObject(NULL)
{
	Constructor();
}

CPlayerObject::CPlayerObject(CUserInfo* pUserInfo): CCharObject(pUserInfo)
{
	Constructor();
}

void CPlayerObject::GetCharName(char *pszCharName)
{
	strcpy(pszCharName, m_szName);
}

void CPlayerObject::SetCharName(const char *pszCharName)
{
	int namelen = min(18, strlen(pszCharName));
	strncpy(m_szName, pszCharName, namelen);
	for (int i = namelen; i < 20; i++)
		m_szName[i] = 0;
}