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
				SendSocketMsgS(GM_CHECKCLIENT, 0, 0, 0, 0, NULL, NULL);
			}
			break;
		}
	}
}