//////////////////////////////////////////////////////////////////////////////
// Filename    : GameContext.cpp
// Description : Accessors for the managers registered on a GameContext.
//
//               The manager types stay incomplete here: an accessor only
//               binds a reference to the registered object, so nothing in
//               this file needs a manager's definition.
//////////////////////////////////////////////////////////////////////////////

#include "GameContext.h"

#include "Assert.h"

namespace de {

ActionFactoryManager& GameContext::actionFactories() const {
    Assert(m_pActionFactoryManager != nullptr);
    return *m_pActionFactoryManager;
}

AlignmentManager& GameContext::alignments() const {
    Assert(m_pAlignmentManager != nullptr);
    return *m_pAlignmentManager;
}

BloodBibleBonusManager& GameContext::bloodBibleBonuses() const {
    Assert(m_pBloodBibleBonusManager != nullptr);
    return *m_pBloodBibleBonusManager;
}

CastleInfoManager& GameContext::castleInfos() const {
    Assert(m_pCastleInfoManager != nullptr);
    return *m_pCastleInfoManager;
}

CastleShrineInfoManager& GameContext::castleShrines() const {
    Assert(m_pCastleShrineInfoManager != nullptr);
    return *m_pCastleShrineInfoManager;
}

CastleSkillInfoManager& GameContext::castleSkills() const {
    Assert(m_pCastleSkillInfoManager != nullptr);
    return *m_pCastleSkillInfoManager;
}

ClientManager& GameContext::clients() const {
    Assert(m_pClientManager != nullptr);
    return *m_pClientManager;
}

CombatInfoManager& GameContext::combatInfo() const {
    Assert(m_pCombatInfoManager != nullptr);
    return *m_pCombatInfoManager;
}

ConditionFactoryManager& GameContext::conditionFactories() const {
    Assert(m_pConditionFactoryManager != nullptr);
    return *m_pConditionFactoryManager;
}

Properties& GameContext::config() const {
    Assert(m_pConfig != nullptr);
    return *m_pConfig;
}

ConnectionInfoManager& GameContext::connectionInfos() const {
    Assert(m_pConnectionInfoManager != nullptr);
    return *m_pConnectionInfoManager;
}

CoupleManager& GameContext::couples() const {
    Assert(m_pCoupleManager != nullptr);
    return *m_pCoupleManager;
}

DarkLightInfoManager& GameContext::darkLights() const {
    Assert(m_pDarkLightInfoManager != nullptr);
    return *m_pDarkLightInfoManager;
}

DatabaseManager& GameContext::databases() const {
    Assert(m_pDatabaseManager != nullptr);
    return *m_pDatabaseManager;
}

DefaultOptionSetInfoManager& GameContext::optionSets() const {
    Assert(m_pDefaultOptionSetInfoManager != nullptr);
    return *m_pDefaultOptionSetInfoManager;
}

DirectiveSetManager& GameContext::directiveSets() const {
    Assert(m_pDirectiveSetManager != nullptr);
    return *m_pDirectiveSetManager;
}

DragonEyeManager& GameContext::dragonEyes() const {
    Assert(m_pDragonEyeManager != nullptr);
    return *m_pDragonEyeManager;
}

DynamicZoneFactoryManager& GameContext::dynamicZoneFactories() const {
    Assert(m_pDynamicZoneFactoryManager != nullptr);
    return *m_pDynamicZoneFactoryManager;
}

DynamicZoneInfoManager& GameContext::dynamicZoneInfos() const {
    Assert(m_pDynamicZoneInfoManager != nullptr);
    return *m_pDynamicZoneInfoManager;
}

DynamicZoneManager& GameContext::dynamicZones() const {
    Assert(m_pDynamicZoneManager != nullptr);
    return *m_pDynamicZoneManager;
}

EffectLoaderManager& GameContext::effectLoaders() const {
    Assert(m_pEffectLoaderManager != nullptr);
    return *m_pEffectLoaderManager;
}

EventQuestLootingManager& GameContext::eventQuestLoot() const {
    Assert(m_pEventQuestLootingManager != nullptr);
    return *m_pEventQuestLootingManager;
}

FlagManager& GameContext::flags() const {
    Assert(m_pFlagManager != nullptr);
    return *m_pFlagManager;
}

GameServerGroupInfoManager& GameContext::gameServerGroups() const {
    Assert(m_pGameServerGroupInfoManager != nullptr);
    return *m_pGameServerGroupInfoManager;
}

GlobalPartyManager& GameContext::parties() const {
    Assert(m_pGlobalPartyManager != nullptr);
    return *m_pGlobalPartyManager;
}

GoodsInfoManager& GameContext::goodsInfos() const {
    Assert(m_pGoodsInfoManager != nullptr);
    return *m_pGoodsInfoManager;
}

GuildManager& GameContext::guilds() const {
    Assert(m_pGuildManager != nullptr);
    return *m_pGuildManager;
}

HolyLandManager& GameContext::holyLands() const {
    Assert(m_pHolyLandManager != nullptr);
    return *m_pHolyLandManager;
}

IncomingPlayerManager& GameContext::incomingPlayers() const {
    Assert(m_pIncomingPlayerManager != nullptr);
    return *m_pIncomingPlayerManager;
}

ItemFactoryManager& GameContext::itemFactories() const {
    Assert(m_pItemFactoryManager != nullptr);
    return *m_pItemFactoryManager;
}

ItemInfoManager& GameContext::itemInfos() const {
    Assert(m_pItemInfoManager != nullptr);
    return *m_pItemInfoManager;
}

ItemLoaderManager& GameContext::itemLoaders() const {
    Assert(m_pItemLoaderManager != nullptr);
    return *m_pItemLoaderManager;
}

ItemMineInfoManager& GameContext::itemMineInfos() const {
    Assert(m_pItemMineInfoManager != nullptr);
    return *m_pItemMineInfoManager;
}

LevelWarZoneInfoManager& GameContext::levelWarZones() const {
    Assert(m_pLevelWarZoneInfoManager != nullptr);
    return *m_pLevelWarZoneInfoManager;
}

LoginServerManager& GameContext::loginServer() const {
    Assert(m_pLoginServerManager != nullptr);
    return *m_pLoginServerManager;
}

MasterLairInfoManager& GameContext::masterLairInfos() const {
    Assert(m_pMasterLairInfoManager != nullptr);
    return *m_pMasterLairInfoManager;
}

MonsterInfoManager& GameContext::monsterInfos() const {
    Assert(m_pMonsterInfoManager != nullptr);
    return *m_pMonsterInfoManager;
}

MonsterNameManager& GameContext::monsterNames() const {
    Assert(m_pMonsterNameManager != nullptr);
    return *m_pMonsterNameManager;
}

OptionInfoManager& GameContext::optionInfos() const {
    Assert(m_pOptionInfoManager != nullptr);
    return *m_pOptionInfoManager;
}

OustersEXPInfoManager& GameContext::oustersExp() const {
    Assert(m_pOustersEXPInfoManager != nullptr);
    return *m_pOustersEXPInfoManager;
}

PCFinder& GameContext::playerCreatures() const {
    Assert(m_pPCFinder != nullptr);
    return *m_pPCFinder;
}

PKZoneInfoManager& GameContext::pkZoneInfos() const {
    Assert(m_pPKZoneInfoManager != nullptr);
    return *m_pPKZoneInfoManager;
}

ParkingCenter& GameContext::parking() const {
    Assert(m_pParkingCenter != nullptr);
    return *m_pParkingCenter;
}

PriceManager& GameContext::prices() const {
    Assert(m_pPriceManager != nullptr);
    return *m_pPriceManager;
}

ScriptManager& GameContext::publicScripts() const {
    Assert(m_pPublicScriptManager != nullptr);
    return *m_pPublicScriptManager;
}

RankBonusInfoManager& GameContext::rankBonuses() const {
    Assert(m_pRankBonusInfoManager != nullptr);
    return *m_pRankBonusInfoManager;
}

ResurrectLocationManager& GameContext::resurrectLocations() const {
    Assert(m_pResurrectLocationManager != nullptr);
    return *m_pResurrectLocationManager;
}

SharedServerManager& GameContext::sharedServer() const {
    Assert(m_pSharedServerManager != nullptr);
    return *m_pSharedServerManager;
}

ShopTemplateManager& GameContext::shopTemplates() const {
    Assert(m_pShopTemplateManager != nullptr);
    return *m_pShopTemplateManager;
}

ShrineInfoManager& GameContext::shrines() const {
    Assert(m_pShrineInfoManager != nullptr);
    return *m_pShrineInfoManager;
}

SkillDomainInfoManager& GameContext::skillDomains() const {
    Assert(m_pSkillDomainInfoManager != nullptr);
    return *m_pSkillDomainInfoManager;
}

SkillHandlerManager& GameContext::skillHandlers() const {
    Assert(m_pSkillHandlerManager != nullptr);
    return *m_pSkillHandlerManager;
}

SkillInfoManager& GameContext::skillInfos() const {
    Assert(m_pSkillInfoManager != nullptr);
    return *m_pSkillInfoManager;
}

SkillPropertyManager& GameContext::skillProps() const {
    Assert(m_pSkillPropertyManager != nullptr);
    return *m_pSkillPropertyManager;
}

StringPool& GameContext::strings() const {
    Assert(m_pStringPool != nullptr);
    return *m_pStringPool;
}

SweeperBonusManager& GameContext::sweeperBonuses() const {
    Assert(m_pSweeperBonusManager != nullptr);
    return *m_pSweeperBonusManager;
}

TimeChecker& GameContext::timeChecker() const {
    Assert(m_pTimeChecker != nullptr);
    return *m_pTimeChecker;
}

TimeManager& GameContext::worldTime() const {
    Assert(m_pTimeManager != nullptr);
    return *m_pTimeManager;
}

VampEXPInfoManager& GameContext::vampireExp() const {
    Assert(m_pVampEXPInfoManager != nullptr);
    return *m_pVampEXPInfoManager;
}

VariableManager& GameContext::variables() const {
    Assert(m_pVariableManager != nullptr);
    return *m_pVariableManager;
}

VolumeInfoManager& GameContext::volumeInfos() const {
    Assert(m_pVolumeInfoManager != nullptr);
    return *m_pVolumeInfoManager;
}

WarSystem& GameContext::warSystem() const {
    Assert(m_pWarSystem != nullptr);
    return *m_pWarSystem;
}

WayPointManager& GameContext::wayPoints() const {
    Assert(m_pWayPointManager != nullptr);
    return *m_pWayPointManager;
}

WeatherInfoManager& GameContext::weatherInfos() const {
    Assert(m_pWeatherInfoManager != nullptr);
    return *m_pWeatherInfoManager;
}

ZoneGroupManager& GameContext::zoneGroups() const {
    Assert(m_pZoneGroupManager != nullptr);
    return *m_pZoneGroupManager;
}

ZoneInfoManager& GameContext::zoneInfos() const {
    Assert(m_pZoneInfoManager != nullptr);
    return *m_pZoneInfoManager;
}

GameContext& gameContext() {
    static GameContext context;
    return context;
}

} // namespace de
