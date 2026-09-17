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

EventQuestLootingManager& GameContext::eventQuestLoot() const {
    Assert(m_pEventQuestLootingManager != nullptr);
    return *m_pEventQuestLootingManager;
}

GameServerGroupInfoManager& GameContext::gameServerGroups() const {
    Assert(m_pGameServerGroupInfoManager != nullptr);
    return *m_pGameServerGroupInfoManager;
}

GoodsInfoManager& GameContext::goodsInfos() const {
    Assert(m_pGoodsInfoManager != nullptr);
    return *m_pGoodsInfoManager;
}

ItemFactoryManager& GameContext::itemFactories() const {
    Assert(m_pItemFactoryManager != nullptr);
    return *m_pItemFactoryManager;
}

ItemLoaderManager& GameContext::itemLoaders() const {
    Assert(m_pItemLoaderManager != nullptr);
    return *m_pItemLoaderManager;
}

MonsterNameManager& GameContext::monsterNames() const {
    Assert(m_pMonsterNameManager != nullptr);
    return *m_pMonsterNameManager;
}

OustersEXPInfoManager& GameContext::oustersExp() const {
    Assert(m_pOustersEXPInfoManager != nullptr);
    return *m_pOustersEXPInfoManager;
}

PCFinder& GameContext::playerCreatures() const {
    Assert(m_pPCFinder != nullptr);
    return *m_pPCFinder;
}

ScriptManager& GameContext::publicScripts() const {
    Assert(m_pPublicScriptManager != nullptr);
    return *m_pPublicScriptManager;
}

RankBonusInfoManager& GameContext::rankBonuses() const {
    Assert(m_pRankBonusInfoManager != nullptr);
    return *m_pRankBonusInfoManager;
}

ShopTemplateManager& GameContext::shopTemplates() const {
    Assert(m_pShopTemplateManager != nullptr);
    return *m_pShopTemplateManager;
}

SkillDomainInfoManager& GameContext::skillDomains() const {
    Assert(m_pSkillDomainInfoManager != nullptr);
    return *m_pSkillDomainInfoManager;
}

SkillPropertyManager& GameContext::skillProps() const {
    Assert(m_pSkillPropertyManager != nullptr);
    return *m_pSkillPropertyManager;
}

StringPool& GameContext::strings() const {
    Assert(m_pStringPool != nullptr);
    return *m_pStringPool;
}

TimeChecker& GameContext::timeChecker() const {
    Assert(m_pTimeChecker != nullptr);
    return *m_pTimeChecker;
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
