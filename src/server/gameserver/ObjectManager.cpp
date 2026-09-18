//////////////////////////////////////////////////////////////////////////////
// Filename    : ObjectManager.cpp
// Written by  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "ObjectManager.h"

#include "AlignmentManager.h"
#include "Assert.h"
#include "CombatInfoManager.h"
#include "CreatureManager.h"
#include "DarkLightInfo.h"
#include "Directive.h"
#include "GameWorldInfoManager.h"
#include "GuildManager.h"
#include "ItemFactoryManager.h"
#include "ItemInfoManager.h"
#include "ItemLoaderManager.h"
#include "ItemMineInfo.h"
#include "LogClient.h"
#include "MonsterInfo.h"
#include "MonsterNameManager.h"
#include "OptionInfo.h"
#include "PCFinder.h"
#include "ParkingCenter.h"
#include "Party.h"
#include "PlayerManager.h"
#include "PriceManager.h"
#include "Properties.h"
#include "ResurrectLocationManager.h"
#include "ScriptManager.h"
#include "ShopTemplate.h"
#include "SkillDomainInfoManager.h"
#include "SkillHandlerManager.h"
#include "SkillInfo.h"
#include "SkillParentInfo.h"
#include "TelephoneCenter.h"
#include "TimeManager.h"
#include "TradeManager.h"
#include "UniqueItemManager.h"
#include "VariableManager.h"
#include "VisionInfo.h"
#include "VolumeInfo.h"
#include "WayPoint.h"
#include "WeatherInfo.h"
#include "ZoneGroupManager.h"
#include "ZoneInfoManager.h"
#include "quest/ActionFactoryManager.h"
#include "quest/ConditionFactoryManager.h"

// #include "AttrBalanceInfo.h"

#include <stdio.h>

#include "EffectLoaderManager.h"
#include "FlagSet.h"
#include "OustersEXPInfo.h"
#include "VampEXPInfo.h"

// by sigi. 2002.8.31
// #include "RankEXPInfo.h"

// by Sequoia 2004.1.8
#include "RankExpTable.h"

// by sigi. 2002.9.2
#include "MasterLairInfoManager.h"
// 2003. 1. 20. by bezz, Sequoia
#include "CastleInfoManager.h"

// by sigi. 2002.10.30
#include "LogNameManager.h"

// by bezz. 2002.11.18
#include "RankBonusInfo.h"

// #include "GuildRegistrationManager.h"
// #include "GuildVoteManager.h"

// #include "MonsterKillQuest.h"

// Adam's holy land bonus
// #include "HolyLandRaceBonus.h"
#include "BloodBibleBonusManager.h"
#include "CastleShrineInfoManager.h"
#include "HolyLandManager.h"
#include "RaceWarLimiter.h"
#include "ShrineInfoManager.h"
#include "SkillPropertyManager.h"
#include "War.h"
#include "WarSystem.h"

// For the couple event
#include "CoupleManager.h"

// For PK zones
#include "PKZoneInfoManager.h"

// Holy land skills
#include "CastleSkillInfo.h"
#include "StringPool.h"
// #include "FameLimitInfo.h"
#include "AdvancementClassExpTable.h"
#include "DefaultOptionSetInfo.h"
#include "DynamicZoneFactoryManager.h"
#include "DynamicZoneInfo.h"
#include "DynamicZoneManager.h"
#include "EventZoneInfo.h"
#include "GQuestCheckPoint.h"
#include "GQuestElement.h"
#include "GQuestInfo.h"
#include "GameContext.h"
#include "GameServerGroupInfoManager.h"
#include "GoodsInfoManager.h"
#include "GuildUnion.h"
#include "ItemGradeManager.h"
#include "LevelNickInfoManager.h"
#include "LevelWarZoneInfoManager.h"
#include "PetAttrInfo.h"
#include "PetExpInfo.h"
#include "PetTypeInfo.h"
#include "RegenZoneManager.h"
#include "SiegeManager.h"
#include "SlayerAttrExpTable.h"
#include "SweeperBonusManager.h"
#include "SystemAvailabilitiesManager.h"
#include "TimeChecker.h"
#include "ctf/FlagManager.h"
#include "mission/EventQuestLootingManager.h"
#include "war/DragonEyeManager.h"

/*// Simple quest
#include "mission/QuestInfoManager.h"
#include "mission/RewardClassInfoManager.h"
*/
//////////////////////////////////////////////////////////////////////////////
// class ObjectManager member methods
//////////////////////////////////////////////////////////////////////////////

ObjectManager::ObjectManager()

