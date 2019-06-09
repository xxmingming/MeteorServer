#include "KcpServer.h"
int udp_output(const char *buf, int len, ikcpcb *kcp, void *user);
DWORD WINAPI KcpWorkerThread(LPVOID CompletionPortID);
INT CreateKcpWorkerThread(int nThread);
void CloseServer(KcpServer * pServer);
HANDLE g_KcpIOCP = NULL;

KcpServer::KcpServer(int roomIdx)
{
	port = KCP_PORTBASE + roomIdx;
	srvIndex = roomIdx;
	millisec = 0;
	logicFrame = 0;
}

KcpServer::~KcpServer()
{
	//遍历所有kcp对象，释放
	PLISTNODE p = users.GetHead();
	while (p != NULL)
	{
		CUserInfo * pUser = users.GetData(p);
		if (pUser != NULL)
		{
			pUser->KcpRelease();
		}
		p = users.GetNext(p);
	}
}

void KcpServer::Update()
{
	millisec += 20;
	logicFrame += 1;
	PLISTNODE p = users.GetHead();
	while (p != NULL)
	{
		CUserInfo * pUser = users.GetData(p);
		if (pUser != NULL)
		{
			pUser->KcpUpdate(millisec, frame);
		}
		p = users.GetNext(p);
	}
	frames.AddNewNode(frame);
	frame = new GameFrames();
}

void KcpServer::OnPlayerLeave(CUserInfo * player)
{
	if (player->m_pRoom != NULL)
	{
		player->m_pGateInfo->OnLeaveRoom(player);
		player->m_pRoom = NULL;
	}
	if (player->m_pxPlayerObject != NULL)
		player->CloseUserHuman();
	player->KcpRelease();
	users.RemoveNodeByData(player);
}

//新一轮开始
void KcpServer::OnNewTurn()
{
	logicFrame = 0;
	//等待一个时间后，开始新对局，提醒客户端转到结算页面-重新开始新的一轮.
	PLISTNODE phead = frames.GetHead();
	while (phead != NULL)
	{
		GameFrames * pFrame = frames.GetData(phead);
		delete pFrame;
		phead = frames.GetNext(phead);
	}
	frames.Clear();
	PLISTNODE phead = kcpCommand.GetHead();
	while (phead != NULL)
	{
		char * pMemory = kcpCommand.GetData(phead);
		delete []pMemory;
		phead = kcpCommand.GetNext(phead);
	}
	kcpCommand.Clear();
	if (frame != NULL)
	{
		delete frame;
		frame = NULL;
	}
	frame = new GameFrames();
	FrameCommand * cmd = frame.add_commands();
	cmd->set_command(MeteorMsg_Command_SyncRandomSeed);
	cmd->set_logicframe(1);
	cmd->set_playerid(0);//id为0时消息为广播.
	SyncInitData sync;
	sync.set_randomseed(::GetTickCount());
	int l = sync.ByteSizeLong();
	char * buffer = new char[l];
	sync.SerializeToArray(buffer, l);
	cmd->set_data(buffer);
}

//角色在关卡内销毁.-角色主动离开地图时.
void KcpServer::OnPlayerDestroy(CUserInfo * player, char * data, int size)
{
	FrameCommand * cmd = frame.add_commands();
	cmd->set_command(MeteorMsg_Command_DestroyPlayer);
	cmd->set_logicframe(logicFrame + 1);
	cmd->set_playerid(player->m_nUserServerIndex);
	char * dst = new char[size];
	memcpy(dst, data, size);
	kcpCommand.AddNewNode(dst);
	cmd->set_data(dst);
	//玩家离开战斗场景，也会导致角色离开房间
	OnPlayerLeave(player);
}

//角色进入关卡-当角色加载地图完成时，在服务器当前帧出生
void KcpServer::OnPlayerSpawn(CUserInfo * player, char * data, int size)
{
	FrameCommand * cmd = frame.add_commands();
	cmd->set_command(MeteorMsg_Command_SpawnPlayer);
	cmd->set_logicframe(logicFrame + 1);
	cmd->set_playerid(player->m_nUserServerIndex);
	char * dst = new char[size];
	memcpy(dst, data, size);
	kcpCommand.AddNewNode(dst);
	cmd->set_data(dst);
}

//进入房间-还未进入关卡
void KcpServer::OnPlayerEnter(CUserInfo * player)
{
	users.AddNewNode(player);
	player->InitKcp(this);

	//frames.mutable_frames().;
}

