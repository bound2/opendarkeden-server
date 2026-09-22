//////////////////////////////////////////////////////////////////////////////
// Filename    : GameContext.h
// Description : The managers a gameserver subsystem works against, handed to
//               it explicitly instead of looked up through a global.
//
//               A subsystem that takes a GameContext& states in its
//               constructor which managers it needs, and can be built in a
//               test over stand-in ones. The context does NOT own the
//               managers: each is still created and destroyed by the startup
//               code that holds it (GameServer for the server-wide ones,
//               ObjectManager for the world ones), and registers itself here
//               as soon as it exists. A manager is therefore null until its
//               creation point is reached, and an accessor asserts on a null
//               one: reading a manager before it exists is a startup-order
//               bug, not a runtime condition to branch on.
//
//               Only forward declarations live here, so the header costs a
//               caller nothing and can be included where none of the
//               managers are linked.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GAME_CONTEXT_H__
#define __GAME_CONTEXT_H__

class ActionFactoryManager;
class AlignmentManager;
class BloodBibleBonusManager;
class CastleInfoManager;
class CastleShrineInfoManager;
class CastleSkillInfoManager;
class ClientManager;
class CombatInfoManager;
class ConditionFactoryManager;
class ConnectionInfoManager;
class CoupleManager;
class DarkLightInfoManager;
class DatabaseManager;
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
class IncomingPlayerManager;
class ItemFactoryManager;
class ItemInfoManager;
class ItemLoaderManager;
class ItemMineInfoManager;
class MasterLairInfoManager;
class MonsterInfoManager;
class MonsterNameManager;
class OptionInfoManager;
class OustersEXPInfoManager;
class PCFinder;
class PKZoneInfoManager;
class PriceManager;
class Properties;
class RankBonusInfoManager;
class ScriptManager;
class ShopTemplateManager;
class SkillDomainInfoManager;
class SkillInfoManager;
class SkillPropertyManager;
class StringPool;
class TimeChecker;
class TimeManager;
class VampEXPInfoManager;
class VariableManager;
class VolumeInfoManager;
class WarSystem;
class WayPointManager;
class WeatherInfoManager;
class ZoneGroupManager;
class ZoneInfoManager;

namespace de {

class GameContext {
public:
    GameContext() = default;

    GameContext(const GameContext&) = delete;
    GameContext& operator=(const GameContext&) = delete;

