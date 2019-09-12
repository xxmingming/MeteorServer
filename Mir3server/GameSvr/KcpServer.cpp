#include "KcpServer.h"
int udp_output(const char *buf, int len, ikcpcb *kcp, void *user);
DWORD WINAPI KcpWorkerThread(LPVOID CompletionPortID);
INT CreateKcpWorkerThread();
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
	//��������kcp�����ͷ�
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
			pUser->KcpUpdate(millisec);
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

//��һ�ֿ�ʼ
void KcpServer::OnNewTurn()
{
	logicFrame = 0;
	//�ȴ�һ��ʱ��󣬿�ʼ�¶Ծ֣����ѿͻ���ת������ҳ��-���¿�ʼ�µ�һ��.
	PLISTNODE phead = frames.GetHead();
	while (phead != NULL)
	{
		GameFrames * pFrame = frames.GetData(phead);
		delete pFrame;
		phead = frames.GetNext(phead);
	}
	frames.Clear();
	phead = kcpCommand.GetHead();
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
	FrameCommand * cmd = frame->add_commands();
	cmd->set_command(MeteorMsg_Command_SyncRandomSeed);
	cmd->set_logicframe(1);
	cmd->set_playerid(0);//idΪ0ʱ��ϢΪ�㲥.
	SyncInitData sync;
	sync.set_randomseed(::GetTickCount());
	int l = sync.ByteSizeLong();
	char * buffer = new char[l];
	sync.SerializeToArray(buffer, l);
	cmd->set_data(buffer);
}

//��ɫ�ڹؿ�������.-��ɫ�����뿪��ͼʱ.
void KcpServer::OnPlayerDestroy(CUserInfo * player, char * data, int size)
{
	FrameCommand * cmd = frame->add_commands();
	cmd->set_command(MeteorMsg_Command_DestroyPlayer);
	cmd->set_logicframe(logicFrame + 1);
	cmd->set_playerid(player->m_nUserServerIndex);
	char * dst = new char[size];
	memcpy(dst, data, size);
	kcpCommand.AddNewNode(dst);
	cmd->set_data(dst);
	//����뿪ս��������Ҳ�ᵼ�½�ɫ�뿪����
	OnPlayerLeave(player);
}

//��ɫ����ؿ�-����ɫ���ص�ͼ���ʱ���ڷ�������ǰ֡����
void KcpServer::OnPlayerSpawn(CUserInfo * player, char * data, int size)
{
	FrameCommand * cmd = frame->add_commands();
	cmd->set_command(MeteorMsg_Command_SpawnPlayer);
	cmd->set_logicframe(logicFrame + 1);
	cmd->set_playerid(player->m_nUserServerIndex);
	char * dst = new char[size];
	memcpy(dst, data, size);
	kcpCommand.AddNewNode(dst);
	cmd->set_data(dst);
}

//���뷿��-��δ����ؿ�
void KcpServer::OnPlayerEnter(CUserInfo * player)
{
	users.AddNewNode(player);
	player->InitKcp(this);

	//frames.mutable_frames().;
}

void KcpServer::OnRecv(int nRecvBytes)
{
	//һ�������Ӧһ��KcpServer���յ��÷����Ӧ��UDP��Ϣʱ����ѯÿ���û���kcp�ڲ��������Ϣ��˭����
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
	ioctlsocket(sock, FIONBIO, (u_long*)&imode);//����Ϊ������
	vprint("kcp bind port:%d", port);
	CreateIoCompletionPort((HANDLE)sock, g_KcpIOCP, (DWORD)this, 0);
	if (Recv() == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
	{
		print("WSARecv() failed");
		CloseServer(this);
	}
}

//kcp�̣߳���������kcpserver�׽ӿ��ϵ�udp��Ϣ����
INT CreateKcpWorkerThread()
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

//����UDP�׽��ֳ���������.
void CloseServer(KcpServer * pServer)
{

}