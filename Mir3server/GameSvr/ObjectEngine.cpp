#include "stdafx.h"

CCharObject::CCharObject(CUserInfo*	pUserInfo)
{ 
	Reset(pUserInfo);
}

CCharObject::~CCharObject()
{
}

void CCharObject::Die()
{
	m_fIsDead = TRUE;
	m_bNeedSend = TRUE;
	m_fDeadTick = 0;
}
