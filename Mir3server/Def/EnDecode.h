
#ifndef _LEGENDOFMIR_ENDECODE
#define _LEGENDOFMIR_ENDECODE

#define _DEFBLOCKSIZE		16
#define _NUM_OF_MAXPLAYER					16//������������.
//��ͷ
typedef struct tag_TMSGHEADER
{
	int		nSocket;//�׽���
	int		nLength;//���л���Ϣ����
	int		nMessage;//������Ϣ�ǹ㲥ʱ������Ϣ�洢�ڴ�
	int		wIdent;//��Ϣ�ţ�����ǹ㲥������һ��λ�ô洢����ʵ��Ϣ
	WORD	wSessionIndex;//�����ص����-�ǹ㲥��Ϣʱ��Ч
	WORD	wUserListIndex;//�ڷ��������û����-�ǹ㲥��Ϣʱ��Ч
	WORD	wUserList[_NUM_OF_MAXPLAYER];//��ɫ�ڷ������ı��-�㲥��Ϣʱ��Ч
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