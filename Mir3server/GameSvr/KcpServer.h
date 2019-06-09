#pragma once
#include "StdAfx.h"
#include "../Def/ikcp.h"
#define MAXRECV 60000
#define KCP_PORTBASE	1013
#define KCP_PACKET_SIZE 4096

#pragma pack(8)
struct CMsg
{
	uint32_t Size;
	uint32_t Message;
};
#pragma pack()
class CUserInfo;
class KcpServer : public CIntLock
{
public:
	KcpServer(int roomIdx);
	~KcpServer();
	void InitKcp();
	void Update();
	void OnPlayerSpawn(CUserInfo * player, char * data, int size);//������ս��ʱ
	void OnPlayerDestroy(CUserInfo * player, char * data, int size);//������ս��ʱ
	void OnPlayerEnter(CUserInfo * player);//����ҽ��뷿��ʱ������һ��KCP������ÿͻ�����Ϣ
	void OnPlayerLeave(CUserInfo * player);//������뿪����ʱ�����ٸ���Ҷ�Ӧ��KCP����.
	SOCKADDR_IN local;//��һ���˿��ϰ󶨣��������kcp����ÿ��kcp����ӵ��һ��ID�ţ�����ͻ�������
	SOCKET sock;
	int	port;
	int millisec;
	int fromlen;
	INT	nOvFlag;
	int				logicFrame;//�߼�֡���
	OVERLAPPED		Overlapped;
	WSABUF			DataBuf;
	CHAR			Buffer[MAXRECV];
	
	sockaddr		from;
	CWHList<CUserInfo*> users;
	CWHList<char*>	kcpCommand;//��ʷ������������
	CWHList<GameFrames*> frames;
	GameFrames*		frame;//��ǰ֡

	int				srvIndex;//�������.
	int  Recv()
	{
		nOvFlag = OVERLAPPED_FLAG::OVERLAPPED_RECV;
		DWORD nRecvBytes = 0, nFlags = 0;

		DataBuf.len = MAXRECV;
		DataBuf.buf = Buffer;

		memset(&Overlapped, 0, sizeof(Overlapped));

		return WSARecvFrom(sock, &DataBuf, 1, &nRecvBytes, &nFlags, &from, &fromlen, &Overlapped, 0);
	}

	void OnRecv(int nRecvBytes);
	void OnNewTurn();
};
