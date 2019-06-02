
#ifndef _GAMEGATE_DEFINE
#define _GAMEGATE_DEFINE
#define MAXSEND 80000
#define MAXRECV 60000
class CSessionInfo
{
public:
	SOCKET			sock;
	WORD			nServerUserIndex;
	WORD			nSessionIndex;
	CIntLock		SendBuffLock;
	CHAR			SendBuffer[MAXSEND];
	int				nSendBufferLen;

	OVERLAPPED		Overlapped;
	WSABUF			DataBuf;
	CHAR			Buffer[MAXRECV];
	INT				bufLen;
	INT				nOvFlag;

	CSessionInfo()
	{
		bufLen	= 0;
	}

	void Reset()
	{
		bufLen = 0;
	}

	int  Recv()
	{
		nOvFlag = OVERLAPPED_FLAG::OVERLAPPED_RECV;
		DWORD nRecvBytes = 0, nFlags = 0;

		DataBuf.len = MAXRECV - bufLen;
		DataBuf.buf = Buffer + bufLen;

		memset( &Overlapped, 0, sizeof( Overlapped ) );

		return WSARecv( sock, &DataBuf, 1, &nRecvBytes, &nFlags, &Overlapped, 0 );
	}

	bool HasCompletionPacket()
	{
		if (bufLen < 8)
			return false;
		return bufLen >= ntohl(*(int*)Buffer);
	}

	int ExtractPacket(char *pPacket, int & message)
	{
		int packetLen = ntohl(*(int*)Buffer);
		message = ntohl(*(int*)(Buffer + 4));
		if (packetLen < 8 || packetLen > DATA_BUFSIZE)
		{
			return -1;
		}
		memcpy(pPacket, Buffer + 8, packetLen - 8);
		memmove(Buffer, Buffer + packetLen, DATA_BUFSIZE - packetLen);
		bufLen -= packetLen;
		return packetLen - 8;
	}
};

typedef struct tag_TSENDBUFF
{
	SOCKET			sock;
	int				nSessionIndex;
	int				nMessage;
	int				nLength;
	int				nIndex;
	char			szData[DATA_BUFSIZE];
}_TSENDBUFF, *_LPTSENDBUFF;

#define LOGPARAM_STR						1
#define LOGPARAM_INT						2

void SendSocketMsgS (int nIdent, WORD wIndex, int nSocket, WORD wSrvIndex, int nLen, char *pszData, char * pMemory);

extern BOOL	g_fTerminated;
void LoadConfig();
void SaveConfig();
extern short g_localPort;
extern short g_GameSvrPort;
extern string	 g_strGameSvrIP;
extern Setting * g_set;
extern SOCKET					g_csock;
extern SOCKET					g_ssock;
extern SOCKADDR_IN					g_caddr;
extern SOCKADDR_IN					g_saddr;
extern CWHDynamicArray<tag_TSENDBUFF> g_memPool;
#endif //_GAMEGATE_DEFINE