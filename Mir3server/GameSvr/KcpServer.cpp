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
	for (int i = 0; i < _NUM_OF_MAXPLAYER; i++)
	{
		len[i] = 0;
		memset(&addr[i], 0x00, sizeof(sockaddr_in));
		kcp[i] = NULL;
		kcpBuffer[i] = new char[KCP_PACKET_SIZE];
		kcpLen[i] = 0;
	}
}

KcpServer::~KcpServer()
{
	//��������kcp�����ͷ�
	for (int i = 0; i < _NUM_OF_MAXPLAYER; i++)
	{
		if (kcp[i] != NULL)
		{
			ikcp_release(kcp[i]);
			kcp[i] = NULL;
		}
	}
}

void KcpServer::Update()
{
	millisec += 20;
	//��ͷ��β����������kcp����ÿ����updateһ��
	for (int i = 0; i < _NUM_OF_MAXPLAYER; i++)
	{
		if (kcp[i] != NULL)
		{
			ikcp_update(kcp[i], millisec);
		}
	}
}

void KcpServer::OnPlayerEnter(int player)
{
	ikcpcb *kcpPlayer = ikcp_create(player, (void*)this);

	// ����kcp���²����������Ϊ udp_output��ģ��udp�����������
	kcpPlayer->output = udp_output;
	ikcp_wndsize(kcpPlayer, 128, 128);
	ikcp_nodelay(kcpPlayer, 1, 10, 2, 0);
	if (kcp[player] != NULL)
		printf("already exist player");
	kcp[player] = kcpPlayer;
}

void KcpServer::OnRecv(int nRecvBytes)
{
	for (int i = 0; i < _NUM_OF_MAXPLAYER; i++)
	{
		if (kcp[i] != NULL)
		{
			if (-1 == ikcp_input(kcp[i], Buffer, nRecvBytes))
			{

			}
			else
			{
				//conv��ͬ
				len[kcp[i]->conv] = fromlen;
				memmove(&addr[kcp[i]->conv], &from, sizeof(from));
				int received = ikcp_recv(kcp[i], (char*)(&kcpBuffer[i] + kcpLen[i]), KCP_PACKET_SIZE - kcpLen[i]);
				kcpLen[i] += received;

				//���Խ��kcp�����Ϣ��
				//4�ֽڳ���-4�ֽ���ϢID
				ExtractPacket(i);
				break;
			}
		}
	}
}

//���Ӧ��ҵİ�
void KcpServer::ExtractPacket(int playerId)
{
	int offset = 0;
	while (kcpLen[playerId] > 8)
	{
		CMsg * pMsg = (CMsg*)(&kcpBuffer[playerId] + offset);
		
		//��������С��С�ڵ�ǰ�Ѵ������ݴ�С
		if (pMsg->Size < kcpLen[playerId])
		{
			char * pData = ((char*)pMsg) + sizeof(CMsg);
			//���������
			switch (pMsg->Message)
			{
				//case METEOR_MSG_
				default:printf("recv kcp message"); break;
			}

			kcpLen[playerId] -= pMsg->Size;
			offset += pMsg->Size;
		}
	}

	//��β���ڴ濽����ǰ����.
	if (kcpLen[playerId] != 0 && offset != 0)
		memmove(&kcpBuffer[playerId] + offset, &kcpBuffer[playerId], kcpLen[playerId]);
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
	CreateIoCompletionPort((HANDLE)sock, g_KcpIOCP, (DWORD)this, 0);
	if (Recv() == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
	{
		print("WSARecv() failed");
		CloseServer(this);
	}
	//�󶨵���ɶ˿���

	//

	//IUINT32 current = iclock();
	//IUINT32 slap = current + 20;
	//IUINT32 index = 0;
	//IUINT32 next = 0;
	//IINT64 sumrtt = 0;
	//int count = 0;
	//int maxrtt = 0;

	//// ���ô��ڴ�С��ƽ���ӳ�200ms��ÿ20ms����һ������
	//// �����ǵ������ط�����������շ�����Ϊ128
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
	//			// û���յ������˳�
	//			if (hr < 0) break;
	//		}
	//		ikcp_send(kcp1, buffer2, hr);
	//		printf("%s", buffer2);
	//		break;
	//	}
	//}

	//ts1 = iclock() - ts1;
	
}

int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
	KcpServer * pServer = (KcpServer*)user;
	sendto(pServer->sock, buf, len, 0, (SOCKADDR*)&pServer->addr[kcp->conv], pServer->len[kcp->conv]);
	return 0;
}

//kcp�̣߳���������kcpserver�׽ӿ��ϵ�udp��Ϣ����
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

//����UDP�׽��ֳ���������.
void CloseServer(KcpServer * pServer)
{

}