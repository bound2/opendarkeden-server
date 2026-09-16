//////////////////////////////////////////////////////////////////////////////
// FileName 	: Zone.cpp
// WrittenBy	:
// Description	:
//////////////////////////////////////////////////////////////////////////////

#include "Zone.h"

#include <stdio.h>
#include <string.h>

#include "Assert.h"
#include "LogClient.h"
#include "MasterLairInfoManager.h"
#include "MasterLairManager.h"
#include "MonsterManager.h"
#include "NPCManager.h"
#include "PCManager.h"
#include "QuestManager.h"
#include "VisionInfo.h"
#include "War.h"
#include "WarScheduler.h"
#include "WarSystem.h"
#include "ZoneGroup.h"
#include "ZoneInfo.h"
#include "ZoneInfoManager.h"
#include "ZoneInternal.h"

// #include "EventMonsterManager.h"

#include <math.h>

#include <fstream>

#include "BloodBibleBonusManager.h"
#include "CastleInfoManager.h"
#include "CombatInfoManager.h"
#include "Creature.h"
#include "DarkLightInfo.h"
#include "DefaultOptionSetInfo.h"
#include "EffectDarkness.h"
#include "EffectDecayCorpse.h"
#include "EffectDecayItem.h"
#include "EffectLoaderManager.h"
#include "EffectManager.h"
#include "EffectSchedule.h"
#include "EffectVampirePortal.h"
#include "FlagSet.h"
#include "GamePlayer.h"
#include "HolyLandManager.h"
#include "Item.h"
#include "ItemFactoryManager.h"
#include "ItemInfo.h"
#include "LevelWarZoneInfoManager.h"
#include "Monster.h"
#include "MonsterCorpse.h"
#include "NPC.h"
#include "NPCInfo.h"
#include "Ousters.h"
#include "OustersCorpse.h"
#include "PCFinder.h"
#include "PKZoneInfoManager.h"
#include "PacketUtil.h"
#include "ParkingCenter.h"
#include "Party.h"
#include "PaySystem.h"
#include "Player.h"
#include "Properties.h"
#include "RegenZoneManager.h"
#include "Relic.h"
#include "RelicUtil.h"
#include "ShrineInfoManager.h"
#include "Slayer.h"
#include "SlayerCorpse.h"
#include "StringPool.h"
#include "SweeperBonusManager.h"
#include "TimeManager.h"
#include "TradeManager.h"
#include "Vampire.h"
#include "VampireCorpse.h"
#include "VariableManager.h"
#include "WeatherManager.h"
#include "ZoneUtil.h"
#include "ctf/FlagManager.h"
#include "repository/ComebackEventRepository.h"
#include "repository/MessageRepository.h"
#include "repository/ZoneInfoRepository.h"
// #include "EffectRevealer.h"
#include "EffectAddItem.h"
#include "EffectAddItemToCorpse.h"
#include "EffectCallMotorcycle.h"
#include "EffectDecayMotorcycle.h"
#include "EffectDeleteItem.h"
#include "EffectGnomesWhisper.h"
#include "EffectHasBloodBible.h"
#include "EffectHasSlayerRelic.h"
#include "EffectHasVampireRelic.h"
#include "EffectObservingEye.h"
#include "EffectRelicTable.h"
#include "EffectSanctuary.h"
#include "EffectShrineGuard.h"
#include "EffectShrineHoly.h"
#include "EffectShrineShield.h"
#include "EffectSlayerRelic.h"
#include "EffectTransportItem.h"
#include "EffectTransportItemToCorpse.h"
#include "EffectVampireRelic.h"
// #include "EffectDropBloodBible.h"
#include "EffectContinualGroundAttack.h"
#include "EffectHasCastleSymbol.h"
#include "EffectPKZoneRegen.h"
#include "EventTransport.h"
#include "GCAddBat.h"
#include "GCAddBurrowingCreature.h"
#include "GCAddEffect.h"
#include "GCAddEffectToTile.h"
#include "GCAddInstalledMineToZone.h"
#include "GCAddMonster.h"
#include "GCAddMonsterCorpse.h"
#include "GCAddMonsterFromBurrowing.h"
#include "GCAddMonsterFromTransformation.h"
#include "GCAddNPC.h"
#include "GCAddNewItemToZone.h"
#include "GCAddOusters.h"
#include "GCAddOustersCorpse.h"
#include "GCAddSlayer.h"
#include "GCAddSlayerCorpse.h"
#include "GCAddVampire.h"
#include "GCAddVampireCorpse.h"
#include "GCAddVampireFromBurrowing.h"
#include "GCAddVampireFromTransformation.h"
#include "GCAddVampirePortal.h"
#include "GCAddWolf.h"
#include "GCDeleteEffectFromTile.h"
#include "GCDeleteObject.h"
#include "GCDropItemToZone.h"
#include "GCFastMove.h"
#include "GCHolyLandBonusInfo.h"
#include "GCKnockBack.h"
#include "GCMineExplosionOK1.h"
#include "GCMineExplosionOK2.h"
#include "GCModifyInformation.h"
#include "GCMove.h"
#include "GCMoveError.h"
#include "GCMoveOK.h"
#include "GCMyStoreInfo.h"
#include "GCNPCInfo.h"
#include "GCNoticeEvent.h"
#include "GCRegenZoneStatus.h"
#include "GCRemoveEffect.h"
#include "GCSetPosition.h"
#include "GCSweeperBonusInfo.h"
#include "GCSystemMessage.h"
#include "GCUnburrowFail.h"
#include "GCUnburrowOK.h"
#include "GCUnionOfferList.h"
#include "GCUntransformFail.h"
#include "GCUntransformOK.h"
// #include "CGItemNameInfoList.h"

#include "DynamicZone.h"
#include "EffectCastingTrap.h"
#include "GDRLairManager.h"
#include "GGCommand.h"
#include "GQuestManager.h"
#include "GameServerInfoManager.h"
#include "GuildManager.h"
#include "GuildUnion.h"
#include "LevelWarManager.h"
#include "LoginServerManager.h"
#include "NicknameBook.h"
#include "Profile.h"
#include "ResurrectLocationManager.h"
#include "SiegeManager.h"
#include "SkillUtil.h"
#include "Store.h"
#include "item/Motorcycle.h"
#include "item/VampirePortalItem.h"

// by sigi.  2002.12.30
// #define __PROFILE_BROADCAST__

#ifdef __PROFILE_BROADCAST__
#define __BEGIN_PROFILE_ZONE(name) beginProfileEx(name);
#define __END_PROFILE_ZONE(name) endProfileEx(name);
#else
#define __BEGIN_PROFILE_ZONE(name) ((void)0);
#define __END_PROFILE_ZONE(name) ((void)0);
#endif

// #define __FULL_PROFILE__

#ifndef __FULL_PROFILE__
#undef beginProfileEx
#define beginProfileEx(name) ((void)0)
#undef endProfileEx
#define endProfileEx(name) ((void)0)
#endif

// 마스터 레어에서 시체/아이템이 바닥에서 사라지는 시간
const Turn_t DELAY_MASTER_LAIR_DECAY_CORPSE = 200;       // 20초
const Turn_t DELAY_MASTER_LAIR_DECAY_ITEM = 400;         // 40초
const Turn_t DELAY_MASTER_LAIR_DECAY_MASTER_CORPSE = 50; // 5초


void strlwr(char* str) {
    while (*str != '\0') {
        *str = tolower(*str);

        str++;
    }
}

//////////////////////////////////////////////////////////////////////////////
// 일반적인 몬스터들이 적으로 인식하느냐 마느냐 하는 함수
//////////////////////////////////////////////////////////////////////////////
bool isPotentialEnemy(Monster* pMonster, Creature* pCreature) {
    Assert(pCreature != NULL);

    if (pMonster->getOwnerObjectID() == pCreature->getObjectID())
        return false;

    if (pMonster->getOwnerObjectID() != 0) {
        Creature* pOwner = pMonster->getZone()->getCreature(pMonster->getOwnerObjectID());
        if (pOwner != NULL && pOwner->getCreatureClass() == pCreature->getCreatureClass() &&
            canAttack(pOwner, pCreature)) {
            return false;
        }

        if (GDRLairManager::Instance().isGDRLairZone(pMonster->getZoneID()) && pCreature->isPC())
            return false;
    }

    if (SiegeManager::Instance().isSiegeZone(pMonster->getZoneID())) {
        /*		if ( pCreature->isFlag( Effect::EFFECT_CLASS_SIEGE_ATTACKER_5 ) ) return true;
                if ( pCreature->isFlag( Effect::EFFECT_CLASS_SIEGE_ATTACKER_4 ) ) return true;
                if ( pCreature->isFlag( Effect::EFFECT_CLASS_SIEGE_ATTACKER_3 ) ) return true;
                if ( pCreature->isFlag( Effect::EFFECT_CLASS_SIEGE_ATTACKER_2 ) ) return true;
                if ( pCreature->isFlag( Effect::EFFECT_CLASS_SIEGE_ATTACKER_1 ) ) return true;

                return false;*/

        if (pCreature->isFlag(Effect::EFFECT_CLASS_SIEGE_DEFENDER) ||
            pCreature->isFlag(Effect::EFFECT_CLASS_SIEGE_REINFORCE))
            return false;
    }

    // 현재로서는 슬레이어나 아우스터스는 무조건 적이다.
    if (pCreature->isSlayer())
        return true;

    if (pCreature->isOusters()) {
        Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);

        if ((pOusters->getLevel() + 10) <= pMonster->getLevel())
            return true;
        if (pOusters->getLevel() > 10)
            return true;
    } else if (pCreature->isVampire()) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);

        // 몬스터의 레벨이 뱀파이어의 레벨보다 10레벨 이상 높을 경우,
        // 적으로 인식한다.
        if ((pVampire->getLevel() + 10) <= pMonster->getLevel()) {
            return true;
        }

        // 10레벨 이상인 뱀파이어는 적이다.
        if (pVampire->getLevel() > 10) {
            return true;
        }
    } else if (pCreature->isMonster()) {
        Monster* pOtherMonster = dynamic_cast<Monster*>(pCreature);

        return pMonster->isFlag(Effect::EFFECT_CLASS_HALLUCINATION) ||
               pMonster->getClanType() != pOtherMonster->getClanType();
    }

    return false;
}


