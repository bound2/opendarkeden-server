//////////////////////////////////////////////////////////////////////////////
// FileName 	: ZoneSpawn.cpp
// Description	: Zone spawn and despawn: putting creatures and objects into the zone and taking them out.
//////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <fstream>

#include "Assert.h"
#include "BloodBibleBonusManager.h"
#include "CastleInfoManager.h"
#include "CombatInfoManager.h"
#include "Creature.h"
#include "DarkLightInfo.h"
#include "DefaultOptionSetInfo.h"
#include "DynamicZone.h"
#include "EffectAddItem.h"
#include "EffectAddItemToCorpse.h"
#include "EffectCastingTrap.h"
#include "EffectContinualGroundAttack.h"
#include "EffectDarkness.h"
#include "EffectDecayCorpse.h"
#include "EffectDecayItem.h"
#include "EffectDecayMotorcycle.h"
#include "EffectDeleteItem.h"
#include "EffectGnomesWhisper.h"
#include "EffectHasBloodBible.h"
#include "EffectHasCastleSymbol.h"
#include "EffectHasSlayerRelic.h"
#include "EffectHasVampireRelic.h"
#include "EffectLoaderManager.h"
#include "EffectManager.h"
#include "EffectObservingEye.h"
#include "EffectPKZoneRegen.h"
#include "EffectRelicTable.h"
#include "EffectSanctuary.h"
#include "EffectSchedule.h"
#include "EffectShrineGuard.h"
#include "EffectShrineHoly.h"
#include "EffectShrineShield.h"
#include "EffectSlayerRelic.h"
#include "EffectTransportItem.h"
#include "EffectTransportItemToCorpse.h"
#include "EffectVampirePortal.h"
#include "EffectVampireRelic.h"
#include "EventTransport.h"
#include "FlagSet.h"
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
#include "GDRLairManager.h"
#include "GGCommand.h"
#include "GQuestManager.h"
#include "GamePlayer.h"
#include "GameServerInfoManager.h"
#include "GuildManager.h"
#include "GuildUnion.h"
#include "HolyLandManager.h"
#include "Item.h"
#include "ItemFactoryManager.h"
#include "ItemInfo.h"
#include "LevelWarManager.h"
#include "LevelWarZoneInfoManager.h"
#include "LogClient.h"
#include "LoginServerManager.h"
#include "MasterLairInfoManager.h"
#include "MasterLairManager.h"
#include "Monster.h"
#include "MonsterCorpse.h"
#include "MonsterManager.h"
#include "NPC.h"
#include "NPCInfo.h"
#include "NPCManager.h"
#include "NicknameBook.h"
#include "Ousters.h"
#include "OustersCorpse.h"
#include "PCFinder.h"
#include "PCManager.h"
#include "PKZoneInfoManager.h"
#include "PacketUtil.h"
#include "ParkingCenter.h"
#include "Party.h"
#include "PaySystem.h"
#include "Player.h"
#include "Profile.h"
#include "Properties.h"
#include "QuestManager.h"
#include "RegenZoneManager.h"
#include "Relic.h"
#include "RelicUtil.h"
#include "ResurrectLocationManager.h"
#include "ShrineInfoManager.h"
#include "SiegeManager.h"
#include "SkillUtil.h"
#include "Slayer.h"
#include "SlayerCorpse.h"
#include "Store.h"
#include "StringPool.h"
#include "SweeperBonusManager.h"
#include "TimeManager.h"
#include "TradeManager.h"
#include "Vampire.h"
#include "VampireCorpse.h"
#include "VariableManager.h"
#include "VisionInfo.h"
#include "War.h"
#include "WarScheduler.h"
#include "WarSystem.h"
#include "WeatherManager.h"
#include "Zone.h"
#include "ZoneGroup.h"
#include "ZoneInfo.h"
#include "ZoneInfoManager.h"
#include "ZoneInternal.h"
#include "ZoneUtil.h"
#include "ctf/FlagManager.h"
#include "item/Motorcycle.h"
#include "item/VampirePortalItem.h"
#include "repository/ComebackEventRepository.h"
#include "repository/MessageRepository.h"
#include "repository/ZoneInfoRepository.h"

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

//////////////////////////////////////////////////////////////////////////////
// STL find_if 알고리즘을 이용하기 위한 비교 클래스
//////////////////////////////////////////////////////////////////////////////
class isSameCreature {
public:
    isSameCreature(Creature* pCreature) : m_Creature(pCreature) {}

    bool operator()(Creature* pCreature) {
        return pCreature->getName() == m_Creature->getName();
    }

private:
    Creature* m_Creature;
};

