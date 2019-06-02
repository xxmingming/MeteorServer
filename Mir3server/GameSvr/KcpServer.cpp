#include "KcpServer.h"
#include "../Def/ikcp.h"
int udp_output(const char *buf, int len, ikcpcb *kcp, void *user);

KcpServer::KcpServer()
{
}


KcpServer::~KcpServer()
{
}

void KcpServer::Init()
{

}
int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
	KcpServer * puser = (KcpServer*)user;
	//sendto(sockMain, buf, len, 0, (SOCKADDR*)&puser->addr, puser->len);
	return 0;
}