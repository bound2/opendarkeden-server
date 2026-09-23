//----------------------------------------------------------------------
// game_context_test.cpp
//
// Pins src/server/gameserver/GameContext.h: the registry a converted
// subsystem is handed instead of reading the g_p* globals.
//
// The managers are never defined here. GameContext.h forward-declares
// them and its accessors only bind a reference to the registered object,
// so a context can be built over stand-in pointers with none of the
// gameserver linked -- which is what makes a subsystem that takes a
// GameContext& testable at all. The suite therefore links only de-kernel
// (for the Assert helper) and the context's own translation unit.
//
// A failing Assert appends to assertion_failed.log in the working
// directory, so the ctest entry runs this from the build tree.
//
// The per-item-class tables get no test here. ItemLoaderManager's loaders
// and ItemInfoManager's info managers are eighty-seven concrete classes
// each, so filling either table means dragging in the whole gameserver and
// reading one means talking to the database. Neither table can be an
// accessor of its own either: keying one needs Item::ItemClass, a nested
// enum this header cannot forward-declare. The R1 ratchet is what pins
// that the globals those tables replaced stay gone.
//----------------------------------------------------------------------

#include <gtest/gtest.h>

#include "Exception.h"
#include "GameContext.h"

namespace {

// Distinct addresses standing in for the managers. Nothing dereferences
// them: the context stores a pointer and hands back a reference to it.
char g_managerStorage[60];

template <class T> T* standIn(int slot) {
    return reinterpret_cast<T*>(&g_managerStorage[slot]);
}

} // namespace

TEST(GameContextTest, AccessorReturnsTheRegisteredManager) {
    de::GameContext context;

    Properties* pConfig = standIn<Properties>(0);
    DatabaseManager* pDatabaseManager = standIn<DatabaseManager>(1);
    ItemFactoryManager* pItemFactoryManager = standIn<ItemFactoryManager>(2);
    PCFinder* pPCFinder = standIn<PCFinder>(3);
    StringPool* pStringPool = standIn<StringPool>(4);
    VariableManager* pVariableManager = standIn<VariableManager>(5);
    ZoneGroupManager* pZoneGroupManager = standIn<ZoneGroupManager>(6);
    ZoneInfoManager* pZoneInfoManager = standIn<ZoneInfoManager>(7);

    context.setConfig(pConfig);
    context.setDatabaseManager(pDatabaseManager);
    context.setItemFactoryManager(pItemFactoryManager);
    context.setPCFinder(pPCFinder);
    context.setStringPool(pStringPool);
    context.setVariableManager(pVariableManager);
    context.setZoneGroupManager(pZoneGroupManager);
    context.setZoneInfoManager(pZoneInfoManager);

    EXPECT_EQ(&context.config(), pConfig);
    EXPECT_EQ(&context.databases(), pDatabaseManager);
    EXPECT_EQ(&context.itemFactories(), pItemFactoryManager);
    EXPECT_EQ(&context.playerCreatures(), pPCFinder);
    EXPECT_EQ(&context.strings(), pStringPool);
    EXPECT_EQ(&context.variables(), pVariableManager);
    EXPECT_EQ(&context.zoneGroups(), pZoneGroupManager);
    EXPECT_EQ(&context.zoneInfos(), pZoneInfoManager);
}

TEST(GameContextTest, QuestScriptingManagersAreReadBack) {
    de::GameContext context;

    ActionFactoryManager* pActionFactoryManager = standIn<ActionFactoryManager>(8);
    ConditionFactoryManager* pConditionFactoryManager = standIn<ConditionFactoryManager>(9);
    ScriptManager* pPublicScriptManager = standIn<ScriptManager>(10);
    ShopTemplateManager* pShopTemplateManager = standIn<ShopTemplateManager>(11);

    context.setActionFactoryManager(pActionFactoryManager);
    context.setConditionFactoryManager(pConditionFactoryManager);
    context.setPublicScriptManager(pPublicScriptManager);
    context.setShopTemplateManager(pShopTemplateManager);

    EXPECT_EQ(&context.actionFactories(), pActionFactoryManager);
    EXPECT_EQ(&context.conditionFactories(), pConditionFactoryManager);
    EXPECT_EQ(&context.publicScripts(), pPublicScriptManager);
    EXPECT_EQ(&context.shopTemplates(), pShopTemplateManager);
}

