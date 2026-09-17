//////////////////////////////////////////////////////////////////////////////
// Filename    : ObjectManager.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __OBJECT_MANAGER_H__
#define __OBJECT_MANAGER_H__

#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ObjectManager;
// 하위 게임 객체들의 매니저를 관리하는 상위 관리 클래스이다.
//////////////////////////////////////////////////////////////////////////////

class ActionFactoryManager;
class CastleShrineInfoManager;
class CastleSkillInfoManager;
class ConditionFactoryManager;
class DarkLightInfoManager;
class DefaultOptionSetInfoManager;
class DirectiveSetManager;
class DragonEyeManager;
class DynamicZoneFactoryManager;
class DynamicZoneInfoManager;
class EventQuestLootingManager;
class GameServerGroupInfoManager;
class GoodsInfoManager;
class ItemLoaderManager;
class MonsterNameManager;
class OustersEXPInfoManager;
class RankBonusInfoManager;
class ScriptManager;
class ShopTemplateManager;
class SkillDomainInfoManager;
class SkillPropertyManager;
class TelephoneCenter;
class TimeChecker;
class UniqueItemManager;
class VampEXPInfoManager;
class VisionInfoManager;
class VolumeInfoManager;
class WayPointManager;
class WeatherInfoManager;

class ObjectManager {
public:
    ObjectManager();
    ~ObjectManager();

public:
    void init();
    void load();
    void save();

private:
    // Managers this class creates and deletes. The ones a subsystem
    // outside this file reads are registered on de::GameContext; the
    // rest are reached only from here.
    ActionFactoryManager* m_pActionFactoryManager = nullptr;
    CastleShrineInfoManager* m_pCastleShrineInfoManager = nullptr;
    CastleSkillInfoManager* m_pCastleSkillInfoManager = nullptr;
    ConditionFactoryManager* m_pConditionFactoryManager = nullptr;
    DarkLightInfoManager* m_pDarkLightInfoManager = nullptr;
    DefaultOptionSetInfoManager* m_pDefaultOptionSetInfoManager = nullptr;
    DirectiveSetManager* m_pDirectiveSetManager = nullptr;
    DragonEyeManager* m_pDragonEyeManager = nullptr;
    DynamicZoneFactoryManager* m_pDynamicZoneFactoryManager = nullptr;
    DynamicZoneInfoManager* m_pDynamicZoneInfoManager = nullptr;
    EventQuestLootingManager* m_pEventQuestLootingManager = nullptr;
    GameServerGroupInfoManager* m_pGameServerGroupInfoManager = nullptr;
    GoodsInfoManager* m_pGoodsInfoManager = nullptr;
    ItemLoaderManager* m_pItemLoaderManager = nullptr;
    MonsterNameManager* m_pMonsterNameManager = nullptr;
    OustersEXPInfoManager* m_pOustersEXPInfoManager = nullptr;
    RankBonusInfoManager* m_pRankBonusInfoManager = nullptr;
    ScriptManager* m_pPublicScriptManager = nullptr;
    ShopTemplateManager* m_pShopTemplateManager = nullptr;
    SkillDomainInfoManager* m_pSkillDomainInfoManager = nullptr;
    SkillPropertyManager* m_pSkillPropertyManager = nullptr;
    TelephoneCenter* m_pTelephoneCenter = nullptr;
    TimeChecker* m_pTimeChecker = nullptr;
    UniqueItemManager* m_pUniqueItemManager = nullptr;
    VampEXPInfoManager* m_pVampEXPInfoManager = nullptr;
    VisionInfoManager* m_pVisionInfoManager = nullptr;
    VolumeInfoManager* m_pVolumeInfoManager = nullptr;
    WayPointManager* m_pWayPointManager = nullptr;
    WeatherInfoManager* m_pWeatherInfoManager = nullptr;
};

#endif
