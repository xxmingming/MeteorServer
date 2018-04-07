

#pragma once

#include "../def/staticArray.h"
#include "../def/_orzex/syncobj.h"

#define _RUNGATE_STATUS_DISCONNECTED		0
#define _RUNGATE_STATUS_GOOD				1
#define _RUNGATE_STATUS_HEAVYTRAFFIC		2

#define LOGPARAM_STR						1
#define LOGPARAM_INT						2

#define _NUM_OF_MAXROOM						16//��󷿼���.
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

void MakeItemToDB(int nReadyUserInfo, char *pszUserId, char *pszCharName, _LPTMAKEITEMRCD lpMakeItemRcd);

typedef struct tagOVERLAPPEDEX
{
	OVERLAPPED				Overlapped;
	INT						nOvFlag;
	WSABUF					DataBuf;
	CHAR					Buffer[DATA_BUFSIZE * 2];
	int						bufLen;
} OVERLAPPEDEX, *LPOVERLAPPEDEX;

typedef struct tag_TSENDBUFF
{
	int				nLen;
	char			szData[DATA_BUFSIZE];
}_TSENDBUFF, *_LPTSENDBUFF;

class CGateInfo
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
	bool							m_bEmpty;

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
public:
	CUserInfo();
	bool IsEmpty();
	void							CloseAccount(char *pszName, int nCertification);
	void							CloseUserHuman();
	void							DoClientCertification(char *pszPacket);
	void							Operate();
	void							ProcessUserMessage(char *pszPacket);
	void							SetName(const char * pszName);
	void							CopyTo(Player_ * pPlayer);
};

void InsertLogMsg(UINT nID);
void InsertLogMsg(LPTSTR lpszMsg);
void InsertLogPacket(char *pszPacket);
void InsertLogMsgParam(UINT nID, void *pParam, BYTE btFlags);
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