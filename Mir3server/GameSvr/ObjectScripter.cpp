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
			strcpy(szText, "�ȳ��ϼ���. ���� ei ��� ����Ʈ �Դϴ�.");
		else
			strcpy(szText, "@���� �罿 1");

		int nPos = fnEncode6BitBufA((unsigned char *)szText, szEncodeText, memlen(szText) - 1, sizeof(szEncodeText));
		szEncodeText[nPos] = '\0';

		ProcessForUserSaid(szEncodeText);
	}
}