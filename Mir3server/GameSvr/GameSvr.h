

#pragma once

#include "../def/staticArray.h"
#include "../def/_orzex/syncobj.h"

#define _RUNGATE_STATUS_DISCONNECTED		0
#define _RUNGATE_STATUS_GOOD				1
#define _RUNGATE_STATUS_HEAVYTRAFFIC		2

#define LOGPARAM_STR						1
#define LOGPARAM_INT						2

#define _NUM_OF_MAXROOM						48//最大房间数.
#define _NUM_OF_MAXPLAYER					16//房间人数上限.

#define HAM_ALL								0
#define HAM_PEACE							1
#define HAM_GROUP							2
#define HAM_GUILD							3
#define HAM_PKATTACK						4

#define BAGGOLD								5000000

#define USERMODE_PLAYGAME					1
#define USERMODE_LOGIN						2
#define USERMODE_LOGOFF						3
#define USERMODE_NOTICE						4

typedef struct tagOVERLAPPEDEX
{
	OVERLAPPED				Overlapped;
	INT						nOvFlag;
	WSABUF					DataBuf;
	CHAR					Buffer[DATA_BUFSIZE];
	int						bufLen;
} OVERLAPPEDEX, *LPOVERLAPPEDEX;

typedef struct tag_TSENDBUFF
{
	int				nLen;
	int				nIndex;//内存池下标
	char			szData[DATA_BUFSIZE];
}_TSENDBUFF, *_LPTSENDBUFF;

class CGateInfo:public CIntLock
{
public:
	SOCKET					m_sock;

	BOOL					m_fDoSending;
	CWHQueue				m_xSendBuffQ;
	OVERLAPPEDEX			OverlappedEx[2];
	
public:
	CGateInfo();

	void	SendGateCheck();
	void	OpenNewUser(char *pszPacket);
	void	OnLeaveRoom(CUserInfo * pUser);
	void	xSend();
	int		Send(_LPTSENDBUFF lpSendBuff);
	int		Recv();
	bool	HasCompletionPacket();
	int		ExtractPacket(char *pPacket);
};

class CUserInfo	: public CIntLock, CStaticArray< CUserInfo >::IArrayData
{
public:
	static int KeyMax;
	bool							m_bEmpty;
	bool							m_bDirty;//数据还未由客户端同步初始化。需要等待
	int								m_sock;

	char							m_szUserID[20];//ID
	char							m_szCharName[20];//角色名称

	WORD							m_nUserGateIndex;//在网关的链接ID
	WORD							m_nUserServerIndex;//在服务端的链接ID
	
	int								m_nCertification;
	int								m_nClientVersion;

	CPlayerObject*					m_pxPlayerObject;
	CGateInfo*						m_pGateInfo;
	CRoomInfo*						m_pRoom;
	BYTE							m_btCurrentMode;
	//按键输入
	byte*							m_pKeys;
	//摇杆
	float							Jx;
	float							Jy;
	//触屏
	float							Mx;//触摸屏x delta
	float							My;//触摸屏y delta
public:
	CUserInfo();
	bool IsEmpty();
	void							CloseAccount(char *pszName, int nCertification);
	void							CloseUserHuman();
	void							DoClientCertification(char *pszPacket);
	void							Operate(Input_ * pInput);
	void							ProcessUserMessage(char *pszPacket);
	void							SetName(const char * pszName);
	void							CopyTo(Player_ * pPlayer);
	void							Update(Player_ * pPlayer);
	BOOL							NeedReborn(float delta);
};

//提供给GameSrv的端口7200
extern short			g_nLocalPort;
//提供给角色网关的端口5500
//extern short			g_nDBSrvPort;
extern Setting			* g_set;
//extern string			g_strDBSource;
//extern string			g_strDBAccount;
//extern string			g_strDBPassword;
//extern char				g_strDBSvrIP[];
extern char				g_strClientPath[];
//extern int				g_nStartLevel;
//extern int				g_nStartGold;
void LoadConfig();
void SaveConfig();
bool TestDsn();
