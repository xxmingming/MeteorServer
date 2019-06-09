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
	void OnPlayerSpawn(CUserInfo * player, char * data, int size);//出现在战场时
	void OnPlayerDestroy(CUserInfo * player, char * data, int size);//销毁在战场时
	void OnPlayerEnter(CUserInfo * player);//当玩家进入房间时，创建一个KCP对象处理该客户端消息
	void OnPlayerLeave(CUserInfo * player);//当玩家离开房间时，销毁该玩家对应的KCP对象.
	SOCKADDR_IN local;//在一个端口上绑定，创建多个kcp对象，每个kcp对象拥有一个ID号，处理客户端输入
	SOCKET sock;
	int	port;
	int millisec;
	int fromlen;
	INT	nOvFlag;
	int				logicFrame;//逻辑帧编号
	OVERLAPPED		Overlapped;
	WSABUF			DataBuf;
	CHAR			Buffer[MAXRECV];
	
	sockaddr		from;
	CWHList<CUserInfo*> users;
	CWHList<char*>	kcpCommand;//历史操作缓冲区，
	CWHList<GameFrames*> frames;
	GameFrames*		frame;//当前帧

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
	void OnNewTurn();
};
