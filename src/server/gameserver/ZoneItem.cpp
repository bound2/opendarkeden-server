//////////////////////////////////////////////////////////////////////////////
// FileName 	: ZoneItem.cpp
// Description	: Zone item tables: dropping, moving and removing the items that lie on the ground.
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

// EffectCallMotorcycle.h is deliberately absent: it reuses
// EffectDecayItem.h's include guard, so including it would hide
// EffectDecayItem from this file.

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
