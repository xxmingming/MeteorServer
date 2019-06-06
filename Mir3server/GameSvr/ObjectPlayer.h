

// ORZ:
#include "../def/staticArray.h"
#include "../def/_orzex/syncobj.h"

class CPlayerObject : public CCharObject, public CIntLock, CStaticArray< CPlayerObject >::IArrayData
{
public:
	bool					m_bEmpty;
	int						m_nArrIndex;
	BOOL					m_fIsAlive;
	BOOL					m_fAdmin;
	DWORD					m_dwLastTalkTime;
public:
	void		Constructor();
	bool		IsEmpty();
	CPlayerObject();
	CPlayerObject(CUserInfo* pUserInfo);
	virtual void	GetCharName(char *pszCharName);
	void			SetCharName(const char *pszCharName);
};
