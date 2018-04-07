#include "stdafx.h"

void CScripterObject::Create(int nX, int nY, CMirMap* pMap)
{
	m_dwRunTime			= GetTickCount();
	m_dwRunNextTick		= 5000;
}

void CScripterObject::Operate()
{
	int n = rand() % 7;


	if (rand() % 10  == 0)
	{
		char szText[128];
		char szEncodeText[128];

		if (rand() % 2 == 0)
			strcpy(szText, "안녕하세요. 저는 ei 운영자 가제트 입니다.");
		else
			strcpy(szText, "@몹젠 사슴 1");

		int nPos = fnEncode6BitBufA((unsigned char *)szText, szEncodeText, memlen(szText) - 1, sizeof(szEncodeText));
		szEncodeText[nPos] = '\0';

		ProcessForUserSaid(szEncodeText);
	}
}