//////////////////////////////////////////////////////////////////////////////
// add PC
//
// PC 를 존에 최초로 추가한다. PC 주변의 다른 PC들에게 새 크리처의 출현을 알려주고,
// 주변을 스캔해서 객체들의 정보를 받아온다.
//////////////////////////////////////////////////////////////////////////////
void Zone::addPC(Creature* pCreature, ZoneCoord_t cx, ZoneCoord_t cy, Dir_t dir)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    if (m_pZoneGroup != NULL)
        m_pZoneGroup->assertOwned();

    __BEGIN_PROFILE_ZONE("Z_ADD_PC")

    Assert(pCreature != NULL);
    Assert(pCreature->isPC());

    TPOINT pt = findSuitablePosition(this, cx, cy, pCreature->getMoveMode());

    if (pt.x != -1) {
        pCreature->setLastTarget(0);

        // 지정된 좌표를 클라이언트로 전송한다.
        GCSetPosition gcSetPosition;
        gcSetPosition.setX(pt.x);
        gcSetPosition.setY(pt.y);
        gcSetPosition.setDir(dir);

        pCreature->getPlayer()->sendPacket(&gcSetPosition);

        // 크리처의 좌표와 방향을 지정한다.
        pCreature->setXYDir(pt.x, pt.y, dir);

        // 적절한 흌일을 찾았으면, 크리처를 실제로
        // PC매니저와 타일에 각각 집어넣는다.
        m_pTiles[pt.x][pt.y].addCreature(pCreature);

        // checkMine(this, pCreature, pt.x, pt.y);	// 여기서도 mine을 폭발시켜야 하나..?

        m_pPCManager->addCreature(pCreature);

        // Sanctuary 플래그가 켜져있으면 꺼준다.
        /*		if ( pCreature->isFlag( Effect::EFFECT_CLASS_SANCTUARY ) && m_pTiles[pt.x][pt.y].getEffect(
        Effect::EFFECT_CLASS_SANCTUARY ) == NULL )
        {
        pCreature->removeFlag( Effect::EFFECT_CLASS_SANCTUARY );
        }*/

        // 패밀리 요금제일경우 Default Option 보너스를 준다.
        GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pCreature->getPlayer());
        if (pGamePlayer->isFamilyPayAvailable() && !pCreature->isFlag(Effect::EFFECT_CLASS_FAMILY_BONUS)) {
            PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
            Assert(pPC != NULL);

            pPC->addDefaultOptionSet(DEFAULT_OPTION_SET_FAMILY_PAY);
            addSimpleCreatureEffect(pPC, Effect::EFFECT_CLASS_FAMILY_BONUS);
            pPC->setFlag(Effect::EFFECT_CLASS_INIT_ALL_STAT);
        }

        // EFFECT_CLASS_INIT_ALL_STAT 이 켜져 있으면 initAllStat을 부르로 Flag 을 끈다.
        if (pCreature->isFlag(Effect::EFFECT_CLASS_INIT_ALL_STAT)) {
            if (pCreature->isSlayer()) {
                Slayer* pInitSlayer = dynamic_cast<Slayer*>(pCreature);
                Assert(pInitSlayer != NULL);

                SLAYER_RECORD prev;
                pInitSlayer->getSlayerRecord(prev);
                pInitSlayer->initAllStat();
                pInitSlayer->sendModifyInfo(prev);
            } else if (pCreature->isVampire()) {
                Vampire* pInitVampire = dynamic_cast<Vampire*>(pCreature);
                Assert(pInitVampire != NULL);

                VAMPIRE_RECORD prev;
                pInitVampire->getVampireRecord(prev);
                pInitVampire->initAllStat();
                pInitVampire->sendModifyInfo(prev);
            } else if (pCreature->isOusters()) {
                Ousters* pInitOusters = dynamic_cast<Ousters*>(pCreature);
                Assert(pInitOusters != NULL);

                OUSTERS_RECORD prev;
                pInitOusters->getOustersRecord(prev);
                pInitOusters->initAllStat();
                pInitOusters->sendModifyInfo(prev);
            }

            pCreature->removeFlag(Effect::EFFECT_CLASS_INIT_ALL_STAT);
        }

        if (pCreature->isSlayer()) {
            ((Slayer*)pCreature)->sendRealWearingInfo();
            ((Slayer*)pCreature)->sendSlayerSkillInfo();
            ((Slayer*)pCreature)->sendTimeLimitItemInfo();
        } else if (pCreature->isVampire()) {
            ((Vampire*)pCreature)->sendRealWearingInfo();
            ((Vampire*)pCreature)->sendVampireSkillInfo();
            ((Vampire*)pCreature)->sendTimeLimitItemInfo();
        } else if (pCreature->isOusters()) {
            ((Ousters*)pCreature)->sendRealWearingInfo();
            ((Ousters*)pCreature)->sendOustersSkillInfo();
            ((Ousters*)pCreature)->sendTimeLimitItemInfo();
        }

        // send RankBonus
        ((PlayerCreature*)pCreature)->sendRankBonusInfo();
        ((PlayerCreature*)pCreature)->sendCurrentQuestInfo();
        ((PlayerCreature*)pCreature)->getQuestManager()->adjustQuestStatus();


        GCModifyInformation gcModifyInformation;

        if (pCreature->isSlayer()) {
            Slayer* pNewSlayer = dynamic_cast<Slayer*>(pCreature);
            Assert(pNewSlayer != NULL);

            Player* pPlayer = pNewSlayer->getPlayer();
            Assert(pPlayer != NULL);

            gcModifyInformation.addShortData(MODIFY_DEFENSE, pNewSlayer->getDefense());
            gcModifyInformation.addShortData(MODIFY_PROTECTION, pNewSlayer->getProtection());
            gcModifyInformation.addShortData(MODIFY_TOHIT, pNewSlayer->getToHit());
            gcModifyInformation.addShortData(MODIFY_MIN_DAMAGE, pNewSlayer->getDamage(ATTR_CURRENT));
            gcModifyInformation.addShortData(MODIFY_MAX_DAMAGE, pNewSlayer->getDamage(ATTR_MAX));

            pPlayer->sendPacket(&gcModifyInformation);
        } else if (pCreature->isVampire()) {
            Vampire* pNewVampire = dynamic_cast<Vampire*>(pCreature);
            Assert(pNewVampire != NULL);

            Player* pPlayer = pNewVampire->getPlayer();
            Assert(pPlayer != NULL);

            gcModifyInformation.addShortData(MODIFY_DEFENSE, pNewVampire->getDefense());
            gcModifyInformation.addShortData(MODIFY_PROTECTION, pNewVampire->getProtection());
            gcModifyInformation.addShortData(MODIFY_TOHIT, pNewVampire->getToHit());
            gcModifyInformation.addShortData(MODIFY_MIN_DAMAGE, pNewVampire->getDamage(ATTR_CURRENT));
            gcModifyInformation.addShortData(MODIFY_MAX_DAMAGE, pNewVampire->getDamage(ATTR_MAX));

            pPlayer->sendPacket(&gcModifyInformation);
        } else if (pCreature->isOusters()) {
            Ousters* pNewOusters = dynamic_cast<Ousters*>(pCreature);
            Assert(pNewOusters != NULL);

            Player* pPlayer = pNewOusters->getPlayer();
            Assert(pPlayer != NULL);

            gcModifyInformation.addShortData(MODIFY_DEFENSE, pNewOusters->getDefense());
            gcModifyInformation.addShortData(MODIFY_PROTECTION, pNewOusters->getProtection());
            gcModifyInformation.addShortData(MODIFY_TOHIT, pNewOusters->getToHit());
            gcModifyInformation.addShortData(MODIFY_MIN_DAMAGE, pNewOusters->getDamage(ATTR_CURRENT));
            gcModifyInformation.addShortData(MODIFY_MAX_DAMAGE, pNewOusters->getDamage(ATTR_MAX));

            pPlayer->sendPacket(&gcModifyInformation);
        }

        //////////////////////////////////////////////////////////////////////////////
        // 보내야할 메시지가 있다면 보낸다.
        // 일단 막아둔다. - bezz 2002. 07. 13
        //////////////////////////////////////////////////////////////////////////////
        if (!pCreature->isFlag(Effect::EFFECT_CLASS_LOGIN_GUILD_MESSAGE)) {
            vector<string> messages = defaultMessageRepository().loadMessages(pCreature->getName());

            for (size_t m = 0; m < messages.size(); m++) {
                GCSystemMessage message;
                message.setMessage(messages[m]);
                pCreature->getPlayer()->sendPacket(&message);
            }

            defaultMessageRepository().deleteMessages(pCreature->getName());

            pCreature->setFlag(Effect::EFFECT_CLASS_LOGIN_GUILD_MESSAGE);
        }

        //////////////////////////////////////////////////////////////////////////////
        // PREMIUM_HALF_EVENT 가 on 되어 있고 유료존이면 클라이언트에 알린다.
        //////////////////////////////////////////////////////////////////////////////
        if (g_pVariableManager->getVariable(PREMIUM_HALF_EVENT) &&
            (m_ZoneID == 61 || m_ZoneID == 64 || m_ZoneID == 1007)) {
            GCNoticeEvent gcNoticeEvent;
            gcNoticeEvent.setCode(NOTICE_EVENT_PREMIUM_HALF_START);

            pCreature->getPlayer()->sendPacket(&gcNoticeEvent);
        }

        // 주변의 PC들에게 알릴 GCAddSlayer or GCAddVampire 패킷을 생성한다.
        Creature::CreatureClass CClass = pCreature->getCreatureClass();
        if (CClass == Creature::CREATURE_CLASS_SLAYER) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
            GCAddSlayer gcAddSlayer;
            makeGCAddSlayer(&gcAddSlayer, pSlayer);

            scan(pCreature, pt.x, pt.y, &gcAddSlayer);

            // 능력치 40 이상인 경우 야전사령부에서 쫓겨난다. by sigi. 2002.11.7
            checkNewbieTransportToGuild(pSlayer);
        } else if (CClass == Creature::CREATURE_CLASS_VAMPIRE) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);
            GCAddVampire gcAddVampire;
            makeGCAddVampire(&gcAddVampire, pVampire);

            scan(pCreature, pt.x, pt.y, &gcAddVampire);

            // 뱀파이어라면 포탈을 이용해 왔을 가능성이 있으므로,
            // 플래그를 꺼준다.
            if (pVampire->isFlag(Effect::EFFECT_CLASS_VAMPIRE_PORTAL)) {
                pVampire->removeFlag(Effect::EFFECT_CLASS_VAMPIRE_PORTAL);
            }
        } else if (CClass == Creature::CREATURE_CLASS_OUSTERS) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);
            GCAddOusters gcAddOusters;
            makeGCAddOusters(&gcAddOusters, pOusters);

            scan(pCreature, pt.x, pt.y, &gcAddOusters);
        } else {
            throw Error("invalid creature class. must be slayer or vampire...");
        }

        // 파티에 가입되어 있다면 로컬 파티에 가입시킨다.
        uint PartyID = pCreature->getPartyID();
        if (PartyID != 0) {
            // 파티가 있다면 걍 더한다.
            m_pLocalPartyManager->addPartyMember(PartyID, pCreature);
        }

        // 요금 정보를 보여준다.
