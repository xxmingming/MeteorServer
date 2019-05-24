

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

	//只允许存储堆上的，否则在析构时，清理内存会delete栈上内存
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