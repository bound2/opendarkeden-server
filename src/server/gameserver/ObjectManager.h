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
// Top-level management class that manages the managers of the sub game objects.
//////////////////////////////////////////////////////////////////////////////

class ActionFactoryManager;
class AlignmentManager;
class BloodBibleBonusManager;
class CastleInfoManager;
class CastleShrineInfoManager;
class CastleSkillInfoManager;
class CombatInfoManager;
class ConditionFactoryManager;
class CoupleManager;
class DarkLightInfoManager;
class DefaultOptionSetInfoManager;
class DirectiveSetManager;
class DragonEyeManager;
class DynamicZoneFactoryManager;
class DynamicZoneInfoManager;
class DynamicZoneManager;
class EffectLoaderManager;
class EventQuestLootingManager;
class FlagManager;
class GameServerGroupInfoManager;
class GlobalPartyManager;
class GoodsInfoManager;
class ItemFactoryManager;
class ItemInfoManager;
class ItemLoaderManager;
class ItemMineInfoManager;
class MasterLairInfoManager;
class MonsterInfoManager;
class MonsterNameManager;
class OustersEXPInfoManager;
class PCFinder;
class PriceManager;
class RankBonusInfoManager;
class ScriptManager;
class ShopTemplateManager;
class SkillDomainInfoManager;
class SkillInfoManager;
class SkillPropertyManager;
class TelephoneCenter;
class TimeChecker;
class TimeManager;
class UniqueItemManager;
class VampEXPInfoManager;
class VisionInfoManager;
class VolumeInfoManager;
class WarSystem;
class WayPointManager;
class WeatherInfoManager;
class ZoneGroupManager;
class ZoneInfoManager;

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
    AlignmentManager* m_pAlignmentManager = nullptr;
    BloodBibleBonusManager* m_pBloodBibleBonusManager = nullptr;
    CastleInfoManager* m_pCastleInfoManager = nullptr;
    CastleShrineInfoManager* m_pCastleShrineInfoManager = nullptr;
    CastleSkillInfoManager* m_pCastleSkillInfoManager = nullptr;
    CombatInfoManager* m_pCombatInfoManager = nullptr;
    ConditionFactoryManager* m_pConditionFactoryManager = nullptr;
    CoupleManager* m_pCoupleManager = nullptr;
    DarkLightInfoManager* m_pDarkLightInfoManager = nullptr;
    DefaultOptionSetInfoManager* m_pDefaultOptionSetInfoManager = nullptr;
    DirectiveSetManager* m_pDirectiveSetManager = nullptr;
    DragonEyeManager* m_pDragonEyeManager = nullptr;
    DynamicZoneFactoryManager* m_pDynamicZoneFactoryManager = nullptr;
    DynamicZoneInfoManager* m_pDynamicZoneInfoManager = nullptr;
    DynamicZoneManager* m_pDynamicZoneManager = nullptr;
    EffectLoaderManager* m_pEffectLoaderManager = nullptr;
    EventQuestLootingManager* m_pEventQuestLootingManager = nullptr;
    FlagManager* m_pFlagManager = nullptr;
    GameServerGroupInfoManager* m_pGameServerGroupInfoManager = nullptr;
    GlobalPartyManager* m_pGlobalPartyManager = nullptr;
    GoodsInfoManager* m_pGoodsInfoManager = nullptr;
    ItemFactoryManager* m_pItemFactoryManager = nullptr;
    ItemInfoManager* m_pItemInfoManager = nullptr;
    ItemLoaderManager* m_pItemLoaderManager = nullptr;
    ItemMineInfoManager* m_pItemMineInfoManager = nullptr;
    MasterLairInfoManager* m_pMasterLairInfoManager = nullptr;
    MonsterInfoManager* m_pMonsterInfoManager = nullptr;
    MonsterNameManager* m_pMonsterNameManager = nullptr;
    OustersEXPInfoManager* m_pOustersEXPInfoManager = nullptr;
    PCFinder* m_pPCFinder = nullptr;
    PriceManager* m_pPriceManager = nullptr;
    RankBonusInfoManager* m_pRankBonusInfoManager = nullptr;
    ScriptManager* m_pPublicScriptManager = nullptr;
    ShopTemplateManager* m_pShopTemplateManager = nullptr;
    SkillDomainInfoManager* m_pSkillDomainInfoManager = nullptr;
    SkillInfoManager* m_pSkillInfoManager = nullptr;
    SkillPropertyManager* m_pSkillPropertyManager = nullptr;
    TelephoneCenter* m_pTelephoneCenter = nullptr;
    TimeChecker* m_pTimeChecker = nullptr;
    TimeManager* m_pTimeManager = nullptr;
    UniqueItemManager* m_pUniqueItemManager = nullptr;
    VampEXPInfoManager* m_pVampEXPInfoManager = nullptr;
    VisionInfoManager* m_pVisionInfoManager = nullptr;
    VolumeInfoManager* m_pVolumeInfoManager = nullptr;
    WarSystem* m_pWarSystem = nullptr;
    WayPointManager* m_pWayPointManager = nullptr;
    WeatherInfoManager* m_pWeatherInfoManager = nullptr;
    ZoneGroupManager* m_pZoneGroupManager = nullptr;
    ZoneInfoManager* m_pZoneInfoManager = nullptr;
};

#endif
