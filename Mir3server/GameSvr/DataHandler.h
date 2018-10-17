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
	const int syncDelta = 50;//ͬ����֡��Ϊ50����һ��
	CHAR						m_chFlag;
	BOOL						m_bEmpty;
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
	BOOL						RemovePlayer(CUserInfo * pUser);
	int							m_totalTime;//��ǰ�ִ�����ʱ��Ϊ���.������������ʱ�����ʱ������.
	void						Update(float delta);
	void						OnUserKeyFrame(KeyFrame k);
	BOOL						IsEmpty() { return m_bEmpty; }
	void						CreateRoom(CMirMap * map, int maxPlayer, int hpMax, int roomIdx);
	void						Close() {
		//m_chFlag &= 'c';
	}
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
