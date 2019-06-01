

#pragma once

#include "../def/staticArray.h"
#include "../def/_orzex/syncobj.h"

#define _RUNGATE_STATUS_DISCONNECTED		0
#define _RUNGATE_STATUS_GOOD				1
#define _RUNGATE_STATUS_HEAVYTRAFFIC		2

#define LOGPARAM_STR						1
#define LOGPARAM_INT						2

#define _NUM_OF_MAXROOM						48//��󷿼���.
#define _NUM_OF_MAXPLAYER					16//������������.

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
	BYTE							m_btCurrentMode;
	//��������
	byte*							m_pKeys;
	//ҡ��
	float							Jx;
	float							Jy;
	//����
	float							Mx;//������x delta
	float							My;//������y delta
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

//�ṩ��GameSrv�Ķ˿�7200
extern short			g_nLocalPort;
//�ṩ����ɫ���صĶ˿�5500
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
