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

//������Ϸroom
class CRoomInfo:public CIntLock
{
public:
	const int syncDelta = 50;//ͬ����֡��Ϊ20-50����һ�� ÿ��50-20��
	CHAR						m_chFlag;
	BOOL						m_bTurnStart;//��ѭ��ͬ�������У�ֻҪ���˽��뷿��
	uint32_t					m_nRoomIndex;//������
	CMirMap *					m_pMap;//����ʹ�õĵ�ͼģ��
	CWHList<_LPTOSOBJECT>*		m_xpObjectList;//��ͼ�ϵ����.
	CWHList<CUserInfo*>			m_pUserList;//��ͼ�ϵĽ�ɫ.
	CHAR						m_szName[20];//��������.8����������.
	CHAR						m_szPassword[8];//6λ��������
	uint32_t					m_nRule;//����.
	uint32_t					m_nGroup1;//��һ��
	uint32_t					m_nGroup2;//�ڶ���
	uint32_t					m_nMaxPlayer;//�������
	uint32_t					m_nHpMax;//Ѫֵ���ޱ�׼.
	uint32_t					m_nCount;//��ǰ����

	int							m_turnTime;//���䵥��ʱ��
	void						OnNewTurn();
	BOOL						RemovePlayer(CUserInfo * pUser);
	UINT						m_delta;//��ǰ
	int							m_totalTime;//��ǰ�ִ�����ʱ��Ϊ���.������������ʱ�����ʱ������.
	DWORD						m_currentTick;//��ǰtick
	void						Update();
	void						OnUserKeyFrame(KeyFrame * pk);
	BOOL						IsEmpty() { return m_nCount == 0; }
	void						OnAllPlayerLeaved();
	void						CreateRoom(CMirMap * map, int maxPlayer, int hpMax, int turnTime, int roomIdx);
	void						Close() {
		//m_chFlag &= 'c';
	}

	//�µ�һ��.
	void						NewTurn();

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
