#include "stdafx.h"

void UpdateStatusBarUsers(BOOL fGrow);
void SendRDBSocket(int nCertification, char *pszData1, char *pszData2, int nData2Len);

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
}

bool CUserInfo::IsEmpty()
{
	return m_bEmpty;
}

void CUserInfo::SetName(const char * pszName)
{
	memcpy(m_szCharName, pszName, min(strlen(pszName), 18));
	m_szCharName[18] = 0;
	m_szCharName[19] = 0;
}

void CUserInfo::ProcessUserMessage(char *pszPacket)
{
	//_LPTDEFAULTMESSAGE lpDefMsg = (_LPTDEFAULTMESSAGE)pszPacket;

	//if (m_pxPlayerObject->m_fIsCapture)
	//{
	//	if (m_pxPlayerObject->m_hCaptureFile)
	//	{
	//		DWORD	dwWrite = 0;

	//		fprintf(m_pxPlayerObject->m_hCaptureFile, "%d, %d, %d, %d, %d\r\n", lpDefMsg->wIdent, lpDefMsg->nRecog, lpDefMsg->wParam, lpDefMsg->wTag, lpDefMsg->wSeries);

	//		switch (lpDefMsg->wIdent)
	//		{
	//			case CM_SAY:
	//			case CM_PICKUP:
	//			{
	//				char szDecodeMsg[512];
	//				int nPos = fnDecode6BitBufA(pszPacket + sizeof(_TDEFAULTMESSAGE), szDecodeMsg, sizeof(szDecodeMsg));
	//				szDecodeMsg[nPos] = '\0';

	//				fprintf(m_pxPlayerObject->m_hCaptureFile, "%s\r\n", szDecodeMsg);
	//			}
	//		}
	//	}
	//}

	//switch (lpDefMsg->wIdent)
	//{
	//	case CM_HIT:
	//	case CM_POWERHIT:
	//	case CM_LONGHIT:
	//	case CM_WIDEHIT:
	//	case CM_HEAVYHIT:
	//	case CM_BIGHIT:
	//	case CM_FIREHIT:
	//		//m_pxPlayerObject->AddProcess(m_pxPlayerObject, lpDefMsg->wIdent, lpDefMsg->wTag, LOWORD(lpDefMsg->nRecog), HIWORD(lpDefMsg->nRecog), lpDefMsg->wParam, NULL);
	//		break;
	//	case CM_TURN:
	//	case CM_WALK:
	//	case CM_RUN:
	//	case CM_SITDOWN:
	//	case CM_RIDE:
	//		//m_pxPlayerObject->AddProcess(m_pxPlayerObject, lpDefMsg->wIdent, lpDefMsg->wTag, LOWORD(lpDefMsg->nRecog), HIWORD(lpDefMsg->nRecog), NULL);
	//		break;
	//	case CM_SAY:
	//		//m_pxPlayerObject->AddProcess(m_pxPlayerObject, lpDefMsg->wIdent, 0, 0, 0, 0, pszPacket + sizeof(_TDEFAULTMESSAGE));
	//		break;
	//	case CM_SPELL:
	//		//m_pxPlayerObject->AddProcess(m_pxPlayerObject, lpDefMsg->wIdent, 
	//		//											lpDefMsg->wTag/*MagicID*/, LOWORD(lpDefMsg->nRecog)/*TargetX*/, HIWORD(lpDefMsg->nRecog)/*TargetY*/, 
	//		//											MAKELONG(lpDefMsg->wSeries, lpDefMsg->wParam)/*TargetObj*/, NULL);
	//		break;
	//	case CM_TAKEONITEM:
	//	case CM_TAKEOFFITEM:
	//		//m_pxPlayerObject->AddProcess(m_pxPlayerObject, lpDefMsg->wIdent, lpDefMsg->wSeries, lpDefMsg->nRecog, lpDefMsg->wParam, lpDefMsg->wTag, pszPacket + sizeof(_TDEFAULTMESSAGE));
	//		break;
 //       case CM_QUERYUSERNAME:
	//		//m_pxPlayerObject->AddProcess(m_pxPlayerObject, lpDefMsg->wIdent, 0, lpDefMsg->nRecog, lpDefMsg->wParam, lpDefMsg->wTag, NULL);
	//		break;
	//	case CM_EAT:
	//	case CM_DROPITEM:
	//		//m_pxPlayerObject->AddProcess(m_pxPlayerObject, lpDefMsg->wIdent, lpDefMsg->wSeries, 0, 0, 0, pszPacket + sizeof(_TDEFAULTMESSAGE));
	//		break;
	//	case CM_PICKUP:
	//		//m_pxPlayerObject->AddProcess(m_pxPlayerObject, lpDefMsg->wIdent, lpDefMsg->wSeries, lpDefMsg->nRecog, lpDefMsg->wTag, lpDefMsg->wParam);
	//		break;
	//	default:
	//		//m_pxPlayerObject->AddProcess(m_pxPlayerObject, lpDefMsg->wIdent, lpDefMsg->wSeries, lpDefMsg->nRecog, lpDefMsg->wTag, NULL);
	//		break;
	//}
}

