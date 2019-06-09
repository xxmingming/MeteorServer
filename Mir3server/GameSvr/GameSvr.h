#pragma once
#include "../def/staticArray.h"
#include "../def/_orzex/syncobj.h"
#include "../Def/ikcp.h"
#define _NUM_OF_MAXROOM						30//最大房间数.
#define _NUM_OF_MAXPLAYER					16//房间人数上限.
#define DATA_GATE_SIZE		60000			//游戏服处理每个网关的缓冲区大小，发送和接收
typedef struct tagOVERLAPPEDEX
{
	OVERLAPPED				Overlapped;
	INT						nOvFlag;
	WSABUF					DataBuf;
	CHAR					Buffer[DATA_GATE_SIZE];
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
	void	DisconnectClient(int client);
	void	SendGateCheck();
	void	OpenNewUser(char *pszPacket);
	void	OnLeaveRoom(CUserInfo * pUser);
	void	xSend();
	int		Recv();
	bool	HasCompletionPacket(int offset);
	int		NextPacketOffset(int offset);
};

class CUserInfo	: public CIntLock, CStaticArray< CUserInfo >::IArrayData
{
public:
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
public:
	CUserInfo();
	bool IsEmpty();
	void							CloseUserHuman();
	void							DoClientCertification(UINT32 clientV, std::string name);
	void							SetName(const char * pszName);
	_inline bool					ClientSafe() { return m_nCertification == 1; }
	KcpServer*						m_pKcpSvr;
	ikcpcb*							m_pKcp;
	int								m_nKcpReveivedBytes;//接收到的KCP数据长度
	CHAR							m_pKcpBuffer[DATA_BUFSIZE];
	sockaddr_in						m_remoteaddr;//kcp对端地址
	int								m_nremoteaddrlen;//对端地址长度
	void							KcpRelease();//清理掉KCP
	void							KcpUpdate(int mill);
	void							InitKcp(KcpServer * pServer);
	int								KcpInput(char * buffer, int size);
	void							OnReceivedMsg();
	void							ExtractPacket();
	void							OnEnterLevel(char * pData, int size);
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
//extern char				g_strClientPath[];
//extern int				g_nStartLevel;
//extern int				g_nStartGold;
void LoadConfig();
void SaveConfig();
bool TestDsn();
