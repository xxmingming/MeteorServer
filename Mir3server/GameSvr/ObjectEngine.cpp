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
	//print("some one died");
	m_fIsDead = TRUE;
	m_bNeedSend = TRUE;
	m_fDeadTick = 0;
}
