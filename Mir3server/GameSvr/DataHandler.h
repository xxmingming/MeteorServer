#pragma once

#pragma pack(1)
#pragma pack(8)

#define OS_MOVINGOBJECT		1
#define OS_ITEMOBJECT		2
#define OS_EVENTOBJECT		3
#define OS_GATEOBJECT		4
#define OS_SWITCHOBJECT		5
#define OS_MAPEVENT			6
#define OS_DOOR				7
#define OS_ROON				8

typedef struct tag_TOSOBJECT
{
	BYTE		btType;
	VOID		*pObject;
	DWORD		dwAddTime;
} _TOSOBJECT, *_LPTOSOBJECT;

class CRoomInfo:public CIntLock, CStaticArray<CRoomInfo>::IArrayData
{
public:
	const int syncDelta = 50;
	BOOL						m_bTurnStart;
	uint32_t					m_nRoomIndex;
	CWHList<CUserInfo*>			m_pUserList;
	CHAR						m_szName[20];
	CHAR						m_szPassword[8];
	uint32_t					m_nRule;
	uint32_t					m_nGroup1;
	uint32_t					m_nGroup2;
	uint32_t					m_nMaxPlayer;
	uint32_t					m_nHpMax;
	uint32_t					m_nCount;

	int							m_turnTime;
	bool						closed;
	DWORD						m_dwMap;
	DWORD						m_dwOwnerId;
	void						OnNewTurn();
	BOOL						RemovePlayer(CUserInfo * pUser);
	UINT						m_delta;
	int							m_totalTime;
	DWORD						m_currentTick;
	DWORD						m_dwWaitClose;
	DWORD						m_dwTurnIndex;
	bool						m_bHasPsd;
	void						Update();
	void						OnUserKeyFrame();
	bool						IsEmpty() { return m_nCount == 0; }
	void						OnPlayerAllLeaved();
	void						Close();
	void						NewTurn();
	void						WaitClose();

	CRoomInfo();
	~CRoomInfo();
};

class CMirMap
{
public:
	char				m_szMapName[16];
	char				m_szMapTextName[40];
	int					m_nLevelIdx;
public:
	CMirMap();
	~CMirMap();

	BOOL			LoadMapData(char *pszName);
};
