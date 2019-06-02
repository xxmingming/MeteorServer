

#pragma once
#include "../Def/protocol.pb.h"
#include "../def/_orzex/queue.h"

class CCharObject;
class CMirMap;
class CUserInfo;
class CRoomInfo;

#pragma pack(8)
class CObjectAbility
{
public:
	int	HP;
	int	MP;
	int	MaxHP;
	int	MaxMP;
	int	Weapon1;
	int	Weapon2;
	int	Weapon;
	int	WeaponPos;
	int	Model;
	int	Camp;
	int	StartPoint;
	int	Frame;
	int	AniSource;
};
#pragma pack(8)

class CCharObject
{
public:
	CUserInfo*					m_pUserInfo;
	Vector3_					m_Pos;
	Quaternion_					m_nRotation;
	char						m_szName[20];
	CObjectAbility				m_Ability;
	void						Reset(CUserInfo*pUserInfo)
	{
		m_fDeadTick = 0;
		m_pUserInfo = pUserInfo;
		m_fIsDead = FALSE;
		ZeroMemory(m_szName, sizeof(m_szName));
		m_bWaitReborn = false;
		m_bNeedSend = false;
	}
	BOOL						m_fIsDead;
	float						m_fDeadTick;
	BOOL						m_bWaitReborn;
	BOOL						m_bNeedSend;
public:
	CCharObject(CUserInfo*	pUserInfo);
	virtual ~CCharObject();
	void	Die();
	virtual void	GetCharName(char *pszCharName) = 0;
};
