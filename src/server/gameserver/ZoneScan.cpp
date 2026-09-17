//////////////////////////////////////////////////////////////////////////////
// FileName 	: ZoneScan.cpp
// Description	: Zone scan and visibility: what a creature sees and who sees it.
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
#include "LogClient.h"
#include "MasterLairInfoManager.h"
#include "MasterLairManager.h"
#include "Monster.h"
#include "MonsterCorpse.h"
#include "MonsterManager.h"
#include "NPC.h"
#include "NPCInfo.h"
#include "NPCManager.h"
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
#include "Properties.h"
#include "QuestManager.h"
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
#include "VisionInfo.h"
#include "War.h"
#include "WarScheduler.h"
#include "WarSystem.h"
#include "WeatherManager.h"
#include "Zone.h"
#include "ZoneGroup.h"
#include "ZoneInfo.h"
#include "ZoneInfoManager.h"
#include "ZoneUtil.h"
#include "ctf/FlagManager.h"
#include "repository/ComebackEventRepository.h"
#include "repository/MessageRepository.h"
#include "repository/ZoneInfoRepository.h"
// #include "EffectRevealer.h"
#include "EffectAddItem.h"
#include "EffectAddItemToCorpse.h"
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
#include "ZoneInternal.h"
#include "item/Motorcycle.h"


#ifdef __PROFILE_BROADCAST__
#define __BEGIN_PROFILE_ZONE(name) beginProfileEx(name);
#define __END_PROFILE_ZONE(name) endProfileEx(name);
#else
#define __BEGIN_PROFILE_ZONE(name) ((void)0);
#define __END_PROFILE_ZONE(name) ((void)0);
#endif


#ifndef __FULL_PROFILE__
#undef beginProfileEx
#define beginProfileEx(name) ((void)0)
#undef endProfileEx
#define endProfileEx(name) ((void)0)
#endif

//////////////////////////////////////////////////////////////////////////////
// detect invisibility등의 효과가 사라진 경우..보이는 놈이 안보이게 될 경우
// pCreature에게 GCDeleteObject를 보내준다. 보고 있던 invisible creature를
// delete한다. 또는 안보인는 넘이 보이게 될 경우등..
//////////////////////////////////////////////////////////////////////////////
void Zone::updateInvisibleScan(Creature* pCreature) {
    __BEGIN_TRY

    Assert(pCreature != NULL && pCreature->isPC());

    Coord_t cx = pCreature->getX();
    Coord_t cy = pCreature->getY();
    Player* pPlayer = pCreature->getPlayer();

    // Revealer 이펙트를 가져온다.

    // ObservingEey 이펙트를 가져온다.
    EffectObservingEye* pEffectObservingEye = NULL;
    if (pCreature->isFlag(Effect::EFFECT_CLASS_OBSERVING_EYE)) {
        pEffectObservingEye =
            dynamic_cast<EffectObservingEye*>(pCreature->findEffect(Effect::EFFECT_CLASS_OBSERVING_EYE));
    }

    EffectGnomesWhisper* pEffectGnomesWhisper = NULL;
    // GnomesWhisper 이펙트를 가져온다.
    if (pCreature->isFlag(Effect::EFFECT_CLASS_GNOMES_WHISPER)) {
        pEffectGnomesWhisper =
            dynamic_cast<EffectGnomesWhisper*>(pCreature->findEffect(Effect::EFFECT_CLASS_GNOMES_WHISPER));
    }

    for (ZoneCoord_t ix = max(0, cx - maxViewportWidth - 1), endx = min(m_Width - 1, cx + maxViewportWidth + 1);
         ix <= endx; ix++) {
        for (ZoneCoord_t iy = max(0, cy - maxViewportUpperHeight - 1),
                         endy = min(m_Height - 1, cy + maxViewportLowerHeight + 1);
             iy <= endy; iy++) {
            // darkness영역 조사.
            // 사실 pCreature는 당연히 slayer다.(updateInvisibleScan이므로..)
            if (pCreature->isSlayer() || pCreature->isOusters()) {
                const forward_list<Object*>& objectList = m_pTiles[ix][iy].getObjectList();
                forward_list<Object*>::const_iterator itr = objectList.begin();

                for (; itr != objectList.end() && (*itr)->getObjectPriority() <= OBJECT_PRIORITY_BURROWING_CREATURE;
                     itr++) {
                    if ((*itr)->getObjectClass() == Object::OBJECT_CLASS_CREATURE) {
                        Creature* pPC = dynamic_cast<Creature*>(*itr);
                        Assert(pPC != NULL);

                        // 자기 자신일 경우 통과
                        if (pCreature == pPC || pPC->isFlag(Effect::EFFECT_CLASS_GHOST))
                            continue;

                        // 숨어있는 대상에 대해서..
                        // SNIPING이나 INVISIBILITY상태일 경우.
                        if (pPC->isFlag(Effect::EFFECT_CLASS_INVISIBILITY) &&
                            pCreature->getVisionState(ix, iy) >= IN_SIGHT) {
                            // Detect Invisibility 이펙트가 있거나 뱀파이어면 볼 수 있다
                            // ObservingEye 이펙트가 있을 경우 상대방을 볼 수 있는 레벨이라면
                            if (pCreature->isFlag(Effect::EFFECT_CLASS_DETECT_INVISIBILITY) || pCreature->isVampire() ||
                                (pEffectObservingEye != NULL && pEffectObservingEye->canSeeInvisibility(pPC)) ||
                                (pEffectGnomesWhisper != NULL && pEffectGnomesWhisper->canSeeInvisibility())) {
                                if (pPC->isVampire()) {
                                    Vampire* pVampire = dynamic_cast<Vampire*>(pPC);
                                    GCAddVampire gcAddVampire;

                                    makeGCAddVampire(&gcAddVampire, pVampire);
                                    pPlayer->sendPacket(&gcAddVampire);
                                } else if (pPC->isMonster()) {
                                    Monster* pMonster = dynamic_cast<Monster*>(pPC);

                                    // by sigi
                                    Packet* pAddMonsterPacket = createMonsterAddPacket(pMonster, pCreature);

                                    if (pAddMonsterPacket != NULL) {
                                        pPlayer->sendPacket(pAddMonsterPacket);

                                        delete pAddMonsterPacket;
                                    }
                                }
                            } else {
                                GCDeleteObject gcDO;
                                gcDO.setObjectID(pPC->getObjectID());
                                pPlayer->sendPacket(&gcDO);
                            }
                        } else if (pPC->isFlag(Effect::EFFECT_CLASS_SNIPING_MODE)) {
                            if ((!pCreature->isVampire() &&
                                 pCreature->isFlag(Effect::EFFECT_CLASS_DETECT_INVISIBILITY)))
                            //								|| ( pEffectRevealer != NULL &&
                            // pEffectRevealer->canSeeSniping( pPC ) ) )
                            {
                                if (pPC->isSlayer()) {
                                    Slayer* pSlayer = dynamic_cast<Slayer*>(pPC);

                                    GCAddSlayer gcAddSlayer;
                                    makeGCAddSlayer(&gcAddSlayer, pSlayer);
                                    pPlayer->sendPacket(&gcAddSlayer);
                                } else {
                                    throw Error("A vampire is in sniping mode.");
                                }
                            } else {
                                GCDeleteObject gcDO;
                                gcDO.setObjectID(pPC->getObjectID());
                                pPlayer->sendPacket(&gcDO);
                            }
                        }
                    }
                }
            } // darkness
        }
    }

    __END_CATCH
}