TEST(GameContextTest, WorldTableManagersAreReadBack) {
    de::GameContext context;

    DynamicZoneFactoryManager* pDynamicZoneFactoryManager = standIn<DynamicZoneFactoryManager>(12);
    MonsterInfoManager* pMonsterInfoManager = standIn<MonsterInfoManager>(53);
    MonsterNameManager* pMonsterNameManager = standIn<MonsterNameManager>(13);
    WeatherInfoManager* pWeatherInfoManager = standIn<WeatherInfoManager>(14);

    context.setDynamicZoneFactoryManager(pDynamicZoneFactoryManager);
    context.setMonsterInfoManager(pMonsterInfoManager);
    context.setMonsterNameManager(pMonsterNameManager);
    context.setWeatherInfoManager(pWeatherInfoManager);

    EXPECT_EQ(&context.dynamicZoneFactories(), pDynamicZoneFactoryManager);
    EXPECT_EQ(&context.monsterInfos(), pMonsterInfoManager);
    EXPECT_EQ(&context.monsterNames(), pMonsterNameManager);
    EXPECT_EQ(&context.weatherInfos(), pWeatherInfoManager);
}

TEST(GameContextTest, ItemDescriptionManagersAreReadBack) {
    de::GameContext context;

    DefaultOptionSetInfoManager* pDefaultOptionSetInfoManager = standIn<DefaultOptionSetInfoManager>(15);
    ItemInfoManager* pItemInfoManager = standIn<ItemInfoManager>(48);
    OptionInfoManager* pOptionInfoManager = standIn<OptionInfoManager>(54);
    VolumeInfoManager* pVolumeInfoManager = standIn<VolumeInfoManager>(16);

    context.setDefaultOptionSetInfoManager(pDefaultOptionSetInfoManager);
    context.setItemInfoManager(pItemInfoManager);
    context.setOptionInfoManager(pOptionInfoManager);
    context.setVolumeInfoManager(pVolumeInfoManager);

    EXPECT_EQ(&context.optionSets(), pDefaultOptionSetInfoManager);
    EXPECT_EQ(&context.itemInfos(), pItemInfoManager);
    EXPECT_EQ(&context.optionInfos(), pOptionInfoManager);
    EXPECT_EQ(&context.volumeInfos(), pVolumeInfoManager);
}

TEST(GameContextTest, WarAndTravelManagersAreReadBack) {
    de::GameContext context;

    CastleInfoManager* pCastleInfoManager = standIn<CastleInfoManager>(51);
    CastleShrineInfoManager* pCastleShrineInfoManager = standIn<CastleShrineInfoManager>(17);
    DragonEyeManager* pDragonEyeManager = standIn<DragonEyeManager>(18);
    EventQuestLootingManager* pEventQuestLootingManager = standIn<EventQuestLootingManager>(19);
    FlagManager* pFlagManager = standIn<FlagManager>(49);
    ParkingCenter* pParkingCenter = standIn<ParkingCenter>(59);
    WarSystem* pWarSystem = standIn<WarSystem>(52);
    WayPointManager* pWayPointManager = standIn<WayPointManager>(20);

    context.setCastleInfoManager(pCastleInfoManager);
    context.setCastleShrineInfoManager(pCastleShrineInfoManager);
    context.setDragonEyeManager(pDragonEyeManager);
    context.setEventQuestLootingManager(pEventQuestLootingManager);
    context.setFlagManager(pFlagManager);
    context.setParkingCenter(pParkingCenter);
    context.setWarSystem(pWarSystem);
    context.setWayPointManager(pWayPointManager);

    EXPECT_EQ(&context.castleInfos(), pCastleInfoManager);
    EXPECT_EQ(&context.castleShrines(), pCastleShrineInfoManager);
    EXPECT_EQ(&context.dragonEyes(), pDragonEyeManager);
    EXPECT_EQ(&context.eventQuestLoot(), pEventQuestLootingManager);
    EXPECT_EQ(&context.flags(), pFlagManager);
    EXPECT_EQ(&context.parking(), pParkingCenter);
    EXPECT_EQ(&context.warSystem(), pWarSystem);
    EXPECT_EQ(&context.wayPoints(), pWayPointManager);
}