#if !defined(__CONNECT_BILLING_SYSTEM__) && (defined(__PAY_SYSTEM_ZONE__) || defined(__PAY_SYSTEM_FREE_LIMIT__))
        if (pCreature->isPC()) {
            GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pCreature->getPlayer());

            // 게임방인 경우에
            // 유료 사용중이면
            // 시간이 얼마 남지 않았을 경우에 요금 정보를 표시해준다.
            if ((pGamePlayer->isPayPlaying() || pGamePlayer->isPremiumPlay()) &&
                pGamePlayer->getPayType() == PAY_TYPE_TIME) {
                Timeval currentTime;
                getCurrentTime(currentTime);
                Timeval payTime = pGamePlayer->getPayPlayTime(currentTime);

                int usedMin = payTime.tv_sec / 60;
                int remainMin = pGamePlayer->getPayPlayAvailableHours() - usedMin;

                // PC방은 남은 시간이 5시간(300분) 이하일 때 출력
                if (pGamePlayer->getPayPlayType() == PAY_PLAY_TYPE_PCROOM) {
                    // cout << "PC방 사용시간 : " << usedMin << "/" << pGamePlayer->getPayPlayAvailableHours() << endl;

                    if (remainMin <= 300) {
                        char str[80];
                        sprintf(str, g_pStringPool->c_str(STRID_PCROOM_REMAIN_PLAY_TIME), remainMin);
                        // sprintf(str, "[PC방] 사용시간이 %d분 남았습니다.", remainMin);
                        GCSystemMessage gcSystemMessage;
                        gcSystemMessage.setMessage(str);
                        pGamePlayer->sendPacket(&gcSystemMessage);
                    }
                }
                // 개인은 남은 시간이 1시간(60분) 이하일 때 출력
                else if (pGamePlayer->getPayPlayType() == PAY_PLAY_TYPE_PERSON) {
                    if (remainMin <= 60) {
                        char str[80];
                        sprintf(str, g_pStringPool->c_str(STRID_PERSONAL_REMAIN_PLAY_TIME), remainMin);
                        // sprintf(str, "[개인] 사용시간이 %d분 남았습니다.", remainMin);
                        GCSystemMessage gcSystemMessage;
                        gcSystemMessage.setMessage(str);
                        pGamePlayer->sendPacket(&gcSystemMessage);
                    }
                }
            }
        }