{
    __BEGIN_TRY

    FlagSet::initialize();

    g_pStringPool = new StringPool();
    g_pZoneInfoManager = new ZoneInfoManager();
    g_pVariableManager = new VariableManager();
    g_pItemInfoManager = new ItemInfoManager();
    g_pItemFactoryManager = new ItemFactoryManager();
    m_pVolumeInfoManager = new VolumeInfoManager();
    m_pItemLoaderManager = new ItemLoaderManager();
    m_pShopTemplateManager = new ShopTemplateManager();
    g_pOptionInfoManager = new OptionInfoManager();
    m_pItemMineInfoManager = new ItemMineInfoManager();
    m_pDirectiveSetManager = new DirectiveSetManager();
    m_pMonsterNameManager = new MonsterNameManager();
    g_pZoneGroupManager = new ZoneGroupManager();
    m_pTimeManager = new TimeManager();
    m_pDarkLightInfoManager = new DarkLightInfoManager();
    m_pVisionInfoManager = new VisionInfoManager();
    m_pWeatherInfoManager = new WeatherInfoManager();
    g_pMonsterInfoManager = new MonsterInfoManager();
    g_pSkillHandlerManager = new SkillHandlerManager();
    g_pSkillInfoManager = new SkillInfoManager();
    m_pSkillDomainInfoManager = new SkillDomainInfoManager();
    // g_pSkillParentInfoManager   = new SkillParentInfoManager ();
    g_pPCFinder = new PCFinder();

    // The context is given the managers created above so that a subsystem
    // can be handed them explicitly. It does not own them: they are created
    // here and deleted in this class's destructor.
    de::GameContext& context = de::gameContext();
    context.setStringPool(g_pStringPool);
    context.setZoneInfoManager(g_pZoneInfoManager);
    context.setVariableManager(g_pVariableManager);
    context.setItemFactoryManager(g_pItemFactoryManager);
    context.setVolumeInfoManager(m_pVolumeInfoManager);
    context.setItemLoaderManager(m_pItemLoaderManager);
    context.setShopTemplateManager(m_pShopTemplateManager);
    context.setItemMineInfoManager(m_pItemMineInfoManager);
    context.setDirectiveSetManager(m_pDirectiveSetManager);
    context.setMonsterNameManager(m_pMonsterNameManager);
    context.setZoneGroupManager(g_pZoneGroupManager);
    context.setTimeManager(m_pTimeManager);
    context.setDarkLightInfoManager(m_pDarkLightInfoManager);
    context.setWeatherInfoManager(m_pWeatherInfoManager);
    context.setSkillDomainInfoManager(m_pSkillDomainInfoManager);
    context.setPCFinder(g_pPCFinder);

    g_pParkingCenter = new ParkingCenter();
    m_pTelephoneCenter = new TelephoneCenter();
    m_pPublicScriptManager = new ScriptManager();
    context.setPublicScriptManager(m_pPublicScriptManager);
    // g_pSkillParentInfoManager   = new SkillParentInfoManager();
    m_pConditionFactoryManager = new ConditionFactoryManager();
    context.setConditionFactoryManager(m_pConditionFactoryManager);
    m_pActionFactoryManager = new ActionFactoryManager(context);
    context.setActionFactoryManager(m_pActionFactoryManager);
    //	g_pDEXBalanceInfoManager    = new DEXBalanceInfoManager();
    //	g_pSTRBalanceInfoManager    = new STRBalanceInfoManager();
    //	g_pINTBalanceInfoManager    = new INTBalanceInfoManager();
    m_pVampEXPInfoManager = new VampEXPInfoManager();
    context.setVampEXPInfoManager(m_pVampEXPInfoManager);
    m_pOustersEXPInfoManager = new OustersEXPInfoManager();
    context.setOustersEXPInfoManager(m_pOustersEXPInfoManager);
    m_pPriceManager = new PriceManager();
    context.setPriceManager(m_pPriceManager);
    m_pEffectLoaderManager = new EffectLoaderManager();
    context.setEffectLoaderManager(m_pEffectLoaderManager);
    g_pGuildManager = new GuildManager();
    //	g_pGuildRegistrationManager = new GuildRegistrationManager();
    //	g_pGuildVoteManager         = new GuildVoteManager();
    g_pResurrectLocationManager = new ResurrectLocationManager();
    m_pAlignmentManager = new AlignmentManager();
    context.setAlignmentManager(m_pAlignmentManager);
    m_pWayPointManager = new WayPointManager();
    context.setWayPointManager(m_pWayPointManager);
    m_pGlobalPartyManager = new GlobalPartyManager();
    context.setGlobalPartyManager(m_pGlobalPartyManager);
    g_pGameWorldInfoManager = new GameWorldInfoManager();
    m_pCombatInfoManager = new CombatInfoManager();
    context.setCombatInfoManager(m_pCombatInfoManager);
    m_pUniqueItemManager = new UniqueItemManager();

    // by sigi. 2002.8.31
    //	g_pRankEXPInfoManager[RANK_TYPE_SLAYER]	= new RankEXPInfoManager();
    //	g_pRankEXPInfoManager[RANK_TYPE_VAMPIRE] = new RankEXPInfoManager();
    //	g_pRankEXPInfoManager[RANK_TYPE_OUSTERS] = new RankEXPInfoManager();

    // by sigi. 2002.9.2
    m_pMasterLairInfoManager = new MasterLairInfoManager();
    context.setMasterLairInfoManager(m_pMasterLairInfoManager);
    // 2003. 1. 20. by bezz,Sequoia
    g_pCastleInfoManager = new CastleInfoManager();

    m_pRankBonusInfoManager = new RankBonusInfoManager();
    context.setRankBonusInfoManager(m_pRankBonusInfoManager);

    //	g_pHolyLandRaceBonus	= new HolyLandRaceBonus();

    g_pWarSystem = new WarSystem();

    g_pShrineInfoManager = new ShrineInfoManager();
    m_pCastleShrineInfoManager = new CastleShrineInfoManager();
    context.setCastleShrineInfoManager(m_pCastleShrineInfoManager);

    g_pHolyLandManager = new HolyLandManager();

    m_pBloodBibleBonusManager = new BloodBibleBonusManager();
    context.setBloodBibleBonusManager(m_pBloodBibleBonusManager);

    m_pSkillPropertyManager = new SkillPropertyManager();
    context.setSkillPropertyManager(m_pSkillPropertyManager);

    m_pCoupleManager = new CoupleManager();
    context.setCoupleManager(m_pCoupleManager);
    g_pPKZoneInfoManager = new PKZoneInfoManager();
    //	g_pFameLimitInfoManager = new FameLimitInfoManager();
    m_pGameServerGroupInfoManager = new GameServerGroupInfoManager();
    context.setGameServerGroupInfoManager(m_pGameServerGroupInfoManager);
    m_pCastleSkillInfoManager = new CastleSkillInfoManager();
    context.setCastleSkillInfoManager(m_pCastleSkillInfoManager);

    m_pGoodsInfoManager = new GoodsInfoManager();
    context.setGoodsInfoManager(m_pGoodsInfoManager);
    m_pEventQuestLootingManager = new EventQuestLootingManager();
    context.setEventQuestLootingManager(m_pEventQuestLootingManager);

    // g_pQuestInfoManager = new QuestInfoManager();
    // g_pRewardClassInfoManager = new RewardClassInfoManager();

    g_pFlagManager = new FlagManager(context);
    m_pDefaultOptionSetInfoManager = new DefaultOptionSetInfoManager();
    context.setDefaultOptionSetInfoManager(m_pDefaultOptionSetInfoManager);

    g_pLevelWarZoneInfoManager = new LevelWarZoneInfoManager();
    g_pSweeperBonusManager = new SweeperBonusManager();
    m_pDragonEyeManager = new DragonEyeManager();
    context.setDragonEyeManager(m_pDragonEyeManager);
    m_pTimeChecker = new TimeChecker();
    context.setTimeChecker(m_pTimeChecker);
    m_pDynamicZoneInfoManager = new DynamicZoneInfoManager();
    context.setDynamicZoneInfoManager(m_pDynamicZoneInfoManager);
    m_pDynamicZoneManager = new DynamicZoneManager();
    context.setDynamicZoneManager(m_pDynamicZoneManager);
    m_pDynamicZoneFactoryManager = new DynamicZoneFactoryManager();
    context.setDynamicZoneFactoryManager(m_pDynamicZoneFactoryManager);

    __END_CATCH
}

