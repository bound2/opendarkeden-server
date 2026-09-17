//////////////////////////////////////////////////////////////////////////////
// FileName 	: ZoneBroadcast.cpp
// Description	: Zone packet broadcast: sending zone events to the players who can see them.
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

//--------------------------------------------------------------------------------
//
// broadcast packet
//
// 특정 존에 존재하는, owner를 제외한 모든 PC 에게 지정된 패킷을 전송한다.
//
//--------------------------------------------------------------------------------
void Zone::broadcastPacket(Packet* pPacket, Creature* owner)

{
    __BEGIN_TRY

    __BEGIN_PROFILE_ZONE("Z_BC_PCMANAGER")
    m_pPCManager->broadcastPacket(pPacket, owner);
    __END_PROFILE_ZONE("Z_BC_PCMANAGER")

    __END_CATCH
}

void Zone::broadcastDarkLightPacket(Packet* pPacket1, Packet* pPacket2, Creature* owner)

{
    __BEGIN_TRY

    __BEGIN_PROFILE_ZONE("Z_BC_DARKLIGHT")
    m_pPCManager->broadcastDarkLightPacket(pPacket1, pPacket2, owner);
    __END_PROFILE_ZONE("Z_BC_DARKLIGHT")

    __END_CATCH
}

//--------------------------------------------------------------------
//
// 채팅을 브로드캐스팅 하는 함수이다. 서로다른 종족간에는 볼 수 없다.-
// 뱀파이어가 보내는 패킷은 isVampire가 True로 날아온다.
//
//--------------------------------------------------------------------
void Zone::broadcastSayPacket(ZoneCoord_t cx, ZoneCoord_t cy, Packet* pPacket, Creature* owner, bool isVampire)

{
    __BEGIN_TRY

    __BEGIN_PROFILE_ZONE("Z_BC_SAY")

    ZoneCoord_t ix = 0;
    ZoneCoord_t iy = 0;
    ZoneCoord_t endx = 0;
    ZoneCoord_t endy = 0;

#ifdef __USE_ENCRYPTER__
    SocketEncryptOutputStream outputStream(NULL, szPacketHeader + pPacket->getPacketSize() + 2);
    outputStream.setEncryptCode(m_EncryptCode);
#else
    SocketOutputStream outputStream(NULL, szPacketHeader + pPacket->getPacketSize() + 2);
#endif
    pPacket->writeHeaderNBody(outputStream);

    //-------------------------------------------------------------------
    // 루프 변수 초기화..
    //
    // Plus 변수가 참일 경우 Range 만큼 더 보내 준다..
    // 광역 마법의 결과를 효과적으로 보여주기 위함이다.
    //
    // *NOTE
    // - 최적화를 한다면 VisionInfo에 PLUS_SIGHT라는 변수를 추가하여 연산
    //-------------------------------------------------------------------
    endx = min(m_Width - 1, cx + maxViewportWidth + 1);
    endy = min(m_Height - 1, cy + maxViewportLowerHeight + 1);

    for (ix = max(0, cx - maxViewportWidth - 1); ix <= endx; ix++) {
        for (iy = max(0, cy - maxViewportUpperHeight - 1); iy <= endy; iy++) {
            Tile& rTile = m_pTiles[ix][iy]; // by sigi. 2002.5.8

            // 타일에 크리처가 있는 경우에만
            if (rTile.hasCreature()) {
                const forward_list<Object*>& objectList = rTile.getObjectList();

                for (forward_list<Object*>::const_iterator itr = objectList.begin();
                     itr != objectList.end() && (*itr)->getObjectPriority() <= OBJECT_PRIORITY_BURROWING_CREATURE;
                     itr++) {
                    Creature* pCreature = dynamic_cast<Creature*>(*itr);
                    Assert(pCreature != NULL);

                    // PC이면서, owner가 아니면서, (x,y)를 볼 수 있는 경우
                    if ((pCreature->isPC() && pCreature != owner && pCreature->getVisionState(cx, cy) >= IN_SIGHT) ||
                        (pCreature->isPC() && pCreature != owner)) {
                        // 숨어 있는 넘이 뭔 짓을 하면 안보여 주는데.. 딴짓 하면 Unborrowing 시켜야 되는디.
                        // 뱀파이어가 보내는 패킷은 isVampire가 True로 날아온다.
                        if (owner != NULL) {
                            // Creature 에서 ObservingEye 이펙트가 있으면 이펙트를 가져온다.
                            EffectObservingEye* pEffectObservingEye = NULL;
                            if (pCreature->isFlag(Effect::EFFECT_CLASS_OBSERVING_EYE)) {
                                pEffectObservingEye = dynamic_cast<EffectObservingEye*>(
                                    pCreature->findEffect(Effect::EFFECT_CLASS_OBSERVING_EYE));
                                // Assert( pEffectObservingEye != NULL );
                            }

                            if (!owner->isFlag(Effect::EFFECT_CLASS_GHOST) &&
                                (!owner->isFlag(Effect::EFFECT_CLASS_HIDE) || pCreature->isVampire() ||
                                 pCreature->isFlag(
                                     Effect::EFFECT_CLASS_DETECT_HIDDEN)) // || ( pEffectRevealer != NULL &&
                                                                          // pEffectRevealer->canSeeHide(owner) ) )
                                && (!owner->isFlag(Effect::EFFECT_CLASS_INVISIBILITY) || pCreature->isVampire() ||
                                    pCreature->isFlag(Effect::EFFECT_CLASS_DETECT_INVISIBILITY) ||
                                    (pEffectObservingEye != NULL && pEffectObservingEye->canSeeInvisibility(owner))) &&
                                (!owner->isFlag(Effect::EFFECT_CLASS_SNIPING_MODE) ||
                                 pCreature->isFlag(
                                     Effect::EFFECT_CLASS_DETECT_INVISIBILITY)) // || ( pEffectRevealer != NULL &&
                                                                                // pEffectRevealer->canSeeSniping(owner)
                                                                                // ) )
                                && ((isVampire && pCreature->isVampire()) || (!isVampire && pCreature->isSlayer()) ||
                                    pCreature->isOusters())) {
                                // pCreature->getPlayer()->sendPacket(pPacket);
                                pCreature->getPlayer()->sendStream(&outputStream);
                            }
                        } else {
                            // pCreature->getPlayer()->sendPacket(pPacket);
                            pCreature->getPlayer()->sendStream(&outputStream);
                        }
                    }
                }
            }
        }
    }

    __END_PROFILE_ZONE("Z_BC_SAY")

    __END_CATCH
}

//--------------------------------------------------------------------------------
//
// broadcast packet
//
// (x,y) 의 사건을 볼 수 있는, owner를 제외한 모든 PC 들에게 패킷을 브로드캐스트한다.
//
// *CAUTION*
//
// unsigned char 를 ZoneCoord_t 로 사용할 때, overflow 및 underflow 를 주의할 것
//
//--------------------------------------------------------------------------------
void Zone::broadcastPacket(ZoneCoord_t cx, ZoneCoord_t cy, Packet* pPacket, Creature* owner, bool Plus, Range_t Range)

