#pragma once
#define _MAX_TIMER 50
class CTimer
{
public:
	CTimer();
	~CTimer();
	int id;
	int milliseconds;//¼ä¸ô
	int tick;//¼ÆÊ±
	void Update(int mill);
};

class TimerMng
{
public:
	TimerMng();
	~TimerMng();
	void SetTimer(int id, int milliseconds);
	static void OnTimerProc(int id);
	void KillTimer(int id);
	void Update(int mill);
	CWHDynamicArray<CTimer> timer;
	static TimerMng * Instance;
};