ObjectManager::~ObjectManager()

{
    __BEGIN_TRY

    SAFE_DELETE(g_pStringPool);
    SAFE_DELETE(m_pActionFactoryManager);
    SAFE_DELETE(m_pConditionFactoryManager);
    SAFE_DELETE(m_pPublicScriptManager);
    SAFE_DELETE(g_pPCFinder);
    SAFE_DELETE(g_pParkingCenter);
    SAFE_DELETE(m_pTelephoneCenter);
    SAFE_DELETE(m_pItemMineInfoManager);
    SAFE_DELETE(g_pOptionInfoManager);
    SAFE_DELETE(g_pSkillInfoManager);
    SAFE_DELETE(m_pSkillDomainInfoManager);
    SAFE_DELETE(g_pMonsterInfoManager);
    SAFE_DELETE(g_pItemInfoManager);
    SAFE_DELETE(m_pWeatherInfoManager);
    SAFE_DELETE(m_pVisionInfoManager);
    SAFE_DELETE(m_pDarkLightInfoManager);
    SAFE_DELETE(m_pTimeManager);
    SAFE_DELETE(m_pDirectiveSetManager);
    SAFE_DELETE(m_pMonsterNameManager);
    SAFE_DELETE(g_pZoneInfoManager);
    SAFE_DELETE(g_pZoneGroupManager);
    // SAFE_DELETE(g_pSkillParentInfoManager);
    SAFE_DELETE(g_pSkillHandlerManager);
    SAFE_DELETE(g_pItemFactoryManager);
    SAFE_DELETE(m_pVolumeInfoManager);
    SAFE_DELETE(m_pItemLoaderManager);
    //	SAFE_DELETE(g_pSTRBalanceInfoManager);
    //	SAFE_DELETE(g_pDEXBalanceInfoManager);
    //	SAFE_DELETE(g_pINTBalanceInfoManager);
    SAFE_DELETE(m_pShopTemplateManager);
    SAFE_DELETE(m_pEffectLoaderManager);
    SAFE_DELETE(m_pPriceManager);
    SAFE_DELETE(m_pVampEXPInfoManager);
    SAFE_DELETE(m_pOustersEXPInfoManager);
    SAFE_DELETE(g_pGuildManager);
    //	SAFE_DELETE(g_pGuildRegistrationManager);
    //	SAFE_DELETE(g_pGuildVoteManager);
    SAFE_DELETE(g_pResurrectLocationManager);
    SAFE_DELETE(m_pAlignmentManager);
    SAFE_DELETE(m_pWayPointManager);
    SAFE_DELETE(m_pGlobalPartyManager);
    SAFE_DELETE(g_pGameWorldInfoManager);
    SAFE_DELETE(g_pVariableManager);
    SAFE_DELETE(m_pCombatInfoManager);
    SAFE_DELETE(m_pUniqueItemManager);
    // by sigi. 2002.8.31
    //	SAFE_DELETE(g_pRankEXPInfoManager[RANK_TYPE_SLAYER]);
    //	SAFE_DELETE(g_pRankEXPInfoManager[RANK_TYPE_VAMPIRE]);
    //	SAFE_DELETE(g_pRankEXPInfoManager[RANK_TYPE_OUSTERS]);
    SAFE_DELETE(m_pMasterLairInfoManager);
    SAFE_DELETE(g_pCastleInfoManager);
    SAFE_DELETE(m_pRankBonusInfoManager);
    //	SAFE_DELETE(g_pHolyLandRaceBonus);
    SAFE_DELETE(g_pWarSystem);
    SAFE_DELETE(g_pShrineInfoManager);
    SAFE_DELETE(m_pCastleShrineInfoManager);

    SAFE_DELETE(g_pHolyLandManager);

    SAFE_DELETE(m_pBloodBibleBonusManager);

    SAFE_DELETE(m_pSkillPropertyManager);

    SAFE_DELETE(m_pCoupleManager);
    SAFE_DELETE(g_pPKZoneInfoManager);
    //	SAFE_DELETE(g_pFameLimitInfoManager);
    SAFE_DELETE(m_pGameServerGroupInfoManager);
    SAFE_DELETE(m_pCastleSkillInfoManager);

    SAFE_DELETE(m_pGoodsInfoManager);
    SAFE_DELETE(m_pEventQuestLootingManager);

    // SAFE_DELETE(g_pQuestInfoManager);
    // SAFE_DELETE(g_pRewardClassInfoManager);

    SAFE_DELETE(g_pFlagManager);
    SAFE_DELETE(m_pDefaultOptionSetInfoManager);

    SAFE_DELETE(g_pLevelWarZoneInfoManager);
    SAFE_DELETE(g_pSweeperBonusManager);
    SAFE_DELETE(m_pDragonEyeManager);
    SAFE_DELETE(m_pTimeChecker);

    SAFE_DELETE(m_pDynamicZoneInfoManager);
    SAFE_DELETE(m_pDynamicZoneManager);
    SAFE_DELETE(m_pDynamicZoneFactoryManager);

    __END_CATCH_NO_RETHROW
}