#endif

        // 불기둥
        if (isMasterLair() && m_pMasterLairManager != NULL) {
            MasterLairInfo* pInfo = g_pMasterLairInfoManager->getMasterLairInfo(getZoneID());
            Assert(pInfo != NULL);

            if (m_pMasterLairManager->getCurrentEvent() == MasterLairManager::EVENT_WAITING_PLAYER) {
                // 연속적인 불기둥 이펙트가 있는 경우에 알려준다.
                if (m_pEffectManager->findEffect(Effect::EFFECT_CLASS_CONTINUAL_GROUND_ATTACK) != NULL) {
                    GCNoticeEvent gcNoticeEvent;
                    gcNoticeEvent.setCode(NOTICE_EVENT_CONTINUAL_GROUND_ATTACK);
                    gcNoticeEvent.setParameter(pInfo->getStartDelay()); // 초

                    broadcastPacket(&gcNoticeEvent);
                }
            }
        }

        //-----------------------------------------------------------------
        // 세금 적용되는 경우
        //-----------------------------------------------------------------
        /*		if (isCastle())
        {
        PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);

        int itemTaxRatio = g_pCastleInfoManager->getItemTaxRatio( pPC );

        if (itemTaxRatio > 100)
        {
        GCNoticeEvent gcNoticeEvent;
        gcNoticeEvent.setCode( NOTICE_EVENT_SHOP_TAX_CHANGE );
        gcNoticeEvent.setParameter( (uint)itemTaxRatio );

        pPC->getPlayer()->sendPacket( &gcNoticeEvent );
        }
        }*/

        //-----------------------------------------------------------------
        // 전쟁 중인 경우는 전쟁정보를 보내준다.
        //-----------------------------------------------------------------
        if (g_pWarSystem->isWarActive()) {
            PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
            g_pWarSystem->sendGCWarList(pPC->getPlayer());
        }

        if (g_pFlagManager->hasFlagWar() && g_pFlagManager->isFlagAllowedZone(getZoneID())) {
            Player* pPlayer = pCreature->getPlayer();
            if (pPlayer != NULL)
                pPlayer->sendPacket(g_pFlagManager->getStatusPacket());
        }

        if (m_pLevelWarManager != NULL && m_pLevelWarManager->hasWar()) {
            // 레벨별 전쟁 중이면 먼가 보내줘야 될 듯
            PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
            m_pLevelWarManager->sendGCWarList(pPC->getPlayer());
        }

        //-----------------------------------------------------------------
        // 아담의 성지에 들어온 경우는 이펙트를 뿌려준다.
        //-----------------------------------------------------------------
        // 이전에 있던 존을 체크해야 될거 같은데? -_-;
        //-----------------------------------------------------------------
        // if (isHolyLand())
        //{
        sendHolyLandWarpEffect(pCreature);
        //}

        if (isHolyLand()) {
            if (g_pWarSystem->hasActiveRaceWar()) {
                PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
                g_pShrineInfoManager->sendBloodBibleStatus(pPC);

                pPC->getPlayer()->sendPacket(RegenZoneManager::getInstance()->getStatusPacket());
            } else {
                GCHolyLandBonusInfo gcHolyLandBonusInfo;
                g_pBloodBibleBonusManager->makeHolyLandBonusInfo(gcHolyLandBonusInfo);
                pCreature->getPlayer()->sendPacket(&gcHolyLandBonusInfo);
            }
        }

        if (g_pSweeperBonusManager->isAble(getZoneID()) &&
            g_pLevelWarZoneInfoManager->isCreatureBonusZone(pCreature, getZoneID())) {
            GCSweeperBonusInfo gcSweeperBonusInfo;
            g_pSweeperBonusManager->makeSweeperBonusInfo(gcSweeperBonusInfo);
            pCreature->getPlayer()->sendPacket(&gcSweeperBonusInfo);
            //			pCreature->setFlag( Effect::EFFECT_CLASS_INIT_ALL_STAT );
        }


        // Player 에게 GCItemNameInfoList 패킷을 보내준다
        /*		PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
        if ( !pPC->isEmptyItemNameInfoList() )
        {
        GCItemNameInfoList	gcItemNamInfoList;
        makeGCItemNameInfoList( &gcItemNamInfoList, pPC );

        pPC->getPlayer()->sendPacket( &gcItemNamInfoList );
        }*/

        // PK존에서 죽어서 되살아나는 경우 부활 이펙트가 붙는다.
        if (pCreature->isFlag(Effect::EFFECT_CLASS_PK_ZONE_RESURRECTION)) {
            Effect* pEffect = pCreature->findEffect(Effect::EFFECT_CLASS_PK_ZONE_RESURRECTION);
            if (pEffect != NULL) {
                // Effect가 끝나서 사라질 때 부활 이펙트 붙여주라는 패킷이 날라간다.
                pEffect->setDeadline(0);
            } else {
                pCreature->removeFlag(Effect::EFFECT_CLASS_PK_ZONE_RESURRECTION);
            }
        }

        // 막 생성된 넘이라면 먼가를 보내준다.
        PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
        if (pPC->getFlagSet()->isOn(FLAGSET_NOT_JUST_CREATED)) {
            GCNoticeEvent gcNoticeEvent;
            gcNoticeEvent.setCode(NOTICE_EVENT_WELCOME_MESSAGE);
            pPC->getPlayer()->sendPacket(&gcNoticeEvent);

            pPC->getFlagSet()->turnOff(FLAGSET_NOT_JUST_CREATED);
            pPC->getFlagSet()->save(pPC->getName());

            GCNoticeEvent gcNoticeEvent2;
            gcNoticeEvent2.setCode(NOTICE_EVENT_LOGIN_JUST_NOW);
            gcNoticeEvent2.setParameter(g_pVariableManager->getHeadPriceBonus());

            pPC->getPlayer()->sendPacket(&gcNoticeEvent2);

            pPC->setFlag(Effect::EFFECT_CLASS_JUST_LOGIN);

            if (g_pVariableManager->getVariable(CHOBO_EVENT)) {
                pPC->getGQuestManager()->getGQuestInventory().saveOne(pPC->getName(), 13);
                pPC->getPlayer()->sendPacket(pPC->getGQuestManager()->getGQuestInventory().getInventoryPacket());
                gcNoticeEvent.setCode(NOTICE_EVENT_GIVE_PRESENT_1);
                pPC->getPlayer()->sendPacket(&gcNoticeEvent);
            }
        } else if (!pPC->isFlag(Effect::EFFECT_CLASS_JUST_LOGIN)) {
            // Comeback-event reminders.
            if (defaultComebackEventRepository().hasUnclaimedItem(pPC->getPlayer()->getID())) {
                GCNPCResponse response;
                response.setCode(NPC_RESPONSE_SHOW_COMMON_MESSAGE_DIALOG);
                response.setParameter(YOU_CAN_GET_EVENT_200501_COMBACK_ITEM);
                pPC->getPlayer()->sendPacket(&response);
            }

            if (defaultComebackEventRepository().hasUnclaimedPremiumItem(pPC->getPlayer()->getID())) {
                GCNPCResponse response;
                response.setCode(NPC_RESPONSE_SHOW_COMMON_MESSAGE_DIALOG);
                response.setParameter(YOU_CAN_GET_EVENT_200501_COMBACK_PREMIUM_ITEM);
                pPC->getPlayer()->sendPacket(&response);
            }

            if (defaultComebackEventRepository().hasUnclaimedRecommendItem(pPC->getPlayer()->getID())) {
                GCNPCResponse response;
                response.setCode(NPC_RESPONSE_SHOW_COMMON_MESSAGE_DIALOG);
                response.setParameter(YOU_CAN_GET_EVENT_200501_COMBACK_RECOMMEND_ITEM);
                pPC->getPlayer()->sendPacket(&response);
            }

            if (g_pVariableManager->getVariable(TODAY_IS_HOLYDAY)) {
                GCNoticeEvent gcNoticeEvent;
                gcNoticeEvent.setCode(NOTICE_EVENT_HOLYDAY);
                gcNoticeEvent.setParameter(g_pVariableManager->getVariable(TODAY_IS_HOLYDAY));

                pPC->getPlayer()->sendPacket(&gcNoticeEvent);
            }

            //			if ( canEnterBeginnerZone( pPC ) && getZoneID() != 1122 )
            if (canEnterBeginnerZone(pPC)) {
                int year = VSDate::currentDate().year() - 2000;
                int month = VSDate::currentDate().month();
                int day = VSDate::currentDate().day();
                int hour = VSTime::currentTime().hour();
                GCNoticeEvent gcNoticeEvent;
                gcNoticeEvent.setCode(NOTICE_EVENT_ENTER_BEGINNER_ZONE);
                gcNoticeEvent.setParameter((year * 1000000) + (month * 10000) + (day * 100) + hour);
                pPC->getPlayer()->sendPacket(&gcNoticeEvent);
            }

            if (g_pVariableManager->isWarActive() && g_pVariableManager->isAutoStartRaceWar() &&
                g_pWarSystem->isRaceWarToday()) {
                GCNoticeEvent gcNoticeEvent;
                gcNoticeEvent.setCode(NOTICE_EVENT_RACE_WAR_SOON);
                gcNoticeEvent.setParameter(g_pWarSystem->getRaceWarTimeParam());
                pPC->getPlayer()->sendPacket(&gcNoticeEvent);
            }

            if (g_pVariableManager->isActiveLevelWar()) {
                ZoneID_t levelWarZoneId = g_pLevelWarZoneInfoManager->getCreatureZoneID(pCreature);

                //				cout << "ZoneID : " << levelWarZoneId << endl;
                if (levelWarZoneId != 1) {
                    Zone* pLevelZone = getZoneByZoneID(levelWarZoneId);
                    Assert(pLevelZone != NULL);

                    LevelWarManager* pLevelWarManager = pLevelZone->getLevelWarManager();
                    Assert(pLevelWarManager != NULL);

                    if (pLevelWarManager->hasToDayWar()) {
                        int year = VSDate::currentDate().year() - 2000;
                        int month = VSDate::currentDate().month();
                        int day = VSDate::currentDate().day();
                        int hour = VSTime::currentTime().hour();
                        int level = 0;
                        if (levelWarZoneId == 1131)
                            level = 1;
                        else if (levelWarZoneId == 1132)
                            level = 2;
                        else if (levelWarZoneId == 1133)
                            level = 3;
                        else if (levelWarZoneId == 1134)
                            level = 4;

                        GCNoticeEvent gcNoticeEvent;
                        gcNoticeEvent.setCode(NOTICE_EVENT_LEVEL_WAR_ARRANGED);
                        //						gcNoticeEvent.setParameter( ((DWORD)((DWORD)month << 24)) |
                        //((DWORD)((DWORD)day << 16)) | ((DWORD)((DWORD)hour << 8)) | ((DWORD)((DWORD)level)) );
                        gcNoticeEvent.setParameter((level * 100000000) + (year * 1000000) + (month * 10000) +
                                                   (day * 100) + hour);
                        pPC->getPlayer()->sendPacket(&gcNoticeEvent);
                    }
                }
            }

            GCNoticeEvent gcNoticeEvent;
            gcNoticeEvent.setCode(NOTICE_EVENT_LOGIN_JUST_NOW);
            gcNoticeEvent.setParameter(g_pVariableManager->getHeadPriceBonus());

            pPC->getPlayer()->sendPacket(&gcNoticeEvent);

            pPC->setFlag(Effect::EFFECT_CLASS_JUST_LOGIN);
        }

        if (pPC->getPetInfo() != NULL)
            sendPetInfo(pGamePlayer);
        // 존 이동할때  넣어주는 패킷
        pPC->getPlayer()->sendPacket(pPC->getNicknameBook()->getNicknameBookListPacket().get());

        Packet* pGQuestPacket = pPC->getGQuestManager()->getStatusInfoPacket();
        pPC->getPlayer()->sendPacket(pGQuestPacket);
        SAFE_DELETE(pGQuestPacket);

        pGQuestPacket = pPC->getGQuestManager()->getGQuestInventory().getInventoryPacket();
        pPC->getPlayer()->sendPacket(pGQuestPacket);

        // GCUnionOfferList
        //

        GCUnionOfferList gcUnionOfferList;

        {
            GuildUnion* pUnion = GuildUnionManager::Instance().getGuildUnion(pPC->getGuildID());
            if (pUnion != NULL) {
                //	cout << "GuildUNION : Union이 있는 PlayerCreature" << endl;

                if (g_pGuildManager->isGuildMaster(pPC->getGuildID(), pPC))
                    //		cout << "GuildUNION : PC가 GuildMaster다" << endl;

                    if (pUnion->getMasterGuildID() == pPC->getGuildID())
                        //		cout << "GuildUNION : 연합의 마스터 길드가 내 길드다" << endl;

                        // 요청한놈이 지가 속한 길드의 마스터인가? || 연합의 마스터길드가 내 길드가 맞나?
                        if (g_pGuildManager->isGuildMaster(pPC->getGuildID(), pPC) &&
                            pUnion->getMasterGuildID() == pPC->getGuildID()) {
                            //		cout << "그러면..OfferList를 만들어서 보내주자.." << endl;

                            if (GuildUnionOfferManager::Instance().makeOfferList(pUnion->getUnionID(),
                                                                                 gcUnionOfferList)) {
                                pPC->getPlayer()->sendPacket(&gcUnionOfferList);
                            }
                        }
            }
        }

        if (g_pWarSystem->isSkyBlack()) {
            GCNoticeEvent gcNE;
            gcNE.setCode(NOTICE_EVENT_RACE_WAR_IN_5);
            pPC->getPlayer()->sendPacket(&gcNE);
        }

        GCMyStoreInfo myStoreInfo;
        myStoreInfo.setStoreInfo(&(pPC->getStore()->getStoreInfo()));
        myStoreInfo.setOpenUI(0);
        pPC->getPlayer()->sendPacket(&myStoreInfo);

        // pPC->getPlayer()->sendPacket( &gcUnionOfferList );

        /*		GCNoticeEvent gcNoticeEvent;
        gcNoticeEvent.setCode( NOTICE_EVENT_CROWN_PRICE );
        gcNoticeEvent.setParameter( g_pVariableManager->getVariable(CROWN_PRICE) );
        pPC->getPlayer()->sendPacket( &gcNoticeEvent );*/
    } else {
        ZoneCoord_t tempX = Random(20, m_Width);
        ZoneCoord_t tempY = Random(20, m_Height);
        addPC(pCreature, tempX, tempY, 0);

        // 맥스카운트 지나도 못 찾은 경우 Assert
        // throw EmptyTileNotExistException("too many pc in this zone.. or too unlucky");
    }


    __END_PROFILE_ZONE("Z_ADD_PC")

    __END_DEBUG
    __END_CATCH
}

