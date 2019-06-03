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

class KcpServer : public CIntLock
{
public:
	KcpServer(int roomIdx);
	~KcpServer();
	void InitKcp();
	void Update();
	void OnPlayerEnter(int playerIdx);//����ҽ��뷿��ʱ������һ��KCP������ÿͻ�����Ϣ
	int						len[_NUM_OF_MAXPLAYER];
	sockaddr_in				addr[_NUM_OF_MAXPLAYER];
	ikcpcb*					kcp[_NUM_OF_MAXPLAYER];
	CHAR *					kcpBuffer[_NUM_OF_MAXPLAYER];//ÿһ��ӵ��4096�ֽڵĻ���������֡ͬ��Э���У�����󳤶�4096
	int						kcpLen[_NUM_OF_MAXPLAYER];//ÿ��kcp��������Ч���ݳ���.
	SOCKADDR_IN local;//��һ���˿��ϰ󶨣��������kcp����ÿ��kcp����ӵ��һ��ID�ţ�����ͻ�������
	SOCKET sock;
	int	port;
	int millisec;
	int fromlen;
	INT	nOvFlag;
	OVERLAPPED		Overlapped;
	WSABUF			DataBuf;
	CHAR			Buffer[MAXRECV];
	sockaddr		from;
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
	void ExtractPacket(int i);
};
