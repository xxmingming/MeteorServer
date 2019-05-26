#include "stdafx.h"

#define _MSG_GOOD		"+GOOD/"
#define _MSG_FAIL		"+FAIL/"

CMirMap*		GetMap(char *pszMapName);
CPlayerObject*	GetUserHuman(char *pszCharName);
void SendRDBSocket(int nCertification, char *pszData1, char *pszData2, int nData2Len);

void CPlayerObject::Constructor()
{
	m_bEmpty				= true;
	m_wObjectType			= _OBJECT_HUMAN;
	m_fIsDead				= FALSE;
	m_fAdmin				= FALSE;
	m_dwLastTalkTime = 0;
	m_fIsCapture			= FALSE;
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

void CPlayerObject::SetRotation(float w, float x, float y, float z)
{
	m_nRotation.set_x(roundl((double)1000 * x));
	m_nRotation.set_y(roundl((double)1000 * y));
	m_nRotation.set_z(roundl((double)1000 * z));
	m_nRotation.set_w(roundl((double)1000 * w));
}

//世界坐标不许超过1E+5，太大越界
void CPlayerObject::SetPosition(float x, float y, float z)
{
	m_Pos.set_x(roundl((double)1000 * x));
	m_Pos.set_y(roundl((double)1000 * y));
	m_Pos.set_z(roundl((double)1000 * z));
}

void CPlayerObject::Reborn()
{
	if (!m_pUserInfo) return;
	if (!m_pUserInfo->m_pRoom)return;
	m_Ability.HP = m_Ability.MaxHP = m_pUserInfo->m_pRoom->m_nHpMax;
	m_Ability.MP = m_Ability.MaxMP = 0;
	//m_Ability.Weapon = m_Ability.Weapon1 = weapon;
	//m_Ability.Weapon2 = 0;
	//m_Ability.WeaponPos = 0;
	m_Ability.StartPoint = rand() % 16;
	m_Ability.AniSource = 0;
	m_Ability.Frame = 0;
	m_bWaitReborn = false;
	m_fIsDead = false;
}

void CPlayerObject::Spawn(int startPoint, int camp, int model, int weapon)
{
	if (!m_pUserInfo) return;
	if (!m_pUserInfo->m_pRoom)return;
	m_Ability.HP = m_Ability.MaxHP = m_pUserInfo->m_pRoom->m_nHpMax;
	m_Ability.MP = m_Ability.MaxMP = 0;
	m_Ability.Weapon = m_Ability.Weapon1 = weapon;
	m_Ability.Weapon2 = 0;
	m_Ability.WeaponPos = 0;
	m_Ability.Camp = camp;
	m_Ability.Model = model;
	m_Ability.StartPoint = startPoint;
	m_Ability.AniSource = 0;
	m_Ability.Frame = 0;
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