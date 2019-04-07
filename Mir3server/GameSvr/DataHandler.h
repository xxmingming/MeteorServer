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

//类似游戏room
class CRoomInfo:public CIntLock
{
public:
	const int syncDelta = 20;//同步的帧率为20-50毫秒一次 每秒50-20次
	CHAR						m_chFlag;
	BOOL						m_bTurnStart;//在循环同步过程中，只要有人进入房间
	uint32_t					m_nRoomIndex;//房间编号
	//CMirMap *					m_pMap;//房间使用的地图模板-暂时是无意义的
	CWHList<_LPTOSOBJECT>*		m_xpObjectList;//地图上的物件.
	CWHList<CUserInfo*>			m_pUserList;//地图上的角色.
	CHAR						m_szName[20];//房间名称.8个汉字以内.
	CHAR						m_szPassword[8];//6位数字密码
	uint32_t					m_nRule;//规则.
	uint32_t					m_nGroup1;//第一队
	uint32_t					m_nGroup2;//第二队
	uint32_t					m_nMaxPlayer;//最大人数
	uint32_t					m_nHpMax;//血值上限标准.
	uint32_t					m_nCount;//当前人数
	int							m_turnTime;//房间单轮时长
	bool						closed;//是否在下一次循环中关闭.
	DWORD						m_dwMap;//地图ID由Chapter * 1000 + LevelId构成.
	DWORD						m_dwOwnerId;//创建者ID
	void						OnNewTurn();
	BOOL						RemovePlayer(CUserInfo * pUser);
	UINT						m_delta;//当前
	int							m_totalTime;//当前轮次运行时长为多久.当房间内无人时，这个时间凝固.
	DWORD						m_currentTick;//当前tick
	DWORD						m_dwWaitClose;//等待关闭房间的计时器
	DWORD						m_dwTurnIndex;//Turn次数,一个turn8帧.
	bool						m_bHasPsd;//有无密码.
	void						Update();
	void						OnUserKeyFrame(TurnFrames * pk);
	BOOL						IsEmpty() { return m_nCount == 0; }
	void						OnAllPlayerLeaved();
	void						Close() {
		this->m_bTurnStart = false;
		this->closed = true;
	}

	//新的一轮.
	void						NewTurn();
	void						WaitClose();//人数为空持续半分钟

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
