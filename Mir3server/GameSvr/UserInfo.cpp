#include "stdafx.h"
int CUserInfo::KeyMax = 20;
CUserInfo::CUserInfo()
{
	m_bEmpty					= true;
	m_bDirty					= true;
	m_pxPlayerObject			= NULL;
	m_pGateInfo					= NULL;
	m_pKeys = new byte[KeyMax];//0 up 1 down
	for (int i = 0; i < KeyMax; i++)
	{
		m_pKeys[i] = false;
	}
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

void CUserInfo::Operate()
{
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

#define REBORN_DELAY 5000

BOOL CUserInfo::NeedReborn(float delta)
{
	if (m_pxPlayerObject != NULL && m_pxPlayerObject->m_fIsDead)
	{
		if (!m_pxPlayerObject->m_bWaitReborn)
		{
			m_pxPlayerObject->m_fDeadTick += delta;
			if (m_pxPlayerObject->m_fDeadTick >= REBORN_DELAY)
			{
				m_pxPlayerObject->m_bWaitReborn = TRUE;
				m_pxPlayerObject->m_fDeadTick = 0;
			}
			else
			{
				//print("deadtick not enough");
			}
		}
	}
	return m_pxPlayerObject->m_bWaitReborn;
}

void CUserInfo::Update(Player_ * pPlayer)
{
	/*if (pPlayer == NULL)
	{
		return;
	}

	m_pxPlayerObject->m_Ability.MP = pPlayer->angry();
	m_pxPlayerObject->m_Ability.HP = pPlayer->hp();
	m_pxPlayerObject->m_Ability.Weapon = pPlayer->weapon();
	m_pxPlayerObject->m_Ability.Weapon1 = m_pxPlayerObject->m_Ability.Weapon;
	m_pxPlayerObject->m_Ability.Weapon2 = pPlayer->weapon2();
	m_pxPlayerObject->m_Ability.WeaponPos = pPlayer->weapon_pos();
	m_pxPlayerObject->m_Ability.Model = pPlayer->model();
	m_pxPlayerObject->m_Ability.AniSource = pPlayer->anisource();
	m_pxPlayerObject->m_Ability.Frame = pPlayer->frame();
	m_pxPlayerObject->m_Pos.set_x(pPlayer->pos().x());
	m_pxPlayerObject->m_Pos.set_y(pPlayer->pos().y());
	m_pxPlayerObject->m_Pos.set_z(pPlayer->pos().z());
	m_pxPlayerObject->m_nRotation.set_w(pPlayer->rotation().w());
	m_pxPlayerObject->m_nRotation.set_x(pPlayer->rotation().x());
	m_pxPlayerObject->m_nRotation.set_y(pPlayer->rotation().y());
	m_pxPlayerObject->m_nRotation.set_z(pPlayer->rotation().z());
	if (m_bDirty)
		m_bDirty = false;
	if (m_pxPlayerObject->m_Ability.HP <= 0 && !m_pxPlayerObject->m_fIsDead)
	{
		m_pxPlayerObject->Die();
	}
	else
	{
	}*/
}

void CUserInfo::CopyTo(Player_ * player)
{
	//player->set_id(m_nUserServerIndex);
	//player->set_name(GBK2UTF8(string(m_pxPlayerObject->m_szName)).c_str());
	//player->set_angry(m_pxPlayerObject->m_Ability.MP);
	//player->set_hp(m_pxPlayerObject->m_Ability.HP);
	//player->set_hpmax(m_pxPlayerObject->m_Ability.MaxHP);
	//player->set_camp(m_pxPlayerObject->m_Ability.Camp);
	//player->set_weapon(m_pxPlayerObject->m_Ability.Weapon);
	//player->set_weapon1(m_pxPlayerObject->m_Ability.Weapon1);
	//player->set_weapon2(m_pxPlayerObject->m_Ability.Weapon2);
	//player->set_weapon_pos(m_pxPlayerObject->m_Ability.WeaponPos);
	//player->set_spawnpoint(m_pxPlayerObject->m_Ability.StartPoint);
	//player->set_model(m_pxPlayerObject->m_Ability.Model);
	//player->set_anisource(m_pxPlayerObject->m_Ability.AniSource);
	////player->set_frame(m_pxPlayerObject->m_Ability.Frame);
	//Vector3_ * v = player->mutable_pos();
	//v->set_x(m_pxPlayerObject->m_Pos.x());
	//v->set_y(m_pxPlayerObject->m_Pos.y());
	//v->set_z(m_pxPlayerObject->m_Pos.z());
	//Quaternion_ * q = player->mutable_rotation();
	//q->set_w(m_pxPlayerObject->m_nRotation.w());
	//q->set_x(m_pxPlayerObject->m_nRotation.x());
	//q->set_y(m_pxPlayerObject->m_nRotation.y());
	//q->set_z(m_pxPlayerObject->m_nRotation.z());
}