list<Packet*>* getRelicEffectPacket(MonsterCorpse* pMonsterCorpse, Effect::EffectClass EClass,
                                    list<Packet*>* pPackets) {
    if (pMonsterCorpse->isFlag(EClass)) {
        Effect* pEffect = pMonsterCorpse->getEffectManager().findEffect(EClass);

        if (pEffect == NULL)
            cout << (int)EClass << endl;

        Assert(pEffect != NULL);

        GCAddEffect* pPacket = new GCAddEffect;
        pPacket->setObjectID(pMonsterCorpse->getObjectID());
        pPacket->setEffectID(pEffect->getSendEffectClass());
        pPacket->setDuration(pEffect->getRemainDuration());

        if (pPackets == NULL)
            pPackets = new list<Packet*>;
        pPackets->push_back(pPacket);
    }

    return pPackets;
}


//////////////////////////////////////////////////////////////////////////////
// sendRelicEffect( MonsterCorpse* )
//////////////////////////////////////////////////////////////////////////////
// pMonsterCorpse에 붙은 Effect를 pPlayer에게 보낸다.
//////////////////////////////////////////////////////////////////////////////
list<Packet*>* createRelicEffect(MonsterCorpse* pMonsterCorpse) {
    list<Packet*>* pPackets = NULL;

    if (pMonsterCorpse->isFlag(Effect::EFFECT_CLASS_SLAYER_RELIC)) {
        GCAddEffect* pPacket = new GCAddEffect;
        pPacket->setObjectID(pMonsterCorpse->getObjectID());
        pPacket->setEffectID(Effect::EFFECT_CLASS_SLAYER_RELIC);
        pPacket->setDuration(65000);

        if (pPackets == NULL)
            pPackets = new list<Packet*>;
        pPackets->push_back(pPacket);
    }

    if (pMonsterCorpse->isFlag(Effect::EFFECT_CLASS_VAMPIRE_RELIC)) {
        GCAddEffect* pPacket = new GCAddEffect;
        pPacket->setObjectID(pMonsterCorpse->getObjectID());
        pPacket->setEffectID(Effect::EFFECT_CLASS_VAMPIRE_RELIC);
        pPacket->setDuration(65000);

        if (pPackets == NULL)
            pPackets = new list<Packet*>;
        pPackets->push_back(pPacket);
    }

    Effect::EffectClass EClass = Effect::EFFECT_CLASS_SHRINE_GUARD;
    if (pMonsterCorpse->isFlag(EClass)) {
        Effect* pEffect = pMonsterCorpse->getEffectManager().findEffect(EClass);
        Assert(pEffect != NULL);

        EffectShrineGuard* pEffectShrineGuard = dynamic_cast<EffectShrineGuard*>(pEffect);
        Assert(pEffectShrineGuard != NULL);

        GCAddEffect* pPacket = new GCAddEffect;
        pPacket->setObjectID(pMonsterCorpse->getObjectID());
        pPacket->setEffectID(EClass + pEffectShrineGuard->getShrineID());
        pPacket->setDuration(65000);

        if (pPackets == NULL)
            pPackets = new list<Packet*>;
        pPackets->push_back(pPacket);
    }

    EClass = Effect::EFFECT_CLASS_SHRINE_HOLY;
    if (pMonsterCorpse->isFlag(EClass)) {
        Effect* pEffect = pMonsterCorpse->getEffectManager().findEffect(EClass);
        Assert(pEffect != NULL);

        EffectShrineHoly* pEffectShrineHoly = dynamic_cast<EffectShrineHoly*>(pEffect);
        Assert(pEffectShrineHoly != NULL);

        GCAddEffect* pPacket = new GCAddEffect;
        pPacket->setObjectID(pMonsterCorpse->getObjectID());
        pPacket->setEffectID(EClass + pEffectShrineHoly->getShrineID());
        pPacket->setDuration(65000);

        if (pPackets == NULL)
            pPackets = new list<Packet*>;
        pPackets->push_back(pPacket);
    }

    EClass = Effect::EFFECT_CLASS_SHRINE_SHIELD;
    if (pMonsterCorpse->isFlag(EClass)) {
        Effect* pEffect = pMonsterCorpse->getEffectManager().findEffect(EClass);
        Assert(pEffect != NULL);

        EffectShrineShield* pEffectShrineShield = dynamic_cast<EffectShrineShield*>(pEffect);
        Assert(pEffectShrineShield != NULL);

        GCAddEffect* pPacket = new GCAddEffect;
        pPacket->setObjectID(pMonsterCorpse->getObjectID());
        pPacket->setEffectID(EClass);
        pPacket->setDuration(65000);

        if (pPackets == NULL)
            pPackets = new list<Packet*>;
        pPackets->push_back(pPacket);
    }

    EClass = Effect::EFFECT_CLASS_HAS_BLOOD_BIBLE;
    if (pMonsterCorpse->isFlag(EClass)) {
        Effect* pEffect = pMonsterCorpse->getEffectManager().findEffect(EClass);
        Assert(pEffect != NULL);

        EffectHasBloodBible* pEffectHasBloodBible = dynamic_cast<EffectHasBloodBible*>(pEffect);
        Assert(pEffectHasBloodBible != NULL);

        GCAddEffect* pPacket = new GCAddEffect;
        pPacket->setObjectID(pMonsterCorpse->getObjectID());
        pPacket->setEffectID(pEffectHasBloodBible->getSendEffectClass());
        pPacket->setDuration(65000);

        if (pPackets == NULL)
            pPackets = new list<Packet*>;
        pPackets->push_back(pPacket);
    }

    EClass = Effect::EFFECT_CLASS_HAS_CASTLE_SYMBOL;
    if (pMonsterCorpse->isFlag(EClass)) {
        Effect* pEffect = pMonsterCorpse->getEffectManager().findEffect(EClass);
        Assert(pEffect != NULL);

        EffectHasCastleSymbol* pEffectHasCastleSymbol = dynamic_cast<EffectHasCastleSymbol*>(pEffect);
        Assert(pEffectHasCastleSymbol != NULL);

        GCAddEffect* pPacket = new GCAddEffect;
        pPacket->setObjectID(pMonsterCorpse->getObjectID());
        pPacket->setEffectID(pEffectHasCastleSymbol->getSendEffectClass());
        pPacket->setDuration(65000);

        if (pPackets == NULL)
            pPackets = new list<Packet*>;
        pPackets->push_back(pPacket);
    }

    EClass = Effect::EFFECT_CLASS_FLAG_INSERT;
    if (pMonsterCorpse->isFlag(EClass)) {
        Effect* pEffect = pMonsterCorpse->getEffectManager().findEffect(EClass);
        Assert(pEffect != NULL);

        GCAddEffect* pPacket = new GCAddEffect;
        pPacket->setObjectID(pMonsterCorpse->getObjectID());
        pPacket->setEffectID(pEffect->getSendEffectClass());
        pPacket->setDuration(65000);

        if (pPackets == NULL)
            pPackets = new list<Packet*>;
        pPackets->push_back(pPacket);
    }

    EClass = Effect::EFFECT_CLASS_KEEP_SWEEPER;
    if (pMonsterCorpse->isFlag(EClass)) {
        Effect* pEffect = pMonsterCorpse->getEffectManager().findEffect(EClass);
        Assert(pEffect != NULL);

        GCAddEffect* pPacket = new GCAddEffect;
        pPacket->setObjectID(pMonsterCorpse->getObjectID());
        pPacket->setEffectID(pEffect->getSendEffectClass());
        pPacket->setDuration(65000);

        if (pPackets == NULL)
            pPackets = new list<Packet*>;
        pPackets->push_back(pPacket);
    }

    pPackets = getRelicEffectPacket(pMonsterCorpse, Effect::EFFECT_CLASS_SLAYER_REGEN_ZONE, pPackets);
    //	pPackets = getRelicEffectPacket( pMonsterCorpse, Effect::EFFECT_CLASS_VAMPIRE_REGEN_ZONE, pPackets );
    //	pPackets = getRelicEffectPacket( pMonsterCorpse, Effect::EFFECT_CLASS_OUSTERS_REGEN_ZONE, pPackets );
    //	pPackets = getRelicEffectPacket( pMonsterCorpse, Effect::EFFECT_CLASS_DEFAULT_REGEN_ZONE, pPackets );
    pPackets = getRelicEffectPacket(pMonsterCorpse, Effect::EFFECT_CLASS_SLAYER_TRYING_1, pPackets);
    pPackets = getRelicEffectPacket(pMonsterCorpse, Effect::EFFECT_CLASS_VAMPIRE_TRYING_1, pPackets);
    pPackets = getRelicEffectPacket(pMonsterCorpse, Effect::EFFECT_CLASS_OUSTERS_TRYING_1, pPackets);

    return pPackets;
}

//////////////////////////////////////////////////////////////////////////////
// sendRelicEffect( MonsterCorpse* )
//////////////////////////////////////////////////////////////////////////////
// pMonsterCorpse에 붙은 Effect를 pPlayer에게 보낸다.
//////////////////////////////////////////////////////////////////////////////
void sendRelicEffect(MonsterCorpse* pMonsterCorpse, Player* pPlayer) {
    list<Packet*>* pPackets = createRelicEffect(pMonsterCorpse);

    if (pPackets != NULL) {
        list<Packet*>::iterator itr = pPackets->begin();
        for (; itr != pPackets->end(); itr++) {
            Packet* pPacket = *itr;
            pPlayer->sendPacket(pPacket);

            SAFE_DELETE(pPacket);
        }
        SAFE_DELETE(pPackets);
    }
}

