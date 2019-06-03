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
	void OnPlayerEnter(int playerIdx);//当玩家进入房间时，创建一个KCP对象处理该客户端消息
	int						len[_NUM_OF_MAXPLAYER];
	sockaddr_in				addr[_NUM_OF_MAXPLAYER];
	ikcpcb*					kcp[_NUM_OF_MAXPLAYER];
	CHAR *					kcpBuffer[_NUM_OF_MAXPLAYER];//每一个拥有4096字节的缓冲区，在帧同步协议中，包最大长度4096
	int						kcpLen[_NUM_OF_MAXPLAYER];//每个kcp缓冲区有效数据长度.
	SOCKADDR_IN local;//在一个端口上绑定，创建多个kcp对象，每个kcp对象拥有一个ID号，处理客户端输入
	SOCKET sock;
	int	port;
	int millisec;
	int fromlen;
	INT	nOvFlag;
	OVERLAPPED		Overlapped;
	WSABUF			DataBuf;
	CHAR			Buffer[MAXRECV];
	sockaddr		from;
	int				srvIndex;//房间序号.
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