{
    __BEGIN_TRY

    __BEGIN_PROFILE_ZONE("Z_BC_NORMAL")

#ifdef __USE_ENCRYPTER__
    SocketEncryptOutputStream outputStream(NULL, szPacketHeader + pPacket->getPacketSize() + 2);
    outputStream.setEncryptCode(m_EncryptCode);
#else
    SocketOutputStream outputStream(NULL, szPacketHeader + pPacket->getPacketSize() + 2);
#endif
    pPacket->writeHeaderNBody(outputStream);

    ZoneCoord_t ix = 0;
    ZoneCoord_t iy = 0;
    ZoneCoord_t endx = 0;
    ZoneCoord_t endy = 0;

    //-------------------------------------------------------------------
    // 루프 변수 초기화..
    //
    // Plus 변수가 참일 경우 Range 만큼 더 보내 준다..
    // 광역 마법의 결과를 효과적으로 보여주기 위함이다.
    //
    // *NOTE
    // - 최적화를 한다면 VisionInfo에 PLUS_SIGHT라는 변수를 추가하여 연산
    //-------------------------------------------------------------------
    endx = min(m_Width - 1, cx + maxViewportWidth + 1 + Range);
    endy = min(m_Height - 1, cy + maxViewportLowerHeight + 1 + Range);

    for (ix = max(0, cx - maxViewportWidth - 1 - Range); ix <= endx; ix++) {
        for (iy = max(0, cy - maxViewportUpperHeight - 1 - Range); iy <= endy; iy++) {
            // (ix,iy)에서 (cx,cy)를 못 볼 경우
            if (VisionInfoManager::getVisionState(ix, iy, cx, cy) == OUT_OF_SIGHT && !Plus)
                continue;
            Tile& rTile = m_pTiles[ix][iy]; // by sigi.2002.5.8

            // 타일에 크리처가 있는 경우에만
            if (rTile.hasCreature()) {
                const forward_list<Object*>& objectList = rTile.getObjectList();

                for (forward_list<Object*>::const_iterator itr = objectList.begin();
                     itr != objectList.end() && (*itr)->getObjectPriority() <= OBJECT_PRIORITY_BURROWING_CREATURE;
                     itr++) {
                    Creature* pCreature = dynamic_cast<Creature*>(*itr);
                    Assert(pCreature != NULL);

                    // by sigi. 2002.5.14
                    if (pCreature->isPC() && pCreature != owner)
                    // 위에서 체크했다. by Sequoia
                    //						&& (pCreature->getVisionState(cx,cy) >= IN_SIGHT || Plus))
                    {
                        // 숨어 있는 넘이 뭔 짓을 하면 안보여 주는데.. 딴짓 하면 Unborrowing 시켜야 되는디.
                        if (owner != NULL) {
                            // canSee 함수로 대체. by bezz 2003.05.29
                            if (canSee(pCreature, owner)) {
                                // pCreature->getPlayer()->sendPacket(pPacket);
                                pCreature->getPlayer()->sendStream(&outputStream);
                            }
                        } else {
                            // pCreature->getPlayer()->sendPacket(pPacket);
                            pCreature->getPlayer()->sendStream(&outputStream);
                        }
                    }
                }
            }
        }
    }

    __END_PROFILE_ZONE("Z_BC_NORMAL")

    __END_CATCH
}

void Zone::broadcastLevelWarBonusPacket(Packet* pPacket, Creature* owner)

{
    __BEGIN_TRY

    __BEGIN_PROFILE_ZONE("Z_BC_PCMANAGER")
    m_pPCManager->broadcastLevelWarBonusPacket(pPacket, owner);
    __END_PROFILE_ZONE("Z_BC_PCMANAGER")

    __END_CATCH
}

//--------------------------------------------------------------------------------
// broadcast packet
// (x1,y1) (x2,y2) 의 사건을 볼 수 있는,
// owner를 제외한 모든 PC 들에게 패킷을 브로드캐스트한다.
// Tile 전용 스킬 broadcastPacket이다.
// *CAUTION*
// unsigned char 를 ZoneCoord_t 로 사용할 때, overflow 및 underflow 를 주의할 것
//--------------------------------------------------------------------------------
list<Creature*> Zone::broadcastSkillPacket(ZoneCoord_t x1, ZoneCoord_t y1, ZoneCoord_t x2, ZoneCoord_t y2,
                                           Packet* pPacket, list<Creature*> creatureList, bool bConcernDarkness)

