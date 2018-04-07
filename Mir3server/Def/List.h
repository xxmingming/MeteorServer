
#pragma once
//线程不安全的数据结构.需要自己调用处使用lock unlock
typedef VOID *	PLISTNODE;
template <class T> class CWHList:public CIntLock
{
public:
	typedef struct tagLINKEDLIST
	{
		T						lpData;
		struct tagLINKEDLIST	*prev;
		struct tagLINKEDLIST	*next;
	} LINKEDLIST, FAR *LPLINKEDLIST;

	LPLINKEDLIST		m_lpHead;
	LPLINKEDLIST		m_lpTail;

	UINT				m_nCount;
public:
	CWHList();
	~CWHList();

	BOOL			AddNewNode(T lpData);
	void			Clear();
	PLISTNODE		RemoveNode(LPLINKEDLIST lpListNode);
	PLISTNODE		RemoveNodeByData(T lpData);
	void			*FindNode(LPLINKEDLIST lpList);
	PLISTNODE		FindData(T t);

	__inline PLISTNODE		GetHead()	
	{ 
		if (m_lpHead) 
			return (PLISTNODE)m_lpHead; 
	
		return NULL; 
	}
	__inline PLISTNODE		GetTail()
	{
		if (m_lpTail)
			return (PLISTNODE)m_lpTail;

		return NULL;
	}
	__inline PLISTNODE		GetNext(PLISTNODE pListNode) { if (pListNode) return ((LPLINKEDLIST)pListNode)->next; return NULL; }
	__inline PLISTNODE		RemoveNode(PLISTNODE pListNode) { if (pListNode) return RemoveNode((LPLINKEDLIST)pListNode); return NULL; }
	__inline UINT			GetCount()	{ return m_nCount; }
	__inline T				GetData(PLISTNODE pListNode) { if (pListNode) return ((LPLINKEDLIST)pListNode)->lpData; return NULL; }
};

template<class T> CWHList<T>::CWHList()
{				  
	m_lpHead = NULL;
	m_lpTail = NULL;
	m_nCount = 0;
}

template<class T> CWHList<T>::~CWHList()
{
	Clear();
}

template<class T> BOOL CWHList<T>::AddNewNode(T lpData)
{
	BOOL fRet = FALSE;
	LPLINKEDLIST lpCD = (LPLINKEDLIST)GlobalAlloc(GPTR, sizeof(LINKEDLIST));

	lpCD->lpData = lpData;

	if (!m_lpHead)
	{
		m_lpHead = lpCD;
		lpCD->prev = (LPLINKEDLIST)NULL;
	}
	else
	{
		m_lpTail->next = lpCD;
		lpCD->prev = m_lpTail;
	}

	lpCD->next = (LPLINKEDLIST)NULL;
	m_lpTail = lpCD;

	m_nCount++;

	fRet = TRUE;
	return fRet;
}

template<class T> void CWHList<T>::Clear()
{
	LPLINKEDLIST	lpNode	= m_lpHead;

	while (lpNode)
		lpNode = (LPLINKEDLIST)RemoveNode(lpNode);
}

template<class T> PLISTNODE CWHList<T>::RemoveNode(LPLINKEDLIST lpList)
{
    LPLINKEDLIST prev = NULL, next;

	next = lpList->next;
	prev = lpList->prev;

	if (prev) prev->next = next;
	else m_lpHead = next;

	if (next) next->prev = prev;
	else m_lpTail = prev;

	GlobalFree(lpList);
	lpList = NULL;

	m_nCount--;

	return (PLISTNODE)next;
}

template <class T>PLISTNODE CWHList<T>::RemoveNodeByData(T lpData)
{

    LPLINKEDLIST	prev	= NULL, next;
	LPLINKEDLIST	lpNode	= m_lpHead;

	while (lpNode)
	{
		if ((T)lpNode->lpData == lpData)
		{
			next = lpNode->next;
			prev = lpNode->prev;

			if (prev) prev->next = next;
			else m_lpHead = next;

			if (next) next->prev = prev;
			else m_lpTail = prev;

			GlobalFree(lpNode);
			lpNode = NULL;

			m_nCount--;

			break;
		}

		lpNode = lpNode->next;
	}

	return (PLISTNODE)next;
}

template <class T> void *CWHList<T>::FindNode(LPLINKEDLIST lpList)
{
	LPLINKEDLIST lpNode = m_lpHead;

	while (lpNode)
	{
		if (lpNode == lpList)
		{
			// ORZ: 内靛 眠啊
			LeaveCriticalSection( &m_cs );
			return lpNode;
		}

		lpNode = lpNode->next;
	}

	return NULL;
}

template <class T>PLISTNODE CWHList<T>::FindData(T t)
{
	LPLINKEDLIST lpNode = m_lpHead;

	while (lpNode)
	{
		if (((T)lpNode->lpData) == t)
		{
			return (PLISTNODE)lpNode;
		}

		lpNode = lpNode->next;
	}

	return (PLISTNODE) NULL;
}
