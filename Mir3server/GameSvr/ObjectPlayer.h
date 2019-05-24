

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
	DWORD					m_dwLastTalkTime;//最后一次说话Tick
	//CWHList<CCharObject*>	m_xSlaveObjList;//宝宝
public:
	void		Constructor();
	bool		IsEmpty();

	CPlayerObject();
	CPlayerObject(CUserInfo* pUserInfo);

	void		Spawn(int startPoint, int camp, int model, int weapon);
	void		Reborn();
	void		SetPosition(float x, float y, float z);
	void		SetRotation(float w, float x, float y, float z);
	virtual void	GetCharName(char *pszCharName);
	void			SetCharName(const char *pszCharName);
};
