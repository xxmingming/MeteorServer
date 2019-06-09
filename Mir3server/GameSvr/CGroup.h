#pragma once
#include "../def/staticArray.h"
#include "../def/_orzex/syncobj.h"
class CGroup;
//排队里同一个队列里的10名选手
//排位系统需要有
//1：角色差异，比如角色的成长方向与基础数值
//A：力量-肉盾，坦克
//B：速度-刺客，DPS
//C：远程- 速度+攻击力
//D: 智力形-法强

//2：特性技能-唯一的被动技能
//3：主动技能-QWER-技能是英雄间的主要区别
//4：武器-道具系统 - 各色武器价格不一致，功效不一致，带不带被动技能-唯一技能-主动技能
//5：BUFF/野怪系统 - 野怪增益 - 野怪增强 按时间
//6：兵线刷怪系统 - 兵线经济 - 小兵增强 按时间
//7：视野系统 - 地形遮蔽 / 草丛视野 眼/真眼 道具
//8：塔系统-包括 塔血量 防御 攻击 频率 增益 
//9：角色等级-属性提升
//综上MOBA系统难以在3.0版本完成故排位按钮暂时无效
class CGroup :public CIntLock, CStaticArray<CGroup>::IArrayData
{
public:
	CGroup();
	~CGroup();
	
	bool IsEmpty();
};

