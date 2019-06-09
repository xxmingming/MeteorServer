#pragma once
#include "../def/staticArray.h"
#include "../def/_orzex/syncobj.h"
#include "../Def/ikcp.h"
#define _NUM_OF_MAXROOM						30//��󷿼���.
#define _NUM_OF_MAXPLAYER					16//������������.
#define DATA_GATE_SIZE		60000			//��Ϸ������ÿ�����صĻ�������С�����ͺͽ���
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
	int				nIndex;//�ڴ���±�
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
	bool							m_bDirty;//���ݻ�δ�ɿͻ���ͬ����ʼ������Ҫ�ȴ�
	int								m_sock;

	char							m_szUserID[20];//ID
	char							m_szCharName[20];//��ɫ����

	WORD							m_nUserGateIndex;//�����ص�����ID
	WORD							m_nUserServerIndex;//�ڷ���˵�����ID
	
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
	int								m_nKcpReveivedBytes;//���յ���KCP���ݳ���
	CHAR							m_pKcpBuffer[DATA_BUFSIZE];
	sockaddr_in						m_remoteaddr;//kcp�Զ˵�ַ
	int								m_nremoteaddrlen;//�Զ˵�ַ����
	void							KcpRelease();//�����KCP
	void							KcpUpdate(int mill);
	void							InitKcp(KcpServer * pServer);
	int								KcpInput(char * buffer, int size);
	void							OnReceivedMsg();
	void							ExtractPacket();
	void							OnEnterLevel(char * pData, int size);
};

//�ṩ��GameSrv�Ķ˿�7200
extern short			g_nLocalPort;
//�ṩ����ɫ���صĶ˿�5500
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