void CUserInfo::Operate(Input_ * pInput)
{
	//把角色的输入同步到数据，广播到此房间的所有玩家
	for (int i = 0; i < KeyMax; i++)
	{
		//pInput->set_c(m_pKeys[i]);
	}
	pInput->set_playerid(m_nUserServerIndex);
	Vector2_ J = pInput->joystick(); 
	J.set_x(Jx);
	J.set_y(Jy);
	Vector2_ d = pInput->mousedelta();
	d.set_x(Mx);
	d.set_y(My);

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
	Unlock();
	//UpdateStatusBarUsers(FALSE);
}

void CUserInfo::CloseAccount(char *pszName, int nCertification)
{
	char	szMsg[256];
	int		nLen = memlen(pszName) - 1;

	szMsg[0] = '%';
	szMsg[1] = 'S';

	memcpy(&szMsg[2], pszName, nLen);

	szMsg[nLen + 2] = '/';

	char *pszPos = ValToAnsiStr(nCertification, &szMsg[nLen + 3]);

	*pszPos++	= '$';
	*pszPos		= '\0';

	send(g_clsock, szMsg, memlen(szMsg) - 1, 0);
}

void CUserInfo::DoClientCertification(char *pszPacket)
{
	char szDecodePacket[64];
	char *pszDecodePacket = &szDecodePacket[0];
	char *pszPos;

	if (m_btCurrentMode == USERMODE_NOTICE)
	{
		int nLen = memlen(pszPacket);

		if (pszPos = (char *)memchr(pszPacket, '!', nLen))
		{
			*pszPos = '\0';

			//             uid  chr	 cer  ver  startnew
			// pszPacket **SSSS/SSSS/SSSS/SSSS/1
			nLen = fnDecode6BitBufA(pszPacket + 2, szDecodePacket, sizeof(szDecodePacket));
			szDecodePacket[nLen] = '\0';

			if (*pszDecodePacket == '*' && *(pszDecodePacket + 1) == '*')
			{
				pszDecodePacket += 2;

				if (!(pszPos = (char *)memchr(pszDecodePacket, '/', nLen))) return;

				*pszPos++ = '\0';
				nLen -= (pszPos - pszDecodePacket);
				memmove(m_szUserID, pszDecodePacket, (pszPos - pszDecodePacket));
				pszDecodePacket = pszPos;

				if (!(pszPos = (char *)memchr(pszDecodePacket, '/', nLen))) return;

				*pszPos++ = '\0';
				nLen -= (pszPos - pszDecodePacket);
				memmove(m_szCharName, pszDecodePacket, (pszPos - pszDecodePacket));
				pszDecodePacket = pszPos;

				if (!(pszPos = (char *)memchr(pszDecodePacket, '/', nLen))) return;

				*pszPos++ = '\0';
				nLen -= (pszPos - pszDecodePacket);
				m_nCertification = AnsiStrToVal(pszDecodePacket);
				pszDecodePacket = pszPos;

				if (!(pszPos = (char *)memchr(pszDecodePacket, '/', nLen))) return;

				*pszPos++ = '\0';
				nLen -= (pszPos - pszDecodePacket);
				m_nClientVersion = AnsiStrToVal(pszDecodePacket);
				pszDecodePacket = pszPos;

				m_btCurrentMode = USERMODE_LOGIN;
//				(*pszDecodePacket == '0' ? StartNew = TRUE : StartNew = FALSE);

				// INSERT:Check pay bill.

//				if (pUserInfo->m_nCertification >= 30)
//				{

//					LoadPlayer(pUserInfo);
//				}
	/*				else
				{
					// INSERT:Close User
				} */
			}
		}
	}
}

void CUserInfo::Update(Player_ * pPlayer)
{
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
}

void CUserInfo::CopyTo(Player_ * player)
{
	player->set_id(m_nUserServerIndex);
	player->set_name(GBK2UTF8(string(m_pxPlayerObject->m_szName)).c_str());
	player->set_angry(m_pxPlayerObject->m_Ability.MP);
	player->set_hp(m_pxPlayerObject->m_Ability.HP);
	player->set_hpmax(m_pxPlayerObject->m_Ability.MaxHP);
	player->set_camp(m_pxPlayerObject->m_Ability.Camp);
	player->set_weapon(m_pxPlayerObject->m_Ability.Weapon);
	player->set_weapon1(m_pxPlayerObject->m_Ability.Weapon1);
	player->set_weapon2(m_pxPlayerObject->m_Ability.Weapon2);
	player->set_weapon_pos(m_pxPlayerObject->m_Ability.WeaponPos);
	player->set_spawnpoint(m_pxPlayerObject->m_Ability.StartPoint);
	player->set_model(m_pxPlayerObject->m_Ability.Model);
	player->set_anisource(m_pxPlayerObject->m_Ability.AniSource);
	player->set_frame(m_pxPlayerObject->m_Ability.Frame);
	Vector3_ * v = player->mutable_pos();
	v->set_x(m_pxPlayerObject->m_Pos.x());
	v->set_y(m_pxPlayerObject->m_Pos.y());
	v->set_z(m_pxPlayerObject->m_Pos.z());
	Quaternion_ * q = player->mutable_rotation();
	q->set_w(m_pxPlayerObject->m_nRotation.w());
	q->set_x(m_pxPlayerObject->m_nRotation.x());
	q->set_y(m_pxPlayerObject->m_nRotation.y());
	q->set_z(m_pxPlayerObject->m_nRotation.z());
}