void ObjectManager::init()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    printf("ObjectManager::load() : SystemAvailabilitiesManager Initialization Start\n");
    SystemAvailabilitiesManager::getInstance()->load();
    printf("ObjectManager::load() : SystemAvailabilitiesManager Initialization Success\n");

    //--------------------------------------------------------------------------------
    // Loading a zone loads its NPCs, and these factories are used for that, so
    // this has to be called before the zones are loaded.
    //--------------------------------------------------------------------------------
    printf("ObjectManager::init() : StringPool Initialization Start....... \n");
    g_pStringPool->load();
    printf("ObjectManager::init() : StringPool Initialization Success....... \n");

    printf("ObjectManager::init() : VariableManager Initialization Start....... \n");
    g_pVariableManager->init();
    printf("ObjectManager::init() : VariableManager Initialization Success....... \n");

    printf("ObjectManager::init() : ConditionFactoryManager Initialization Start\n");
    m_pConditionFactoryManager->init();
    printf("ObjectManager::init() : ConditionFactoryManager Initialization Success\n");

    printf("ObjectManager::init() : ActionFactoryManager Initialization Start\n");
    m_pActionFactoryManager->init();
    printf("ObjectManager::init() : ActionFactoryManager Initialization Success\n");

    printf("ObjectManager::init() : ShopTemplate Initialization Start\n");
    m_pShopTemplateManager->init();
    printf("ObjectManager::init() : ShopTemplate Initialization Success\n");

    printf("ObjectManager::init() : DirectiveSetManager Initialization Start\n");
    m_pDirectiveSetManager->init();
    printf("ObjectManager::init() : DirectiveSetManager Initialization Success\n");

    printf("ObjectManager::init() : MonsterNameManager Initialization Start\n");
    m_pMonsterNameManager->init();
    printf("ObjectManager::init() : MonsterNameManager Initialization Success\n");

    printf("ObjectManager::init() : TimeManager Initialization Start\n");
    m_pTimeManager->init();
    printf("ObjectManager::init() : TimeManager Initialization Success\n");

    printf("ObjectManager::init() : PublicScriptManager Initialization Start\n");
    m_pPublicScriptManager->init();
    printf("ObjectManager::init() : PublicScriptManager Initialization Success\n");

    // Options have to be load()ed before itemInfo.
    printf("ObjectManager::init() : OptionInfoManager Initialization Start\n");
    g_pOptionInfoManager->init();
    printf("ObjectManager::init() : OptionInfoManager Initialization Success\n");

    printf("ObjectManager::init() : SweeperBonusManager Initialization Start....... \n");
    g_pSweeperBonusManager->init();
    printf("ObjectManager::init() : SweeperBonusManager Initialization Success....... \n");

    printf("ObjectManager::init() : ItemInfoManager Initialization Start\n");
    g_pItemInfoManager->init();
    printf("ObjectManager::init() : ItemInfoManager Initialization Success\n");

    printf("ObjectManager::init() : ItemMineInfoManager Initialization Start\n");
    m_pItemMineInfoManager->load();
    printf("ObjectManager::init() : ItemMineInfoManager Initialization Success\n");

    printf("ObjectManager::init() : WarID Initialization Start\n");
    g_pWarSystem->setWarIDSuccessor(g_pConfig->getPropertyInt("ServerCount"));
    War::initWarIDRegistry();
    printf("ObjectManager::init() : WarID Initialization Success\n");

    printf("ObjectManager::init() : VolumeInfoManager Initialization Start\n");
    m_pVolumeInfoManager->init();
    printf("ObjectManager::init() : VolumeInfoManager Initialization Success\n");

    printf("ObjectManager::init() : ItemFactory Initialization Start\n");
    g_pItemFactoryManager->init();
    printf("ObjectManager::init() : ItemFactory Initialization Success\n");

    printf("ObjectManager::init() : ItemLoaderManager Initialization Start\n");
    m_pItemLoaderManager->init();
    printf("ObjectManager::init() : ItemLoaderManager Initialization Success\n");

    printf("ObjectManager::init() : DarkLightInfoManager Initialization Start\n");
    m_pDarkLightInfoManager->init();
    printf("ObjectManager::init() : DarkLightInfoManager Initialization Success\n");

    printf("ObjectManager::init() : MonsterInfoManager Initialization Start\n");
    g_pMonsterInfoManager->init();
    printf("ObjectManager::init() : MonsterInfoManager Initialization Success\n");

    // Must be loaded before ZoneInfoManager.
    // Must be loaded after OptionInfo.
    /*	printf("ObjectManager::load() : RewardClassInfoManager Initialization Start\n");
        g_pRewardClassInfoManager->load();
        printf("ObjectManager::load() : RewardClassInfoManager Initialization Success\n");*/

    printf("ObjectManager::init() : EffectLoaderManager Initialization Start\n");
    m_pEffectLoaderManager->init();
    printf("ObjectManager::init() : EffectLoaderManager Initialization Success\n");

    printf("ObjectManager::init() : ZoneInfoManager Initialization Start\n");
    g_pZoneInfoManager->init();
    printf("ObjectManager::init() : ZoneInfoManager Initialization Success\n");

    // by sigi. 2002.9.2
    printf("ObjectManager::load() : MasterLairInfoManager Initialization Start\n");
    m_pMasterLairInfoManager->init(); // load after ZoneInfo and MonsterManager, before Zone
    printf("ObjectManager::load() : MasterLairInfoManager Initialization Success\n");

    // by bezz,Sequoia. 2003. 1. 20.
    printf("ObjectManager::load() : CastleInfoManager Initialization Start\n");
    g_pCastleInfoManager->init(); // load after ZoneInfo and MonsterManager, before Zone
    printf("ObjectManager::load() : CastleInfoManager Initialization Success\n");

    //	printf("ObjectManager::load() : HolyLandRaceBonus Initialization Start\n");
    //	g_pHolyLandRaceBonus->refresh();
    //	printf("ObjectManager::load() : HolyLandRaceBonus Initialization Success\n");

    printf("ObjectManager::init() : ZoneGroupManager Initialization Start\n");
    g_pZoneGroupManager->init();
    printf("ObjectManager::init() : ZoneGroupManager Initialization Success\n");

    printf("ObjectManager::load() : BloodBibleBonusManager Initialization Start\n");
    m_pBloodBibleBonusManager->init();
    printf("ObjectManager::load() : BloodBibleBonusManager Initialization Success\n");

    // ShrineInfoManager must be called only after every Zone has been loaded.
    // It sets the BloodBible-owning race on BloodBibleBonusManager, so it must come after that manager loads.
    printf("ObjectManager::init() : ShrineInfoManager Initialization Start\n");
    g_pShrineInfoManager->init();
    printf("ObjectManager::init() : ShrineInfoManager Initialization Success\n");

    printf("ObjectManager::init() : CastleShrineInfoManager Initialization Start\n");
    m_pCastleShrineInfoManager->init();
    printf("ObjectManager::init() : CastleShrineInfoManager Initialization Success\n");

    printf("ObjectManager::init() : WeatherInfoManager Initialization Start\n");
    m_pWeatherInfoManager->init();
    printf("ObjectManager::init() : WeatherInfoManager Initialization Success\n");

    // WayPointManager must also be called only after every Zone has been loaded.
    printf("ObjectManager::load() : WayPointManager Initialization Start\n");
    m_pWayPointManager->load();
    printf("ObjectManager::load() : WayPointManager Initialization Success\n");

    printf("ObjectManager::load() : LevelWarZoneInfoManager Initialization Start\n");
    g_pLevelWarZoneInfoManager->init(); // may be loaded at any time
    printf("ObjectManager::load() : LevelWarZoneInfoManager Initialization Success\n");

    printf("ObjectManager::load() : LevelNickInfoManager Initialization Start\n");
    LevelNickInfoManager::Instance().load(); // may be loaded at any time
    printf("ObjectManager::load() : LevelNickInfoManager Initialization Success\n");

    printf("ObjectManager::load() : DragonEyeManagerManager Initialization Start\n");
    m_pDragonEyeManager->init(); // after the item info is loaded
    printf("ObjectManager::load() : DragonEyeManagerManager Initialization Success\n");

    printf("ObjectManager::init() : TimeChecker Initialization Start\n");
    m_pTimeChecker->init(); // may be loaded at any time
    printf("ObjectManager::init() : TimeChecker Initialization Success\n");


    __END_DEBUG
    __END_CATCH
}


