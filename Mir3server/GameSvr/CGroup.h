#pragma once
#include "../def/staticArray.h"
#include "../def/_orzex/syncobj.h"
class CGroup;
class CGroup :public CIntLock, CStaticArray<CGroup>::IArrayData
{
public:
	CGroup();
	~CGroup();
	bool IsEmpty();
};