void KcpServer::OnRecv(int nRecvBytes)
{
	PLISTNODE p = users.GetHead();
	while (p != NULL)
	{
		CUserInfo * pUser = users.GetData(p);
		if (pUser != NULL)
		{
			if (pUser->KcpInput(Buffer, nRecvBytes) != -1)
			{
				pUser->OnReceivedMsg();
				break;
			}
		}
		p = users.GetNext(p);
	}
}

void KcpServer::InitKcp()
{
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	local.sin_addr.S_un.S_addr = INADDR_ANY;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	int len = sizeof(sockaddr);
	
	int a = EADDRINUSE;
	while (a == EADDRINUSE)
	{
		port = port + 1;
		local.sin_port = htons(port);
		a = bind(sock, (sockaddr*)&local, len);
	}
	int imode = 1;
	ioctlsocket(sock, FIONBIO, (u_long*)&imode);//设置为非阻塞
	vprint("kcp bind port:%d", port);
	CreateIoCompletionPort((HANDLE)sock, g_KcpIOCP, (DWORD)this, 0);
	if (Recv() == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
	{
		print("WSARecv() failed");
		CloseServer(this);
	}
	//绑定到完成端口上

	//

	//IUINT32 current = iclock();
	//IUINT32 slap = current + 20;
	//IUINT32 index = 0;
	//IUINT32 next = 0;
	//IINT64 sumrtt = 0;
	//int count = 0;
	//int maxrtt = 0;

	//// 配置窗口大小：平均延迟200ms，每20ms发送一个包，
	//// 而考虑到丢包重发，设置最大收发窗口为128
	//


	//char buffer[2000];
	//char buffer2[2000];
	//int hr;

	//IUINT32 ts1 = iclock();

	//while (1) {
	//	isleep(1);
	//	current = iclock();
	//	
	//	sockaddr_in addr_from;
	//	addr_from.sin_family = AF_INET;
	//	int len = sizeof(addr_from);
	//	while (1) {
	//		KcpContext * ctx = (KcpContext*)kcp1->user;
	//		//if (ctx->len == 0)
	//		{
	//			int k = recvfrom(ctx->socket, buffer, 2000, 0, (sockaddr*)&addr_from, &len);
	//			if (k <= 0)
	//				break;
	//			hr = 
	//			if (hr < 0)
	//				break;
	//			hr = ikcp_recv(kcp1, buffer2, 2000);

	//			memcpy(&ctx->addr, &addr_from, len);
	//			ctx->len = len;
	//			// 没有收到包就退出
	//			if (hr < 0) break;
	//		}
	//		ikcp_send(kcp1, buffer2, hr);
	//		printf("%s", buffer2);
	//		break;
	//	}
	//}

	//ts1 = iclock() - ts1;
	
}

//kcp线程，处理所有kcpserver套接口上的udp消息接收
INT CreateKcpWorkerThread(int nThread)
{
	DWORD	dwThreadID;

	if (g_KcpIOCP != INVALID_HANDLE_VALUE)
	{
		SYSTEM_INFO SystemInfo;

		if ((g_KcpIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
			return -1;

		GetSystemInfo(&SystemInfo);
		for (UINT i = 0; i < SystemInfo.dwNumberOfProcessors; i++)
		{
			HANDLE ThreadHandle;

			if ((ThreadHandle = CreateThread(NULL, 0, KcpWorkerThread, g_KcpIOCP, 0, &dwThreadID)) == NULL)
				return -1;

			CloseHandle(ThreadHandle);
		}

		return 1;
	}

	return -1;
}

DWORD WINAPI KcpWorkerThread(LPVOID CompletionPortID)
{
	DWORD					dwBytesTransferred;
	KcpServer*				pServer;
	LPOVERLAPPED			lpOverlapped;
	while (TRUE)
	{
		GetQueuedCompletionStatus((HANDLE)CompletionPortID, &dwBytesTransferred, (LPDWORD)&pServer,
			(LPOVERLAPPED *)&lpOverlapped, INFINITE);
		if (g_fTerminated) return 0;
		if (dwBytesTransferred == 0)
		{
			printf("kcp server closed");
			CloseServer(pServer);
			continue;
		}

		if (pServer->nOvFlag == OVERLAPPED_FLAG::OVERLAPPED_RECV)
		{
			pServer->OnRecv(dwBytesTransferred);
			if (pServer->Recv() == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
			{
				print("WSARecv() failed");
				CloseServer(pServer);
				continue;
			}
		}
	}

	return 0L;
}

//房间UDP套接字出现了问题.
void CloseServer(KcpServer * pServer)
{

}