void ObjectManager::load()

{
    __BEGIN_TRY

    //--------------------------------------------------------------------------------
    // Loading a zone group loads its zones internally.
    // That loads the NPCs belonging to each zone, and the SetPosition action
    // needs ZoneInfoManager to be initialized first.
    // So the initialization order is ZoneInfoManager -> ZoneGroupManager.
    //--------------------------------------------------------------------------------
    // printf("ObjectManager::init() : ZoneInfoManager Initialization Start\n");
    // g_pZoneInfoManager->init();
    // printf("ObjectManager::init() : ZoneInfoManager Initialization Success\n");

    // printf("ObjectManager::init() : ZoneGroupManager Initialization Start\n");
    // g_pZoneGroupManager->init();
    // printf("ObjectManager::init() : ZoneGroupManager Initialization Success\n");


    // printf("ObjectManager::init() : SkillParentInfoManager Initialization Start\n");
    // g_pSkillParentInfoManager->init();
    // printf("ObjectManager::init() : SkillParentInfoManager Initialization Success\n");

    printf("ObjectManager::init() : GuildManager Initialization Start\n");
    if (g_pGuildManager != NULL)
        g_pGuildManager->init();
    printf("ObjectManager::init() : GuildManager Initialization Success\n");

    printf("ObjectManager::init() : GuildUnionManager Initialization Start\n");
    GuildUnionManager::Instance().load();
    printf("ObjectManager::init() : GuildUnionManager Initialization Success\n");

    printf("ObjectManager::init() : SkillHandlerManager Initialization Start\n");
    g_pSkillHandlerManager->init();
    printf("ObjectManager::init() : SkillHandlerManager Initialization Success\n");

    printf("ObjectManager::init() : SkillInfoManager Initialization Start\n");
    g_pSkillInfoManager->init();
    printf("ObjectManager::init() : SkillInfoManager Initialization Success\n");


    printf("ObjectManager::init() : SkillDomainInfoManager Initialization Start\n");
    m_pSkillDomainInfoManager->init();
    printf("ObjectManager::init() : SkillDomainInfoManager Initialization Success\n");


    printf("ObjectManager::init() : ResurrectLocationManager Initialization Start\n");
    g_pResurrectLocationManager->init();
    printf("ObjectManager::init() : ResurrectLocationManager Initialization Success\n");

    // balnce info manager init//abcd
    printf("ObjectManager::init() : STR Exp Table Initialization Start\n");
    //	g_pSTRBalanceInfoManager->init();
    SlayerAttrExpTable::s_SlayerAttrExpTable[ATTR_KIND_STR].load();
    printf("ObjectManager::init() : STR Exp Table Initialization Success\n");

    printf("ObjectManager::init() : DEX Exp Table Initialization Start\n");
    //	g_pDEXBalanceInfoManager->init();
    SlayerAttrExpTable::s_SlayerAttrExpTable[ATTR_KIND_DEX].load();
    printf("ObjectManager::init() : DEX Exp Table Initialization Success\n");

    printf("ObjectManager::init() : INT Exp Table Initialization Start\n");
    //	g_pINTBalanceInfoManager->init();
    SlayerAttrExpTable::s_SlayerAttrExpTable[ATTR_KIND_INT].load();
    printf("ObjectManager::init() : INT Exp Table Initialization Success\n");

    printf("ObjectManager::init() : VampExpInfoManager Initialization Start\n");
    m_pVampEXPInfoManager->init();
    printf("ObjectManager::init() : VampExpInfoManager Initialization Success\n");

    printf("ObjectManager::init() : OustersEXPInfoManager Initialization Start\n");
    m_pOustersEXPInfoManager->init();
    printf("ObjectManager::init() : OustersEXPInfoManager Initialization Success\n");

    printf("ObjectManager::init() : AdvancementClassExpTable Initialization Start\n");
    AdvancementClassExpTable::s_AdvancementClassExpTable.load();
    printf("ObjectManager::init() : AdvancementClassExpTable Initialization Success\n");

    printf("ObjectManager::init() : VisionInfoManager Initialization Start\n");
    m_pVisionInfoManager->init();
    printf("ObjectManager::init() : VisionInfoManager Initialization Success\n");

    //	printf("ObjectManager::init() : GuildRegistrationManager Initialization Start\n");
    //	g_pGuildRegistrationManager->init();
    //	printf("ObjectManager::init() : GuildRegistrationManager Initialization Success\n");

    //	printf("ObjectManager::init() : GuildVoteManager Initialization Start\n");
    //	g_pGuildVoteManager->init();
    //	printf("ObjectManager::init() : GuildVoteManager Initialization Success\n");

    printf("ObjectManager::load() : GameWorldInfoManager Initialization Start\n");
    g_pGameWorldInfoManager->load();
    printf("ObjectManager::load() : GameWorldInfoManager Initialization Success\n");

    printf("ObjectManager::load() : CombatInfoManager Initialization Start\n");
    m_pCombatInfoManager->initModify();
    printf("ObjectManager::load() : CombatInfoManager Initialization Success\n");

    printf("ObjectManager::load() : UniqueItemManager Initialization Start\n");
    m_pUniqueItemManager->init();
    printf("ObjectManager::load() : UniqueItemManager Initialization Success\n");

    // by sigi. 2002.8.31
    printf("ObjectManager::load() : RankExpTables Initialization Start\n");
    //	g_pRankEXPInfoManager[RANK_TYPE_SLAYER]->init(RANK_TYPE_SLAYER);
    //	g_pRankEXPInfoManager[RANK_TYPE_VAMPIRE]->init(RANK_TYPE_VAMPIRE);
    //	g_pRankEXPInfoManager[RANK_TYPE_OUSTERS]->init(RANK_TYPE_OUSTERS);

    // by Sequoia 2004.1.8
    RankExpTable::s_RankExpTables[RANK_TYPE_SLAYER].load();
    RankExpTable::s_RankExpTables[RANK_TYPE_VAMPIRE].load();
    RankExpTable::s_RankExpTables[RANK_TYPE_OUSTERS].load();
    printf("ObjectManager::load() : RankExpTables Initialization Success\n");

    printf("ObjectManager::load() : LogNameManager Initialization Start\n");
    LogNameManager::getInstance().init();
    printf("ObjectManager::load() : LogNameManager Initialization Success\n");

    printf("ObjectManager::load() : RankBonusInfoManager Initialization Start\n");
    m_pRankBonusInfoManager->load();
    printf("ObjectManager::load() : RankBonusInfoManager Initialization Success\n");

    printf("ObjectManager::load() : WarSystem Initialization Start\n");
    g_pWarSystem->init();
    printf("ObjectManager::load() : WarSystem Initialization Success\n");

    printf("ObjectManager::load() : RaceWarLimiter Initialization Start\n");
    RaceWarLimiter::getInstance()->load();
    printf("ObjectManager::load() : RaceWarLimiter Initialization Success\n");

    printf("ObjectManager::load() : PKZoneInfoManager Initialization Start\n");
    g_pPKZoneInfoManager->load();
    printf("ObjectManager::load() : PKZoneInfoManager Initialization Success\n");

    //	printf("ObjectManager::load() : FameLimitInfoManager Initialization Start\n");
    //	g_pFameLimitInfoManager->load();
    //	printf("ObjectManager::load() : FameLimitInfoManager Initialization Success\n");

    printf("ObjectManager::load() : GameServerGroupInfoManager Initialization Start\n");
    m_pGameServerGroupInfoManager->init();
    printf("ObjectManager::load() : GameServerGroupInfoManager Initialization Success\n");

    printf("ObjectManager::load() : CastleSkillInfoManager Initialization Start\n");
    m_pCastleSkillInfoManager->load();
    printf("ObjectManager::load() : CastleSkillInfoManager Initialization Success\n");

    printf("ObjectManager::load() : GoodsInfoManager Initialization Start\n");
    m_pGoodsInfoManager->load();
    printf("ObjectManager::load() : GoodsInfoManager Initialization Success\n");

    printf("ObjectManager::load() : EventQuestLootingManager Initialization Start\n");
    m_pEventQuestLootingManager->load();
    printf("ObjectManager::load() : EventQuestLootingManager Initialization Success\n");

    printf("ObjectManager::load() : FlagManager Initialization Start\n");
    g_pFlagManager->init();
    printf("ObjectManager::load() : FlagManager Initialization Success\n");

    printf("ObjectManager::load() : RegenZoneManager Initialization Start\n");
    RegenZoneManager::getInstance()->load();
    printf("ObjectManager::load() : RegenZoneManager Initialization Success\n");

    printf("ObjectManager::load() : DefaultOptionSetInfoManager Initialization Start\n");
    m_pDefaultOptionSetInfoManager->load();
    printf("ObjectManager::load() : DefaultOptionSetInfoManager Initialization Success\n");

    printf("ObjectManager::load() : PetTypeInfoManager Initialization Start\n");
    PetTypeInfoManager::getInstance()->load();
    printf("ObjectManager::load() : PetTypeInfoManager Initialization Success\n");

    printf("ObjectManager::load() : PetAttrInfoManager Initialization Start\n");
    PetAttrInfoManager::Instance().load();
    printf("ObjectManager::load() : PetAttrInfoManager Initialization Success\n");

    printf("ObjectManager::load() : PetExpInfoManager Initialization Start\n");
    PetExpInfoManager::Instance().load();
    printf("ObjectManager::load() : PetExpInfoManager Initialization Success\n");

    printf("ObjectManager::load() : ItemGradeManager Initialization Start\n");
    ItemGradeManager::Instance().load();
    printf("ObjectManager::load() : ItemGradeManager Initialization Success\n");

    printf("ObjectManager::load() : EventZoneInfoManager Initialization Start\n");
    EventZoneInfoManager::Instance().load();
    printf("ObjectManager::load() : EventZoneInfoManager Initialization Success\n");

    printf("ObjectFactory::load() : GQuestElementFactory Initialization Start\n");
    GQuestElementFactory::Instance().init();
    printf("ObjectFactory::load() : GQuestElementFactory Initialization Success\n");

    printf("ObjectManager::load() : GQuestInfoManager Initialization Start\n");
    GQuestInfoManager::Instance().load();
    printf("ObjectManager::load() : GQuestInfoManager Initialization Success\n");

    printf("ObjectManager::load() : GQuestCheckPoint Initialization Start\n");
    GQuestCheckPoint::Instance().load();
    printf("ObjectManager::load() : GQuestCheckPoint Initialization Success\n");

    printf("ObjectManager::load() : SiegeManager Initialization Start\n");
    SiegeManager::Instance().init();
    printf("ObjectManager::load() : SiegeManager Initialization Success\n");

    printf("ObjectManager::load() : DynamicZoneInfoManager Initialization Start\n");
    m_pDynamicZoneInfoManager->init();
    printf("ObjectManager::load() : DynamicZoneInfoManager Initialization Success\n");

    // Called after DynamicZoneInfoManager init
    printf("ObjectManager::load() : DynamicZoneManager Initialization Start\n");
    m_pDynamicZoneManager->init();
    printf("ObjectManager::load() : DynamicZoneManager Initialization Success\n");

    printf("ObjectManager::load() : DynamicZoneFactoryManager Initialization Start\n");
    m_pDynamicZoneFactoryManager->init();
    printf("ObjectManager::load() : DynamicZoneFactoryManager Initialization Success\n");

    /*
    printf("ObjectManager::load() : QuestInfoManager Initialization Start\n");
    g_pQuestInfoManager->load();
    printf("ObjectManager::load() : QuestInfoManager Initialization Success\n");
    printf("ObjectManager::load() : DarkLightInfoManager Loading Start\n");
    m_pDarkLightInfoManager->load();
    printf("ObjectManager::load() : DarkLightInfoManager Loading Success\n");

    printf("ObjectManager::load() : MonsterInfoManager Loading Start\n");
    g_pMonsterInfoManager->load();
    printf("ObjectManager::load() : MonsterInfoManager Loading Success\n");

    printf("ObjectManager::load() : WeatherInfoManager Loading Start\n");
    m_pWeatherInfoManager->load();
    printf("ObjectManager::load() : WeatherInfoManager Loading Success\n");

    printf("ObjectManager::load() : ZoneGroupManager Loading Start\n");
    g_pZoneGroupManager->load();
    printf("ObjectManager::load() : ZoneGroupManager Loading Success\n");

    printf("ObjectManager::load() : ZoneInfoManager Loading Start\n");
    g_pZoneInfoManager->load();
    printf("ObjectManager::load() : ZoneInfoManager Loading Success\n");

    printf("ObjectManager::load() : PublicScriptManager Loading Start\n");
    m_pPublicScriptManager->load("");
    printf("ObjectManager::load() : PublicScriptManager Loading Success\n");
    */

    __END_CATCH
}

void ObjectManager::save()

{
    __BEGIN_TRY

    throw UnsupportedError();

    __END_CATCH
}
