
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
#endif //_LEGENDOFMIR_ENDECODE