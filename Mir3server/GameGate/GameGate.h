
#ifndef _GAMEGATE_DEFINE
#define _GAMEGATE_DEFINE

//网关与客户端的链接
class CSessionInfo
{
public:
	SOCKET			sock;
	WORD			nServerUserIndex;//游戏服，该客户端对应的用户下标
	WORD			nSessionIndex;//单个网关服下的序号，[0-500)
	CIntLock		SendBuffLock;
	CHAR			SendBuffer[DATA_PACKETMAX];
	int				nSendBufferLen;

	// For Overlapped I/O
	OVERLAPPED		Overlapped;
	WSABUF			DataBuf;
	CHAR			Buffer[DATA_PACKETMAX];
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

		DataBuf.len = DATA_BUFSIZE - bufLen;
		DataBuf.buf = Buffer + bufLen;

		memset( &Overlapped, 0, sizeof( Overlapped ) );

		return WSARecv( sock, &DataBuf, 1, &nRecvBytes, &nFlags, &Overlapped, 0 );
	}

	//客户端发来的包，4字节长度 4字节消息，如果长度>8后面的全部为该消息带的参数.
	bool HasCompletionPacket()
	{
		if (bufLen < 8)
			return false;
		return bufLen >= ntohl(*(int*)Buffer);
	}

	// 解客户端发来的包.前8字节4字节长度，4字节Message号
	int ExtractPacket(char *pPacket, int & message)
	{
		int packetLen = ntohl(*(int*)Buffer);
		message = ntohl(*(int*)(Buffer + 4));
		if (packetLen < 8 || packetLen > DATA_BUFSIZE)
		{
			//包非法，断开连接
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
	int				nLength;//szData的有效区域.
	char			szData[DATA_BUFSIZE];
}_TSENDBUFF, *_LPTSENDBUFF;

#define LOGPARAM_STR						1
#define LOGPARAM_INT						2

void SendSocketMsgS (_LPTMSGHEADER lpMsg, int nLen1, char *pszData1, int nLen2, char *pszData2);
void SendSocketMsgS (int nIdent, WORD wIndex, int nSocket, WORD wSrvIndex, int nLen, char *pszData);

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
#endif //_GAMEGATE_DEFINE