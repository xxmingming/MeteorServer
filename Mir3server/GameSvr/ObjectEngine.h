

#pragma once
#include "../Def/protocol.pb.h"
#include "../def/_orzex/queue.h"


#define DEFSPEED					14

#define _CHAT_COLOR1				0			//RGB(  0,   0,   0); // ������.
#define _CHAT_COLOR2				1			//RGB( 10,  10,  10); // ������.
#define _CHAT_COLOR3				2			//RGB(255, 255, 255); // ���.
#define _CHAT_COLOR4				3			//RGB(255,   0,   0); // ����.
#define _CHAT_COLOR5				4			//RGB(  0, 255,   0); // ���.
#define _CHAT_COLOR6				5			//RGB(  0,   0, 255); // Ǫ����
#define _CHAT_COLOR7				6			//RGB(255, 255,   0); // �����.
#define _CHAT_COLOR8				7			//RGB(255, 128,   0); // ��Ȳ��

#define DR_UP						0
#define DR_UPRIGHT					1
#define DR_RIGHT					2
#define DR_DOWNRIGHT				3
#define DR_DOWN						4
#define DR_DOWNLEFT					5
#define DR_LEFT						6
#define DR_UPLEFT					7

#define _DOOR_NOT					0
#define _DOOR_OPEN					1
#define _DOOR_MAPMOVE_FRONT			2
#define _DOOR_MAPMOVE_BACK			3

// Status
#define MAX_STATUS_ATTRIBUTE		12

#define POISON_DECHEALTH			0
#define POISON_DAMAGEARMOR			1
#define POISON_LOCKSPELL			2
#define POISON_DONTMOVE				4
#define POISON_STONE				5
#define STATE_TRANSPARENT			8
#define STATE_DEFENCEUP				9
#define STATE_MAGDEFENCEUP			10
#define STATE_BUBBLEDEFENCEUP		11

#define	STATE_STONE_MODE			0x00000001;
#define	STATE_OPENHEATH				0x00000002;  //ü�� ��������

class CCharObject;
class CMirMap;
class CUserInfo;
class CRoomInfo;


#define _OBJECT_HUMAN			1
#define _OBJECT_MONSTER			2
#define _OBJECT_ANIMAL			6
#define _OBJECT_NPC				8
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
	CObjectAbility				m_WAbility;
	UINT						m_nCharStatusEx;
	UINT						m_nCharStatus;
	void						Reset(CUserInfo*pUserInfo)
	{
		m_fDeadTick = 0;
		m_pUserInfo = pUserInfo;
		//ZeroMemory(m_szName, sizeof(m_szName));
		m_nCharStatusEx = 0;
		m_nCharStatus = 0;
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
	virtual void	GetCharName(char *pszCharName) = 0;
};