//--------------------------------------------------------------------------------
// add Creature
// 크리처가 존에 최초로 들어갈 때, 크리처 주변의 PC들에게 새 크리처의 출현을 알려준다.
//--------------------------------------------------------------------------------
void Zone::addCreature(Creature* pCreature, ZoneCoord_t cx, ZoneCoord_t cy, Dir_t dir)

{
    __BEGIN_TRY

    if (m_pZoneGroup != NULL)
        m_pZoneGroup->assertOwned();

    __BEGIN_PROFILE_ZONE("Z_ADD_CREATURE")

    TPOINT pt = findSuitablePosition(this, cx, cy, pCreature->getMoveMode());

    // 찾은 경우 체크
    if (pt.x != -1) {
        //--------------------------------------------------------------------------------
        // OID 를 할당받는다.
        //--------------------------------------------------------------------------------
        m_ObjectRegistry.registerObject(pCreature);

        //--------------------------------------------------------------------------------
        // 적절한 타일을 찾았으면, 크리처를 크리처매니저와 타일에 각각 집어넣는다.
        // Monster 일 경우, MonsterManager에 추가하며, NPC 일 경우, NPCManager 에 추가한다.
        //--------------------------------------------------------------------------------
        if (pCreature->isMonster()) {
            // #ifdef __XMAS_EVENT_CODE__
            Monster* pMonster = dynamic_cast<Monster*>(pCreature);

            if (isDynamicZone()) {
                pMonster->setScanEnemy(true);
            }

            m_pMonsterManager->addCreature(pCreature);
            if (pCreature->getClanType() != CLAN_VAMPIRE_MONSTER)
                cout << pCreature->toString() << " regens at " << pt.x << " , " << pt.y << endl;

            switch (pMonster->getMonsterType()) {
            case 717:
            case 721:
            case 723:
            case 724:
            case 725: {
                unordered_map<ObjectID_t, Creature*>::iterator itr = m_pPCManager->getCreatures().begin();
                unordered_map<ObjectID_t, Creature*>::iterator endItr = m_pPCManager->getCreatures().end();

                for (; itr != endItr; ++itr) {
                    pMonster->addPotentialEnemy(itr->second);
                }
                break;
            }
            default:
                break;
            }

            /*
            switch (pMonster->getMonsterType())
            {
                case 358:
                case 359:
                case 360:
                case 361:
                    m_pEventMonsterManager->addCreature(pCreature);
                    break;

                default:
                    m_pMonsterManager->addCreature(pCreature);
                    break;
            }
            */
            // #else
            //			m_pMonsterManager->addCreature(pCreature);
            /*
            #endif
            */

        } else if (pCreature->isNPC()) {
            m_pNPCManager->addCreature(pCreature);
        }

        // cout << "타일에 몬스터 추가하기" << endl;
        m_pTiles[pt.x][pt.y].addCreature(pCreature, false);

        //--------------------------------------------------------------------------------
        // 크리처의 좌표를 지정한다.
        //--------------------------------------------------------------------------------
        pCreature->setXYDir(pt.x, pt.y, dir);
        pCreature->setZone(this);

        // scanPC(pCreature);

        //--------------------------------------------------------------------------------
        // 주변의 PC들에게 알릴 GCAddNPC or GCAddMonster 패킷을 생성한다.
        //--------------------------------------------------------------------------------
        // cout << "주변의 PC들에게 알릴 패킷 만들기" << endl;
        Creature::CreatureClass CClass = pCreature->getCreatureClass();

        if (CClass == Creature::CREATURE_CLASS_NPC) {
            NPC* pNPC = dynamic_cast<NPC*>(pCreature);
            GCAddNPC gcAddNPC;
            makeGCAddNPC(&gcAddNPC, pNPC);
            broadcastPacket(pt.x, pt.y, &gcAddNPC);
        } else if (CClass == Creature::CREATURE_CLASS_MONSTER) {
            // cout << "몬스터용 패킷 만들기" << endl;
            Monster* pMonster = dynamic_cast<Monster*>(pCreature);

            // zone에 처음 들어갈때도 여러가지 상태가 있다.. by sigi
            Packet* pAddMonsterPacket = createMonsterAddPacket(pMonster, NULL);

            if (pAddMonsterPacket != NULL) {
                broadcastPacket(cx, cy, pAddMonsterPacket, pMonster);
                /*				ZoneCoord_t ix = 0;
                                ZoneCoord_t iy = 0;
                                ZoneCoord_t endx = 0;
                                ZoneCoord_t endy = 0;


                                //////////////////////////////////////////////////////////////////////////////
                                // 루프 변수 초기화..
                                //////////////////////////////////////////////////////////////////////////////
                                int Range = 0;
                                endx = min(m_Width - 1, cx + maxViewportWidth + 1 + Range);
                                endy = min(m_Height - 1, cy + maxViewportLowerHeight  + 1 + Range);

                                for (ix =  max(0, cx - maxViewportWidth - 1 - Range); ix <= endx ; ix++)
                                {
                                    for (iy = max(0, cy - maxViewportUpperHeight - 1 -  Range); iy <= endy ; iy++)
                                    {
                                        // 타일에 크리처가 있는 경우에만
                                        if (m_pTiles[ix][iy].hasCreature())
                                        {
                                            const slist<Object*> & objectList = m_pTiles[ix][iy].getObjectList();
                                            slist<Object*>::const_iterator itr = objectList.begin();

                                            for (; itr != objectList.end() && (*itr)->getObjectPriority() <=
                   OBJECT_PRIORITY_BURROWING_CREATURE; itr++)
                                            {
                                                Creature* pOtherCreature = dynamic_cast<Creature*>(*itr);
                                                Assert(pOtherCreature != NULL);

                                                if (pOtherCreature->isPC())
                                                {
                                                    if ( canSee( pOtherCreature, pMonster ) )
                                                    {
                                                        pOtherCreature->getPlayer()->sendPacket(pAddMonsterPacket);
                                                    }
                                                } // if

                                            } // for
                                        }//if
                                    }//for
                                }//for
                */
                delete pAddMonsterPacket;
            }

            // by sigi. 2002.9.6
            // 포탈을 통해서 나타나는 모습을 보여준다.
            // 플래그를 꺼준다.
            if (pMonster->isFlag(Effect::EFFECT_CLASS_VAMPIRE_PORTAL)) {
                pMonster->removeFlag(Effect::EFFECT_CLASS_VAMPIRE_PORTAL);
            }
        } else {
            throw Error("invalid creature type");
        }
    } else {
        throw EmptyTileNotExistException("too many creature in this zone.. or too unlucky");
    }

    __END_PROFILE_ZONE("Z_ADD_CREATURE")

    __END_CATCH
}

