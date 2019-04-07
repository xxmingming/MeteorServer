

#pragma once
#include "../Def/protocol.pb.h"
#include "../def/_orzex/queue.h"

#define _CHAT_COLOR1				0			//RGB(  0,   0,   0); // ������.
#define _CHAT_COLOR2				1			//RGB( 10,  10,  10); // ������.
#define _CHAT_COLOR3				2			//RGB(255, 255, 255); // ���.
#define _CHAT_COLOR4				3			//RGB(255,   0,   0); // ����.
#define _CHAT_COLOR5				4			//RGB(  0, 255,   0); // ���.
#define _CHAT_COLOR6				5			//RGB(  0,   0, 255); // Ǫ����
#define _CHAT_COLOR7				6			//RGB(255, 255,   0); // �����.
#define _CHAT_COLOR8				7			//RGB(255, 128,   0); // ��Ȳ��

class CCharObject;
class CMirMap;
class CUserInfo;
class CRoomInfo;

#pragma pack(1)
class CObjectAbility
{
public:
	//BYTE	Level;
	int	HP;//��Ѫ
	int	MP;//ŭ��
	int	MaxHP;//��Ѫ����
	int	MaxMP;//ŭ������
	int	Weapon1;//1��λ����
	int	Weapon2;//2��λ����
	int	Weapon;//��ǰ������
	int	WeaponPos;//��������
	int	Model;//ģ��ID
	int	Camp;//��Ӫ0���ǣ�1����
	int	StartPoint;//��ͼ���������.ÿ������ÿ����ˢ��
	int	Frame;//����֡
	int	AniSource;//����Դ
};
#pragma pack(8)

class CCharObject
{
public:
	CUserInfo*					m_pUserInfo;
	Vector3_					m_Pos;
	Quaternion_					m_nRotation;
	char						m_szName[20];
	CObjectAbility				m_Ability;
	void						Reset(CUserInfo*pUserInfo)
	{
		m_fDeadTick = 0;
		m_pUserInfo = pUserInfo;
		m_fIsDead = FALSE;
		ZeroMemory(m_szName, sizeof(m_szName));
		m_bWaitReborn = false;
		m_bNeedSend = false;
	}
	BOOL						m_fIsDead;
	float						m_fDeadTick;
	BOOL						m_bWaitReborn;//�ȴ��ͻ�������ʬ�壬������.
	BOOL						m_bNeedSend;//�Ƿ�����ͻ��˷�������ʬ����
public:
	CCharObject(CUserInfo*	pUserInfo);
	virtual ~CCharObject();
	void	Die();
	UINT	GetCharStatus();
	virtual void	GetCharName(char *pszCharName) = 0;
};