    void setActionFactoryManager(ActionFactoryManager* pActionFactoryManager) {
        m_pActionFactoryManager = pActionFactoryManager;
    }
    void setAlignmentManager(AlignmentManager* pAlignmentManager) {
        m_pAlignmentManager = pAlignmentManager;
    }
    void setBloodBibleBonusManager(BloodBibleBonusManager* pBloodBibleBonusManager) {
        m_pBloodBibleBonusManager = pBloodBibleBonusManager;
    }
    void setCastleInfoManager(CastleInfoManager* pCastleInfoManager) {
        m_pCastleInfoManager = pCastleInfoManager;
    }
    void setCastleShrineInfoManager(CastleShrineInfoManager* pCastleShrineInfoManager) {
        m_pCastleShrineInfoManager = pCastleShrineInfoManager;
    }
    void setCastleSkillInfoManager(CastleSkillInfoManager* pCastleSkillInfoManager) {
        m_pCastleSkillInfoManager = pCastleSkillInfoManager;
    }
    void setClientManager(ClientManager* pClientManager) {
        m_pClientManager = pClientManager;
    }
    void setCombatInfoManager(CombatInfoManager* pCombatInfoManager) {
        m_pCombatInfoManager = pCombatInfoManager;
    }
    void setConditionFactoryManager(ConditionFactoryManager* pConditionFactoryManager) {
        m_pConditionFactoryManager = pConditionFactoryManager;
    }
    void setConfig(Properties* pConfig) {
        m_pConfig = pConfig;
    }
    void setConnectionInfoManager(ConnectionInfoManager* pConnectionInfoManager) {
        m_pConnectionInfoManager = pConnectionInfoManager;
    }
    void setCoupleManager(CoupleManager* pCoupleManager) {
        m_pCoupleManager = pCoupleManager;
    }
    void setDarkLightInfoManager(DarkLightInfoManager* pDarkLightInfoManager) {
        m_pDarkLightInfoManager = pDarkLightInfoManager;
    }
    void setDatabaseManager(DatabaseManager* pDatabaseManager) {
        m_pDatabaseManager = pDatabaseManager;
    }
    void setDefaultOptionSetInfoManager(DefaultOptionSetInfoManager* pDefaultOptionSetInfoManager) {
        m_pDefaultOptionSetInfoManager = pDefaultOptionSetInfoManager;
    }
    void setDirectiveSetManager(DirectiveSetManager* pDirectiveSetManager) {
        m_pDirectiveSetManager = pDirectiveSetManager;
    }
    void setDragonEyeManager(DragonEyeManager* pDragonEyeManager) {
        m_pDragonEyeManager = pDragonEyeManager;
    }
    void setDynamicZoneFactoryManager(DynamicZoneFactoryManager* pDynamicZoneFactoryManager) {
        m_pDynamicZoneFactoryManager = pDynamicZoneFactoryManager;
    }
    void setDynamicZoneInfoManager(DynamicZoneInfoManager* pDynamicZoneInfoManager) {
        m_pDynamicZoneInfoManager = pDynamicZoneInfoManager;
    }
    void setDynamicZoneManager(DynamicZoneManager* pDynamicZoneManager) {
        m_pDynamicZoneManager = pDynamicZoneManager;
    }
    void setEffectLoaderManager(EffectLoaderManager* pEffectLoaderManager) {
        m_pEffectLoaderManager = pEffectLoaderManager;
    }
    void setEventQuestLootingManager(EventQuestLootingManager* pEventQuestLootingManager) {
        m_pEventQuestLootingManager = pEventQuestLootingManager;
    }
    void setFlagManager(FlagManager* pFlagManager) {
        m_pFlagManager = pFlagManager;
    }
    void setGameServerGroupInfoManager(GameServerGroupInfoManager* pGameServerGroupInfoManager) {
        m_pGameServerGroupInfoManager = pGameServerGroupInfoManager;
    }
    void setGlobalPartyManager(GlobalPartyManager* pGlobalPartyManager) {
        m_pGlobalPartyManager = pGlobalPartyManager;
    }
    void setGoodsInfoManager(GoodsInfoManager* pGoodsInfoManager) {
        m_pGoodsInfoManager = pGoodsInfoManager;
    }
    void setIncomingPlayerManager(IncomingPlayerManager* pIncomingPlayerManager) {
        m_pIncomingPlayerManager = pIncomingPlayerManager;
    }
    void setItemFactoryManager(ItemFactoryManager* pItemFactoryManager) {
        m_pItemFactoryManager = pItemFactoryManager;
    }
    void setItemInfoManager(ItemInfoManager* pItemInfoManager) {
        m_pItemInfoManager = pItemInfoManager;
    }
    void setItemLoaderManager(ItemLoaderManager* pItemLoaderManager) {
        m_pItemLoaderManager = pItemLoaderManager;
    }
    void setItemMineInfoManager(ItemMineInfoManager* pItemMineInfoManager) {
        m_pItemMineInfoManager = pItemMineInfoManager;
    }
    void setMasterLairInfoManager(MasterLairInfoManager* pMasterLairInfoManager) {
        m_pMasterLairInfoManager = pMasterLairInfoManager;
    }
    void setMonsterInfoManager(MonsterInfoManager* pMonsterInfoManager) {
        m_pMonsterInfoManager = pMonsterInfoManager;
    }
    void setMonsterNameManager(MonsterNameManager* pMonsterNameManager) {
        m_pMonsterNameManager = pMonsterNameManager;
    }
    void setOptionInfoManager(OptionInfoManager* pOptionInfoManager) {
        m_pOptionInfoManager = pOptionInfoManager;
    }
    void setOustersEXPInfoManager(OustersEXPInfoManager* pOustersEXPInfoManager) {
        m_pOustersEXPInfoManager = pOustersEXPInfoManager;
    }
    void setPCFinder(PCFinder* pPCFinder) {
        m_pPCFinder = pPCFinder;
    }
    void setPKZoneInfoManager(PKZoneInfoManager* pPKZoneInfoManager) {
        m_pPKZoneInfoManager = pPKZoneInfoManager;
    }
    void setPriceManager(PriceManager* pPriceManager) {
        m_pPriceManager = pPriceManager;
    }
    void setPublicScriptManager(ScriptManager* pPublicScriptManager) {
        m_pPublicScriptManager = pPublicScriptManager;
    }
    void setRankBonusInfoManager(RankBonusInfoManager* pRankBonusInfoManager) {
        m_pRankBonusInfoManager = pRankBonusInfoManager;
    }
    void setShopTemplateManager(ShopTemplateManager* pShopTemplateManager) {
        m_pShopTemplateManager = pShopTemplateManager;
    }
    void setSkillDomainInfoManager(SkillDomainInfoManager* pSkillDomainInfoManager) {
        m_pSkillDomainInfoManager = pSkillDomainInfoManager;
    }
    void setSkillInfoManager(SkillInfoManager* pSkillInfoManager) {
        m_pSkillInfoManager = pSkillInfoManager;
    }
    void setSkillPropertyManager(SkillPropertyManager* pSkillPropertyManager) {
        m_pSkillPropertyManager = pSkillPropertyManager;
    }
    void setStringPool(StringPool* pStringPool) {
        m_pStringPool = pStringPool;
    }
    void setTimeChecker(TimeChecker* pTimeChecker) {
        m_pTimeChecker = pTimeChecker;
    }
    void setTimeManager(TimeManager* pTimeManager) {
        m_pTimeManager = pTimeManager;
    }
    void setVampEXPInfoManager(VampEXPInfoManager* pVampEXPInfoManager) {
        m_pVampEXPInfoManager = pVampEXPInfoManager;
    }
    void setVariableManager(VariableManager* pVariableManager) {
        m_pVariableManager = pVariableManager;
    }
    void setVolumeInfoManager(VolumeInfoManager* pVolumeInfoManager) {
        m_pVolumeInfoManager = pVolumeInfoManager;
    }
    void setWarSystem(WarSystem* pWarSystem) {
        m_pWarSystem = pWarSystem;
    }
    void setWayPointManager(WayPointManager* pWayPointManager) {
        m_pWayPointManager = pWayPointManager;
    }
    void setWeatherInfoManager(WeatherInfoManager* pWeatherInfoManager) {
        m_pWeatherInfoManager = pWeatherInfoManager;
    }
    void setZoneGroupManager(ZoneGroupManager* pZoneGroupManager) {
        m_pZoneGroupManager = pZoneGroupManager;
    }
    void setZoneInfoManager(ZoneInfoManager* pZoneInfoManager) {
        m_pZoneInfoManager = pZoneInfoManager;
    }

