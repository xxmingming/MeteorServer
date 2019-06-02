#pragma once
#ifndef _DEFAULT_SOCKET_HANDLER
#define _DEFAULT_SOCKET_HANDLER
#define DATA_BUFSIZE 512 * 1024

class OVERLAPPED_FLAG
{
public:
	static const int OVERLAPPED_RECV = 0;
	static const int OVERLAPPED_SEND = 1;
};

typedef struct tag_TCOMPLETIONPORT
{
   OVERLAPPED		Overlapped;
   WSABUF			DataBuf;
   CHAR				Buffer[DATA_BUFSIZE];
   INT				nOvFlag;
} _TCOMPLETIONPORT, * _LPTCOMPLETIONPORT;

typedef struct tag_TOVERLAPPEDEX
{
   CHAR				Buffer[DATA_BUFSIZE];
   WSABUF			DataBuf;
   SOCKET			Socket;
   WSAOVERLAPPED	Overlapped;
   DWORD			BytesSEND;
   DWORD			BytesRECV;
   INT				nOvFlag;
} _TOVERLAPPEDEX, * _LPTOVERLAPPEDEX;

BOOL InitServerSocket(SOCKET &s, SOCKADDR_IN* addr, int nPort);
BOOL ConnectToServer(SOCKET &s, SOCKADDR_IN* addr, LPCSTR lpServerIP, DWORD dwIP, int nPort);
BOOL ConnectToServer(SOCKET &s, SOCKADDR_IN* addr, LPCTSTR lpServerIP, DWORD dwIP, int nPort);
BOOL ClearSocket(SOCKET &s);
BOOL InitServerThreadForMsg();
BOOL InitThread(LPTHREAD_START_ROUTINE lpRoutine);

BOOL CheckSocketError(LPARAM lParam);

#endif