TEST(GameContextTest, ZoneAmbienceManagersAreReadBack) {
    de::GameContext context;

    DarkLightInfoManager* pDarkLightInfoManager = standIn<DarkLightInfoManager>(21);
    DirectiveSetManager* pDirectiveSetManager = standIn<DirectiveSetManager>(22);
    DynamicZoneInfoManager* pDynamicZoneInfoManager = standIn<DynamicZoneInfoManager>(23);
    PKZoneInfoManager* pPKZoneInfoManager = standIn<PKZoneInfoManager>(55);

    context.setDarkLightInfoManager(pDarkLightInfoManager);
    context.setDirectiveSetManager(pDirectiveSetManager);
    context.setDynamicZoneInfoManager(pDynamicZoneInfoManager);
    context.setPKZoneInfoManager(pPKZoneInfoManager);

    EXPECT_EQ(&context.darkLights(), pDarkLightInfoManager);
    EXPECT_EQ(&context.directiveSets(), pDirectiveSetManager);
    EXPECT_EQ(&context.dynamicZoneInfos(), pDynamicZoneInfoManager);
    EXPECT_EQ(&context.pkZoneInfos(), pPKZoneInfoManager);
}

TEST(GameContextTest, ProgressionTableManagersAreReadBack) {
    de::GameContext context;

    GoodsInfoManager* pGoodsInfoManager = standIn<GoodsInfoManager>(24);
    OustersEXPInfoManager* pOustersEXPInfoManager = standIn<OustersEXPInfoManager>(25);
    RankBonusInfoManager* pRankBonusInfoManager = standIn<RankBonusInfoManager>(26);
    SkillDomainInfoManager* pSkillDomainInfoManager = standIn<SkillDomainInfoManager>(27);
    SkillInfoManager* pSkillInfoManager = standIn<SkillInfoManager>(50);
    SkillPropertyManager* pSkillPropertyManager = standIn<SkillPropertyManager>(28);
    VampEXPInfoManager* pVampEXPInfoManager = standIn<VampEXPInfoManager>(29);

    context.setGoodsInfoManager(pGoodsInfoManager);
    context.setOustersEXPInfoManager(pOustersEXPInfoManager);
    context.setRankBonusInfoManager(pRankBonusInfoManager);
    context.setSkillDomainInfoManager(pSkillDomainInfoManager);
    context.setSkillInfoManager(pSkillInfoManager);
    context.setSkillPropertyManager(pSkillPropertyManager);
    context.setVampEXPInfoManager(pVampEXPInfoManager);

    EXPECT_EQ(&context.goodsInfos(), pGoodsInfoManager);
    EXPECT_EQ(&context.oustersExp(), pOustersEXPInfoManager);
    EXPECT_EQ(&context.rankBonuses(), pRankBonusInfoManager);
    EXPECT_EQ(&context.skillDomains(), pSkillDomainInfoManager);
    EXPECT_EQ(&context.skillInfos(), pSkillInfoManager);
    EXPECT_EQ(&context.skillProps(), pSkillPropertyManager);
    EXPECT_EQ(&context.vampireExp(), pVampEXPInfoManager);
}

TEST(GameContextTest, ClientSessionManagersAreReadBack) {
    de::GameContext context;

    ClientManager* pClientManager = standIn<ClientManager>(30);
    ConnectionInfoManager* pConnectionInfoManager = standIn<ConnectionInfoManager>(31);
    GameServerGroupInfoManager* pGameServerGroupInfoManager = standIn<GameServerGroupInfoManager>(32);

    context.setClientManager(pClientManager);
    context.setConnectionInfoManager(pConnectionInfoManager);
    context.setGameServerGroupInfoManager(pGameServerGroupInfoManager);

    EXPECT_EQ(&context.clients(), pClientManager);
    EXPECT_EQ(&context.connectionInfos(), pConnectionInfoManager);
    EXPECT_EQ(&context.gameServerGroups(), pGameServerGroupInfoManager);
}

TEST(GameContextTest, InterServerLinkManagersAreReadBack) {
    de::GameContext context;

    SharedServerManager* pSharedServerManager = standIn<SharedServerManager>(58);

    context.setSharedServerManager(pSharedServerManager);

    EXPECT_EQ(&context.sharedServer(), pSharedServerManager);
}

TEST(GameContextTest, CharacterLoadingManagersAreReadBack) {
    de::GameContext context;

    CastleSkillInfoManager* pCastleSkillInfoManager = standIn<CastleSkillInfoManager>(33);
    ItemLoaderManager* pItemLoaderManager = standIn<ItemLoaderManager>(34);
    TimeChecker* pTimeChecker = standIn<TimeChecker>(35);

    context.setCastleSkillInfoManager(pCastleSkillInfoManager);
    context.setItemLoaderManager(pItemLoaderManager);
    context.setTimeChecker(pTimeChecker);

    EXPECT_EQ(&context.castleSkills(), pCastleSkillInfoManager);
    EXPECT_EQ(&context.itemLoaders(), pItemLoaderManager);
    EXPECT_EQ(&context.timeChecker(), pTimeChecker);
}

