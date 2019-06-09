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

class KcpServer;
class CRoomInfo:public CIntLock, CStaticArray<CRoomInfo>::IArrayData
{
public:
	const int syncDelta = 50;
	BOOL						m_bGameStart;
	uint32_t					m_nRoomIndex;
	KcpServer *					m_pKcpServer;
	CWHList<CUserInfo*>			m_pUserList;
	CHAR						m_szName[20];
	CHAR						m_szPassword[8];//�������6λ������
	uint32_t					m_nRule;
	uint32_t					m_nPattern;//�ز�����-ÿ�������֮������¼�����ݲ��ţ����뷿���޹���.
	uint32_t                    m_nVersion;//�汾107/907
	char *						m_pRecordData;//¼������.
	uint32_t					m_nGroup1;
	uint32_t					m_nGroup2;
	uint32_t					m_nMaxPlayer;
	uint32_t					m_nHpMax;
	uint32_t					m_nCount;

	int							m_turnTime;
	bool						running;
	DWORD						m_dwMap;//�½ں� �� 1000 + �ؿ����½�������
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
	bool						IsEmpty() { return m_nCount == 0 && !running; }
	void						OnPlayerAllLeaved();
	void						Close();
	void						NewTurn();
	void						WaitClose();
	int							InitKcpServer();
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