{
    __BEGIN_TRY

#ifdef __USE_ENCRYPTER__
    SocketEncryptOutputStream outputStream(NULL, szPacketHeader + pPacket->getPacketSize() + 2);
    outputStream.setEncryptCode(m_EncryptCode);
#else
    SocketOutputStream outputStream(NULL, szPacketHeader + pPacket->getPacketSize() + 2);
#endif
    pPacket->writeHeaderNBody(outputStream);

    list<Creature*> cList;

    __BEGIN_PROFILE_ZONE("Z_BC_SKILL")

    ZoneCoord_t ix = 0;
    ZoneCoord_t iy = 0;
    ZoneCoord_t endx = 0;
    ZoneCoord_t endy = 0;

    //-------------------------------------------------------------------
    // 루프 변수 초기화..
    //
    // Plus 변수가 참일 경우 Range 만큼 더 보내 준다..
    // 광역 마법의 결과를 효과적으로 보여주기 위함이다.
    //
    // *NOTE
    //
    // - 최적화를 한다면 VisionInfo에 PLUS_SIGHT라는 변수를 추가하여 연산
    //
    //-------------------------------------------------------------------
    endx = min(m_Width - 1, x1 + maxViewportWidth + 1);
    endy = min(m_Height - 1, y1 + maxViewportLowerHeight + 1);

    for (ix = max(0, x1 - maxViewportWidth - 1); ix <= endx; ix++) {
        for (iy = max(0, y1 - maxViewportUpperHeight - 1); iy <= endy; iy++) {
            Tile& rTile = m_pTiles[ix][iy]; // by sigi.2002.5.8

            // 타일에 크리처가 있는 경우에만
            if (rTile.hasCreature()) {
                const forward_list<Object*>& objectList = rTile.getObjectList();

                for (forward_list<Object*>::const_iterator itr = objectList.begin();
                     itr != objectList.end() && (*itr)->getObjectPriority() <= OBJECT_PRIORITY_BURROWING_CREATURE;
                     itr++) {
                    Creature* pCreature = dynamic_cast<Creature*>(*itr);
                    Assert(pCreature != NULL);

                    // PC이면서, creature list에 속하지 않으면서 (x,y)를 볼 수 있는 경우
                    if (pCreature->isPC()) {
                        // 이 패킷을 발생시킨 놈들인지를 체크한다.
                        bool belong = false;
                        for (list<Creature*>::const_iterator itr = creatureList.begin(); itr != creatureList.end();
                             itr++) {
                            /*
                            if( pCreature->isMonster() )
                            {
                                Monster* pMonster = dynamic_cast<Monster*>(pCreature);
                                // edit by sonic 2006.12.29  錦攣훙잚퓜癎珞加뎌뵨加濫檢
                                if(pMonster->getMonsterType() 	== 482 ||
                                     pMonster->getMonsterType() 	== 673 )
                                     {
                                            belong =true;
                                            break;
                                     }
                                }
                            // end by sonic
                            */
                            if (pCreature == *itr) {
                                belong = true;
                                break;
                            }
                        }

                        if (!belong && pCreature->getVisionState(x1, y1) >= IN_SIGHT &&
                            pCreature->getVisionState(x2, y2) >= IN_SIGHT) {
                            // 숨어 있는 넘이 안 보이면서 스킬을 쓸 이유가 없다.. 따라서 HIDE체크는 하지 않는다.
                            Player* pPlayer = pCreature->getPlayer();
                            // pPlayer->sendPacket(pPacket);
                            pPlayer->sendStream(&outputStream);
                            cList.push_back(pCreature);
                        }
                    }
                }
            }
        }
    }

    // add creature list to cList
    for (list<Creature*>::const_iterator itr = creatureList.begin(); itr != creatureList.end(); itr++) {
        cList.push_back(*itr);
    }

    __END_PROFILE_ZONE("Z_BC_SKILL")

    return cList;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// (x,y) 의 사건을 볼 수 있는, creatureList 에 소속된 크리처를 제외한 모든 PC 들에게
// 패킷을 브로드캐스트한다.
//
// *NOTE*
// 지속 Tile Magic일 경우 Plus 를 True로 두게 되며 Plus 변수가 True일 경우..
// Magic 범위의 반지름 만큼 더 범위를 확장하여 보내준다.. 광역 마법이 짤리지 않고,
// 효과적으로 보여주기 위함이다.
//
// *CAUTION*
//
// unsigned char 를 ZoneCoord_t 로 사용할 때, overflow 및 underflow 를 주의할 것
//////////////////////////////////////////////////////////////////////////////
void Zone::broadcastPacket(ZoneCoord_t cx, ZoneCoord_t cy, Packet* pPacket, const list<Creature*>& creatureList,
                           bool Plus, Range_t Range)

{
    __BEGIN_TRY

    __BEGIN_PROFILE_ZONE("Z_BC_EXCLIST")

#ifdef __USE_ENCRYPTER__
    SocketEncryptOutputStream outputStream(NULL, szPacketHeader + pPacket->getPacketSize() + 2);
    outputStream.setEncryptCode(m_EncryptCode);
#else
    SocketOutputStream outputStream(NULL, szPacketHeader + pPacket->getPacketSize() + 2);
#endif
    pPacket->writeHeaderNBody(outputStream);

    ZoneCoord_t ix = 0;
    ZoneCoord_t iy = 0;
    ZoneCoord_t endx = 0;
    ZoneCoord_t endy = 0;

    //////////////////////////////////////////////////////////////////////////////
    // 루프 변수 초기화..
    //
    // Plus 변수가 참일 경우 Range 만큼 더 보내 준다..
    // 광역 마법의 결과를 효과적으로 보여주기 위함이다.
    //
    // *NOTE
    // - 최적화를 한다면 VisionInfo에 PLUS_SIGHT라는 변수를 추가하여 연산
    //////////////////////////////////////////////////////////////////////////////
    endx = min(m_Width - 1, cx + maxViewportWidth + 1 + Range);
    endy = min(m_Height - 1, cy + maxViewportLowerHeight + 1 + Range);

    for (ix = max(0, cx - maxViewportWidth - 1 - Range); ix <= endx; ix++) {
        for (iy = max(0, cy - maxViewportUpperHeight - 1 - Range); iy <= endy; iy++) {
            if (VisionInfoManager::getVisionState(ix, iy, cx, cy) == OUT_OF_SIGHT || Plus)
                continue;
            Tile& rTile = m_pTiles[ix][iy]; // by sigi. 2002.5.8

            // 타일에 크리처가 있는 경우에만
            if (rTile.hasCreature()) {
                const forward_list<Object*>& objectList = rTile.getObjectList();
                forward_list<Object*>::const_iterator itr = objectList.begin();

                for (; itr != objectList.end() && (*itr)->getObjectPriority() <= OBJECT_PRIORITY_BURROWING_CREATURE;
                     itr++) {
                    Creature* pCreature = dynamic_cast<Creature*>(*itr);
                    Assert(pCreature != NULL);

                    // PC이면서, creatureList에 소속되지도 않으면서, (x,y)를 볼 수 있는 경우
                    if (pCreature->isPC()) {
                        bool belong = false;
                        for (list<Creature*>::const_iterator itr = creatureList.begin(); itr != creatureList.end();
                             itr++) {
                            /*
                            // edit by sonic 2006.12.29  錦攣훙잚퓜癎珞加뎌뵨加濫檢
                            if( pCreature->isMonster() )
                            {
                                Monster* pMonster = dynamic_cast<Monster*>(pCreature);

                                if(pMonster->getMonsterType() 	== 482 ||
                                     pMonster->getMonsterType() 	== 673 )
                                     {
                                            belong =true;
                                            break;
                                     }
                                }
                            // end by sonic
                            */
                            if (pCreature == *itr) {
                                belong = true;
                                break;
                            }
                        } // for

                        //						if ((!belong && pCreature->getVisionState(cx,cy) >= IN_SIGHT) ||(!belong
                        //&& Plus))
                        if (!belong) {
                            // pCreature->getPlayer()->sendPacket(pPacket);
                            pCreature->getPlayer()->sendStream(&outputStream);
                        } // if
                    } // if

                } // for
            } // if
        } // for
    } // for

    __END_PROFILE_ZONE("Z_BC_EXCLIST")

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// (x1,y1)에서 (x2,y2)로 PC가 이동할 경우, 그 PC가 자리가 바뀜에 따라
// 새로 보게 되는 것들에 대한 정보를 보내줘야 하고, 그 PC를 새로 보게 되는
// 다른 크리쳐들에게도 정보를 보내줘야 한다.
//
// bSendMove는 move packet을 보내는가에 대한 변수.
// bKnockback은 현재의 움직임이 정상적인 움직임인가, 아니면 knockback에
// 의한 강제적인 움직임인가를 나타내는 변수
//////////////////////////////////////////////////////////////////////////////
void Zone::movePCBroadcast(Creature* pPC, ZoneCoord_t x1, ZoneCoord_t y1, ZoneCoord_t x2, ZoneCoord_t y2,
                           bool bSendMove, bool bKnockback) {
    __BEGIN_TRY

    __BEGIN_PROFILE_ZONE("Z_BC_MOVEPC");

    try {
        // 이 메쏘드는 PC 를 대상으로 한다.
        Assert(pPC->isPC());

        //////////////////////////////////////////////////////////////////////////////
        // 자신의 이동을 나타내는 GCMove 패킷을 만들어둔다. 클라이언트에게 GCMove를
        // 전송할때, (x,y)는 이전 좌표여야 하며, dir 은 바라보는(이동할) 방향이어야 한다.
        // 그것이 현재의 정책!
        //////////////////////////////////////////////////////////////////////////////
        GCMove gcMove;
        if (bSendMove) {
            gcMove.setObjectID(pPC->getObjectID());
            gcMove.setX(x1);
            gcMove.setY(y1);
            gcMove.setDir(pPC->getDir());
        }
        GCKnockBack gcKnockback;
        if (bKnockback) {
            gcKnockback.setObjectID(pPC->getObjectID());
            gcKnockback.setOrigin(x1, y1);
            gcKnockback.setTarget(x2, y2);
        }

#ifdef __USE_ENCRYPTER__
        SocketEncryptOutputStream outputStream(
            NULL, szPacketHeader + (bSendMove ? gcMove.getPacketSize() : gcKnockback.getPacketSize()) + 2);
        outputStream.setEncryptCode(m_EncryptCode);
#else
        SocketOutputStream outputStream(
            NULL, szPacketHeader + (bSendMove ? gcMove.getPacketSize() : gcKnockback.getPacketSize()) + 2);
#endif
        if (bSendMove)
            gcMove.writeHeaderNBody(outputStream);
        else
            gcKnockback.writeHeaderNBody(outputStream);

        //////////////////////////////////////////////////////////////////////////////
        // 움직이는 PC를 새로 보게될 다른 PC들을 위해서 PC의 타입에 따라 GCAdd 패킷을
        // 만들어둔다.  현재의 정책에 의하면, GCAdd 패킷은 현재의 좌표를 바탕으로 한다.
        //////////////////////////////////////////////////////////////////////////////
        Packet* pGCAddXXX = NULL;

        if (pPC->getCreatureClass() == Creature::CREATURE_CLASS_SLAYER) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pPC);

            //		GCAddSlayer* pGCAddSlayer = new GCAddSlayer(pSlayer->getSlayerInfo3());
            //		pGCAddSlayer->setEffectInfo(pSlayer->getEffectInfo());
            GCAddSlayer* pGCAddSlayer = new GCAddSlayer;
            makeGCAddSlayer(pGCAddSlayer, pSlayer);
            pGCAddXXX = pGCAddSlayer;
        } else if (pPC->getCreatureClass() == Creature::CREATURE_CLASS_VAMPIRE) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pPC);

            // 음.. hide상태에서 움직일 수는 없지만. 미래를 대비.
            if (pPC->isFlag(Effect::EFFECT_CLASS_HIDE)) {
                GCAddBurrowingCreature* pGCABC = new GCAddBurrowingCreature();
                pGCABC->setObjectID(pVampire->getObjectID());
                pGCABC->setName(pVampire->getName());
                pGCABC->setX(x2);
                pGCABC->setY(y2);
                pGCAddXXX = pGCABC;
            } else {
                //			GCAddVampire* pGCAddVampire = new GCAddVampire(pVampire->getVampireInfo3());
                //			pGCAddVampire->setEffectInfo(pVampire->getEffectInfo());
                GCAddVampire* pGCAddVampire = new GCAddVampire;
                makeGCAddVampire(pGCAddVampire, pVampire);
                pGCAddXXX = pGCAddVampire;
            }
        } else if (pPC->getCreatureClass() == Creature::CREATURE_CLASS_OUSTERS) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pPC);

            //		GCAddOusters* pGCAddOusters = new GCAddOusters( pOusters->getOustersInfo3() );
            //		pGCAddOusters->setEffectInfo(pOusters->getEffectInfo());
            GCAddOusters* pGCAddOusters = new GCAddOusters;
            makeGCAddOusters(pGCAddOusters, pOusters);
            pGCAddXXX = pGCAddOusters;
        }

        //////////////////////////////////////////////////////////////////////////////
        // PC가 움직이므로, 보고있던 놈들 중에서 이 PC를 못 보게
        // 되는 놈들도 있다. 이들에게 보내줄 GCDeleteObject 패킷을
        // 만들어둔다.
        //////////////////////////////////////////////////////////////////////////////
        GCDeleteObject gcDeleteObject;
        gcDeleteObject.setObjectID(pPC->getObjectID());

        Player* pPlayer = pPC->getPlayer();
        Assert(pPlayer != NULL);

        // loop 안에 있던걸 이쪽으로 뺐다. by sigi. 2002.5.8
        //	Sight_t sight = pPC->getSight();
        //	VisionInfo* pVisionInfo = g_pVisionInfoManager->getVisionInfo(sight, pPC->getDir());

        //    // ObservingEye 이펙트를 가져온다.
        //	EffectObservingEye* pEffectObservingEye = NULL;
        //	if ( pPC->isFlag(Effect::EFFECT_CLASS_OBSERVING_EYE ) )
        //	{
        //		pEffectObservingEye =
        // dynamic_cast<EffectObservingEye*>(pPC->findEffect(Effect::EFFECT_CLASS_OBSERVING_EYE));
        //		//Assert( pEffectObservingEye != NULL );
        //	}

        //////////////////////////////////////////////////////////////////////////////
        // 시야 영역의 상하좌우 모두 + 1 씩 증가시킨다.
        // 이유는 방향에 따른 ON_SIGHT 영역이 증가되기 때문이다.
        //////////////////////////////////////////////////////////////////////////////
        for (ZoneCoord_t ix = max(0, x2 - maxViewportWidth - 1), endx = min(m_Width - 1, x2 + maxViewportWidth + 1);
             ix <= endx; ix++) {
            for (ZoneCoord_t iy = max(0, y2 - maxViewportUpperHeight - 1),
                             endy = min(m_Height - 1, y2 + maxViewportLowerHeight + 1);
                 iy <= endy; iy++) {
                // if (pPC->isFlag(Effect::EFFECT_CLASS_DARKNESS)) sight = DARKNESS_SIGHT;

                // 현재 타일 위에 있는 모든 오브젝트들에 대해 반복한다.
                const forward_list<Object*>& objectList = m_pTiles[ix][iy].getObjectList();

                forward_list<Object*>::const_iterator itr = objectList.begin();

                //
                // object가 있는 경우만
                // pVisionInfo->getVisionState()를 체크 하기 위해서
                // if - do~while 을 사용했다. by sigi. 2002.5.8
                //
                if (itr != objectList.end()) {
                    // 이전 좌표 P(x1,y1)에서 I(ix,iy)가 어떻게 보이는가?
                    //				VisionState prevVisionState = pVisionInfo->getVisionState(x1,y1,ix,iy);
                    VisionState prevVisionState = VisionInfoManager::getVisionState(x1, y1, ix, iy);
                    // 현재 좌표 Q(x2,y2)에서 I(ix,iy)가 어떻게 보이는가?
                    //				VisionState curVisionState = pVisionInfo->getVisionState(x2,y2,ix,iy);
                    VisionState curVisionState = VisionInfoManager::getVisionState(x2, y2, ix, iy);

                    do {
                        Assert(*itr != NULL);

                        Object* pDebugObject = *itr;

                        Object::ObjectClass OClass = pDebugObject->getObjectClass();

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

                            if (pCreature == pPC) {
                                // 넉백일 경우, 자신의 의지에 의해 움직이는 것이 아니라,
                                // 타인에 의해 움직이는 것이므로 보내줘야 한다.
                                if (bKnockback) {
                                    // pPC->getPlayer()->sendPacket(&gcKnockback);
                                    pPC->getPlayer()->sendStream(&outputStream);
                                    // 넉백을 보내줬으면 continue한다.
                                }

                                // 자기 자신의 이동 정보는 받을 필요가 없다.
                                continue;
                            }

                            Creature::CreatureClass CClass = pCreature->getCreatureClass();

                            // Monster > Slayer > Vampire > NPC 순이라고 판단해서
                            // if 순서를 바꿨다. 길드 건물 같은 곳은 좀 다르겠지만?
                            // by sigi. 2002.5.8
                            if (CClass == Creature::CREATURE_CLASS_MONSTER) {
                                Monster* pMonster = dynamic_cast<Monster*>(pCreature);

                                //--------------------------------------------------------------------------------
                                // 이전 좌표에서는 이 몬스터를 볼 수 없었으나, 도착 좌표에서 이 몬스터를 보게 될
                                // 경우 GCAddMonster 패킷을 전송한다.
                                //--------------------------------------------------------------------------------
                                if (prevVisionState == OUT_OF_SIGHT && curVisionState >= IN_SIGHT) {
                                    // by sigi
                                    Packet* pAddMonsterPacket = createMonsterAddPacket(pMonster, pPC);

                                    if (pAddMonsterPacket != NULL) {
                                        pPlayer->sendPacket(pAddMonsterPacket);

                                        delete pAddMonsterPacket;
                                    }
                                }

                                // PC를 몬스터의 잠재적인 적으로 지정해준다.
                                VisionState vs = pMonster->getVisionState(x2, y2);

                                // Aggressive 몬스터에게만 적으로 등록시켜준다.
                                if (vs >= IN_SIGHT && pMonster->getAlignment() == ALIGNMENT_AGGRESSIVE) {
                                    if (isPotentialEnemy(pMonster, pPC)) {
                                        pMonster->addPotentialEnemy(pPC);
                                    }
                                }
                            } else if (CClass == Creature::CREATURE_CLASS_SLAYER) {
                                // 현재 타일이 원래는 안 보이다가 이제 보이는 경우에,
                                // 이 타일에 크리쳐가 서 있다면...
                                // 움직이고 있는 PC에게 이 타일에 서 있는 놈의 정보를 보내주어야 한다.
                                if (curVisionState >= IN_SIGHT && prevVisionState == OUT_OF_SIGHT) {
                                    // 현재 움직이는 크리쳐에게 스나이핑 상태가 걸려있지 않거나,
                                    // 걸려있다면 디텍트 인비저빌러티가 걸려있어야 볼 수 있다.
                                    // canSee 로 대체. by bezz 2003.05.29
                                    //								if ( canSee( pPC, pCreature, pEffectObservingEye ) )
                                    if (canSee(pPC, pCreature)) {
                                        Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
                                        //									GCAddSlayer
                                        // gcAddSlayer(pSlayer->getSlayerInfo3());
                                        //									gcAddSlayer.setEffectInfo(pSlayer->getEffectInfo());
                                        GCAddSlayer gcAddSlayer;
                                        makeGCAddSlayer(&gcAddSlayer, pSlayer);
                                        pPlayer->sendPacket(&gcAddSlayer);
                                    }
                                }

                                Assert(pCreature->getPlayer() != NULL);

                                //////////////////////////////////////////////////////////////////////////////
                                // Q(x2,y2)가 이 크리처의 시야 사각형의 경계에 위치하면서, P(x1,y1)은 사각형의 외부,
                                // 즉 보이지 않는 경우에만 GCAddXXX 패킷을 전송한다. 이렇게 하지 않으면, PC
                                // 크리처가 pCreature의 시야 경계에서 계속 움직이게 되면 계속 서버는 GCAddXXX 패킷을
                                // 보내야만 한다.
                                //
                                // 요약하면,
                                //
                                // OUT_OF_SIGHT -> ON_SIGHT/NEW_SIGHT : GCAddXXX
                                // IN_SIGHT/ON_SIGHT/NEW_SIGHT -> IN_SIGHT/ON_SIGHT/NEW_SIGHT : GCMove
                                //////////////////////////////////////////////////////////////////////////////
                                VisionState prevVS = pCreature->getVisionState(x1, y1);
                                VisionState currVS = pCreature->getVisionState(x2, y2);

                                // 보이지 않는 영역에서, 경계 영역을 거치지 않고 바로
                                // 시야 내부 영역으로 들어온다는 것은 불가능하다.
                                // 이거 이제 쌩~ IN_SIGHT밖에 없다.
                                //							Assert(prevVS != OUT_OF_SIGHT || currVS != IN_SIGHT);

                                if (canSee(pCreature, pPC)) {
                                    if (prevVS == OUT_OF_SIGHT && currVS >= IN_SIGHT) {
                                        pCreature->getPlayer()->sendPacket(pGCAddXXX);
                                    } else if (prevVS >= IN_SIGHT && currVS >= IN_SIGHT) {
                                        // if (bSendMove)
                                        //	pCreature->getPlayer()->sendPacket(&gcMove);
                                        // else if (bKnockback)
                                        //	pCreature->getPlayer()->sendPacket(&gcKnockback);
                                        pCreature->getPlayer()->sendStream(&outputStream);
                                    } else if (prevVS >= IN_SIGHT && currVS == OUT_OF_SIGHT) {
                                        pCreature->getPlayer()->sendPacket(&gcDeleteObject);
                                    }
                                }
                            } else if (CClass == Creature::CREATURE_CLASS_VAMPIRE) {
                                //////////////////////////////////////////////////////////////////////////////
                                // 이전 좌표에서는 보이지 않다가, 이번 좌표에서 새로 보이게 된 크리처만
                                // GCAddXXX 를 받아온다. 이전에도 NEW_SIGHT 이고, 지금도 NEW_SIGHT 이면,
                                // 새로 받아오지 않는다.
                                //////////////////////////////////////////////////////////////////////////////
                                if (curVisionState >= IN_SIGHT && prevVisionState == OUT_OF_SIGHT) {
                                    if (canSee(pPC, pCreature)) {
                                        if (pCreature->isFlag(Effect::EFFECT_CLASS_HIDE)) {
                                            //										if
                                            //(!pCreature->isFlag(Effect::EFFECT_CLASS_GHOST)
                                            //											&& (pPC->isVampire()
                                            //												||
                                            // pPC->isFlag(Effect::EFFECT_CLASS_DETECT_HIDDEN) )// || ( pEffectRevealer
                                            //!= NULL && pEffectRevealer->canSeeHide( pCreature ) ) )
                                            //											)
                                            {
                                                GCAddBurrowingCreature gcABC;
                                                gcABC.setObjectID(pCreature->getObjectID());
                                                gcABC.setName(pCreature->getName());
                                                gcABC.setX(ix);
                                                gcABC.setY(iy);
                                                pPlayer->sendPacket(&gcABC);
                                            }
                                        } else {
                                            //										if
                                            //(!pCreature->isFlag(Effect::EFFECT_CLASS_GHOST)
                                            //											&&
                                            //(!pCreature->isFlag(Effect::EFFECT_CLASS_INVISIBILITY)
                                            //												|| pPC->isVampire()
                                            //												||
                                            // pPC->isFlag(Effect::EFFECT_CLASS_DETECT_INVISIBILITY) || (
                                            // pEffectObservingEye != NULL && pEffectObservingEye->canSeeInvisibility(
                                            // pCreature ) ) )
                                            //											)
                                            {
                                                Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);
                                                //											GCAddVampire
                                                // gcAddVampire(pVampire->getVampireInfo3());
                                                //											gcAddVampire.setEffectInfo(pVampire->getEffectInfo());
                                                GCAddVampire gcAddVampire;
                                                makeGCAddVampire(&gcAddVampire, pVampire);

                                                pPlayer->sendPacket(&gcAddVampire);
                                            }
                                        }
                                    }
                                }

                                Assert(pCreature->getPlayer() != NULL);

                                //////////////////////////////////////////////////////////////////////////////
                                // Q(x2,y2)가 이 크리처의 시야 사각형의 경계에 위치하면서, P(x1,y1)은 사각형의 외부,
                                // 즉 보이지 않는 경우에만 GCAddXXX 패킷을 전송한다. 이렇게 하지 않으면, PC
                                // 크리처가 pCreature의 시야 경계에서 계속 움직이게 되면 계속 서버는 GCAddXXX 패킷을
                                // 보내야만 한다.
                                //
                                // 요약하면,
                                //
                                // OUT_OF_SIGHT -> ON_SIGHT/NEW_SIGHT : GCAddXXX
                                // IN_SIGHT/ON_SIGHT/NEW_SIGHT -> IN_SIGHT/ON_SIGHT/NEW_SIGHT : GCMove
                                //////////////////////////////////////////////////////////////////////////////
                                VisionState prevVS = pCreature->getVisionState(x1, y1);
                                VisionState currVS = pCreature->getVisionState(x2, y2);

                                // 보이지 않는 영역에서, 경계 영역을 거치지 않고 바로
                                // 시야 내부 영역으로 들어온다는 것은 불가능하다.
                                // 이거 이제 쌩~ IN_SIGHT밖에 없다.
                                //							Assert(prevVS != OUT_OF_SIGHT || currVS != IN_SIGHT);

                                // 상대는 뱀파이어이므로 나의 darkness상태는 관계없다.
                                // Hide도 관계없다.
                                //							if (!pPC->isFlag(Effect::EFFECT_CLASS_GHOST)
                                //								&& (!pPC->isSlayer()
                                //									|| !pPC->isFlag(Effect::EFFECT_CLASS_SNIPING_MODE))
                                //								)
                                if (canSee(pCreature, pPC)) {
                                    if (prevVS == OUT_OF_SIGHT && currVS >= IN_SIGHT) {
                                        pCreature->getPlayer()->sendPacket(pGCAddXXX);
                                    } else if (prevVS >= IN_SIGHT && currVS >= IN_SIGHT) {
                                        // if (bSendMove) pCreature->getPlayer()->sendPacket(&gcMove);
                                        ////else if (bKnockback) pCreature->getPlayer()->sendPacket(&gcKnockback);
                                        pCreature->getPlayer()->sendStream(&outputStream);
                                    } else if (prevVS >= IN_SIGHT && currVS == OUT_OF_SIGHT) {
                                        pCreature->getPlayer()->sendPacket(&gcDeleteObject);
                                    }
                                }
                            }

                            else if (CClass == Creature::CREATURE_CLASS_OUSTERS) {
                                //////////////////////////////////////////////////////////////////////////////
                                // 이전 좌표에서는 보이지 않다가, 이번 좌표에서 새로 보이게 된 크리처만
                                // GCAddXXX 를 받아온다. 이전에도 NEW_SIGHT 이고, 지금도 NEW_SIGHT 이면,
                                // 새로 받아오지 않는다.
                                //////////////////////////////////////////////////////////////////////////////
                                if (curVisionState >= IN_SIGHT && prevVisionState == OUT_OF_SIGHT) {
                                    //								if (!pCreature->isFlag(Effect::EFFECT_CLASS_GHOST) )
                                    if (canSee(pPC, pCreature)) {
                                        Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);
                                        //									GCAddOusters
                                        // gcAddOusters(pOusters->getOustersInfo3());
                                        //									gcAddOusters.setEffectInfo(pOusters->getEffectInfo());
                                        GCAddOusters gcAddOusters;
                                        makeGCAddOusters(&gcAddOusters, pOusters);
                                        pPlayer->sendPacket(&gcAddOusters);
                                    }
                                }

                                Assert(pCreature->getPlayer() != NULL);

                                //////////////////////////////////////////////////////////////////////////////
                                // Q(x2,y2)가 이 크리처의 시야 사각형의 경계에 위치하면서, P(x1,y1)은 사각형의 외부,
                                // 즉 보이지 않는 경우에만 GCAddXXX 패킷을 전송한다. 이렇게 하지 않으면, PC
                                // 크리처가 pCreature의 시야 경계에서 계속 움직이게 되면 계속 서버는 GCAddXXX 패킷을
                                // 보내야만 한다.
                                //
                                // 요약하면,
                                //
                                // OUT_OF_SIGHT -> ON_SIGHT/NEW_SIGHT : GCAddXXX
                                // IN_SIGHT/ON_SIGHT/NEW_SIGHT -> IN_SIGHT/ON_SIGHT/NEW_SIGHT : GCMove
                                //////////////////////////////////////////////////////////////////////////////
                                VisionState prevVS = pCreature->getVisionState(x1, y1);
                                VisionState currVS = pCreature->getVisionState(x2, y2);

                                // 보이지 않는 영역에서, 경계 영역을 거치지 않고 바로
                                // 시야 내부 영역으로 들어온다는 것은 불가능하다.
                                //							Assert(prevVS != OUT_OF_SIGHT || currVS != IN_SIGHT);

                                if (canSee(pCreature, pPC)) {
                                    if (prevVS == OUT_OF_SIGHT && currVS >= IN_SIGHT) {
                                        pCreature->getPlayer()->sendPacket(pGCAddXXX);
                                    } else if (prevVS >= IN_SIGHT && currVS >= IN_SIGHT) {
                                        // if (bSendMove) pCreature->getPlayer()->sendPacket(&gcMove);
                                        // else if (bKnockback) pCreature->getPlayer()->sendPacket(&gcKnockback);
                                        pCreature->getPlayer()->sendStream(&outputStream);
                                    } else if (prevVS >= IN_SIGHT && currVS == OUT_OF_SIGHT) {
                                        pCreature->getPlayer()->sendPacket(&gcDeleteObject);
                                    }
                                }
                            }

                            else if (CClass == Creature::CREATURE_CLASS_NPC) {
                                NPC* pNPC = dynamic_cast<NPC*>(pCreature);

                                //--------------------------------------------------------------------------------
                                //
                                // 이전 좌표에서는 이 몬스터를 볼 수 없었으나, 도착 좌표에서 이 몬스터를 보게 될
                                // 경우 GCAddMonster 패킷을 전송한다.
                                //
                                //--------------------------------------------------------------------------------
                                if (prevVisionState == OUT_OF_SIGHT && curVisionState >= IN_SIGHT) {
                                    GCAddNPC gcAddNPC;
                                    makeGCAddNPC(&gcAddNPC, pNPC);
                                    pPlayer->sendPacket(&gcAddNPC);
                                }
                            } else {
                                throw Error("invalid creature class");
                            }
                        }
                        ////////////////////////////////////////////////////////////
                        // 타일 위에 아이템이 있을 경우
                        ////////////////////////////////////////////////////////////
                        else if (OClass == Object::OBJECT_CLASS_ITEM) {
                            if (curVisionState >= IN_SIGHT && prevVisionState == OUT_OF_SIGHT) {
                                Item* pItem = dynamic_cast<Item*>(*itr);

                                Item::ItemClass IClass = pItem->getItemClass();

                                if (IClass == Item::ITEM_CLASS_CORPSE) {
                                    ItemType_t IType = pItem->getItemType();

                                    if (IType == SLAYER_CORPSE) {
                                        SlayerCorpse* pSlayerCorpse = dynamic_cast<SlayerCorpse*>(pItem);
                                        GCAddSlayerCorpse gcAddSlayerCorpse;
                                        makeGCAddSlayerCorpse(&gcAddSlayerCorpse, pSlayerCorpse);
                                        pPlayer->sendPacket(&gcAddSlayerCorpse);
                                    } else if (IType == VAMPIRE_CORPSE) {
                                        VampireCorpse* pVampireCorpse = dynamic_cast<VampireCorpse*>(pItem);
                                        GCAddVampireCorpse gcAddVampireCorpse;
                                        makeGCAddVampireCorpse(&gcAddVampireCorpse, pVampireCorpse);
                                        pPlayer->sendPacket(&gcAddVampireCorpse);
                                    } else if (IType == OUSTERS_CORPSE) {
                                        OustersCorpse* pOustersCorpse = dynamic_cast<OustersCorpse*>(pItem);
                                        GCAddOustersCorpse gcAddOustersCorpse;
                                        makeGCAddOustersCorpse(&gcAddOustersCorpse, pOustersCorpse);
                                        pPlayer->sendPacket(&gcAddOustersCorpse);
                                    } else if (IType == NPC_CORPSE) {
                                        throw UnsupportedError();
                                    } else if (MONSTER_CORPSE) {
                                        MonsterCorpse* pMonsterCorpse = dynamic_cast<MonsterCorpse*>(pItem);
                                        GCAddMonsterCorpse gcAddMonsterCorpse;
                                        makeGCAddMonsterCorpse(&gcAddMonsterCorpse, pMonsterCorpse, ix, iy);
                                        pPlayer->sendPacket(&gcAddMonsterCorpse);

                                        sendRelicEffect(pMonsterCorpse, pPlayer);
                                    } else {
                                        Assert(false);
                                    }
                                } else if (pItem->getItemClass() == Item::ITEM_CLASS_MINE &&
                                           pItem->isFlag(Effect::EFFECT_CLASS_INSTALL)) {
                                    if (pPC->isFlag(Effect::EFFECT_CLASS_REVEALER)) {
                                        GCAddInstalledMineToZone gcAddMine;
                                        gcAddMine.setObjectID(pItem->getObjectID());
                                        gcAddMine.setX(ix);
                                        gcAddMine.setY(iy);
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
                            }
                        }
                        ////////////////////////////////////////////////////////////
                        // 타일 위에 이펙트가 있을 경우
                        ////////////////////////////////////////////////////////////
                        else if (OClass == Object::OBJECT_CLASS_EFFECT) {
                            Effect* pEffect = dynamic_cast<Effect*>(*itr);

                            // broadcasting Effect 인지 체크 추가 2003.3.31 by Sequoia
                            if (pEffect->isBroadcastingEffect() && curVisionState >= IN_SIGHT &&
                                prevVisionState == OUT_OF_SIGHT) {
                                if (pEffect->getEffectClass() == Effect::EFFECT_CLASS_VAMPIRE_PORTAL) {
                                    EffectVampirePortal* pEffectVampirePortal =
                                        dynamic_cast<EffectVampirePortal*>(pEffect);
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
                                } else {
                                    GCAddEffectToTile gcAddEffectToTile;

                                    gcAddEffectToTile.setObjectID(pEffect->getObjectID());
                                    gcAddEffectToTile.setXY(ix, iy);
                                    gcAddEffectToTile.setEffectID(pEffect->getSendEffectClass());
                                    gcAddEffectToTile.setDuration(pEffect->getRemainDuration());

                                    pPlayer->sendPacket(&gcAddEffectToTile);
                                }
                            }
                        }
                        ////////////////////////////////////////////////////////////
                        // 타일 위에 장애물이 있을 경우
                        ////////////////////////////////////////////////////////////
                        else if (OClass == Object::OBJECT_CLASS_OBSTACLE) {
                        }
                        ////////////////////////////////////////////////////////////
                        // 타일 위에 포탈이 있을 경우
                        ////////////////////////////////////////////////////////////
                        else if (OClass == Object::OBJECT_CLASS_PORTAL) {
                            // darkness
                        } else {
                            throw Error("invalid object class");
                        }

                    } while (++itr != objectList.end()); // by sigi. 2002.5.8
                } // for 오브젝트들에 대한 반복
            } // for Y 좌표에 대한 반복
        } // for X 좌표에 대한 반복

        SAFE_DELETE(pGCAddXXX);

    } catch (Throwable& t) {
        filelog("Zone_movePCBroadcast.log", "%s", t.toString().c_str());
        throw t;
    }

    __END_PROFILE_ZONE("Z_BC_MOVEPC");

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// moveCreatureBroadcast
//
// PC가 아닌 크리처(NPC,몬스터)가 P(x1,y1)에서 Q(x2,y2)로 이동했을 때,
// 주변 영역에 존재하는 PC들에게 브로드캐스트하는 메쏘드이다.
//
// 이 메쏘드를 호출하기 전에, 3 가지 패킷은 만들어둬야만 한다.
//////////////////////////////////////////////////////////////////////////////
void Zone::moveCreatureBroadcast(Creature* pCreature, ZoneCoord_t x1, ZoneCoord_t y1, ZoneCoord_t x2, ZoneCoord_t y2,
                                 bool bSendMove, bool bKnockback)

