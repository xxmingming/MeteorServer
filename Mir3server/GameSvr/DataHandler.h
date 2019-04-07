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
	const int syncDelta = 20;//ͬ����֡��Ϊ20-50����һ�� ÿ��50-20��
	CHAR						m_chFlag;
	BOOL						m_bTurnStart;//��ѭ��ͬ�������У�ֻҪ���˽��뷿��
	uint32_t					m_nRoomIndex;//������
	//CMirMap *					m_pMap;//����ʹ�õĵ�ͼģ��-��ʱ���������
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
	bool						closed;//�Ƿ�����һ��ѭ���йر�.
	DWORD						m_dwMap;//��ͼID��Chapter * 1000 + LevelId����.
	DWORD						m_dwOwnerId;//������ID
	void						OnNewTurn();
	BOOL						RemovePlayer(CUserInfo * pUser);
	UINT						m_delta;//��ǰ
	int							m_totalTime;//��ǰ�ִ�����ʱ��Ϊ���.������������ʱ�����ʱ������.
	DWORD						m_currentTick;//��ǰtick
	DWORD						m_dwWaitClose;//�ȴ��رշ���ļ�ʱ��
	DWORD						m_dwTurnIndex;//Turn����,һ��turn8֡.
	bool						m_bHasPsd;//��������.
	void						Update();
	void						OnUserKeyFrame(TurnFrames * pk);
	BOOL						IsEmpty() { return m_nCount == 0; }
	void						OnAllPlayerLeaved();
	void						Close() {
		this->m_bTurnStart = false;
		this->closed = true;
	}

	//�µ�һ��.
	void						NewTurn();
	void						WaitClose();//����Ϊ�ճ��������

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
