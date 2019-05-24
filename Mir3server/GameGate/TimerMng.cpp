#include "StdAfx.h"
#include "TimerMng.h"

CTimer::CTimer()
{
	
}

CTimer::~CTimer()
{

}

void CTimer::Update(int mill)
{
	if (tick - mill <= 0)
	{
		TimerMng::OnTimerProc(id);
		tick = milliseconds;
	}
	else
		tick -= mill;
}

TimerMng * TimerMng::Instance = NULL;
TimerMng::TimerMng()
{
	Instance = this;
}

TimerMng::~TimerMng()
{
}

void TimerMng::SetTimer(int id, int milliseconds)
{
	for (int i = 0; i < _MAX_TIMER; i++)
	{
		CTimer * pT = timer.GetData(i);
		if (pT != NULL && pT->id == id)
		{
			pT->milliseconds = milliseconds;
			pT->tick = milliseconds;
			return;
		}
	}

	int index = timer.GetAvailablePosition();
	if (index >= 0)
	{
		CTimer * pNew = timer.GetEmptyElement(index);
		pNew->id = id;
		pNew->milliseconds = milliseconds;
		pNew->tick = milliseconds;
	}
}

void TimerMng::KillTimer(int id)
{
	for (int i = 0; i < _MAX_TIMER; i++)
	{
		CTimer * pT = timer.GetData(i);
		if (pT != NULL && pT->id == id)
		{
			timer.DettachData(i);
		}
	}
}

void TimerMng::Update(int mill)
{
	for (int i = 0; i < _MAX_TIMER; i++)
	{
		CTimer * pT = timer.GetData(i);
		if (pT != NULL)
		{
			pT->Update(mill);
		}
	}
}

void TimerMng::OnTimerProc(int idEvent)
{
	switch (idEvent)
	{
		case _ID_TIMER_KEEPALIVE:
		{
			if (g_csock != INVALID_SOCKET)
			{
				SendSocketMsgS(GM_CHECKCLIENT, 0, 0, 0, 0, NULL);
			}
			break;
		}
		case _ID_TIMER_CONNECTSERVER:
		{
			if (g_csock == INVALID_SOCKET)
			{
				//ConnectToServer(g_csock, &g_caddr, _IDM_CLIENTSOCK_MSG, g_strGameSvrIP.c_str(), NULL, g_GameSvrPort, FD_CONNECT | FD_READ | FD_CLOSE);

				g_csock = socket(AF_INET, SOCK_STREAM, 0);

				g_caddr.sin_family = AF_INET;
				g_caddr.sin_port = htons(g_GameSvrPort);
				g_caddr.sin_addr.s_addr = inet_addr(g_strGameSvrIP.c_str());
			}

			if (connect(g_csock, (const struct sockaddr FAR*)&g_caddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
			{
				print(L"can not connect to svr");
			}
			else
			{
				if (InitServerThreadForMsg())
				{
					TimerMng::Instance->KillTimer(_ID_TIMER_CONNECTSERVER);
					TimerMng::Instance->SetTimer(_ID_TIMER_KEEPALIVE, 50000);
					//链接上了服务器
					printf("conneted with gamesvr!");
					UINT			dwThreadIDForMsg = 0;
					unsigned long	hThreadForMsg = 0;
					g_ClientIoEvent = WSACreateEvent();
					hThreadForMsg = _beginthreadex(NULL, 0, ClientWorkerThread, NULL, 0, &dwThreadIDForMsg);
				}

				InitServerSocket(g_ssock, &g_caddr, g_localPort);
			}
			break;
		}
	}
}