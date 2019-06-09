

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
	int	Weapon;
	int	Model;
	int	Camp;
};
#pragma pack(8)

class CCharObject
{
public:
	CUserInfo*					m_pUserInfo;
	char						m_szName[20];
	CObjectAbility				m_Ability;
	void						Reset(CUserInfo*pUserInfo)
	{
		m_pUserInfo = pUserInfo;
		ZeroMemory(m_szName, sizeof(m_szName));
	}
public:
	CCharObject(CUserInfo*	pUserInfo);
	virtual ~CCharObject();
	virtual void	GetCharName(char *pszCharName) = 0;
};