//--------------------------------------------------------------------------------
// Delete PC from PC Manager (only do this)
//--------------------------------------------------------------------------------
void Zone::deletePC(Creature* pCreature)
// NoSuchElementException, Error)
{
    __BEGIN_TRY

    Assert(pCreature != NULL);
    m_pPCManager->deleteCreature(pCreature->getObjectID());


    __END_CATCH
}

//--------------------------------------------------------------------------------
// Delete Queue PC
//--------------------------------------------------------------------------------
void Zone::deleteQueuePC(Creature* pCreature)

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    Assert(pCreature != NULL);

    list<Creature*>::iterator itr = find_if(m_PCListQueue.begin(), m_PCListQueue.end(), isSameCreature(pCreature));

    if (itr != m_PCListQueue.end()) {
        m_PCListQueue.erase(itr);
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

//--------------------------------------------------------------------------------
// Add PC to PC Manager (only do this)
//--------------------------------------------------------------------------------
void Zone::addPC(Creature* pCreature)

{
    __BEGIN_TRY

    if (m_pZoneGroup != NULL)
        m_pZoneGroup->assertOwned();

    Assert(pCreature != NULL);
    m_pPCManager->addCreature(pCreature);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// Replace a PC with another PC object
//
// Used when a character is rebuilt as a different race: pFrom is taken off the
// tile it stands on and out of the PC manager, then pTo is put on the tile at
// (nx, ny), given that position and dir, and put into the PC manager. The
// destination may be the tile pFrom stood on.
//
// With bFindSuitablePosition set, (nx, ny) is only the starting point of a
// search for the nearest tile that is free for pTo's move mode and carries no
// portal. The search runs after pFrom has been removed, so the tile it vacated
// is itself a candidate.
//
// bCheckEffect and bCheckPortal are handed to Tile::addCreature: they decide
// whether the destination tile's effects (poison, darkness, trying position)
// are applied to pTo, and whether a portal on that tile is activated for it.
//
// The caller keeps everything else: broadcasting the swap, sending the new
// character's info, updating scans and disposing of pFrom.
//--------------------------------------------------------------------------------
void Zone::replacePC(Creature* pFrom, Creature* pTo, ZoneCoord_t nx, ZoneCoord_t ny, Dir_t dir,
                     bool bFindSuitablePosition, bool bCheckEffect, bool bCheckPortal) {
    if (m_pZoneGroup != NULL)
        m_pZoneGroup->assertOwned();

    Assert(pFrom != NULL);
    Assert(pTo != NULL);

    getTile(pFrom->getX(), pFrom->getY()).deleteCreature(pFrom->getObjectID());
    deletePC(pFrom);

    if (bFindSuitablePosition) {
        TPOINT pt = findSuitablePosition(this, nx, ny, pTo->getMoveMode());
        nx = pt.x;
        ny = pt.y;
    }

    getTile(nx, ny).addCreature(pTo, bCheckEffect, bCheckPortal);
    pTo->setXYDir(nx, ny, dir);
    addPC(pTo);
}

//--------------------------------------------------------------------------------
// Put a creature on the tile at (x, y), and nothing else.
//
// The creature manager membership, the creature's own coordinates and move
// mode, and every broadcast stay with the caller. A caller that puts a
// creature into the zone as a whole wants addPC/addCreature instead; this is
// for the two cases that live below them: re-adding a creature to a tile to
// change the move mode it is filed under, and moving a creature between tiles
// without disturbing the manager that owns it.
//
// bCheckEffect and bCheckPortal are handed to Tile::addCreature: they decide
// whether the tile's effects (poison, darkness, trying position) are applied
// to the creature, and whether a portal on the tile is activated for it.
// Returns false when such a portal was activated, which has already carried
// the creature on to the portal's destination.
//--------------------------------------------------------------------------------
bool Zone::addCreatureToTile(Creature* pCreature, ZoneCoord_t x, ZoneCoord_t y, bool bCheckEffect, bool bCheckPortal) {
    if (m_pZoneGroup != NULL)
        m_pZoneGroup->assertOwned();

    Assert(pCreature != NULL);

    return getTile(x, y).addCreature(pCreature, bCheckEffect, bCheckPortal);
}

//--------------------------------------------------------------------------------
// Take a creature off the tile at (x, y), and nothing else.
//
// The counterpart of addCreatureToTile: no creature manager is touched, so a
// creature removed this way is off the map but still owned by its manager,
// still heartbeaten, and still reachable by name or id. That is what the
// corpse paths want — dropping a dead player from the PC manager would stop
// its effect manager's heartbeat. A caller that wants the creature out of the
// zone entirely wants deleteCreature instead.
//
// A creature that is not on that tile is a no-op: Tile::deleteCreature swallows
// the mismatch, logging it to tileError.txt when the tile is empty outright.
//--------------------------------------------------------------------------------
void Zone::deleteCreatureFromTile(Creature* pCreature, ZoneCoord_t x, ZoneCoord_t y) {
    if (m_pZoneGroup != NULL)
        m_pZoneGroup->assertOwned();

    Assert(pCreature != NULL);

    getTile(x, y).deleteCreature(pCreature->getObjectID());
}

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
void Zone::deleteCreature(Creature* pCreature, ZoneCoord_t x, ZoneCoord_t y)

{
    __BEGIN_TRY

    if (m_pZoneGroup != NULL)
        m_pZoneGroup->assertOwned();

    __BEGIN_PROFILE_ZONE("Z_DELETE_CREATURE")

    try {
        Assert(pCreature->getX() == x && pCreature->getY() == y);

        // 해당되는 CreatureManager 에서 크리처를 삭제한다.
        if (pCreature->isPC()) {
            m_pPCManager->deleteCreature(pCreature->getObjectID());


            // 파티 초대중이라면 PartyInviteInfo를 삭제해준다.
            m_pPartyInviteInfoManager->cancelInvite(pCreature);

            // 파티에 가입되어 있었다면 로컬 파티에서 삭제해 준다.
            uint PartyID = pCreature->getPartyID();
            if (PartyID != 0) {
                m_pLocalPartyManager->deletePartyMember(PartyID, pCreature);
            }

            // 트레이드 중이었다면 트레이드 관련 정보를 삭제해준다.
            TradeInfo* pInfo = m_pTradeManager->getTradeInfo(pCreature->getName());
            if (pInfo != NULL) {
                m_pTradeManager->cancelTrade(pCreature);
            }
        } else if (pCreature->isMonster()) {
            // #ifdef __XMAS_EVENT_CODE__
            //			Monster* pMonster = dynamic_cast<Monster*>(pCreature);
            m_pMonsterManager->deleteCreature(pCreature->getObjectID());
            /*			switch (pMonster->getMonsterType())
                        {
                            case 358:
                            case 359:
                            case 360:
                            case 361:
                                m_pEventMonsterManager->deleteCreature(pCreature->getObjectID());
                                break;

                            default:
                                m_pMonsterManager->deleteCreature(pCreature->getObjectID());
                                break;
                        }*/
            // #else
            //			m_pMonsterManager->deleteCreature(pCreature->getObjectID());
            /*
            #endif
            */
        } else if (pCreature->isNPC()) {
            m_pNPCManager->deleteCreature(pCreature->getObjectID());
        }

        // 타일에서 크리처를 삭제한다.
        try {
            getTile(x, y).deleteCreature(pCreature->getObjectID());
        } catch (NoSuchElementException& nsee) {
            // by sigi. 2002.12.10
            // Player캐릭터가 죽을때..
            // [1] PCManager::killCreature()에서 tile에서는 지우고 목표존 설정하고
            // [2] EventResurrect에서 IncomingPlayer로 보내면.. 거기서 적절한 Zone에 들어가는데..
            // 이 두 과정.. 사이에서 아직 ZonePlayerManager에 있는 동안 Pay정보같은걸로 인해서
            // transport되면.. tile에서 지우려고 할때 문제가 생긴다..고 보여진다.
            // 일단, 그 부분(ZPM::pay체크)에서는 GPS_NORMAL인 경우만 하도록 하겠지만.
            // 이것도 무시할만하다고 보여지므로.. 일단 로그만 남기자.
            filelog("zoneDeleteCreatureError.log", "%s", nsee.toString().c_str());
        }

        // 주변의 PC들에게 크리처가 사라졌다는 사실을 브로드캐스트한다.
        GCDeleteObject gcDeleteObject(pCreature->getObjectID());
        broadcastPacket(x, y, &gcDeleteObject, pCreature);
    } catch (Throwable& t) {
        cerr << t.toString() << endl;
        filelog("zoneDeleteCreatureError.log", "Zone::deleteCreature() : %s", t.toString().c_str());
    }

    __END_PROFILE_ZONE("Z_DELETE_CREATURE")

    __END_CATCH
}

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
void Zone::deleteObject(Object* pObject, ZoneCoord_t x, ZoneCoord_t y)

{
    __BEGIN_TRY

    __BEGIN_PROFILE_ZONE("Z_DELETE_OBJECT")

    //--------------------------------------------------
    // 존에서 객체를 삭제한다.
    //--------------------------------------------------
    getTile(x, y).deleteObject(pObject->getObjectID());

    //--------------------------------------------------
    // 주변의 PC들에게 객체가 사라졌다는 사실을 브로드캐스트한다.
    //--------------------------------------------------
    GCDeleteObject gcDeleteObject(pObject->getObjectID());

    broadcastPacket(x, y, &gcDeleteObject);

    __END_PROFILE_ZONE("Z_DELETE_OBJECT")

    __END_CATCH
}

//-------------------------------------------------------------
// create MonsterAddPacket
//-------------------------------------------------------------
// monster의 상태에 따라서 GCAddXXX packet을 생성한다. by sigi
//-------------------------------------------------------------
Packet* Zone::createMonsterAddPacket(Monster* pMonster, Creature* pPC) const

{
    Assert(pMonster != NULL);

    // 보는 사람이 설정되지 않은 경우
    // 다~ 볼 수 있는 상태라고 설정한다.
    // 일단 packet을 생성해두고 체크하기 위해서다.
    if (pPC != NULL && !canSee(pPC, pMonster))
        return NULL;
    //	bool canSeeAll = (pPC==NULL);

    // ObservingEye 이펙트를 가져온다.
    //	EffectObservingEye* pEffectObservingEye = NULL;
    //	if ( pPC != NULL && pPC->isFlag( Effect::EFFECT_CLASS_OBSERVING_EYE ) )
    //	{
    //		pEffectObservingEye = dynamic_cast<EffectObservingEye*>(pPC->findEffect( Effect::EFFECT_CLASS_OBSERVING_EYE
    //) );
    //		//Assert( pEffectObservingEye );
    //	}

    if (pMonster->isFlag(Effect::EFFECT_CLASS_HIDE)) {
        // 뱀파거나 볼 수 있다면..
        //		if (canSeeAll
        //			|| pPC->isVampire()
        //			|| pPC->isFlag(Effect::EFFECT_CLASS_DETECT_HIDDEN) )
        //			|| ( pEffectRevealer != NULL && pEffectRevealer->canSeeHide( pMonster ) ) )
        {
            GCAddBurrowingCreature* pPacket = new GCAddBurrowingCreature();

            pPacket->setObjectID(pMonster->getObjectID());
            pPacket->setName(pMonster->getName());
            pPacket->setX(pMonster->getX());
            pPacket->setY(pMonster->getY());

            return pPacket;
        }

    }
    // 박쥐인 상태
    else if (pMonster->isFlag(Effect::EFFECT_CLASS_TRANSFORM_TO_BAT)) {
        GCAddBat* pPacket = new GCAddBat();
        pPacket->setObjectID(pMonster->getObjectID());
        pPacket->setName(pMonster->getName());
        pPacket->setXYDir(pMonster->getX(), pMonster->getY(), pMonster->getDir());
        pPacket->setItemType(0); // 아직 안 쓴다.
        pPacket->setMaxHP(pMonster->getHP(ATTR_MAX));
        pPacket->setCurrentHP(pMonster->getHP(ATTR_CURRENT));
        pPacket->setGuildID(1);
        pPacket->setColor(0);

        return pPacket;
    }
    // 늑대인 상태
    else if (pMonster->isFlag(Effect::EFFECT_CLASS_TRANSFORM_TO_WOLF)) {
        GCAddWolf* pPacket = new GCAddWolf();
        pPacket->setObjectID(pMonster->getObjectID());
        pPacket->setName(pMonster->getName());
        pPacket->setXYDir(pMonster->getX(), pMonster->getY(), pMonster->getDir());
        pPacket->setItemType(0); // 아직 안 쓴다.
        pPacket->setMaxHP(pMonster->getHP(ATTR_MAX));
        pPacket->setCurrentHP(pMonster->getHP(ATTR_CURRENT));
        pPacket->setGuildID(1);

        return pPacket;
    }
    // invisiblity 상태
    else if (pMonster->isFlag(Effect::EFFECT_CLASS_INVISIBILITY)) {
        // 보이나? 뱀파거나 볼수 있다면..
        //		if (canSeeAll
        //			|| pPC->isVampire()
        //			|| pPC->isFlag(Effect::EFFECT_CLASS_DETECT_INVISIBILITY)
        //			|| ( pEffectObservingEye != NULL && pEffectObservingEye->canSeeInvisibility( pMonster ) ) )
        {
            // FIXME
            // 설정에따라서 어떻게 보일지 결정된 후..
            GCAddMonster* pPacket = new GCAddMonster();
            makeGCAddMonster(pPacket, pMonster);
            pPacket->setEffectInfo(pMonster->getEffectInfo());

            return pPacket;
        }
    } else {
        GCAddMonster* pPacket = new GCAddMonster();
        makeGCAddMonster(pPacket, pMonster);

        return pPacket;
    }

    return NULL;
}

bool Zone::deleteNPC(Creature* pCreature)

{
    __BEGIN_TRY

    if (pCreature == NULL || !pCreature->isNPC())
        return false;

    try {
        deleteCreature(pCreature, pCreature->getX(), pCreature->getY());
        g_pPCFinder->deleteNPC(pCreature->getName());

    } catch (NoSuchElementException) {
        cout << "NoSuchNPC : " << pCreature->getName().c_str() << ", (" << pCreature->getX() << ", "
             << pCreature->getY() << ")" << endl;

        return false;
    }

    NPC* pNPC = dynamic_cast<NPC*>(pCreature);
    removeNPCInfo(pNPC);

    SAFE_DELETE(pCreature);

    return true;

    __END_CATCH
}

void Zone::deleteNPCs(Race_t race)

{
    __BEGIN_TRY

    const unordered_map<ObjectID_t, Creature*>& NPCs =
        m_pNPCManager->getCreatures(); // unordered_map을 복사해서 써야한다.
    unordered_map<ObjectID_t, Creature*>::const_iterator itr = NPCs.begin();

    list<ObjectID_t> creatures;

    // 일단 ObjectID들을 저장해둔다.
    for (; itr != NPCs.end(); itr++) {
        Creature* pCreature = itr->second;
        creatures.push_back(pCreature->getObjectID());
    }

    list<ObjectID_t>::iterator oitr = creatures.begin();

    // NPC를 지운다.
    for (; oitr != creatures.end(); oitr++) {
        Creature* pCreature = m_pNPCManager->getCreature(*oitr);

        if (pCreature != NULL) {
            Assert(pCreature->isNPC());

            NPC* pNPC = dynamic_cast<NPC*>(pCreature);

            if (pNPC->getRace() == race) {
                deleteNPC(pCreature);
            }
        }
    }

    sendNPCInfo();

    __END_CATCH
}

void Zone::killAllMonsters()

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    unordered_map<ObjectID_t, Creature*>& monsters = m_pMonsterManager->getCreatures();
    unordered_map<ObjectID_t, Creature*>::iterator itr = monsters.begin();

    for (; itr != monsters.end(); itr++) {
        Creature* pCreature = itr->second;
        Monster* pMonster = dynamic_cast<Monster*>(pCreature);

        if (!pMonster->isFlag(Effect::EFFECT_CLASS_NO_DAMAGE)) {
            pMonster->setHP(0);
        }
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

void Zone::killAllMonsters_UNLOCK()

{
    __BEGIN_TRY

    unordered_map<ObjectID_t, Creature*>& monsters = m_pMonsterManager->getCreatures();
    unordered_map<ObjectID_t, Creature*>::iterator itr = monsters.begin();

    for (; itr != monsters.end(); itr++) {
        Creature* pCreature = itr->second;
        Monster* pMonster = dynamic_cast<Monster*>(pCreature);

        if (!pMonster->isFlag(Effect::EFFECT_CLASS_NO_DAMAGE)) {
            pMonster->setHP(0);
        }
    }

    __END_CATCH
}

void Zone::killAllPCs() {
    __BEGIN_TRY

    unordered_map<ObjectID_t, Creature*>& pcs = m_pPCManager->getCreatures();
    unordered_map<ObjectID_t, Creature*>::iterator itr = pcs.begin();

    for (; itr != pcs.end(); itr++) {
        Creature* pCreature = itr->second;

        if (pCreature->isSlayer()) {
            dynamic_cast<Slayer*>(pCreature)->setHP(0);
        } else if (pCreature->isVampire()) {
            dynamic_cast<Vampire*>(pCreature)->setHP(0);
        } else if (pCreature->isOusters()) {
            dynamic_cast<Ousters*>(pCreature)->setHP(0);
        }
    }

    __END_CATCH
}