TEST(GameContextTest, CombatStateManagersAreReadBack) {
    de::GameContext context;

    AlignmentManager* pAlignmentManager = standIn<AlignmentManager>(36);
    BloodBibleBonusManager* pBloodBibleBonusManager = standIn<BloodBibleBonusManager>(37);
    CombatInfoManager* pCombatInfoManager = standIn<CombatInfoManager>(38);
    SkillHandlerManager* pSkillHandlerManager = standIn<SkillHandlerManager>(57);

    context.setAlignmentManager(pAlignmentManager);
    context.setBloodBibleBonusManager(pBloodBibleBonusManager);
    context.setCombatInfoManager(pCombatInfoManager);
    context.setSkillHandlerManager(pSkillHandlerManager);

    EXPECT_EQ(&context.alignments(), pAlignmentManager);
    EXPECT_EQ(&context.bloodBibleBonuses(), pBloodBibleBonusManager);
    EXPECT_EQ(&context.combatInfo(), pCombatInfoManager);
    EXPECT_EQ(&context.skillHandlers(), pSkillHandlerManager);
}

TEST(GameContextTest, WorldClockAndInstanceManagersAreReadBack) {
    de::GameContext context;

    DynamicZoneManager* pDynamicZoneManager = standIn<DynamicZoneManager>(39);
    EffectLoaderManager* pEffectLoaderManager = standIn<EffectLoaderManager>(40);
    MasterLairInfoManager* pMasterLairInfoManager = standIn<MasterLairInfoManager>(41);
    TimeManager* pTimeManager = standIn<TimeManager>(42);

    context.setDynamicZoneManager(pDynamicZoneManager);
    context.setEffectLoaderManager(pEffectLoaderManager);
    context.setMasterLairInfoManager(pMasterLairInfoManager);
    context.setTimeManager(pTimeManager);

    EXPECT_EQ(&context.dynamicZones(), pDynamicZoneManager);
    EXPECT_EQ(&context.effectLoaders(), pEffectLoaderManager);
    EXPECT_EQ(&context.masterLairInfos(), pMasterLairInfoManager);
    EXPECT_EQ(&context.worldTime(), pTimeManager);
}

TEST(GameContextTest, PlayerGroupingManagersAreReadBack) {
    de::GameContext context;

    CoupleManager* pCoupleManager = standIn<CoupleManager>(43);
    GlobalPartyManager* pGlobalPartyManager = standIn<GlobalPartyManager>(44);
    GuildManager* pGuildManager = standIn<GuildManager>(56);
    IncomingPlayerManager* pIncomingPlayerManager = standIn<IncomingPlayerManager>(45);

    context.setCoupleManager(pCoupleManager);
    context.setGlobalPartyManager(pGlobalPartyManager);
    context.setGuildManager(pGuildManager);
    context.setIncomingPlayerManager(pIncomingPlayerManager);

    EXPECT_EQ(&context.couples(), pCoupleManager);
    EXPECT_EQ(&context.parties(), pGlobalPartyManager);
    EXPECT_EQ(&context.guilds(), pGuildManager);
    EXPECT_EQ(&context.incomingPlayers(), pIncomingPlayerManager);
}

TEST(GameContextTest, TradeManagersAreReadBack) {
    de::GameContext context;

    ItemMineInfoManager* pItemMineInfoManager = standIn<ItemMineInfoManager>(46);
    PriceManager* pPriceManager = standIn<PriceManager>(47);

    context.setItemMineInfoManager(pItemMineInfoManager);
    context.setPriceManager(pPriceManager);

    EXPECT_EQ(&context.itemMineInfos(), pItemMineInfoManager);
    EXPECT_EQ(&context.prices(), pPriceManager);
}

TEST(GameContextTest, ReregisteringReplacesTheManager) {
    de::GameContext context;

    context.setConfig(standIn<Properties>(0));
    context.setConfig(standIn<Properties>(1));

    EXPECT_EQ(&context.config(), standIn<Properties>(1));
}

