
#ifndef _LEGENDOFMIR_ENDECODE
#define _LEGENDOFMIR_ENDECODE

#define _DEFBLOCKSIZE		16
#define _NUM_OF_MAXPLAYER					16//房间人数上限.
//包头
typedef struct tag_TMSGHEADER
{
	int		nSocket;//套接字
	int		nLength;//序列化信息长度
	int		nMessage;//当主消息是广播时，辅消息存储在此
	int		wIdent;//消息号，如果是广播，则上一个位置存储了真实消息
	WORD	wSessionIndex;//在网关的序号-非广播消息时起效
	WORD	wUserListIndex;//在服务器的用户序号-非广播消息时起效
	WORD	wUserList[_NUM_OF_MAXPLAYER];//角色在服务器的编号-广播消息时起效
} _TMSGHEADER, *_LPTMSGHEADER;

//void WINAPI fnMakeDefMessage(_LPTDEFAULTMESSAGE lptdm, WORD wIdent, int nRecog, WORD wParam, WORD wTag, WORD wSeries);
#ifdef _UNICODE
#define fnEncode6BitBuf	fnEncode6BitBufW
#define fnDecode6BitBuf fnDecode6BitBufW
#define fnEncodeMessage fnEncodeMessageW
#define fnDecodeMessage fnDecodeMessageW
#define fnMakeDefMessage fnMakeDefMessageW
#else
#define fnEncode6BitBuf	fnEncode6BitBufA
#define fnDecode6BitBuf fnDecode6BitBufA
#define fnEncodeMessage fnEncodeMessageA
#define fnDecodeMessage fnDecodeMessageA
#define fnMakeDefMessage fnMakeDefMessageA
#endif
int  WINAPI fnEncode6BitBufW(unsigned char *pszSrc, TCHAR *pszDest, int nSrcLen, int nDestLen);
int  WINAPI fnDecode6BitBufW(TCHAR *pwszSrc, char *pszDest, int nDestLen);
int  WINAPI fnEncode6BitBufA(unsigned char *pszSrc, char *pszDest, int nSrcLen, int nDestLen);
int  WINAPI fnDecode6BitBufA(char *pwszSrc, char *pszDest, int nDestLen);
#endif //_LEGENDOFMIR_ENDECODE