//--------------------------------------------------------------------------------
// update hidden scan
// detect hidden등의 효과가 사라진 경우..보이는 놈이 안보이게 될 경우
// pCreature에게 GCDeleteObject를 보내준다.
// 보고 있던 burrow creature를 delete한다. 또는 안보인는 넘이 보이게 될 경우등..
// ABCD
//--------------------------------------------------------------------------------
void Zone::updateHiddenScan(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL && pCreature->isPC());

    Coord_t cx = pCreature->getX();
    Coord_t cy = pCreature->getY();
    Player* pPlayer = pCreature->getPlayer();

    // Revealer 이펙트를 가져온다.

    for (ZoneCoord_t ix = max(0, cx - maxViewportWidth - 1), endx = min(m_Width - 1, cx + maxViewportWidth + 1);
         ix <= endx; ix++) {
        for (ZoneCoord_t iy = max(0, cy - maxViewportUpperHeight - 1),
                         endy = min(m_Height - 1, cy + maxViewportLowerHeight + 1);
             iy <= endy; iy++) {
            // darkness영역 조사.
            // 사실 pCreature는 당연히 slayer다.(updateHiddenScan이므로..)
            if (pCreature->isSlayer()) {
                const forward_list<Object*>& objectList = m_pTiles[ix][iy].getObjectList();

                for (forward_list<Object*>::const_iterator itr = objectList.begin();
                     itr != objectList.end() && (*itr)->getObjectPriority() <= OBJECT_PRIORITY_BURROWING_CREATURE;
                     itr++) {
                    if ((*itr)->getObjectClass() == Object::OBJECT_CLASS_CREATURE) {
                        Creature* pPC = dynamic_cast<Creature*>(*itr);
                        Assert(pPC != NULL);

                        // 자기 자신일 경우 통과
                        if (pCreature == pPC)
                            continue;

                        // 숨어있는 대상에 대해서..
                        if (pPC->isFlag(Effect::EFFECT_CLASS_HIDE) && pCreature->getVisionState(ix, iy) >= IN_SIGHT) {
                            if (pCreature->isFlag(Effect::EFFECT_CLASS_DETECT_HIDDEN) || pCreature->isVampire())
                            //								|| ( pEffectRevealer != NULL && pEffectRevealer->canSeeHide(
                            // pPC ) ) )
                            {
                                GCAddBurrowingCreature gcABC;
                                gcABC.setObjectID(pPC->getObjectID());
                                gcABC.setName(pPC->getName());
                                gcABC.setX(ix);
                                gcABC.setY(iy);
                                pPlayer->sendPacket(&gcABC);
                            } else {
                                GCDeleteObject gcDO;
                                gcDO.setObjectID(pPC->getObjectID());
                                pPlayer->sendPacket(&gcDO);
                            }
                        }
                    }
                }
            } // darkness
        }
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Detect 기능이 생기거나 없어질 경우.
// 보는 Creature 의 Creature 추가 삭제 처리
// 아우스터스 용
//////////////////////////////////////////////////////////////////////////////
void Zone::updateDetectScan(Creature* pCreature) {
    __BEGIN_TRY

    Assert(pCreature != NULL && pCreature->isPC());

    Coord_t cx = pCreature->getX();
    Coord_t cy = pCreature->getY();
    Player* pPlayer = pCreature->getPlayer();

    EffectGnomesWhisper* pEffectGnomesWhisper = NULL;
    if (pCreature->isFlag(Effect::EFFECT_CLASS_GNOMES_WHISPER)) {
        pEffectGnomesWhisper =
            dynamic_cast<EffectGnomesWhisper*>(pCreature->findEffect(Effect::EFFECT_CLASS_GNOMES_WHISPER));
    }

    for (ZoneCoord_t ix = max(0, cx - maxViewportWidth - 1), endx = min(m_Width - 1, cx + maxViewportWidth + 1);
         ix <= endx; ix++) {
        for (ZoneCoord_t iy = max(0, cy - maxViewportUpperHeight - 1),
                         endy = min(m_Height - 1, cy + maxViewportLowerHeight + 1);
             iy <= endy; iy++) {
            if (pCreature->getVisionState(ix, iy) == OUT_OF_SIGHT)
                continue;

            if (pCreature->isOusters()) {
                const forward_list<Object*>& objectList = m_pTiles[ix][iy].getObjectList();
                forward_list<Object*>::const_iterator itr = objectList.begin();

                for (; itr != objectList.end() && (*itr)->getObjectPriority() <= OBJECT_PRIORITY_BURROWING_CREATURE;
                     itr++) {
                    if ((*itr)->getObjectClass() == Object::OBJECT_CLASS_CREATURE) {
                        Creature* pPC = dynamic_cast<Creature*>(*itr);
                        Assert(pPC != NULL);

                        // 자기 자신일 경우 통과
                        if (pCreature == pPC || pPC->isFlag(Effect::EFFECT_CLASS_GHOST))
                            continue;

                        if (pPC->isSlayer()) {
                            if (pPC->isFlag(Effect::EFFECT_CLASS_SNIPING_MODE)) {
                                if (canSee(pCreature, pPC, NULL, pEffectGnomesWhisper)) {
                                    Slayer* pSlayer = dynamic_cast<Slayer*>(pPC);

                                    GCAddSlayer gcAddSlayer;
                                    makeGCAddSlayer(&gcAddSlayer, pSlayer);
                                    pPlayer->sendPacket(&gcAddSlayer);
                                } else {
                                    GCDeleteObject gcDO;
                                    gcDO.setObjectID(pPC->getObjectID());
                                    pPlayer->sendPacket(&gcDO);
                                }
                            }
                        } else if (pPC->isVampire()) {
                            if (pPC->isFlag(Effect::EFFECT_CLASS_INVISIBILITY)) {
                                if (canSee(pCreature, pPC, NULL, pEffectGnomesWhisper)) {
                                    Vampire* pVampire = dynamic_cast<Vampire*>(pPC);
                                    GCAddVampire gcAddVampire;

                                    makeGCAddVampire(&gcAddVampire, pVampire);
                                    pPlayer->sendPacket(&gcAddVampire);
                                } else {
                                    GCDeleteObject gcDO;
                                    gcDO.setObjectID(pPC->getObjectID());
                                    pPlayer->sendPacket(&gcDO);
                                }
                            }

                            if (pPC->isFlag(Effect::EFFECT_CLASS_HIDE)) {
                                if (canSee(pCreature, pPC, NULL, pEffectGnomesWhisper)) {
                                    GCAddBurrowingCreature gcABC;
                                    gcABC.setObjectID(pPC->getObjectID());
                                    gcABC.setName(pPC->getName());
                                    gcABC.setX(ix);
                                    gcABC.setY(iy);
                                    pPlayer->sendPacket(&gcABC);
                                } else {
                                    GCDeleteObject gcDO;
                                    gcDO.setObjectID(pPC->getObjectID());
                                    pPlayer->sendPacket(&gcDO);
                                }
                            }
                        } else if (pPC->isMonster()) {
                            if (pPC->isFlag(Effect::EFFECT_CLASS_INVISIBILITY) ||
                                pPC->isFlag(Effect::EFFECT_CLASS_HIDE)) {
                                if (canSee(pCreature, pPC, NULL, pEffectGnomesWhisper)) {
                                    Monster* pMonster = dynamic_cast<Monster*>(pPC);

                                    Packet* pAddMonsterPacket = createMonsterAddPacket(pMonster, pCreature);

                                    if (pAddMonsterPacket != NULL) {
                                        pPlayer->sendPacket(pAddMonsterPacket);

                                        delete pAddMonsterPacket;
                                    }
                                } else {
                                    GCDeleteObject gcDO;
                                    gcDO.setObjectID(pPC->getObjectID());
                                    pPlayer->sendPacket(&gcDO);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    __END_CATCH
}

//--------------------------------------------------------------------------------
// update mine scan
// detect mine등의 효과가 사라진 경우..보이는 mine이 안보이게 될 경우
// pCreature에게 GCDeleteObject를 보내준다.
//--------------------------------------------------------------------------------
void Zone::updateMineScan(Creature* pCreature) {
    __BEGIN_TRY

    Assert(pCreature != NULL && pCreature->isPC());

    Coord_t cx = pCreature->getX();
    Coord_t cy = pCreature->getY();
    Player* pPlayer = pCreature->getPlayer();

    for (ZoneCoord_t ix = max(0, cx - maxViewportWidth - 1), endx = min(m_Width - 1, cx + maxViewportWidth + 1);
         ix <= endx; ix++) {
        for (ZoneCoord_t iy = max(0, cy - maxViewportUpperHeight - 1),
                         endy = min(m_Height - 1, cy + maxViewportLowerHeight + 1);
             iy <= endy; iy++) {
            if (pCreature->getVisionState(ix, iy) == OUT_OF_SIGHT)
                continue;

            // 사실 pCreature는 당연히 slayer다.(updateMineScan이므로..)
            if (pCreature->isSlayer()) {
                Item* pItem = m_pTiles[ix][iy].getItem();
                if (pItem) {
                    if (pItem->getItemClass() == Item::ITEM_CLASS_MINE && pItem->isFlag(Effect::EFFECT_CLASS_INSTALL)) {
                        if (pCreature->isFlag(Effect::EFFECT_CLASS_REVEALER)) {
                            GCAddInstalledMineToZone gcAddMine;
                            gcAddMine.setObjectID(pItem->getObjectID());
                            gcAddMine.setX(ix);
                            gcAddMine.setY(iy);
                            gcAddMine.setItemClass(pItem->getItemClass());
                            gcAddMine.setItemType(pItem->getItemType());
                            gcAddMine.setOptionType(pItem->getOptionTypeList());
                            gcAddMine.setDurability(pItem->getDurability());

                            pPlayer->sendPacket(&gcAddMine);
                        } else {
                            GCDeleteObject gcDO;
                            gcDO.setObjectID(pItem->getObjectID());
                            pPlayer->sendPacket(&gcDO);
                        }
                    }
                }
            } // darkness
        }
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// scan
// (x,y)에서 시야 영역안에 존재하는 모든 객체들의 정보를 받아온다.
//////////////////////////////////////////////////////////////////////////////
void Zone::scan(Creature* pPC, ZoneCoord_t cx, ZoneCoord_t cy, Packet* pPacket) {
    __BEGIN_TRY

    __BEGIN_PROFILE_ZONE("Z_SCAN")

#ifdef __USE_ENCRYPTER__
    SocketEncryptOutputStream outputStream(NULL, szPacketHeader + pPacket->getPacketSize() + 2);
    outputStream.setEncryptCode(m_EncryptCode);
#else
    SocketOutputStream outputStream(NULL, szPacketHeader + pPacket->getPacketSize() + 2);
#endif
    pPacket->writeHeaderNBody(outputStream);

    Assert(pPC->isPC());

    Player* pPlayer = pPC->getPlayer();
    Assert(pPlayer);

    for (ZoneCoord_t ix = max(0, cx - maxViewportWidth - 1), endx = min(m_Width - 1, cx + maxViewportWidth + 1);
         ix <= endx; ix++) {
        for (ZoneCoord_t iy = max(0, cy - maxViewportUpperHeight - 1),
                         endy = min(m_Height - 1, cy + maxViewportLowerHeight + 1);
             iy <= endy; iy++) {
            if (pPC->getVisionState(ix, iy) == OUT_OF_SIGHT)
                continue;

            const forward_list<Object*>& objectList = m_pTiles[ix][iy].getObjectList();

            for (forward_list<Object*>::const_iterator itr = objectList.begin(); itr != objectList.end(); itr++) {
                Assert(*itr != NULL);

                //--------------------------------------------------------------------------------
                //
                // 각 객체의 OBJECT CLASS에 따라서 적합한 GCAddXXX 패킷을 만들어서
                // owner 에게 전송한다.
                //
                // *NOTES*
                //
                // 가장 출현 확률이 높은 객체 CLASS 가 case 앞부분에 나와야 한다.
                //
                //--------------------------------------------------------------------------------
                switch ((*itr)->getObjectClass()) {
                //--------------------------------------------------------------------------------
                // 타일 위에 크리처가 있을 경우
                //--------------------------------------------------------------------------------
                case Object::OBJECT_CLASS_CREATURE: {
                    //--------------------------------------------------------------------------------
                    // PC의 경우 pPacket을 전송해야 하며, !PC인 경우에는 전송할 필요가 없다.
                    // 또한 모든 크리처의 정보를 owner에게 전송해야 한다.
                    //--------------------------------------------------------------------------------
                    Creature* pCreature = dynamic_cast<Creature*>(*itr);
                    Assert(pCreature != NULL);

                    if (pCreature == pPC) // 자기 자신의 정보는 받을 필요가 없다.
                        continue;

                    // 안보이면 쌩
                    bool bCanSee = canSee(pPC, pCreature);

                    switch (pCreature->getCreatureClass()) {
                    case Creature::CREATURE_CLASS_MONSTER: {
                        Monster* pMonster = dynamic_cast<Monster*>(pCreature);
                        if (bCanSee) {
                            // by sigi
                            Packet* pAddMonsterPacket = createMonsterAddPacket(pMonster, pPC);

                            if (pAddMonsterPacket != NULL) {
                                pPlayer->sendPacket(pAddMonsterPacket);

                                delete pAddMonsterPacket;
                            }
                        }

                        //--------------------------------------------------------------------------------
                        // 몬스터가 PC 를 볼 수 있는 경우, PC 를 몬스터의 Enemy 로 지정한다.
                        //--------------------------------------------------------------------------------
                        VisionState vs = pMonster->getVisionState(cx, cy);

                        // Aggressive 몬스터일 경우에만 적으로 등록해준다.
                        if (vs >= IN_SIGHT && pMonster->getAlignment() == ALIGNMENT_AGGRESSIVE) {
                            if (isPotentialEnemy(pMonster, pPC)) {
                                pMonster->addPotentialEnemy(pPC);
                            }
                        }
                    } break;

                    case Creature::CREATURE_CLASS_SLAYER: {
                        // 내가 그곳을 볼 수 있다면(darkness와 관련하여)
                        if (bCanSee) {
                            //											if
                            //(!pCreature->isFlag(Effect::EFFECT_CLASS_GHOST)
                            //												&&
                            //! pCreature->isFlag(Effect::EFFECT_CLASS_SNIPING_MODE) ||
                            // pPC->isFlag(Effect::EFFECT_CLASS_DETECT_INVISIBILITY) )
                            //													|| ( pEffectRevealer != NULL &&
                            // pEffectRevealer->canSeeSniping( pCreature ) ) )
                            //											{
                            Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
                            GCAddSlayer gcAddSlayer;
                            makeGCAddSlayer(&gcAddSlayer, pSlayer);
                            pPlayer->sendPacket(&gcAddSlayer);
                            //											}
                        }

                        // 상대(슬레이어)가 나를 볼 수 있다면
                        if (pPacket && pCreature->getVisionState(cx, cy) >= IN_SIGHT) {
                            Assert(pCreature->getPlayer() != NULL);
                            // canSee 로 대체. 2003.05.29 by bezz
                            if (canSee(pCreature, pPC)) {
                                pCreature->getPlayer()->sendStream(&outputStream);
                            }
                        }
                    } break;

                    case Creature::CREATURE_CLASS_VAMPIRE: {
                        if (bCanSee) {
                            // PC가 ObservingEye 이펙트를 가지고 있다면 이펙트를 가져온다.
                            //												//Assert( pEffectObservingEye != NULL );

                            if (pCreature->isFlag(Effect::EFFECT_CLASS_HIDE)) {
                                GCAddBurrowingCreature gcABC;
                                gcABC.setObjectID(pCreature->getObjectID());
                                gcABC.setName(pCreature->getName());
                                gcABC.setX(ix);
                                gcABC.setY(iy);

                                pPlayer->sendPacket(&gcABC);
                                //				}
                            } else {
                                Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);
                                GCAddVampire gcAddVampire;
                                makeGCAddVampire(&gcAddVampire, pVampire);
                                pPlayer->sendPacket(&gcAddVampire);
                                //				}
                            }
                        }

                        // 상대가 나를 볼 수 있다면..
                        // 상대는 vampire이므로 시야만 가능하다면 darkness와는 관계가 없다.
                        // 뱀파이어가 상대일땐 스나이핑 모드라면 절대 못 본다...
                        //
                        // 근데 scan 함수 특성상 snipping 모드를 해제 하지 않고 넘어갈 수는 없다.
                        // canSee로 대체
                        if (pPacket && pCreature->getVisionState(cx, cy) >= IN_SIGHT && canSee(pCreature, pPC)) {
                            Assert(pCreature->getPlayer() != NULL);
                            pCreature->getPlayer()->sendStream(&outputStream);
                        }
                    } break;

                    case Creature::CREATURE_CLASS_OUSTERS: {
                        if (bCanSee) {
                            // PC가 ObservingEye 이펙트를 가지고 있다면 이펙트를 가져온다.
                            //												//Assert( pEffectObservingEye != NULL );

                            Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);
                            GCAddOusters gcAddOusters;
                            makeGCAddOusters(&gcAddOusters, pOusters);
                            pPlayer->sendPacket(&gcAddOusters);
                            //											}
                        }

                        if (pPacket && pCreature->getVisionState(cx, cy) >= IN_SIGHT && canSee(pCreature, pPC)) {
                            Assert(pCreature->getPlayer() != NULL);
                            pCreature->getPlayer()->sendStream(&outputStream);
                        }
                    } break;

                    case Creature::CREATURE_CLASS_NPC: {
                        if (bCanSee) {
                            NPC* pNPC = dynamic_cast<NPC*>(pCreature);
                            GCAddNPC gcAddNPC;
                            makeGCAddNPC(&gcAddNPC, pNPC);
                            pPlayer->sendPacket(&gcAddNPC);
                        }
                    } break;

                    default:
                        throw Error("invalid creature class");

                    } // switch (pCreature->getCreatureClass())
                } // case Object::OBJECT_CLASS_CREATURE :

                break;

                //--------------------------------------------------------------------------------
                // 타일 위에 아이템이 있을 경우
                //--------------------------------------------------------------------------------
                case Object::OBJECT_CLASS_ITEM: {
                    Item* pItem = dynamic_cast<Item*>(*itr);

                    if (pItem->getItemClass() == Item::ITEM_CLASS_CORPSE) {
                        switch (pItem->getItemType()) {
                        case SLAYER_CORPSE: {
                            SlayerCorpse* pSlayerCorpse = dynamic_cast<SlayerCorpse*>(pItem);
                            GCAddSlayerCorpse gcAddSlayerCorpse;
                            makeGCAddSlayerCorpse(&gcAddSlayerCorpse, pSlayerCorpse);
                            pPlayer->sendPacket(&gcAddSlayerCorpse);
                        } break;
                        case VAMPIRE_CORPSE: {
                            VampireCorpse* pVampireCorpse = dynamic_cast<VampireCorpse*>(pItem);
                            GCAddVampireCorpse gcAddVampireCorpse;
                            makeGCAddVampireCorpse(&gcAddVampireCorpse, pVampireCorpse);
                            pPlayer->sendPacket(&gcAddVampireCorpse);
                        } break;
                        case OUSTERS_CORPSE: {
                            OustersCorpse* pOustersCorpse = dynamic_cast<OustersCorpse*>(pItem);
                            GCAddOustersCorpse gcAddOustersCorpse;
                            makeGCAddOustersCorpse(&gcAddOustersCorpse, pOustersCorpse);
                            pPlayer->sendPacket(&gcAddOustersCorpse);
                        } break;
                        case NPC_CORPSE: {
                            throw UnsupportedError();
                        } break;
                        case MONSTER_CORPSE: {
                            MonsterCorpse* pMonsterCorpse = dynamic_cast<MonsterCorpse*>(pItem);
                            GCAddMonsterCorpse gcAddMonsterCorpse;
                            makeGCAddMonsterCorpse(&gcAddMonsterCorpse, pMonsterCorpse, ix, iy);
                            pPlayer->sendPacket(&gcAddMonsterCorpse);

                            sendRelicEffect(pMonsterCorpse, pPlayer);
                        } break;
                        } // switch
                    } else if (pItem->getItemClass() == Item::ITEM_CLASS_MINE &&
                               pItem->isFlag(Effect::EFFECT_CLASS_INSTALL)) {
                        if (pPC->isFlag(Effect::EFFECT_CLASS_REVEALER)) {
                            GCAddInstalledMineToZone gcAddMine;
                            gcAddMine.setObjectID(pItem->getObjectID());
                            gcAddMine.setX(cx);
                            gcAddMine.setY(cy);
                            gcAddMine.setItemClass(pItem->getItemClass());
                            gcAddMine.setItemType(pItem->getItemType());
                            gcAddMine.setOptionType(pItem->getOptionTypeList());
                            gcAddMine.setDurability(pItem->getDurability());
                            pPlayer->sendPacket(&gcAddMine);
                        }
                    } else {
                        GCAddNewItemToZone gcAddNewItemToZone;
                        makeGCAddNewItemToZone(&gcAddNewItemToZone, pItem, ix, iy);
                        pPlayer->sendPacket(&gcAddNewItemToZone);
                    }
                    //							}
                } break;

                //--------------------------------------------------------------------------------
                // 타일 위에 이펙트가 있을 경우
                //--------------------------------------------------------------------------------
                case Object::OBJECT_CLASS_EFFECT: {
                    Effect* pEffect = dynamic_cast<Effect*>(*itr);
                    if (pEffect->getEffectClass() == Effect::EFFECT_CLASS_VAMPIRE_PORTAL) {
                        EffectVampirePortal* pEffectVampirePortal = dynamic_cast<EffectVampirePortal*>(pEffect);
                        ZONE_COORD zonecoord = pEffectVampirePortal->getZoneCoord();

                        GCAddVampirePortal gcAddVampirePortal;
                        gcAddVampirePortal.setObjectID(pEffect->getObjectID());
                        gcAddVampirePortal.setOwnerID(pEffectVampirePortal->getOwnerID());
                        gcAddVampirePortal.setX(ix);
                        gcAddVampirePortal.setY(iy);
                        gcAddVampirePortal.setTargetZoneID(zonecoord.id);
                        gcAddVampirePortal.setTargetX(zonecoord.x);
                        gcAddVampirePortal.setTargetY(zonecoord.y);
                        gcAddVampirePortal.setDuration(pEffectVampirePortal->getRemainDuration());
                        gcAddVampirePortal.setCreateFlag(0);

                        pPlayer->sendPacket(&gcAddVampirePortal);
                    }
                    // by sigi. 2002.6.10
                    else if (pEffect->getEffectClass() == Effect::EFFECT_CLASS_SANCTUARY) {
                        EffectSanctuary* pEffectSanctuary = dynamic_cast<EffectSanctuary*>(pEffect);

                        ZoneCoord_t centerX = pEffectSanctuary->getCenterX();
                        ZoneCoord_t centerY = pEffectSanctuary->getCenterY();

                        // sanctuary는 중심좌표인 경우만 packet을 보낸다.
                        if (centerX == ix && centerY == iy) {
                            GCAddEffectToTile gcAddEffectToTile;

                            gcAddEffectToTile.setObjectID(pEffect->getObjectID());
                            gcAddEffectToTile.setXY(ix, iy);
                            gcAddEffectToTile.setEffectID(pEffect->getSendEffectClass());
                            gcAddEffectToTile.setDuration(pEffect->getRemainDuration());

                            pPlayer->sendPacket(&gcAddEffectToTile);
                        }
                    }
                    // Broadcasting Effect 인지 체크 추가 by Sequoia 2003.3.31
                    else if (pEffect->isBroadcastingEffect()) {
                        GCAddEffectToTile gcAddEffectToTile;

                        gcAddEffectToTile.setObjectID(pEffect->getObjectID());
                        gcAddEffectToTile.setXY(ix, iy);
                        gcAddEffectToTile.setEffectID(pEffect->getSendEffectClass());
                        gcAddEffectToTile.setDuration(pEffect->getRemainDuration());

                        pPlayer->sendPacket(&gcAddEffectToTile);
                    }
                    //							}
                } break;


                //--------------------------------------------------------------------------------
                // 타일 위에 장애물이 있을 경우
                //--------------------------------------------------------------------------------
                case Object::OBJECT_CLASS_OBSTACLE: {
                    /*
                     */
                } break;

                //--------------------------------------------------------------------------------
                // 타일 위에 포탈이 있을 경우
                //--------------------------------------------------------------------------------
                case Object::OBJECT_CLASS_PORTAL: {
                    /*
                     */
                } break;

                default:
                    throw Error("invalid object class");

                } // switch ((*itr)->getObjectClass())
            } // for

        } // for
    } // for

    __END_PROFILE_ZONE("Z_SCAN")

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// 몬스터가 존의 (x,y)에 새로 리젠되었을 경우, 시야 영역안에 존재하는 모든 PC들에게
// GCAddXXX 패킷을 보내면서, 동시에 그 PC 를 잠재적인 적으로 간주한다.
//////////////////////////////////////////////////////////////////////////////
void Zone::scanPC(Creature* pCreature) {
    __BEGIN_TRY

    __BEGIN_PROFILE_ZONE("Z_SCAN_PC")

    Monster* pMonster = NULL;

    Assert(pCreature != NULL);

    ZoneCoord_t cx = pCreature->getX();
    ZoneCoord_t cy = pCreature->getY();

    Packet* pGCAddXXX = NULL;

    // 크리쳐의 종류에 따라, 패킷을 만들어둔다.
    Creature::CreatureClass CClass = pCreature->getCreatureClass();

    bool isMonster = pCreature->isMonster();

    if (CClass == Creature::CREATURE_CLASS_MONSTER) {
        pMonster = dynamic_cast<Monster*>(pCreature);

        // by sigi
        pGCAddXXX = createMonsterAddPacket(pMonster, NULL);


    } else if (CClass == Creature::CREATURE_CLASS_NPC) {
        NPC* pNPC = dynamic_cast<NPC*>(pCreature);

        GCAddNPC* pGCAddNPC = new GCAddNPC;

        makeGCAddNPC(pGCAddNPC, pNPC);

        pGCAddXXX = pGCAddNPC;
    } else {
        Assert(false);
    }

    for (ZoneCoord_t ix = max(0, cx - maxViewportWidth - 1), endx = min(m_Width - 1, cx + maxViewportWidth + 1);
         ix <= endx; ix++) {
        for (ZoneCoord_t iy = max(0, cy - maxViewportUpperHeight - 1),
                         endy = min(m_Height - 1, cy + maxViewportLowerHeight + 1);
             iy <= endy; iy++) {
            const forward_list<Object*>& objectList = m_pTiles[ix][iy].getObjectList();
            forward_list<Object*>::const_iterator itr = objectList.begin();

            for (; itr != objectList.end() && (*itr)->getObjectPriority() <= OBJECT_PRIORITY_BURROWING_CREATURE;
                 itr++) {
                if ((*itr)->getObjectClass() == Object::OBJECT_CLASS_CREATURE) {
                    Creature* pPC = dynamic_cast<Creature*>(*itr);
                    Assert(pPC != NULL);

                    // PC 이면서, 크리처를 볼 수 있는 경우
                    if (pPC->isPC() && pPC->getVisionState(cx, cy) >= IN_SIGHT && canSee(pPC, pCreature))
                    //						&& !pPC->isFlag(Effect::EFFECT_CLASS_GHOST)
                    {
                        // Creature 가 Revealer 이펙트를 가지고 있다면 이펙트를 가져온다.

                        // Creature 가 ObservingEye 이펙트를 가지고 있다면 이펙트를 가져온다.
                        //							//Assert( pEffectObservingEye != NULL );

                        // 몬스터가 스나이핑을 쓸리는 없다 그래서 DETECT_HIDDEN과 INVISIBILITY만 체크 한다.
                        pPC->getPlayer()->sendPacket(pGCAddXXX);
                        //						}

                        if (isMonster) {
                            // (cx,cy)에 있는 몬스터가 (ix,iy)에 있는 PC를 볼 수 있는가?
                            VisionState vs = pMonster->getVisionState(ix, iy);
                            if (vs >= IN_SIGHT && pMonster->getAlignment() == ALIGNMENT_AGGRESSIVE &&
                                canSee(pCreature, pPC)) {
                                if (isPotentialEnemy(pMonster, pPC)) {
                                    pMonster->addPotentialEnemy(pPC);
                                }
                            }
                        }
                    }

                } // if (creature)

            } // for itr

        } // for y
    } // for x

    if (pGCAddXXX != NULL) {
        delete pGCAddXXX;
    }

    __END_PROFILE_ZONE("Z_SCAN_PC")

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// pTargetCreture를 볼 수 있는 자(player)들의 list를 돌려준다.
// **********************************
//////////////////////////////////////////////////////////////////////////////
list<Creature*> Zone::getWatcherList(ZoneCoord_t x, ZoneCoord_t y, Creature* pTargetCreature)

{
    __BEGIN_TRY

    list<Creature*> cList;

    __BEGIN_PROFILE_ZONE("Z_GET_WATCHERLIST")

    if (pTargetCreature == NULL)
        return cList;

    ////////////////////////////////////////////////////////////
    // 시야 영역의 상하좌우 모두 + 1 씩 증가시킨다.
    // 이유는 방향에 따른 ON_SIGHT 영역이 증가되기 때문이다.
    ////////////////////////////////////////////////////////////
    for (ZoneCoord_t ix = max(0, x - maxViewportWidth - 1), endx = min(m_Width - 1, x + maxViewportWidth + 1);
         ix <= endx; ix++) {
        for (ZoneCoord_t iy = max(0, y - maxViewportUpperHeight - 1),
                         endy = min(m_Height - 1, y + maxViewportLowerHeight + 1);
             iy <= endy; iy++) {
            const forward_list<Object*>& objectList = m_pTiles[ix][iy].getObjectList();
            forward_list<Object*>::const_iterator itr = objectList.begin();
            for (; itr != objectList.end() && (*itr)->getObjectPriority() <= OBJECT_PRIORITY_BURROWING_CREATURE;
                 itr++) {
                Assert(*itr != NULL);

                Creature* pCreature = dynamic_cast<Creature*>(*itr);

                Assert(pCreature != NULL);

                if (pCreature->isPC()) {
                    Assert(pCreature->getPlayer() != NULL);

                    // 자기 자신의 정보는 받을 필요가 없다.
                    if (pTargetCreature == pCreature || pCreature->isFlag(Effect::EFFECT_CLASS_GHOST))
                        continue;

                    VisionState vs = pCreature->getVisionState(x, y);

                    if (vs >= IN_SIGHT) {
                        if (canSee(pCreature, pTargetCreature)) {
                            cList.push_back(pCreature);
                        }
                    }
                } // if

            } // for

        } // for

    } // for

    __END_PROFILE_ZONE("Z_GET_WATCHERLIST")

    return cList;

    __END_CATCH
}

void Zone::monsterScan(Monster* pMonster, ZoneCoord_t x, ZoneCoord_t y, Dir_t dir)

{
    __BEGIN_TRY


    // [TEST CODE]

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

    ZoneCoord_t x2 = x;
    ZoneCoord_t y2 = y;

    //////////////////////////////////////////////////////////////////////////////
    // 시야 영역의 상하좌우 모두 + 1 씩 증가시킨다.
    // 이유는 방향에 따른 ON_SIGHT 영역이 증가되기 때문이다.
    //////////////////////////////////////////////////////////////////////////////
    int sight = pMonster->getSight();

    for (ZoneCoord_t ix = max(0, x2 - sight - 1), endx = min(m_Width - 1, x2 + sight + 1); ix <= endx; ix++) {
        for (ZoneCoord_t iy = max(0, y2 - sight - 1), endy = min(m_Height - 1, y2 + sight + 1); iy <= endy; iy++) {
            // 현재 타일 위에 있는 모든 오브젝트들에 대해 반복한다.
            const forward_list<Object*>& objectList = m_pTiles[ix][iy].getObjectList();

            forward_list<Object*>::const_iterator itr = objectList.begin();

            //
            // object가 있는 경우만
            // pVisionInfo->getVisionState()를 체크 하기 위해서
            // if - do~while 을 사용했다. by sigi. 2002.5.8
            //
            if (itr != objectList.end()) {
                do {
                    Assert(*itr != NULL);

                    Object::ObjectClass OClass = (*itr)->getObjectClass();

                    ////////////////////////////////////////////////////////////
                    // 각 객체의 OBJECT CLASS에 따라서 적합한 GCAddXXX 패킷을
                    // 만들어서 owner 에게 전송한다.
                    ////////////////////////////////////////////////////////////

                    ////////////////////////////////////////////////////////////
                    // 타일 위에 크리처가 있을 경우
                    ////////////////////////////////////////////////////////////
                    if (OClass == Object::OBJECT_CLASS_CREATURE) {
                        Creature* pCreature = dynamic_cast<Creature*>(*itr);
                        Assert(pCreature != NULL);

                        Creature::CreatureClass CClass = pCreature->getCreatureClass();

                        if (pCreature->isPC()) {
                            if (pMonster->isEnemyToAttack(pCreature)) {
                                pMonster->addPotentialEnemy(pCreature);
                            }
                        } else if (CClass == Creature::CREATURE_CLASS_MONSTER) {
                            Monster* pOtherMonster = dynamic_cast<Monster*>(pCreature);

                            VisionState vs = pOtherMonster->getVisionState(x2, y2);

                            // Aggressive 몬스터에게만 적으로 등록시켜준다.
                            if (vs >= IN_SIGHT) {
                                if (isPotentialEnemy(pMonster, pOtherMonster)) {
                                    pMonster->addPotentialEnemy(pOtherMonster);
                                    pOtherMonster->addPotentialEnemy(pMonster);
                                }
                            }
                        }
                    } // if (OClass==OBJECT_CLASS_CREATURE)
                } while (++itr != objectList.end()); // do
            } // if (itr != objectList.end())
        } // for iy
    } // for ix


    __END_CATCH
}
