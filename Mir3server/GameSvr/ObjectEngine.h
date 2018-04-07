

#pragma once
#include "../Def/protocol.pb.h"
#include "../def/_orzex/queue.h"


#define DEFSPEED					14

#define _CHAT_COLOR1				0			//RGB(  0,   0,   0); // Åõ°ú»ö.
#define _CHAT_COLOR2				1			//RGB( 10,  10,  10); // °ËÁ¤»ö.
#define _CHAT_COLOR3				2			//RGB(255, 255, 255); // Èò»ö.
#define _CHAT_COLOR4				3			//RGB(255,   0,   0); // »¡°­.
#define _CHAT_COLOR5				4			//RGB(  0, 255,   0); // ³ì»ö.
#define _CHAT_COLOR6				5			//RGB(  0,   0, 255); // Çª¸¥»ö
#define _CHAT_COLOR7				6			//RGB(255, 255,   0); // ³ë¶û»ö.
#define _CHAT_COLOR8				7			//RGB(255, 128,   0); // ÁÖÈ²»ö

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
#define	STATE_OPENHEATH				0x00000002;  //Ã¼·Â °ø°³»óÅÂ

class CCharObject;
class CMirMap;
class CUserInfo;
class CRoomInfo;
typedef struct tag_TPROCESSMSG
{
	WORD			wIdent;
	WORD			wParam;
	DWORD			lParam1;
	DWORD			lParam2;
	DWORD			lParam3;

	DWORD			dwDeliveryTime;

	CCharObject*	pCharObject;

	char			*pszData;
} _TPROCESSMSG, *_LPTPROCESSMSG;

/*
*/

#define _OBJECT_HUMAN			1
#define _OBJECT_MONSTER			2
#define _OBJECT_ANIMAL			6
#define _OBJECT_NPC				8

typedef struct tag_TOBJECTFEATURE
{
	BYTE		btGender;
	BYTE		btWear;
	BYTE		btHair;
	BYTE		btWeapon;
} _TOBJECTFEATURE, _LPTOBJECTFEATURE;

typedef struct tag_TOBJECTFEATUREEX
{
	BYTE		btHorse;
	WORD		dwHairColor;
	WORD		dwWearColor;
} _TOBJECTFEATUREEX, _LPTOBJECTFEATUREEX;

class CVisibleObject
{
public:
	int				nVisibleFlag;
	CCharObject*	pObject;
};

class CVisibleEvent
{
public:
	int				nVisibleFlag;
	CEvent*			pEvent;
};

class CVisibleMapItem
{
public:
	int				nVisibleFlag;
	WORD			wX;
	WORD			wY;
	CMapItem*		pMapItem;
};

#pragma pack(1)
class CObjectAbility
{
public:
	//BYTE	Level;
	WORD	HP;//ÆøÑª
	WORD	MP;//Å­Æø
    WORD	MaxHP;//ÆøÑªÉÏÏÞ
    WORD	MaxMP;//Å­ÆøÉÏÏÞ
	WORD	Weapon1;//1ºÅÎ»ÎäÆ÷
	WORD	Weapon2;//2ºÅÎ»ÎäÆ÷
	WORD	Weapon;//µ±Ç°Ö÷ÎäÆ÷
	WORD	WeaponPos;//ÎäÆ÷×ËÊÆ
	WORD	Model;//Ä£ÐÍID
	WORD	Camp;//ÕóÓª0Á÷ÐÇ£¬1ºûµû
	WORD	StartPoint;//µØÍ¼Ëæ»ú³öÉúµã.Ã¿½áÊø£¬Ã¿ËÀÍöË¢ÐÂ
	WORD	Frame;//¶¯»­Ö¡
	WORD	AniSource;//¶¯»­Ô´
};

class CObjectAddAbility	// ¾ÆÀÌÅÛ Âø¿ëÀ¸·Î ´Ã¾î³ª´Â ´É·ÂÄ¡
{
public:
	WORD	HP;
	WORD	MP;
	WORD	HIT;
	WORD	SPEED;
	WORD	AC;
	WORD	MAC;
	WORD	DC;
	WORD	MC;
	WORD	SC;
	WORD	AntiPoison;
	WORD	PoisonRecover;
	WORD	HealthRecover;
	WORD	SpellRecover;
	WORD	AntiMagic;			//¸¶¹ý È¸ÇÇÀ²
	BYTE	Luck;				//Çà¿î Æ÷ÀÎÆ®
	BYTE	UnLuck;				//ºÒÇà Æ÷ÀÎÆ®
	BYTE	WeaponStrong;
	short	HitSpeed;
};
#pragma pack(8)

class CCharObject
{
public:
	CUserInfo*					m_pUserInfo;
	Vector3_					m_Pos;
	Quaternion_					m_nRotation;
	WORD						m_wObjectType;
	CWHList<CCharObject*>		m_xCacheObjectList;
	char						m_szName[20];
	CObjectAbility				m_Ability;
	CObjectAbility				m_WAbility;
	CObjectAddAbility			m_AddAbility;
	UINT						m_nCharStatusEx;
	UINT						m_nCharStatus;
	WORD						m_wStatusArr[MAX_STATUS_ATTRIBUTE];
	DWORD						m_dwStatusTime[MAX_STATUS_ATTRIBUTE];
	BOOL						m_fIsDead;
public:
	CCharObject(CUserInfo*	pUserInfo);
	virtual ~CCharObject();
	void	SendSocket(char *pszPacket);
	void	Die();
	UINT	GetCharStatus();
	virtual void	GetCharName(char *pszCharName) = 0;
};