TEST(GameContextTest, UnregisteredManagerAsserts) {
    de::GameContext context;

    // Reading a manager the startup code has not registered yet is a
    // startup-order bug, so every accessor asserts rather than returning
    // something the caller could test.
    EXPECT_THROW(context.actionFactories(), AssertionError);
    EXPECT_THROW(context.alignments(), AssertionError);
    EXPECT_THROW(context.bloodBibleBonuses(), AssertionError);
    EXPECT_THROW(context.castleInfos(), AssertionError);
    EXPECT_THROW(context.castleShrines(), AssertionError);
    EXPECT_THROW(context.castleSkills(), AssertionError);
    EXPECT_THROW(context.clients(), AssertionError);
    EXPECT_THROW(context.combatInfo(), AssertionError);
    EXPECT_THROW(context.conditionFactories(), AssertionError);
    EXPECT_THROW(context.config(), AssertionError);
    EXPECT_THROW(context.connectionInfos(), AssertionError);
    EXPECT_THROW(context.couples(), AssertionError);
    EXPECT_THROW(context.darkLights(), AssertionError);
    EXPECT_THROW(context.databases(), AssertionError);
    EXPECT_THROW(context.directiveSets(), AssertionError);
    EXPECT_THROW(context.dragonEyes(), AssertionError);
    EXPECT_THROW(context.dynamicZoneFactories(), AssertionError);
    EXPECT_THROW(context.dynamicZoneInfos(), AssertionError);
    EXPECT_THROW(context.dynamicZones(), AssertionError);
    EXPECT_THROW(context.effectLoaders(), AssertionError);
    EXPECT_THROW(context.eventQuestLoot(), AssertionError);
    EXPECT_THROW(context.flags(), AssertionError);
    EXPECT_THROW(context.gameServerGroups(), AssertionError);
    EXPECT_THROW(context.goodsInfos(), AssertionError);
    EXPECT_THROW(context.guilds(), AssertionError);
    EXPECT_THROW(context.incomingPlayers(), AssertionError);
    EXPECT_THROW(context.itemFactories(), AssertionError);
    EXPECT_THROW(context.itemInfos(), AssertionError);
    EXPECT_THROW(context.itemLoaders(), AssertionError);
    EXPECT_THROW(context.itemMineInfos(), AssertionError);
    EXPECT_THROW(context.masterLairInfos(), AssertionError);
    EXPECT_THROW(context.monsterInfos(), AssertionError);
    EXPECT_THROW(context.monsterNames(), AssertionError);
    EXPECT_THROW(context.optionInfos(), AssertionError);
    EXPECT_THROW(context.optionSets(), AssertionError);
    EXPECT_THROW(context.oustersExp(), AssertionError);
    EXPECT_THROW(context.parking(), AssertionError);
    EXPECT_THROW(context.parties(), AssertionError);
    EXPECT_THROW(context.pkZoneInfos(), AssertionError);
    EXPECT_THROW(context.playerCreatures(), AssertionError);
    EXPECT_THROW(context.prices(), AssertionError);
    EXPECT_THROW(context.publicScripts(), AssertionError);
    EXPECT_THROW(context.rankBonuses(), AssertionError);
    EXPECT_THROW(context.sharedServer(), AssertionError);
    EXPECT_THROW(context.shopTemplates(), AssertionError);
    EXPECT_THROW(context.skillDomains(), AssertionError);
    EXPECT_THROW(context.skillHandlers(), AssertionError);
    EXPECT_THROW(context.skillInfos(), AssertionError);
    EXPECT_THROW(context.skillProps(), AssertionError);
    EXPECT_THROW(context.strings(), AssertionError);
    EXPECT_THROW(context.timeChecker(), AssertionError);
    EXPECT_THROW(context.vampireExp(), AssertionError);
    EXPECT_THROW(context.variables(), AssertionError);
    EXPECT_THROW(context.volumeInfos(), AssertionError);
    EXPECT_THROW(context.warSystem(), AssertionError);
    EXPECT_THROW(context.wayPoints(), AssertionError);
    EXPECT_THROW(context.weatherInfos(), AssertionError);
    EXPECT_THROW(context.worldTime(), AssertionError);
    EXPECT_THROW(context.zoneGroups(), AssertionError);
    EXPECT_THROW(context.zoneInfos(), AssertionError);
}

TEST(GameContextTest, RegisteringOneManagerLeavesTheOthersAsserting) {
    de::GameContext context;

    context.setZoneGroupManager(standIn<ZoneGroupManager>(6));

    EXPECT_EQ(&context.zoneGroups(), standIn<ZoneGroupManager>(6));
    EXPECT_THROW(context.config(), AssertionError);
}

TEST(GameContextTest, ProcessWideContextIsOneInstance) {
    EXPECT_EQ(&de::gameContext(), &de::gameContext());
}
