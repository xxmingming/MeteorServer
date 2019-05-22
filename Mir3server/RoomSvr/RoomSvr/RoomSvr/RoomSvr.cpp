//// RoomSvr.cpp: 定义控制台应用程序的入口点。
////
//
#include "stdafx.h"
#include "ikcp.h"
#include <iostream>
#include <Windows.h>
#include <string>
#include <Shlwapi.h>
#include <map>
#include <list>
#include "../../../Def/ServerSockHandler.h"
using namespace std;
#include "../../../Def/Setting.h"

BOOL ConnectToServer(SOCKET &s, SOCKADDR_IN* addr, LPCSTR lpServerIP, DWORD dwIP, int nPort);

Setting *		g_set = NULL;
short			g_GameSvrPort = 5000;//对内 往游戏服转发数据.
string			g_strGameSvrIP;
SOCKET			g_csock = INVALID_SOCKET;
SOCKADDR_IN		g_caddr;

int main()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	/*sockMain = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	SOCKADDR_IN from;
	SOCKADDR_IN local;
	local.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	local.sin_family = AF_INET;
	local.sin_port = htons(9000);
	int len = sizeof(sockaddr);
	KcpCtx = new KcpContext();
	int a = bind(sockMain, (sockaddr*)&local, len);
	test();*/

	//主线程创建套接字，链接到配置中的服务器，链接成功后，主要由内部接收服务器的输入，以及自身事件往服务器的发送

	ConnectToServer(g_csock, &g_caddr, g_strGameSvrIP.c_str(), 0, g_GameSvrPort);

    return 0;
}

BOOL ConnectToServer(SOCKET &s, SOCKADDR_IN* addr, LPCSTR lpServerIP, DWORD dwIP, int nPort)
{
	if (s != INVALID_SOCKET)
	{
		closesocket(s);
		s = INVALID_SOCKET;
	}

	s = socket(AF_INET, SOCK_STREAM, 0);

	addr->sin_family = AF_INET;
	addr->sin_port = htons(nPort);

	if (lpServerIP)
	{
		addr->sin_addr.s_addr = inet_addr(lpServerIP);
	}
	else
	{
		DWORD dwReverseIP = 0;

		dwReverseIP = (LOBYTE(LOWORD(dwIP)) << 24) | (HIBYTE(LOWORD(dwIP)) << 16) | (LOBYTE(HIWORD(dwIP)) << 8) | (HIBYTE(HIWORD(dwIP)));
		addr->sin_addr.s_addr = dwReverseIP;
	}

	if (connect(s, (const struct sockaddr FAR*)addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return TRUE;
	}

	return FALSE;
}

void LoadConfig()
{
	if (g_set == NULL)
		g_set = new Setting((char*)CFG_GAMEROOM);

	g_strGameSvrIP = g_set->GetValueString(DEFAULTSEC, "GameSvrIP");
	if (g_strGameSvrIP == "")
	{
		g_strGameSvrIP = "127.0.0.1";
		g_set->SetValueString(DEFAULTSEC, "GameSvrIP", g_strGameSvrIP);
	}

	g_GameSvrPort = g_set->GetValueInt(DEFAULTSEC, "GameSvrPort");
	if (g_GameSvrPort == 0)
	{
		g_GameSvrPort = 5000;
		g_set->SetValueInt(DEFAULTSEC, "GameSvrPort", g_GameSvrPort);
	}
}

void SaveConfig()
{
	if (g_set != NULL)
	{
		g_set->SetValueString(DEFAULTSEC, "GameSvrIP", g_strGameSvrIP);
		g_set->SetValueInt(DEFAULTSEC, "GameSvrPort", g_GameSvrPort);
	}
}