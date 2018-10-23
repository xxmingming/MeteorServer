
#ifndef _LEGENDOFMIR_ENDECODE
#define _LEGENDOFMIR_ENDECODE

#define _DEFBLOCKSIZE		16

//包头
typedef struct tag_TMSGHEADER
{
	int		nSocket;//套接字
	int		nLength;//序列化信息长度
	int		wIdent;//消息号
	WORD	wSessionIndex;//连接序号
	WORD	wUserListIndex;//在服务器的用户序号
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