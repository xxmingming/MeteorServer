
#ifndef _GAMEGATE_DEFINE
#define _GAMEGATE_DEFINE

//������ͻ��˵�����
class CSessionInfo
{
public:
	SOCKET			sock;
	WORD			nServerUserIndex;//��Ϸ�����ÿͻ��˶�Ӧ���û��±�
	WORD			nSessionIndex;//�������ط��µ���ţ�[0-500)
	CIntLock		SendBuffLock;
	CHAR			SendBuffer[DATA_BUFSIZE];
	int				nSendBufferLen;

	// For Overlapped I/O
	OVERLAPPED		Overlapped;
	WSABUF			DataBuf;
	CHAR			Buffer[DATA_BUFSIZE];
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

	//�ͻ��˷����İ���4�ֽڳ��� 4�ֽ���Ϣ���������>8�����ȫ��Ϊ����Ϣ���Ĳ���.
	bool HasCompletionPacket()
	{
		if (bufLen < 8)
			return false;
		return bufLen >= ntohl(*(int*)Buffer);
	}

	// ��ͻ��˷����İ�.ǰ8�ֽ�4�ֽڳ��ȣ�4�ֽ�Message��
	int ExtractPacket(char *pPacket, int & message)
	{
		int packetLen = ntohl(*(int*)Buffer);
		message = ntohl(*(int*)(Buffer + 4));
		if (packetLen < 8 || packetLen > DATA_BUFSIZE)
		{
			//���Ƿ����Ͽ�����
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
	int				nMessage;//�ͻ��˷������ص���ϢID
	int				nLength;//szData����Ч����.���л�����Ϣ����
	int				nIndex;//�ڴ���±�
	char			szData[DATA_BUFSIZE];//�ͻ��˷������صģ����л�����Ϣ
}_TSENDBUFF, *_LPTSENDBUFF;

#define LOGPARAM_STR						1
#define LOGPARAM_INT						2

void SendSocketMsgS (int nIdent, WORD wIndex, int nSocket, WORD wSrvIndex, int nLen, char *pszData);

extern BOOL	g_fTerminated;
void LoadConfig();
void SaveConfig();
extern short g_localPort;
extern short g_GameSvrPort;
extern string	 g_strGameSvrIP;
extern Setting * g_set;
extern CWHDynamicArray<tag_TSENDBUFF> g_memPool;
#endif //_GAMEGATE_DEFINE