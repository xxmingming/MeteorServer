

#pragma once

#include "../def/_orzex/queue.h"
#include "..\Def\_orzEx/syncobj.h"
class CWHQueue : public CQueue< BYTE >, public CIntLock
{
public:
	virtual ~CWHQueue()
	{
		ClearAll();
	}

	//ֻ����洢���ϵģ�����������ʱ�������ڴ��deleteջ���ڴ�
	bool PushQ( BYTE *lpbtQ )
	{
		Lock();
		bool bRet = Enqueue( lpbtQ );
		Unlock();

		return bRet;
	}

	BYTE * PopQ()
	{
		Lock();
		BYTE *pData = Dequeue();
		Unlock();

		return pData;
	}
};