    ActionFactoryManager& actionFactories() const;
    AlignmentManager& alignments() const;
    BloodBibleBonusManager& bloodBibleBonuses() const;
    CastleInfoManager& castleInfos() const;
    CastleShrineInfoManager& castleShrines() const;
    CastleSkillInfoManager& castleSkills() const;
    ClientManager& clients() const;
    CombatInfoManager& combatInfo() const;
    ConditionFactoryManager& conditionFactories() const;
    Properties& config() const;
    ConnectionInfoManager& connectionInfos() const;
    CoupleManager& couples() const;
    DarkLightInfoManager& darkLights() const;
    DatabaseManager& databases() const;
    DefaultOptionSetInfoManager& optionSets() const;
    DirectiveSetManager& directiveSets() const;
    DragonEyeManager& dragonEyes() const;
    DynamicZoneFactoryManager& dynamicZoneFactories() const;
    DynamicZoneInfoManager& dynamicZoneInfos() const;
    DynamicZoneManager& dynamicZones() const;
    EffectLoaderManager& effectLoaders() const;
    EventQuestLootingManager& eventQuestLoot() const;
    FlagManager& flags() const;
    GameServerGroupInfoManager& gameServerGroups() const;
    GlobalPartyManager& parties() const;
    GoodsInfoManager& goodsInfos() const;
    IncomingPlayerManager& incomingPlayers() const;
    ItemFactoryManager& itemFactories() const;
    ItemInfoManager& itemInfos() const;
    ItemLoaderManager& itemLoaders() const;
    ItemMineInfoManager& itemMineInfos() const;
    MasterLairInfoManager& masterLairInfos() const;
    MonsterInfoManager& monsterInfos() const;
    MonsterNameManager& monsterNames() const;
    OptionInfoManager& optionInfos() const;
    OustersEXPInfoManager& oustersExp() const;
    PCFinder& playerCreatures() const;
    PKZoneInfoManager& pkZoneInfos() const;
    PriceManager& prices() const;
    ScriptManager& publicScripts() const;
    RankBonusInfoManager& rankBonuses() const;
    ShopTemplateManager& shopTemplates() const;
    SkillDomainInfoManager& skillDomains() const;
    SkillInfoManager& skillInfos() const;
    SkillPropertyManager& skillProps() const;
    StringPool& strings() const;
    TimeChecker& timeChecker() const;
    TimeManager& worldTime() const;
    VampEXPInfoManager& vampireExp() const;
    VariableManager& variables() const;
    VolumeInfoManager& volumeInfos() const;
    WarSystem& warSystem() const;
    WayPointManager& wayPoints() const;
    WeatherInfoManager& weatherInfos() const;
    ZoneGroupManager& zoneGroups() const;
    ZoneInfoManager& zoneInfos() const;

private:
    ActionFactoryManager* m_pActionFactoryManager = nullptr;
    AlignmentManager* m_pAlignmentManager = nullptr;
    BloodBibleBonusManager* m_pBloodBibleBonusManager = nullptr;
    CastleInfoManager* m_pCastleInfoManager = nullptr;
    CastleShrineInfoManager* m_pCastleShrineInfoManager = nullptr;
    CastleSkillInfoManager* m_pCastleSkillInfoManager = nullptr;
    ClientManager* m_pClientManager = nullptr;
    CombatInfoManager* m_pCombatInfoManager = nullptr;
    ConditionFactoryManager* m_pConditionFactoryManager = nullptr;
    Properties* m_pConfig = nullptr;
    ConnectionInfoManager* m_pConnectionInfoManager = nullptr;
    CoupleManager* m_pCoupleManager = nullptr;
    DarkLightInfoManager* m_pDarkLightInfoManager = nullptr;
    DatabaseManager* m_pDatabaseManager = nullptr;
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
    IncomingPlayerManager* m_pIncomingPlayerManager = nullptr;
    ItemFactoryManager* m_pItemFactoryManager = nullptr;
    ItemInfoManager* m_pItemInfoManager = nullptr;
    ItemLoaderManager* m_pItemLoaderManager = nullptr;
    ItemMineInfoManager* m_pItemMineInfoManager = nullptr;
    MasterLairInfoManager* m_pMasterLairInfoManager = nullptr;
    MonsterInfoManager* m_pMonsterInfoManager = nullptr;
    MonsterNameManager* m_pMonsterNameManager = nullptr;
    OptionInfoManager* m_pOptionInfoManager = nullptr;
    OustersEXPInfoManager* m_pOustersEXPInfoManager = nullptr;
    PCFinder* m_pPCFinder = nullptr;
    PKZoneInfoManager* m_pPKZoneInfoManager = nullptr;
    PriceManager* m_pPriceManager = nullptr;
    ScriptManager* m_pPublicScriptManager = nullptr;
    RankBonusInfoManager* m_pRankBonusInfoManager = nullptr;
    ShopTemplateManager* m_pShopTemplateManager = nullptr;
    SkillDomainInfoManager* m_pSkillDomainInfoManager = nullptr;
    SkillInfoManager* m_pSkillInfoManager = nullptr;
    SkillPropertyManager* m_pSkillPropertyManager = nullptr;
    StringPool* m_pStringPool = nullptr;
    TimeChecker* m_pTimeChecker = nullptr;
    TimeManager* m_pTimeManager = nullptr;
    VampEXPInfoManager* m_pVampEXPInfoManager = nullptr;
    VariableManager* m_pVariableManager = nullptr;
    VolumeInfoManager* m_pVolumeInfoManager = nullptr;
    WarSystem* m_pWarSystem = nullptr;
    WayPointManager* m_pWayPointManager = nullptr;
    WeatherInfoManager* m_pWeatherInfoManager = nullptr;
    ZoneGroupManager* m_pZoneGroupManager = nullptr;
    ZoneInfoManager* m_pZoneInfoManager = nullptr;
};

// The process-wide context the startup code fills. A converted subsystem is
// handed the context and never calls this; the call belongs at the boundary
// where a subsystem is created from code that still reads globals.
GameContext& gameContext();

} // namespace de

#endif