//////////////////////////////////////////////////////////////////////////////
// sendRelicEffect( MonsterCorpse* )
//////////////////////////////////////////////////////////////////////////////
// pMonsterCorpse에 붙은 Effect를 (x,y)에 뿌린다.
//////////////////////////////////////////////////////////////////////////////
void sendRelicEffect(MonsterCorpse* pMonsterCorpse, Zone* pZone, ZoneCoord_t x, ZoneCoord_t y) {
    list<Packet*>* pPackets = createRelicEffect(pMonsterCorpse);

    if (pPackets != NULL) {
        list<Packet*>::iterator itr = pPackets->begin();
        for (; itr != pPackets->end(); itr++) {
            Packet* pPacket = *itr;
            pZone->broadcastPacket(x, y, pPacket);

            SAFE_DELETE(pPacket);
        }
        SAFE_DELETE(pPackets);
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
Zone::Zone(ZoneID_t zoneID)

{
    m_Mutex.setName("Zone");
    m_MutexEffect.setName("ZoneEffect");

    m_ZoneID = zoneID;
    m_pZoneGroup = NULL;
    m_Width = 0;
    m_Height = 0;
    m_pTiles = NULL;
    m_NPCCount = 0;
    m_MonsterCount = 0;
    m_pPCManager = new PCManager();
    m_pNPCManager = new NPCManager();
    m_pMonsterManager = new MonsterManager(this);
    m_pMasterLairManager = NULL;
    m_pWarScheduler = NULL;
    m_pLevelWarManager = NULL;

    //	m_pEventMonsterManager    = new EventMonsterManager(this);

    m_pWeatherManager = new WeatherManager(this);
    m_pEffectManager = new EffectManager();
    m_pLockedEffectManager = new EffectManager();
    m_pVampirePortalManager = new EffectManager();
    m_pEffectScheduleManager = new EffectScheduleManager();
    m_pLocalPartyManager = new LocalPartyManager();
    m_pPartyInviteInfoManager = new PartyInviteInfoManager();
    m_pTradeManager = new TradeManager;

    m_bPayPlay = false;
    m_bPremiumZone = false;
    m_bPKZone = false;
    m_bNoPortalZone = false;
    m_bMasterLair = false;
    m_bCastle = false;
    m_bHolyLand = false;
    m_bHasRelicTable = false;

    getCurrentTime(m_LoadValueStartTime);

    m_LoadValue = 0;

    m_bTimeStop = false;

    getCurrentTime(m_UpdateTimebandTime);

    m_pDynamicZone = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
Zone::Zone(ZoneID_t zoneID, ZoneCoord_t width, ZoneCoord_t height)

{
    __BEGIN_TRY

    Assert(false);
    /*
    m_Mutex.setName("Zone");
    m_MutexEffect.setName("ZoneEffect");

    m_ZoneID     = zoneID;
    m_pZoneGroup = NULL;
    m_Width      = width;
    m_Height     = height;

    getCurrentTime( m_LoadValueStartTime );

    m_pTiles     = NULL;

    Assert(m_ZoneID > 0);

    m_pTiles = new Tile* [ m_Width ];
    for (uint i = 0 ; i < m_Width ; i++) m_pTiles[i] = new Tile [m_Height];

    m_ppLevel = new (ZoneLevel_t*)[ m_Width ];
    for (uint i = 0; i < m_Width ; i++) m_ppLevel[i] = new ZoneLevel_t[m_Height];

    m_pPCManager              = new PCManager();
    m_pNPCManager             = new NPCManager();
    m_pMonsterManager         = new MonsterManager(this);
    m_pMasterLairManager         = NULL;
    m_pWarScheduler         = NULL;

    m_pEventMonsterManager    = new EventMonsterManager(this);

    m_pWeatherManager         = new WeatherManager(this);
    m_pEffectManager          = new EffectManager();
    m_pLockedEffectManager    = new EffectManager();
    m_pVampirePortalManager   = new EffectManager();
    m_pEffectScheduleManager  = new EffectScheduleManager();
    m_pLocalPartyManager      = new LocalPartyManager();
    m_pPartyInviteInfoManager = new PartyInviteInfoManager();
    m_pTradeManager           = new TradeManager;
    m_pDynamicZone = NULL;
    */

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
Zone::~Zone()

{
    __BEGIN_TRY

    if (m_pTiles != NULL) {
        for (uint i = 0; i < m_Width; i++)
            SAFE_DELETE_ARRAY(m_pTiles[i]);
        SAFE_DELETE_ARRAY(m_pTiles);
    }

    if (m_ppLevel != NULL) {
        for (uint i = 0; i < m_Width; i++)
            SAFE_DELETE_ARRAY(m_ppLevel[i]);
        SAFE_DELETE_ARRAY(m_ppLevel);
    }

    if (m_pSectors != NULL) {
        for (int i = 0; i < m_SectorWidth; i++)
            SAFE_DELETE_ARRAY(m_pSectors[i]);
        SAFE_DELETE_ARRAY(m_pSectors);
    }

    SAFE_DELETE(m_pPCManager);
    SAFE_DELETE(m_pNPCManager);
    SAFE_DELETE(m_pMonsterManager);
    SAFE_DELETE(m_pMasterLairManager);
    SAFE_DELETE(m_pWarScheduler);

    //	SAFE_DELETE(m_pEventMonsterManager);

    SAFE_DELETE(m_pWeatherManager);
    SAFE_DELETE(m_pEffectManager);
    SAFE_DELETE(m_pLockedEffectManager);
    SAFE_DELETE(m_pVampirePortalManager);
    SAFE_DELETE(m_pEffectScheduleManager);
    SAFE_DELETE(m_pLocalPartyManager);
    SAFE_DELETE(m_pPartyInviteInfoManager);

    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////////////
// 타일에 지정된 존 레벨을 리턴한다.
//////////////////////////////////////////////////////////////////////////////
ZoneLevel_t Zone::getZoneLevel(ZoneCoord_t x, ZoneCoord_t y) const

{
    __BEGIN_TRY

    // Assert(x < m_Width && y < m_Height);

    // assert 제거.
    // 이 값이 한계를 넘어서 assert나서 죽었다.
    // 이렇게 가도 무리가 없을 듯..
    // by sigi. 2002.8.13
    if (x < m_Width && y < m_Height) {
        return m_ppLevel[x][y];
    }

    return COMPLETE_SAFE_ZONE;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// getTile
//////////////////////////////////////////////////////////////////////////////
const Tile& Zone::getTile(ZoneCoord_t x, ZoneCoord_t y) const

{
    __BEGIN_TRY

    Assert(x < m_Width && y < m_Height);
    return m_pTiles[x][y];

    __END_CATCH
}

Tile& Zone::getTile(ZoneCoord_t x, ZoneCoord_t y)

{
    __BEGIN_TRY

    Assert(x < m_Width && y < m_Height);
    return m_pTiles[x][y];

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// getSector
//////////////////////////////////////////////////////////////////////////////
Sector* Zone::getSector(ZoneCoord_t x, ZoneCoord_t y)

{
    __BEGIN_TRY

    Assert(x < m_Width && y < m_Height);

    int sx = x / SECTOR_SIZE;
    int sy = y / SECTOR_SIZE;

    Assert(sx < m_SectorWidth && y < m_SectorHeight);

    return &(m_pSectors[sx][sy]);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// 특정 위치에 아이템을 떨어뜨린다.
// Zone ::addItem()
// 7x7 영역을 검사해서 빈칸이 존재하면 떨어뜨린다. 문제는 재수없는 경우 빈칸이
// 존재하지 않을 경우인데.. 이때 예외를 던짐으로써 그 처리를 상위에게 맡기면
// 될 듯...
//--------------------------------------------------------------------------------
TPOINT Zone::addItem(Item* pItem, ZoneCoord_t cx, ZoneCoord_t cy, bool bAllowCreature, Turn_t decayTurn,
                     ObjectID_t DropPetOID)

{
    __BEGIN_TRY

    __BEGIN_DEBUG

    TPOINT pt;

    __BEGIN_PROFILE_ZONE("Z_ADD_ITEM")

    Item::ItemClass IClass = pItem->getItemClass();

    bool bAllowSafeZone = true;

    if (isRelicItem(IClass) || pItem->isFlagItem())
        bAllowSafeZone = false;

    bool bDropForce = false;
    if (pItem->isFlag(Effect::EFFECT_CLASS_DROP_FORCE)) {
        pItem->removeFlag(Effect::EFFECT_CLASS_DROP_FORCE);
        bDropForce = true;
    }
    pt = findSuitablePositionForItem(this, cx, cy, bAllowCreature, bAllowSafeZone, bDropForce);

    // 놓을 위치를 찾아낸 경우
    if (pt.x != -1) {
        m_pTiles[pt.x][pt.y].addItem(pItem);
        addToItemList(pItem);

        if (IClass == Item::ITEM_CLASS_CORPSE) {
            ItemType_t itemType = pItem->getItemType();

            Turn_t DelayTime = 0;

            bool isShrine = false;
            bool isFlag = false;

            if (itemType == SLAYER_CORPSE) {
                SlayerCorpse* pSlayerCorpse = dynamic_cast<SlayerCorpse*>(pItem);
                pSlayerCorpse->setXY(pt.x, pt.y);

                GCAddSlayerCorpse gcAddSlayerCorpse;
                makeGCAddSlayerCorpse(&gcAddSlayerCorpse, pSlayerCorpse);
                broadcastPacket(pt.x, pt.y, &gcAddSlayerCorpse);

                // 마스터 레어에서는 시체가 빨리 사라진다.
                if (isMasterLair())
                    DelayTime = DELAY_MASTER_LAIR_DECAY_CORPSE;
                else
                    DelayTime = 6000;
            } else if (itemType == VAMPIRE_CORPSE) {
                VampireCorpse* pVampireCorpse = dynamic_cast<VampireCorpse*>(pItem);
                pVampireCorpse->setXY(pt.x, pt.y);

                GCAddVampireCorpse gcAddVampireCorpse;
                makeGCAddVampireCorpse(&gcAddVampireCorpse, pVampireCorpse);
                broadcastPacket(pt.x, pt.y, &gcAddVampireCorpse);

                // 마스터 레어에서는 시체가 빨리 사라진다.
                if (isMasterLair())
                    DelayTime = DELAY_MASTER_LAIR_DECAY_CORPSE;
                else
                    DelayTime = 6000;
            } else if (itemType == OUSTERS_CORPSE) {
                OustersCorpse* pOustersCorpse = dynamic_cast<OustersCorpse*>(pItem);
                pOustersCorpse->setXY(pt.x, pt.y);

                GCAddOustersCorpse gcAddOustersCorpse;
                makeGCAddOustersCorpse(&gcAddOustersCorpse, pOustersCorpse);
                broadcastPacket(pt.x, pt.y, &gcAddOustersCorpse);

                // 마스터 레어에서는 시체가 빨리 사라진다.
                if (isMasterLair())
                    DelayTime = DELAY_MASTER_LAIR_DECAY_CORPSE;
                else
                    DelayTime = 6000;
            } else if (itemType == NPC_CORPSE) {
                Assert(false);
            } else if (itemType == MONSTER_CORPSE) {
                MonsterCorpse* pMonsterCorpse = dynamic_cast<MonsterCorpse*>(pItem);
                GCAddMonsterCorpse gcAddMonsterCorpse;
                makeGCAddMonsterCorpse(&gcAddMonsterCorpse, pMonsterCorpse, pt.x, pt.y);
                broadcastPacket(pt.x, pt.y, &gcAddMonsterCorpse);

                isFlag = g_pFlagManager->isFlagPole(pMonsterCorpse);

                // 마스터 레어에서는 시체가 빨리 사라진다.
                if (isMasterLair()) {
                    MonsterType_t mt = pMonsterCorpse->getMonsterType();
                    const MonsterInfo* pMonsterInfo = g_pMonsterInfoManager->getMonsterInfo(mt);
                    Assert(pMonsterInfo != NULL);

                    // 마스터 시체인 경우는 더 빨리 사라진다.
                    if (pMonsterInfo->isMaster()) {
                        // 아이템이 없어서 더 빨리 사라지기 때문이다. * 10
                        DelayTime = DELAY_MASTER_LAIR_DECAY_MASTER_CORPSE * 10;
                    } else {
                        DelayTime = DELAY_MASTER_LAIR_DECAY_CORPSE;
                    }
                } else
                    DelayTime = 600;

                sendRelicEffect(pMonsterCorpse, this, pt.x, pt.y);

                // 이건 임시다. -_-;
                // 원래 item에는 zone좌표가 들어가지 않는데
                // 특별히 성물보관대에는 필요하기 때문에..
                pMonsterCorpse->setX(pt.x);
                pMonsterCorpse->setY(pt.y);
                pMonsterCorpse->setZone(this);

                isShrine = pMonsterCorpse->isShrine() && !g_pFlagManager->isFlagPole(pMonsterCorpse);

                // Shrine인 경우 미니맵에 보여준다.
                if (isShrine) {
                    NPCInfo* pNPCInfo = new NPCInfo();
                    pNPCInfo->setName(pMonsterCorpse->getName());
                    pNPCInfo->setNPCID(pMonsterCorpse->getMonsterType());
                    pNPCInfo->setX(pt.x);
                    pNPCInfo->setY(pt.y);

                    addNPCInfo(pNPCInfo);
                }
            } else {
                Assert(false);
            }

            // 아이템이 들어가있지 않은 시체라면 딜레이 시간을 줄인다.
            Corpse* pCorpse = dynamic_cast<Corpse*>(pItem);
            if (pCorpse->getTreasureCount() == 0) {
                DelayTime = DelayTime / 10;
            }
            // Relic인 경우에는 시간의 지연에 따라 아이템이 사라지지 않는다.
            if (!isShrine && !isFlag && !pCorpse->isFlag(Effect::EFFECT_CLASS_SLAYER_RELIC_TABLE) &&
                !pCorpse->isFlag(Effect::EFFECT_CLASS_VAMPIRE_RELIC_TABLE) &&
                !pCorpse->isFlag(Effect::EFFECT_CLASS_SHRINE_GUARD) &&
                !pCorpse->isFlag(Effect::EFFECT_CLASS_SHRINE_HOLY) && pCorpse->getTreasureCount() < 200) {
                // 강제로 지정한 delay
                if (decayTurn != 0)
                    DelayTime = decayTurn;

                // 바닥에 떨어지는 아이템은 일정 시간이 지나면 사라지게 된다.
                EffectDecayCorpse* pEffectDecayCorpse =
                    new EffectDecayCorpse(this, pt.x, pt.y, (Corpse*)pItem, DelayTime);
                //				pEffectDecayCorpse->setNextTime(999999);
                m_ObjectRegistry.registerObject(pEffectDecayCorpse);
                addEffect(pEffectDecayCorpse);
            } else {
                // 깃대인 경우엔 block되면 안된다.
                if (!isFlag) {
                    // 성물 보관대는 아이템(시체)이지만
                    // Block이 되어야 한다.
                    Tile& rTile = getTile(pt.x, pt.y);

                    rTile.setBlocked(Creature::MOVE_MODE_WALKING);
                    rTile.setBlocked(Creature::MOVE_MODE_BURROWING);

                    // 성물 보관대의 정보를 저장한다.
                    m_RelicTableOID = pCorpse->getObjectID();
                    m_RelicTableX = pt.x;
                    m_RelicTableY = pt.y;

                    // cout << "Relic인 경우에는 시체가 사라지지 않습니다" << endl;
                }
            }
        } else {
            GCDropItemToZone gcDropItemToZone;
            makeGCDropItemToZone(&gcDropItemToZone, pItem, pt.x, pt.y);
            gcDropItemToZone.setDropPetOID(DropPetOID);

            broadcastPacket(pt.x, pt.y, &gcDropItemToZone);

            // 모터사이클은 시간이 지나도 사라지지 않는다.
            if (IClass == Item::ITEM_CLASS_MOTORCYCLE) {
                // transport인 경우를 대비해서 체크해제해야한다.
                MotorcycleBox* pMotorcycleBox = g_pParkingCenter->getMotorcycleBox(pItem->getItemID());

                if (pMotorcycleBox != NULL) {
                    Motorcycle* pMotorcycle = pMotorcycleBox->getMotorcycle();
                    Assert(pMotorcycle != NULL);

                    // 아이템 저장 최적화. by sigi. 2002.5.15
                    char pField[80];
                    sprintf(pField, "OwnerID='', Storage=%d, StorageID=%u, X=%d, Y=%d", STORAGE_ZONE, getZoneID(),
                            (int)pt.x, (int)pt.y);

                    pMotorcycle->tinysave(pField);

                    pMotorcycleBox->setZone(this);
                    pMotorcycleBox->setX(pt.x);
                    pMotorcycleBox->setY(pt.y);

                    pMotorcycleBox->setTransport(false);
                }
            } else if (isRelicItem(IClass)) {
                // relic은 사라지지 않는다.
                addEffectRelicPosition(pItem, getZoneID(), pt);
                char pField[80];
                sprintf(pField, "OwnerID='', Storage=%d, StorageID=%u, X=%d, Y=%d", STORAGE_ZONE, getZoneID(), pt.x,
                        pt.y);
                pItem->tinysave(pField);
            } else {
                // 2002.10.30 장홍창
                // 아이템 삭제 시간을 현행 10분에서 3분으로 줄인다.
                // Turn_t DelayTime = 6000;
                Turn_t DelayTime = 1800;

                // 마스터 레어에서는 아이템이 빨리 사라진다.
                if (isMasterLair()) {
                    DelayTime = DELAY_MASTER_LAIR_DECAY_ITEM;
                }

                if (!pItem->isFlagItem() && IClass != Item::ITEM_CLASS_SWEEPER) {
                    // 강제로 지정한 delay
                    if (decayTurn != 0)
                        DelayTime = decayTurn;

                    // 바닥에 떨어지는 아이템은 일정 시간이 지나면 사라지게 된다.
                    EffectDecayItem* pEffectDecayItem = new EffectDecayItem(this, pt.x, pt.y, (Item*)pItem, DelayTime);
                    pEffectDecayItem->setNextTime(999999);
                    m_ObjectRegistry.registerObject(pEffectDecayItem);
                    addEffect(pEffectDecayItem);
                } else {
                    char pField[80];
                    sprintf(pField, "OwnerID='', Storage=%d, StorageID=%u, X=%d, Y=%d", STORAGE_ZONE, getZoneID(), pt.x,
                            pt.y);
                    pItem->tinysave(pField);
                }
            }
        }

        return pt;
    } else {
        TPOINT pt_error;
        pt_error.x = -1;
        pt_error.y = -1;

        return pt_error;
    }

    __END_PROFILE_ZONE("Z_ADD_ITEM")
    return pt;

    __END_DEBUG
    __END_CATCH
}


//--------------------------------------------------------------------------------
// get Item
//--------------------------------------------------------------------------------
Item* Zone::getItem(ObjectID_t id) const

{
    unordered_map<ObjectID_t, Item*>::const_iterator iItem = m_Items.find(id);

    if (iItem != m_Items.end()) {
        return iItem->second;
    }

    return NULL;
}


//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void Zone::addEffect(Effect* pEffect)

{
    __BEGIN_TRY

    Assert(pEffect != NULL);

    m_pEffectManager->addEffect(pEffect);

    __END_CATCH
}

void Zone::deleteEffect(ObjectID_t id)

{
    __BEGIN_TRY

    m_pEffectManager->deleteEffect(id);

    __END_CATCH
}

Effect* Zone::findEffect(Effect::EffectClass eid)

{
    __BEGIN_TRY

    return m_pEffectManager->findEffect(eid);

    __END_CATCH
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void Zone::addEffect_LOCKING(Effect* pEffect)

{
    __BEGIN_TRY

    Assert(pEffect != NULL);

    __ENTER_CRITICAL_SECTION(m_MutexEffect)

    m_pLockedEffectManager->addEffect(pEffect);

    __LEAVE_CRITICAL_SECTION(m_MutexEffect)

    __END_CATCH
}

void Zone::deleteEffect_LOCKING(ObjectID_t id)

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_MutexEffect)

    m_pLockedEffectManager->deleteEffect(id);

    __LEAVE_CRITICAL_SECTION(m_MutexEffect)

    __END_CATCH
}


//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
void Zone::deleteItem(Object* pObject, ZoneCoord_t x, ZoneCoord_t y)

{
    __BEGIN_TRY

    __BEGIN_PROFILE_ZONE("Z_DELETE_ITEM")

    deleteFromItemList(pObject->getObjectID());

    //--------------------------------------------------
    // 존에서 객체를 삭제한다.
    //--------------------------------------------------
    getTile(x, y).deleteItem();


    if (pObject->getObjectClass() == Object::OBJECT_CLASS_ITEM) {
        // 성물 보관함일 경우 Block 을 해제해야 한다.
        Item* pItem = dynamic_cast<Item*>(pObject);
        Assert(pItem != NULL);
        if (pItem->getItemClass() == Item::ITEM_CLASS_CORPSE && pItem->getItemType() == MONSTER_CORPSE) {
            MonsterCorpse* pCorpse = dynamic_cast<MonsterCorpse*>(pItem);
            Assert(pCorpse != NULL);

            if (pCorpse->isFlag(Effect::EFFECT_CLASS_SLAYER_RELIC_TABLE) ||
                pCorpse->isFlag(Effect::EFFECT_CLASS_VAMPIRE_RELIC_TABLE) ||
                pCorpse->isFlag(Effect::EFFECT_CLASS_SHRINE_GUARD) ||
                pCorpse->isFlag(Effect::EFFECT_CLASS_SHRINE_HOLY)) {
                Tile& rTile = getTile(x, y);

                // 블록 날리기
                rTile.clearBlocked(Creature::MOVE_MODE_WALKING);
                rTile.clearBlocked(Creature::MOVE_MODE_BURROWING);
            }
        }

        if (isRelicItem(pItem)) {
            deleteEffectRelicPosition(pItem);
        }
    }

    //--------------------------------------------------
    // 주변의 PC들에게 객체가 사라졌다는 사실을 브로드캐스트한다.
    //--------------------------------------------------
    //	GCDeleteObject gcDeleteObject(pObject->getObjectID());

    //	broadcastPacket(x, y, &gcDeleteObject);

    __END_PROFILE_ZONE("Z_DELETE_ITEM")

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// 일정 주기마다 해줘야 하는 기능들을 여기에 추가하도록 한다.
//////////////////////////////////////////////////////////////////////////////
void Zone::heartbeat()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    try {
        __ENTER_CRITICAL_SECTION(m_Mutex)

        beginProfileEx("Z_PCQUEUE");

        // PCQueue의 PC를 존에 추가한다.
        while (!m_PCListQueue.empty()) {
            Creature* pCreature = m_PCListQueue.front();
            Assert(pCreature != NULL);
            Assert(pCreature->getZone() == this);

            // 존에 추가하고, 주변 PC들에게 브로드캐스트한다.
            addPC(pCreature, pCreature->getX(), pCreature->getY(), DOWN);

            m_PCListQueue.pop_front();
        }

        endProfileEx("Z_PCQUEUE");

        beginProfileEx("Z_PC");
        m_pPCManager->processCreatures(); // process all PC
        endProfileEx("Z_PC");

        // 마스터 레어 매니저가 있다면 마스터 레어이다
        // by sigi. 2002.9.2
        if (m_pMasterLairManager != NULL)
            m_pMasterLairManager->heartbeat(); // process master lair

        // WarScheduler가 있다면 성이지..
        // by sigi. 2003.1.24
        if (m_pWarScheduler != NULL && g_pVariableManager->isWarActive()) {
            Work* pWork = m_pWarScheduler->heartbeat();

            if (pWork != NULL) {
                War* pWar = dynamic_cast<War*>(pWork);
                Assert(pWar != NULL);

                g_pWarSystem->addWarDelayed(pWar);
            }
        }

        if (m_pLevelWarManager != NULL && g_pVariableManager->isActiveLevelWar()) {
            m_pLevelWarManager->heartbeat();
            // LevelWar Zone 에는 시간 별로 유료 무료 사용자 출입제한이 이상해서 해줘야 함.
            m_pLevelWarManager->freeUserTimeCheck();
        }

        //		if ( m_pLevelWarManager != NULL )
        //		{
        //		}

        // player가 있어야 monster를 heartbeat한다.
        // 즉, player가 없는 zone은 monster가 가만히 있는다.
        // monster의 EffectManager가 안 돌아가므로 문제가 될 수도 있지만,
        // 크게 문제가 없다고 보고.. -_-; .. by sigi. 2002.5.6
        // if ( m_ZoneID >= 1121 && m_ZoneID <= 1124)
        //	m_pCombatMonsterManager->processCreatures(); // 전투용 몬스터의 AI를 처리하는 부분, 김경석
        // else
        //{
        if (getPCCount() > 0 || (isDynamicZone() && (m_pDynamicZone->getStatus() == DYNAMIC_ZONE_STATUS_RUNNING))) {
            beginProfileEx("Z_MONSTER");
            m_pMonsterManager->processCreatures(); // process all monsters
            endProfileEx("Z_MONSTER");
        }

        //			m_pEventMonsterManager->processCreatures();
        //}

        beginProfileEx("Z_NPC");
        m_pNPCManager->processCreatures(); // process all npcs
        endProfileEx("Z_NPC");

        beginProfileEx("Z_ESCH");
        // 먼저 이펙트 스케쥴을 먼저 실행시킨다.
        m_pEffectScheduleManager->heartbeat();
        endProfileEx("Z_ESCH");

        // Item의 EffectManager에서 getCurrentTime을 호출하지 않게 하기 위해서.
        // by sigi. 2002.5.8
        Timeval currentTime;
        getCurrentTime(currentTime);

        // Delete expired effects
        beginProfileEx("Z_EFFECT");
        m_pVampirePortalManager->heartbeat(currentTime);

        __ENTER_CRITICAL_SECTION(m_MutexEffect)
        m_pLockedEffectManager->heartbeat(currentTime);
        __LEAVE_CRITICAL_SECTION(m_MutexEffect)

        // Debug: 统计 zone 的 effects
        static time_t lastZoneLogTime = 0;
        size_t zoneEffects = m_pEffectManager->getSize();

        m_pEffectManager->heartbeat(currentTime);

        endProfileEx("Z_EFFECT");

        beginProfileEx("Z_WEATHER");
        // weather changing...
        m_pWeatherManager->heartbeat();
        endProfileEx("Z_WEATHER");

        beginProfileEx("Z_ITEM");
        // item heartbeaet

        int i = 0;
        size_t totalItemEffects = 0;

        for (unordered_map<ObjectID_t, Item*>::iterator itr = m_Items.begin(); itr != m_Items.end(); itr++) {
            Item* pItem = itr->second;
            Assert(pItem != NULL);

            // by sigi. for debugging. 2002.12.23
            m_LastItemClass = (int)pItem->getItemClass();

            EffectManager& rEffectManager = pItem->getEffectManager();
            totalItemEffects += rEffectManager.getSize();
            rEffectManager.heartbeat(currentTime);
            i++;
        }

        endProfileEx("Z_ITEM");

        beginProfileEx("Z_PARTY");
        // party heartbeat
        m_pLocalPartyManager->heartbeat();
        endProfileEx("Z_PARTY");

        __LEAVE_CRITICAL_SECTION(m_Mutex)

        Timeval currentTime;
        getCurrentTime(currentTime);

        // time band 를 갱신한다.
        if (m_UpdateTimebandTime < currentTime) {
            if (!m_bTimeStop) {
                m_Timeband = g_pTimeManager->getTimeband();
            }

            // 5초마다 timeband 를 갱신한다. 게임 시간으로 2분
            m_UpdateTimebandTime.tv_sec += 5;
        }

        // DynamicZone 일 경우
        if (isDynamicZone())
            m_pDynamicZone->heartbeat();
    } catch (Throwable& t) {
        filelog("ZoneBug.txt", "%s : %s", "Zone::heartbeat(2)", t.toString().c_str());
        cerr << t.toString() << endl;
        throw;
    }

    m_LoadValue++;

    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// PCManager, MonsterManager, NPCManager 에서 지정된 OID 를 가진 크리처를
// 찾아서 리턴한다. 없을 경우 NoSuchElementException 을 던진다.
//
// 이 메쏘드는 찾고자 하는 크리처의 타입(PC,NPC,Monster)를 모를 경우에
// 사용한다. 웬만하면, 타입을 알아내서 getCreature(Creature::CreatureClass,ObjectID_t)
// 메쏘드를 사용하도록 한다.
//////////////////////////////////////////////////////////////////////////////
Creature* Zone::getCreature(ObjectID_t objectID) const
// NoSuchElementException, Error)
{
    __BEGIN_TRY

    // NoSuchElementException을 안 쓰는 버전 by sigi. 2002.5.2
    Creature* pCreature = NULL;

    pCreature = m_pMonsterManager->getCreature(objectID);

    if (pCreature == NULL) {
        pCreature = m_pPCManager->getCreature(objectID);

        if (pCreature == NULL) {
            pCreature = m_pNPCManager->getCreature(objectID);

            //			if (pCreature==NULL)
            //			{
            //				pCreature = m_pEventMonsterManager->getCreature(objectID);
            //			}
        }
    }

    return pCreature;

    /*
    try
    {
        return m_pMonsterManager->getCreature(objectID);
    }
    catch (NoSuchElementException)
    {
        // not exist? go next
    }
    */

    /*
    //#ifdef __XMAS_EVENT_CODE__
        try
        {
            return m_pEventMonsterManager->getCreature(objectID);
        }
        catch (NoSuchElementException)
        {
            // not exist? go next
        }
    //#endif
    */
    /*
    try
    {
        return m_pPCManager->getCreature(objectID);
    }
    catch (NoSuchElementException)
    {
        // not exist? go next
    }

    try
    {
        return m_pNPCManager->getCreature(objectID);
    }
    catch (NoSuchElementException)
    {
        throw;
    }
    */

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// PCManager, MonsterManager, NPCManager 에서 지정된 Name을 가진 크리처를 찾아서
// 리턴한다. 없을 경우 NoSuchElementException 을 던진다.
//
// 이 메쏘드는 찾고자 하는 크리처의 타입(PC,NPC,Monster)를 모를 경우에 사용한다.
// 웬만하면, 타입을 알아내서 getCreature(Creature::CreatureClass,Name)
// 메쏘드를 사용하도록 한다.
//////////////////////////////////////////////////////////////////////////////
Creature* Zone::getCreature(const string& Name) const
// NoSuchElementException, Error)
{
    __BEGIN_TRY

    // NoSuchElementException을 안 쓰는 버전 by sigi. 2002.5.2
    Creature* pCreature = NULL;

    pCreature = m_pPCManager->getCreature(Name);

    if (pCreature == NULL) {
        pCreature = m_pMonsterManager->getCreature(Name);

        if (pCreature == NULL) {
            pCreature = m_pNPCManager->getCreature(Name);

            //			if(pCreature==NULL)
            //			{
            //				pCreature = m_pEventMonsterManager->getCreature(Name);
            //			}
        }
    }

    return pCreature;

    /*
    try
    {
        return m_pPCManager->getCreature(Name);
    }
    catch (NoSuchElementException)
    {
        // not exist? go next
    }

    try
    {
        return m_pMonsterManager->getCreature(Name);
    }
    catch (NoSuchElementException)
    {
        // not exist? go next
    }
    */

    /*
    #ifdef __XMAS_EVENT_CODE__
        try
        {
            return m_pEventMonsterManager->getCreature(Name);
        }
        catch (NoSuchElementException)
        {
            // not exist? go next
        }
    #endif
    */

    /*
    try
    {
        return m_pNPCManager->getCreature(Name);
    }
    catch (NoSuchElementException)
    {
        throw;
    }
    */

    __END_CATCH
}

//--------------------------------------------------------------------------------
//
// 존에서 특정 OID를 가진 특정 크리처 타입을 가진 크리처를 찾아서 리턴한다.
//
//--------------------------------------------------------------------------------
Creature* Zone::getCreature(Creature::CreatureClass creatureClass, ObjectID_t objectID) const

{
    __BEGIN_TRY

    if (creatureClass == Creature::CREATURE_CLASS_SLAYER) {
        return m_pPCManager->getCreature(objectID);
    } else if (creatureClass == Creature::CREATURE_CLASS_VAMPIRE) {
        return m_pPCManager->getCreature(objectID);
    } else if (creatureClass == Creature::CREATURE_CLASS_OUSTERS) {
        return m_pPCManager->getCreature(objectID);
    } else if (creatureClass == Creature::CREATURE_CLASS_NPC) {
        return m_pNPCManager->getCreature(objectID);
    } else if (creatureClass == Creature::CREATURE_CLASS_MONSTER) {
        return m_pMonsterManager->getCreature(objectID);
    }
    /*
    #ifdef __XMAS_EVENT_CODE__
            try
            {
                return m_pMonsterManager->getCreature(objectID);
            }
            catch (NoSuchElementException& nsee)
            {
            }

            return m_pEventMonsterManager->getCreature(objectID);
    #else
    */
    /*
    #endif
    */

    return NULL; // evade warning.

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// get debug string
//////////////////////////////////////////////////////////////////////////////
string Zone::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "Zone(" << "ZoneID:" << (int)m_ZoneID << ",ZoneGroupID:" << (int)m_pZoneGroup->getZoneGroupID()
        << ",ZoneType:" << (int)m_ZoneType << ",ZoneLevel:" << (int)m_ZoneLevel
        << ",ZoneAccessMode:" << (int)m_ZoneAccessMode << ",OwnerID:" << m_OwnerID << ",DarkLevel:" << (int)m_DarkLevel
        << ",LightLevel:" << (int)m_LightLevel << ",WeatherManager:" << m_pWeatherManager->toString();

    msg << ",#NPC:" << (int)m_NPCCount;
    for (uint i = 0; i < m_NPCCount; i++)
        msg << ",NPC[" << i << "] : " << (int)m_NPCTypes[i];

    msg << ",#Monster:" << (int)m_MonsterCount;

    Assert(m_MonsterCount < maxMonsterPerZone); // by sigi
    for (uint i = 0; i < m_MonsterCount; i++)
        msg << ",Monster[" << i << "] : " << (int)m_MonsterTypes[i];

    msg << ",Width:" << (int)m_Width << ",Height:" << (int)m_Height << ")";

    return msg.toString();

    __END_CATCH
}

void Zone::addToItemList(Item* pItem) {
    __BEGIN_TRY

    m_Items[pItem->getObjectID()] = pItem;

    __END_CATCH
}

void Zone::deleteFromItemList(ObjectID_t id) {
    __BEGIN_TRY

    unordered_map<ObjectID_t, Item*>::iterator itr = m_Items.find(id);

    if (itr == m_Items.end())
    // throw NoSuchElementException();
    //  NoSuch제거. by sigi. 2002.5.3
    {
        return;
    }

    m_Items.erase(itr);

    __END_CATCH
}

void Zone::addVampirePortal(ZoneCoord_t cx, ZoneCoord_t cy, Vampire* pVampire, const ZONE_COORD& ZoneCoord)

{
    __BEGIN_TRY

    Assert(m_OuterRect.ptInRect(cx, cy));
    Assert(pVampire != NULL);

    // 뱀파이어의 능력에 따라 들어갈 수 있는 인원과, 지속 시간을 계산한다.
    // 존에 바로 추가되는 것이 아니므로, 약간의 딜레이를 추가해준다.
    Duration_t duration = (60 + (pVampire->getINT(ATTR_CURRENT) - 20) / 3) * 10 + 20; // 0.1초 단위기 때문에...
    int count = 3 + (pVampire->getINT(ATTR_CURRENT) - 20) / 10;

    // 일단 이펙트 객체 자체를 생성한다.
    EffectVampirePortal* pEffectVampirePortal = new EffectVampirePortal(this, cx, cy);
    pEffectVampirePortal->setDeadline(duration);
    pEffectVampirePortal->setOwnerID(pVampire->getName());
    pEffectVampirePortal->setZoneCoord(ZoneCoord.id, ZoneCoord.x, ZoneCoord.y);
    pEffectVampirePortal->setCount(count);

    // 이펙트 스케쥴을 생성해서 더한다.
    EffectSchedule* pEffectSchedule = new EffectSchedule;
    pEffectSchedule->setEffect(pEffectVampirePortal);
    pEffectSchedule->addWork(WORKCODE_ADD_VAMPIRE_PORTAL, NULL);
    m_pEffectScheduleManager->addEffectSchedule(pEffectSchedule);

    __END_CATCH
}

//-------------------------------------------------------------
// deleteMotorcycle( x, y, pMotorcycle )
//-------------------------------------------------------------
// 바로 지우지 않고.. zone의 heartbeat할때 지우도록
// EffectDecayItem을 붙여둔다.
//-------------------------------------------------------------
void Zone::deleteMotorcycle(ZoneCoord_t cx, ZoneCoord_t cy, Motorcycle* pMotorcycle)

{
    __BEGIN_TRY

    Assert(m_OuterRect.ptInRect(cx, cy));
    Assert(pMotorcycle != NULL);

    EffectDecayItem* pEffectDecayItem = new EffectDecayItem(this, cx, cy, (Item*)pMotorcycle, 0,
                                                            false); // DB에서는 지우지 않는다.
    pEffectDecayItem->setNextTime(999999);
    m_ObjectRegistry.registerObject(pEffectDecayItem);
    addEffect_LOCKING(pEffectDecayItem);

    __END_CATCH
}

/*
void Zone::decayMotorcycle(ZoneCoord_t cx, ZoneCoord_t cy, Motorcycle* pMotorcycle, Slayer* pSlayer)

{
    __BEGIN_TRY

    cout << "Zone::decayMotorcycle	" << endl;

    Assert(m_OuterRect.ptInRect(cx, cy));
    Assert(pMotorcycle != NULL);

    // 존에서 오토바이를 지우는 이펙트를 추가한다.
    EffectDecayMotorcycle* pEffectDecayMotorcycle = new EffectDecayMotorcycle(this, cx, cy, (Item*)pMotorcycle, 0,
                                                                  false); // DB에서는 지우지 않는다.
    pEffectDecayMotorcycle->setNextTime(999999);
    m_ObjectRegistry.registerObject(pEffectDecayMotorcycle);
    addEffect_LOCKING(pEffectDecayMotorcycle);

    __END_CATCH
}
*/

//-------------------------------------------------------------
// transportItemToCorpse
//-------------------------------------------------------------
// 현재 존의 pItem을 pZone의 (cx, cy)로 옮긴다.
// EffectTransportItem을 붙여서 옮긴다.
//-------------------------------------------------------------
void Zone::transportItemToCorpse(Item* pItem, Zone* pTargetZone, ObjectID_t corpseObjectID)

{
    __BEGIN_TRY

    // cout << "transportItemToCorpse : " << (int)pZone->getZoneID() << ", (" << cx << ", " << cy << ")" << endl;
    Assert(pItem != NULL);

    if (pTargetZone->getZoneGroup() == this->getZoneGroup()) {
        // cout << "same zone - to corpse" << endl;
        //  같은 zone이면 바로 옮긴다.
        // deleteFromItemList(pItem->getObjectID());

        Item* pCorpseItem = pTargetZone->getItem(corpseObjectID);

        if (pCorpseItem == NULL) {
            StringStream msg;
            msg << "[" << (int)m_ZoneID << "] 시체가 없네: corpseObjectID=" << (int)corpseObjectID;

            throw Error(msg.toString());
        } else if (pCorpseItem->getItemClass() != Item::ITEM_CLASS_CORPSE) {
            StringStream msg;
            msg << "[" << (int)m_ZoneID << "] 시체가 아니네: corpseObjectID=" << (int)corpseObjectID
                << ", itemClass=" << (int)pCorpseItem->getItemClass()
                << ", itemType=" << (int)pCorpseItem->getItemType();

            throw Error(msg.toString());
        } else {
            Corpse* pCorpse = dynamic_cast<Corpse*>(pCorpseItem);
            Assert(pCorpse != NULL);

            pCorpse->addTreasure(pItem);
        }
    } else {
        // cout << "transportItemToCorpse" << endl;
        EffectTransportItemToCorpse* pEffectTransportItem =
            new EffectTransportItemToCorpse(this, pItem, pTargetZone, corpseObjectID, 0);
        pEffectTransportItem->setNextTime(999999);
        m_ObjectRegistry.registerObject(pEffectTransportItem);
        addEffect_LOCKING(pEffectTransportItem);
    }

    __END_CATCH
}

//-------------------------------------------------------------
// transportItem
//-------------------------------------------------------------
// 현재 존의 pItem을 pZone의 (cx, cy)로 옮긴다.
// EffectTransportItem을 붙여서 옮긴다.
//-------------------------------------------------------------
void Zone::transportItem(ZoneCoord_t x, ZoneCoord_t y, Item* pItem, Zone* pZone, ZoneCoord_t cx, ZoneCoord_t cy)

{
    __BEGIN_TRY

    // cout << "transportItem : " << (int)pZone->getZoneID() << ", (" << cx << ", " << cy << ")" << endl;

    // 이거 잘못해놔가 다운돼다. ㅜ.ㅜ; by sigi
    Assert(m_OuterRect.ptInRect(x, y));
    Assert(pItem != NULL);

    if (pZone->getZoneGroup() == this->getZoneGroup()) {
        // cout << "same zone" << endl;
        //  같은 zone group 이면 바로 옮긴다.
        deleteFromItemList(pItem->getObjectID());
        getTile(x, y).deleteItem();

        // 아이템이 사라졌다는 패킷을 날린다.
        GCDeleteObject gcDeleteObject;
        gcDeleteObject.setObjectID(pItem->getObjectID());

        broadcastPacket(x, y, &gcDeleteObject);

        pZone->getObjectRegistry().registerObject(pItem);
        pZone->addItem(pItem, cx, cy);
    } else {
        // cout << "transportItem" << endl;
        EffectTransportItem* pEffectTransportItem = new EffectTransportItem(this, x, y, pZone, cx, cy, pItem, 0);
        pEffectTransportItem->setNextTime(999999);
        m_ObjectRegistry.registerObject(pEffectTransportItem);
        addEffect_LOCKING(pEffectTransportItem);
    }

    __END_CATCH
}

//-------------------------------------------------------------
// add Item To Corpse Delayed
//-------------------------------------------------------------
// 아이템을 추가하는데.. 다른 thread에서 해도 된다.
// 다른 heartbeat에서 추가된다.
//-------------------------------------------------------------
void Zone::addItemToCorpseDelayed(Item* pItem, ObjectID_t corpseItemID)

{
    __BEGIN_TRY

    Assert(pItem != NULL);

    EffectAddItemToCorpse* pEffectAddItem = new EffectAddItemToCorpse(this, pItem, corpseItemID, 0);
    pEffectAddItem->setNextTime(999999);
    m_ObjectRegistry.registerObject(pEffectAddItem);
    addEffect_LOCKING(pEffectAddItem);

    __END_CATCH
}

//-------------------------------------------------------------
// add Item Delayed
//-------------------------------------------------------------
// 아이템을 추가하는데.. 다른 thread에서 해도 된다.
// 다른 heartbeat에서 추가된다.
//-------------------------------------------------------------
void Zone::addItemDelayed(Item* pItem, ZoneCoord_t cx, ZoneCoord_t cy, bool bAllowCreature)

{
    __BEGIN_TRY

    Assert(m_OuterRect.ptInRect(cx, cy));
    Assert(pItem != NULL);

    EffectAddItem* pEffectAddItem = new EffectAddItem(this, cx, cy, pItem, 0, bAllowCreature);
    pEffectAddItem->setNextTime(999999);
    m_ObjectRegistry.registerObject(pEffectAddItem);
    addEffect_LOCKING(pEffectAddItem);

    __END_CATCH
}

// 아직 테스트 안 해본 코드.
void Zone::deleteItemDelayed(Object* pObject, ZoneCoord_t x, ZoneCoord_t y)

{
    __BEGIN_TRY

    Assert(m_OuterRect.ptInRect(x, y));
    Assert(pObject != NULL);

    EffectDeleteItem* pEffectDeleteItem = new EffectDeleteItem(this, x, y, pObject, 0);
    pEffectDeleteItem->setNextTime(999999);
    m_ObjectRegistry.registerObject(pEffectDeleteItem);
    addEffect_LOCKING(pEffectDeleteItem);

    __END_CATCH
}

//-------------------------------------------------------------
// add Relic Item
//-------------------------------------------------------------
// 아이템을 추가하는데.. 다른 thread에서 해도 된다.
// 다른 heartbeat에서 추가된다.
//-------------------------------------------------------------
bool Zone::addRelicItem(int relicIndex)

{
    __BEGIN_TRY

    // cout << "[addRelicItem] ZoneID=" << (int)m_ZoneID << ", relicIndex=" << relicIndex << endl;

    const RelicInfo* pRelicInfo = dynamic_cast<RelicInfo*>(g_pRelicInfoManager->getItemInfo(relicIndex));

    int cx = pRelicInfo->x;
    int cy = pRelicInfo->y;

    Assert(m_OuterRect.ptInRect(cx, cy));

    // 이미 성물 보관대가 있는 경우
    if (m_bHasRelicTable) {
        return false;
        // 현재 존의 성물 보관대를 찾는다. (addItem될때 Zone에 좌표 x,y를 기억해두자)
        // 성물 보관대에 아무런 성물도 없다면 return
        // 아니면, 자기 성물이 아닌 성물을 원래의 성물보관대에 넣는다.
        // addItemDelayed를 사용해서 원래의 Zone에 추가해버리면 된다.
    } else {
        // Monster를 생성한다.
        Monster* pMonster = NULL;
        try {
            pMonster = new Monster(pRelicInfo->monsterType);

            m_ObjectRegistry.registerObject(pMonster);

        } catch (Throwable&) {
            SAFE_DELETE(pMonster);
            return false;
        }

        // cout << "new Monster OK" << endl;

        // MonsterCorpse를 생성한다. (성물 보관대)
        MonsterCorpse* pMonsterCorpse = NULL;
        try {
            pMonsterCorpse = new MonsterCorpse(pMonster);
            pMonsterCorpse->setDir(2);
            pMonsterCorpse->setZone(this);
            pMonsterCorpse->setX(cx);
            pMonsterCorpse->setY(cy);
            Assert(pMonsterCorpse != NULL);
        } catch (Throwable& t) {
            // cout << t.toString().c_str() << endl;
        }

        // cout << "new MonsterCorpse OK" << endl;

        if (pRelicInfo->relicType == RELIC_TYPE_SLAYER) {
            Effect* pRelicTable = new EffectSlayerRelicTable(pMonsterCorpse);
            pRelicTable->setNextTime(999999);
            m_ObjectRegistry.registerObject(pRelicTable);

            pMonsterCorpse->getEffectManager().addEffect(pRelicTable);
            pMonsterCorpse->setFlag(Effect::EFFECT_CLASS_SLAYER_RELIC_TABLE);

            g_pCombatInfoManager->setRelicOwner(relicIndex, CombatInfoManager::RELIC_OWNER_SLAYER);
        } else {
            Effect* pRelicTable = new EffectVampireRelicTable(pMonsterCorpse);
            pRelicTable->setNextTime(999999);
            m_ObjectRegistry.registerObject(pRelicTable);

            pMonsterCorpse->getEffectManager().addEffect(pRelicTable);
            pMonsterCorpse->setFlag(Effect::EFFECT_CLASS_VAMPIRE_RELIC_TABLE);

            g_pCombatInfoManager->setRelicOwner(relicIndex, CombatInfoManager::RELIC_OWNER_VAMPIRE);
        }

        // Relic을 생성한다.
        list<OptionType_t> optionNULL;
        Item* pItem = g_pItemFactoryManager->createItem(Item::ITEM_CLASS_RELIC, relicIndex, optionNULL);
        Assert(pItem != NULL);

        // cout << "new RelicItem OK" << endl;

        // 이 Zone은 RelicTable을 갖고 있다고 표시한다.
        m_bHasRelicTable = true;

        pMonsterCorpse->addTreasure(pItem);

        // 일단 relic은 DB에 생성한다.
        // 대신 CGDissectionCorpseHandler에서 create하지 않는다.
        // 보관대에서 꺼낼때마다 create되지 않게하기 위해서이다.
        pItem->create("", STORAGE_CORPSE, pMonsterCorpse->getObjectID(), 0, 0);

        if (pRelicInfo->relicType == RELIC_TYPE_SLAYER) {
            EffectSlayerRelic* pEffect = new EffectSlayerRelic(pMonsterCorpse);
            pMonsterCorpse->getEffectManager().addEffect(pEffect);
            pMonsterCorpse->setFlag(Effect::EFFECT_CLASS_SLAYER_RELIC);
            pEffect->affect(pMonsterCorpse);
            g_pCombatInfoManager->setRelicOwner(relicIndex, CombatInfoManager::RELIC_OWNER_SLAYER);
        } else {
            EffectVampireRelic* pEffect = new EffectVampireRelic(pMonsterCorpse);
            pMonsterCorpse->getEffectManager().addEffect(pEffect);
            pMonsterCorpse->setFlag(Effect::EFFECT_CLASS_VAMPIRE_RELIC);
            pEffect->affect(pMonsterCorpse);
            g_pCombatInfoManager->setRelicOwner(relicIndex, CombatInfoManager::RELIC_OWNER_VAMPIRE);
        }

        // cout << "addTreasure OK" << endl;

        // 바로 Zone에 추가하면 안되므로(동기화 문제)
        // Effect를 사용해서 추가하도록 한다.
        EffectAddItem* pEffectAddItem = new EffectAddItem(this, cx, cy, pMonsterCorpse, 0, false);
        pEffectAddItem->setNextTime(999999);
        m_ObjectRegistry.registerObject(pEffectAddItem);

        addEffect_LOCKING(pEffectAddItem);

        // cout << "addRelic OK" << endl;
    }

    return true;

    __END_CATCH
}

//-------------------------------------------------------------
// delete Relic Item
//-------------------------------------------------------------
// 아이템을 삭제하는데.. 다른 thread에서 해도 된다.
// 다른 heartbeat에서 삭제된다.
//-------------------------------------------------------------
bool Zone::deleteRelicItem()

{
    __BEGIN_TRY

    // 성물 보관대가 없다면 리턴
    if (!m_bHasRelicTable) {
        return false;
    }

    // 성물 보관대를 찾는다.
    Item* pItem = dynamic_cast<Item*>(getTile(m_RelicTableX, m_RelicTableY).getObject(m_RelicTableOID));
    Assert(pItem != NULL);

    // 바로 Zone에 추가하면 안되므로(동기화 문제)
    // Effect를 사용해서 추가하도록 한다.
    EffectDeleteItem* pEffectDeleteItem = new EffectDeleteItem(this, m_RelicTableX, m_RelicTableY, pItem, 0);
    pEffectDeleteItem->setNextTime(999999);
    m_ObjectRegistry.registerObject(pEffectDeleteItem);

    addEffect_LOCKING(pEffectDeleteItem);

    // cout << "delete Relic OK" << endl;

    m_bHasRelicTable = false;

    return true;

    __END_CATCH
}


list<NPCInfo*>* Zone::getNPCInfos(void) {
    return &m_NPCInfos;
}

void Zone::addNPCInfo(NPCInfo* pInfo) {
    // 이거 zone delete할때 지워야된데이.. - -;	by sigi
    m_NPCInfos.push_back(pInfo);
}

const BPOINT& Zone::getRandomMonsterRegenPosition() const {
    return m_MonsterRegenPositions[rand() % m_MonsterRegenPositions.size()];
}

const BPOINT& Zone::getRandomEmptyTilePosition() const {
    return m_EmptyTilePositions[rand() % m_EmptyTilePositions.size()];
}

void Zone::initLoadValue() {
    m_LoadValue = 0;
    getCurrentTime(m_LoadValueStartTime);
}

DWORD Zone::getLoadValue() const {
    Timeval currentTime;
    getCurrentTime(currentTime);

    Timeval elapsedTime = timediff(currentTime, m_LoadValueStartTime);

    if (elapsedTime.tv_sec == 0) {
        return 200;
    }

    // 10초당 loop수
    DWORD loadValue = m_LoadValue * 10 / elapsedTime.tv_sec;

    return loadValue;
}

/*
void Zone::setNPCMarketCondition(MarketCond_t NPCSell, MarketCond_t NPCBuy)

{
    __BEGIN_TRY

    unordered_map<ObjectID_t, Creature*> NPCMap = m_pNPCManager->getCreatures();
    for (unordered_map<ObjectID_t, Creature*>::const_iterator i = NPCMap.begin(); i != NPCMap.end(); i++)
    {
        NPC* pNPC = dynamic_cast<NPC*>(i->second);

        pNPC->setMarketCondBuy( NPCBuy );
        pNPC->setMarketCondSell( NPCSell );

        pNPC->increaseShopVersion(SHOP_RACK_SPECIAL);
    }

    __END_CATCH
}
*/


void Zone::sendNPCInfo()

{
    __BEGIN_TRY

    // NPC에 대한 정보를 클라이언트에게 보내준다.
    GCNPCInfo gcNPCInfo;

    list<NPCInfo*>::const_iterator itr = m_NPCInfos.begin();
    for (; itr != m_NPCInfos.end(); itr++) {
        NPCInfo* pInfo = *itr;
        gcNPCInfo.addNPCInfo(pInfo);
    }

    broadcastPacket(&gcNPCInfo);

    __END_CATCH
}


bool Zone::removeNPCInfo(NPC* pNPC) {
    __BEGIN_TRY

    list<NPCInfo*>::iterator itr = m_NPCInfos.begin();

    for (; itr != m_NPCInfos.end(); itr++) {
        NPCInfo* pNPCInfo = *itr;

        if (pNPCInfo->getNPCID() == pNPC->getNPCID()) {
            SAFE_DELETE(pNPCInfo);
            m_NPCInfos.erase(itr);

            return true;
        }
    }

    return false;

    __END_CATCH
}

void Zone::releaseSafeZone()

{
    __BEGIN_TRY

    m_ZoneLevel = NO_SAFE_ZONE;

    // 존 레벨을 초기화시킨다.
    for (ZoneCoord_t x = 0; x < m_Width; x++)
        for (ZoneCoord_t y = 0; y < m_Height; y++)
            m_ppLevel[x][y] = m_ZoneLevel;

    __END_CATCH
}

void Zone::resetSafeZone()

{
    __BEGIN_TRY

    m_ZoneLevel = g_pZoneInfoManager->getZoneInfo(m_ZoneID)->getZoneLevel();

    // 존 레벨을 초기화시킨다.
    for (ZoneCoord_t x = 0; x < m_Width; x++)
        for (ZoneCoord_t y = 0; y < m_Height; y++)
            m_ppLevel[x][y] = m_ZoneLevel;

    __END_CATCH
}

void Zone::resetDarkLightInfo()

{
    __BEGIN_TRY

    m_pWeatherManager->resetDarkLightInfo();

    __END_CATCH
}


// 종족 전쟁에 참가하는 사람만 남긴다. 나머지는 kick한다.
void Zone::remainRaceWarPlayers()

{
    __BEGIN_TRY

    try {
        // 참가 인원 제한을 하지 않는다면 무시한다.
        if (!g_pVariableManager->isActiveRaceWarLimiter())
            return;

        __ENTER_CRITICAL_SECTION(m_Mutex)

        unordered_map<ObjectID_t, Creature*>& creatures = m_pPCManager->getCreatures();
        unordered_map<ObjectID_t, Creature*>::iterator itr = creatures.begin();

        for (; itr != creatures.end(); itr++) {
            Creature* pCreature = itr->second;

            if (pCreature->isFlag(Effect::EFFECT_CLASS_RACE_WAR_JOIN_TICKET))
                continue;

            Player* pPlayer = pCreature->getPlayer();
            Assert(pPlayer != NULL);

            GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);

            Event* pEvent = pGamePlayer->getEvent(Event::EVENT_CLASS_TRANSPORT);
            if (pEvent == NULL) {
                PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
                Assert(pPC != NULL);

                ZONE_COORD ZC;
                g_pResurrectLocationManager->getPosition(pPC, ZC);

                ZoneInfo* pZoneInfo = g_pZoneInfoManager->getZoneInfo(ZC.id);
                Assert(pZoneInfo != NULL);

                EventTransport* pEventTransport = new EventTransport(pGamePlayer);

                pEventTransport->setDeadline(15 * 10);
                pEventTransport->setTargetZone(ZC.id, ZC.x, ZC.y);
                pEventTransport->setZoneName(pZoneInfo->getFullName());

                // 몇 초후에 어디로 이동한다.고 보내준다.
                pEventTransport->sendMessage();

                pGamePlayer->addEvent(pEventTransport);
            } else {
                EventTransport* pEventTransport = dynamic_cast<EventTransport*>(pEvent);
                pEventTransport->sendMessage();
            }
        }

        __LEAVE_CRITICAL_SECTION(m_Mutex)

    } catch (Throwable& t) {
        cout << t.toString().c_str() << endl;
        throw;
    }

    __END_CATCH
}

bool Zone::isLevelWarZone() const {
    switch (m_ZoneID) {
    case 1131:
    case 1132:
    case 1133:
    case 1134: {
        return true;
    } break;
    default: {
        return false;
    } break;
    }
}

void Zone::remainPayPlayer()

{
    __BEGIN_TRY

    try {
        unordered_map<ObjectID_t, Creature*>& creatures = m_pPCManager->getCreatures();
        unordered_map<ObjectID_t, Creature*>::iterator itr = creatures.begin();

        for (; itr != creatures.end(); itr++) {
            Creature* pCreature = itr->second;
            Player* pPlayer = pCreature->getPlayer();
            Assert(pPlayer != NULL);

            GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);

            if (pGamePlayer->isPayPlaying())
                continue;

            Event* pEvent = pGamePlayer->getEvent(Event::EVENT_CLASS_TRANSPORT);
            if (pEvent == NULL) {
                PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
                Assert(pPC != NULL);

                ZONE_COORD ZC;
                g_pResurrectLocationManager->getPosition(pPC, ZC);

                ZoneInfo* pZoneInfo = g_pZoneInfoManager->getZoneInfo(ZC.id);
                Assert(pZoneInfo != NULL);

                EventTransport* pEventTransport = new EventTransport(pGamePlayer);

                pEventTransport->setDeadline(60 * 10);
                pEventTransport->setTargetZone(ZC.id, ZC.x, ZC.y);
                pEventTransport->setZoneName(pZoneInfo->getFullName());

                char msg[100];

                sprintf(msg, g_pStringPool->c_str(STRID_LEVEL_WAR_ZONE_FREE_CLOSE_1));

                GCSystemMessage gcSystemMessage;
                gcSystemMessage.setMessage(msg);
                pPlayer->sendPacket(&gcSystemMessage);

                sprintf(msg, g_pStringPool->c_str(STRID_LEVEL_WAR_ZONE_FREE_CLOSE_2), pZoneInfo->getFullName().c_str());

                gcSystemMessage.setMessage(msg);
                pPlayer->sendPacket(&gcSystemMessage);

                // 몇 초후에 어디로 이동한다.고 보내준다.
                //              pEventTransport->sendMessage();

                pGamePlayer->addEvent(pEventTransport);
            } else {
                EventTransport* pEventTransport = dynamic_cast<EventTransport*>(pEvent);
                pEventTransport->sendMessage();
            }
        }
    } catch (Throwable& t) {
        cout << t.toString().c_str() << endl;
        throw;
    }

    __END_CATCH
}
