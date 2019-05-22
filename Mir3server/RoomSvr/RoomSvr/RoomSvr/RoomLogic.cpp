//=====================================================================
//
// test.cpp - kcp 测试用例
//
// 说明：
// gcc test.cpp -o test -lstdc++
//
//=====================================================================
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "ikcp.h"
//#include "ikcp.c"

class KcpContext
{
public:
	KcpContext()
	{
		len = 0;
	}
	int len;
	SOCKADDR_IN addr;
	SOCKET socket;
};


static KcpContext * KcpCtx;
// 模拟网络：模拟发送一个 udp包
int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
	union { int id; void *ptr; } parameter;
	KcpContext * puser = (KcpContext*)user;
	//sendto(sockMain, buf, len, 0, (SOCKADDR*)&puser->addr, puser->len);
	return 0;
}

// 测试用例
void test()
{
	// 创建两个端点的 kcp对象，第一个参数 conv是会话编号，同一个会话需要相同
	// 最后一个是 user参数，用来传递标识
	ikcpcb *kcp1 = ikcp_create(1, (void*)KcpCtx);

	// 设置kcp的下层输出，这里为 udp_output，模拟udp网络输出函数
	kcp1->output = udp_output;

	IUINT32 current = iclock();
	IUINT32 slap = current + 20;
	IUINT32 index = 0;
	IUINT32 next = 0;
	IINT64 sumrtt = 0;
	int count = 0;
	int maxrtt = 0;

	// 配置窗口大小：平均延迟200ms，每20ms发送一个包，
	// 而考虑到丢包重发，设置最大收发窗口为128
	ikcp_wndsize(kcp1, 128, 128);


	char buffer[2000];
	char buffer2[2000];
	int hr;

	IUINT32 ts1 = iclock();

	while (1) {
		isleep(1);
		current = iclock();
		ikcp_update(kcp1, iclock());
		sockaddr_in addr_from;
		addr_from.sin_family = AF_INET;
		int len = sizeof(addr_from);
		while (1) {
			KcpContext * ctx = (KcpContext*)kcp1->user;
			//if (ctx->len == 0)
			{
				int k = recvfrom(ctx->socket, buffer, 2000, 0, (sockaddr*)&addr_from, &len);
				if (k <= 0)
					break;
				hr = ikcp_input(kcp1, buffer, k);
				if (hr < 0)
					break;
				hr = ikcp_recv(kcp1, buffer2, 2000);

				memcpy(&ctx->addr, &addr_from, len);
				ctx->len = len;
				// 没有收到包就退出
				if (hr < 0) break;
			}
			ikcp_send(kcp1, buffer2, hr);
			printf("%s", buffer2);
			break;
		}
	}

	ts1 = iclock() - ts1;
	ikcp_release(kcp1);
}