
#ifndef _GAMEGATE_DEFINE
#define _GAMEGATE_DEFINE

//������ͻ��˵�����
class CSessionInfo
{
public:
	SOCKET			sock;
	WORD			nServerUserIndex;//��Ϸ�����ÿͻ��˶�Ӧ���û��±�
	WORD			nSessionIndex;//�������ط��µ���ţ�[0-5000)
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
		if (packetLen - 8 > 1024)
		{
			//���Ե���
			printf("packet size > 1024 message = %d packet size = %d", message, packetLen);
			memmove(Buffer, Buffer + packetLen, DATA_BUFSIZE - packetLen);
			bufLen -= packetLen;
		}
		else
		{
			memcpy(pPacket, Buffer + 8, packetLen - 8);
			memmove(Buffer, Buffer + packetLen, DATA_BUFSIZE - packetLen);
			bufLen -= packetLen;
		}
		return packetLen - 8;
	}
};

typedef struct tag_TSENDBUFF
{
	SOCKET			sock;
	int				nSessionIndex;
	int				nMessage;
	int				nLength;//szData����Ч����.
	char			szData[1024];
}_TSENDBUFF, *_LPTSENDBUFF;

#define LOGPARAM_STR						1
#define LOGPARAM_INT						2

void InsertLogMsg(UINT nID);
void InsertLogMsg(LPTSTR lpszMsg);
void InsertLogPacket(char *pszPacket);
void InsertLogMsgParam(UINT nID, void *pParam, BYTE btFlags);

void SendSocketMsgS (_LPTMSGHEADER lpMsg, int nLen1, char *pszData1, int nLen2, char *pszData2);
void SendSocketMsgS (int nIdent, WORD wIndex, int nSocket, WORD wSrvIndex, int nLen, char *pszData);

extern BOOL	g_fTerminated;
void LoadConfig();
void SaveConfig();
extern short g_localPort;
extern short g_GameSvrPort;
extern string	 g_strGameSvrIP;
extern Setting * g_set;
#endif //_GAMEGATE_DEFINE