{
    __BEGIN_TRY

    __BEGIN_PROFILE_ZONE("Z_BC_MOVE_CREATURE")

    try {
        Monster* pMonster = NULL;
        NPC* pNPC = NULL;

        Assert(pCreature != NULL);
        Assert(pCreature->isNPC() || pCreature->isMonster());

        // 현재의 정책에 의하면, GCMove 패킷은 이전 좌표와 현재 방향을 전송하게 되어있다.
        GCMove gcMove;
        if (bSendMove) {
            gcMove.setObjectID(pCreature->getObjectID());
            gcMove.setX(x1);
            gcMove.setY(y1);
            gcMove.setDir(pCreature->getDir());
        }
        GCKnockBack gcKnockback;
        if (bKnockback) {
            gcKnockback.setObjectID(pCreature->getObjectID());
            gcKnockback.setOrigin(x1, y1);
            gcKnockback.setTarget(x2, y2);
        }

#ifdef __USE_ENCRYPTER__
        SocketEncryptOutputStream outputStream(
            NULL, szPacketHeader + (bSendMove ? gcMove.getPacketSize() : gcKnockback.getPacketSize()) + 2);
        outputStream.setEncryptCode(m_EncryptCode);
#else
        SocketOutputStream outputStream(
            NULL, szPacketHeader + (bSendMove ? gcMove.getPacketSize() : gcKnockback.getPacketSize()) + 2);
#endif
        if (bSendMove)
            gcMove.writeHeaderNBody(outputStream);
        else
            gcKnockback.writeHeaderNBody(outputStream);

        // GCAddNPC/GCAddMonster 패킷을 만들어둔다.
        Packet* pGCAddXXX = NULL;

        bool isMonster = !pCreature->isNPC();
        //	bool isMonsterHide = false;
        //	bool isMonsterInvisibility = false;

        if (!isMonster) {
            pNPC = dynamic_cast<NPC*>(pCreature);
            GCAddNPC* pGCAddNPC = new GCAddNPC();
            makeGCAddNPC(pGCAddNPC, pNPC);
            pGCAddXXX = pGCAddNPC;
        } else // case of Monster
        {
            pMonster = dynamic_cast<Monster*>(pCreature);

            // 일단 다 볼 수 있는 상태(NULL)로 설정해서 packet을 생성한다. by sigi
            pGCAddXXX = createMonsterAddPacket(pMonster, NULL);

            // monster의 상태를 기억해둔다.
            //		isMonsterHide = pMonster->isFlag(Effect::EFFECT_CLASS_HIDE);
            //		isMonsterInvisibility = pMonster->isFlag(Effect::EFFECT_CLASS_INVISIBILITY);
        }

        // 시야에서 벗어날 때, GCDeleteObject 패킷을 보낸다.
        GCDeleteObject gcDeleteObject;
        gcDeleteObject.setObjectID(pCreature->getObjectID());

        //////////////////////////////////////////////////////////////////////////////
        // 시야 영역의 상하좌우 모두 + 1 씩 증가시킨다.
        // 이유는 방향에 따른 ON_SIGHT 영역이 증가되기 때문이다.
        //////////////////////////////////////////////////////////////////////////////
        for (ZoneCoord_t ix = max(0, x2 - maxViewportWidth - 1), endx = min(m_Width - 1, x2 + maxViewportWidth + 1);
             ix <= endx; ix++) {
            for (ZoneCoord_t iy = max(0, y2 - maxViewportUpperHeight - 1),
                             endy = min(m_Height - 1, y2 + maxViewportLowerHeight + 1);
                 iy <= endy; iy++) {
                const forward_list<Object*>& objectList = m_pTiles[ix][iy].getObjectList();
                forward_list<Object*>::const_iterator itr = objectList.begin();
                for (; itr != objectList.end() && (*itr)->getObjectPriority() <= OBJECT_PRIORITY_BURROWING_CREATURE;
                     itr++) {
                    Assert(*itr != NULL);

                    Creature* pPC = dynamic_cast<Creature*>(*itr);

                    Assert(pPC != NULL);

                    // PC 일 경우에만 GCMove, GCAddMonster 패킷을 보내준다. 몬스터한테는 보낼 필요가 없쥐~
                    if (pPC->isPC()) {
                        Assert(pPC->getPlayer() != NULL);

                        //////////////////////////////////////////////////////////////////////////////
                        // OUT_OF_SIGHT -> ON_SIGHT/NEW_SIGHT : GCAddXXX
                        // IN_SIGHT/ON_SIGHT/NEW_SIGHT -> IN_SIGHT/ON_SIGHT/NEW_SIGHT : GCMove
                        //////////////////////////////////////////////////////////////////////////////
                        VisionState prevVS = pPC->getVisionState(x1, y1);
                        VisionState currVS = pPC->getVisionState(x2, y2);

                        // 보이지 않는 영역에서, 경계 영역을 거치지 않고 바로
                        // 시야 내부 영역으로 들어온다는 것은 불가능하다.
                        // 이제 쌩~ IN_SIGHT밖에 없다.
                        //					Assert(prevVS != OUT_OF_SIGHT || currVS != IN_SIGHT);

                        // ObservingEye 이펙트를 가져온다.
                        //					EffectObservingEye* pEffectObservingEye = NULL;
                        //					if ( pPC->isFlag( Effect::EFFECT_CLASS_OBSERVING_EYE ) )
                        //					{
                        //						pEffectObservingEye =
                        // dynamic_cast<EffectObservingEye*>(pPC->findEffect(Effect::EFFECT_CLASS_OBSERVING_EYE));
                        //						//Assert( pEffectObservingEye != NULL );
                        //					}

                        if (prevVS == OUT_OF_SIGHT && currVS >= IN_SIGHT) {
                            if (isMonster) {
                                if (canSee(pPC, pMonster)) {
                                    pPC->getPlayer()->sendPacket(pGCAddXXX);
                                }
                            } else {
                                pPC->getPlayer()->sendPacket(pGCAddXXX);
                            }
                        } else if (prevVS >= IN_SIGHT && currVS >= IN_SIGHT) {
                            // if (bSendMove)
                            //{
                            //	pPC->getPlayer()->sendPacket(&gcMove);
                            // }
                            // else if (bKnockback)
                            //{
                            //	pPC->getPlayer()->sendPacket(&gcKnockback);
                            // }
                            pPC->getPlayer()->sendStream(&outputStream);
                        } else if (prevVS >= IN_SIGHT && currVS == OUT_OF_SIGHT) {
                            pPC->getPlayer()->sendPacket(&gcDeleteObject);
                        }

                        //--------------------------------------------------------------------------------
                        // 브로드캐스트를 했으면, 이제 이 PC를 잠재적인 적으로 등록해버리자.
                        //--------------------------------------------------------------------------------
                        if (pCreature->isMonster()) {
                            // 저 위에서 이미 dynamic_cast 된 상태이다.
                            VisionState vs = pMonster->getVisionState(ix, iy);
                            if (vs >= IN_SIGHT && pMonster->getAlignment() == ALIGNMENT_AGGRESSIVE) {
                                if (isPotentialEnemy(pMonster, pPC)) {
                                    pMonster->addPotentialEnemy(pPC);
                                }
                            }
                        }

                    } // if

                } // for

            } // for

        } // for

        // 생성한 패킷을 삭제한다.
        SAFE_DELETE(pGCAddXXX);

        // by sigi. 2002.12.15
    } catch (Throwable& t) {
        filelog("moveCreatureBroadcastError.log", "%s", t.toString().c_str());

        // 다시 던져서 죽이자 - -;
        throw;
    }

    __END_PROFILE_ZONE("Z_BC_MOVE_CREATURE")

    __END_CATCH
}
