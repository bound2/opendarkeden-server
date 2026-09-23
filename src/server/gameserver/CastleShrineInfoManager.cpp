#include "CastleShrineInfoManager.h"

#include <stdio.h>

#include "CastleInfoManager.h"
#include "CastleSymbol.h"
#include "CreatureUtil.h"
#include "EffectShrineGuard.h"
#include "EffectShrineHoly.h"
#include "EffectShrineShield.h"
#include "GCAddEffect.h"
#include "GCAddEffectToTile.h"
#include "GCDeleteInventoryItem.h"
#include "GCRemoveEffect.h"
#include "GCSystemMessage.h"
#include "GameContext.h"
#include "GlobalItemPosition.h"
#include "GlobalItemPositionLoader.h"
#include "Item.h"
#include "ItemFactoryManager.h"
#include "MonsterCorpse.h"
#include "Player.h"
#include "PlayerCreature.h"
#include "RelicUtil.h"
#include "StringPool.h"
#include "StringStream.h"
#include "War.h"
#include "WarSystem.h"
#include "Zone.h"
#include "ZoneGroupManager.h"
#include "ZoneItemPosition.h"
#include "ZoneUtil.h"
#include "repository/GameInfoRepository.h"

string CastleShrineSet::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "ShrineSet(" << "ShrineID:" << (int)m_ShrineID << "," << m_GuardShrine.toString() << ","
        << m_HolyShrine.toString() << ",ItemType:" << (int)m_ItemType << ")";

    return msg.toString();

    __END_CATCH
}

CastleShrineInfoManager::~CastleShrineInfoManager() {
    clear();
}

void CastleShrineInfoManager::clear() {
    HashMapShrineSetItor itr = m_ShrineSets.begin();
    for (; itr != m_ShrineSets.end(); itr++) {
        SAFE_DELETE(itr->second);
    }
    m_ShrineSets.clear();
}

void CastleShrineInfoManager::init()

{
    __BEGIN_TRY

    load();

    addAllShrineToZone();

    __END_CATCH
}

void CastleShrineInfoManager::load()

{
    __BEGIN_TRY

    vector<CastleShrineRow> rows = defaultGameInfoRepository().loadCastleShrines();

    for (size_t r = 0; r < rows.size(); r++) {
        CastleShrineSet* pShrineSet = new CastleShrineSet();

        pShrineSet->m_ShrineID = rows[r].id;
        pShrineSet->m_GuardShrine.setName(rows[r].name);
        pShrineSet->m_HolyShrine.setName(pShrineSet->m_GuardShrine.getName());
        pShrineSet->m_ItemType = rows[r].itemType;
        pShrineSet->m_GuardShrine.setZoneID(rows[r].guardZoneID);
        pShrineSet->m_GuardShrine.setX(rows[r].guardX);
        pShrineSet->m_GuardShrine.setY(rows[r].guardY);
        pShrineSet->m_GuardShrine.setMonsterType(rows[r].guardMonsterType);
        pShrineSet->m_HolyShrine.setZoneID(rows[r].holyZoneID);
        pShrineSet->m_HolyShrine.setX(rows[r].holyX);
        pShrineSet->m_HolyShrine.setY(rows[r].holyY);
        pShrineSet->m_HolyShrine.setMonsterType(rows[r].holyMonsterType);

        pShrineSet->m_GuardShrine.setShrineType(ShrineInfo::SHRINE_GUARD);
        pShrineSet->m_HolyShrine.setShrineType(ShrineInfo::SHRINE_HOLY);

        // ItemType and the shrine ID must match; a mismatch is a DB configuration error and stops the load.
        if (pShrineSet->m_ItemType != pShrineSet->m_ShrineID) {
            cout << "ShrineID 와 ItemType이 맞지 않습니다. DB설정을 점검하세요." << endl;
            Assert(false);
        }

        addShrineSet(pShrineSet);
    }

    __END_CATCH
}

ZoneID_t CastleShrineInfoManager::getGuardShrineZoneID(ZoneID_t castleZoneID) const

{
    __BEGIN_TRY

    HashMapShrineSetConstItor itr = m_ShrineSets.begin();

    // The shrineID for castleZoneID cannot be looked up, so compare them one by one.
    for (; itr != m_ShrineSets.end(); itr++) {
        CastleShrineSet* pShrineSet = itr->second;

        ZoneID_t guardZoneID = pShrineSet->m_GuardShrine.getZoneID();
        ZoneID_t guardCastleZoneID;

        if (de::gameContext().castleInfos().getCastleZoneID(guardZoneID, guardCastleZoneID)) {
            if (castleZoneID == guardCastleZoneID) {
                return guardZoneID;
            }
        }
    }

    StringStream msg;
    msg << "CastleZoneID와 관련된 GuardZoneID가 없다[" << (int)castleZoneID << "]";
    throw Error(msg.toString());

    __END_CATCH
}

void CastleShrineInfoManager::addAllShrineToZone()

{
    __BEGIN_TRY

    HashMapShrineSetItor itr = m_ShrineSets.begin();
    for (; itr != m_ShrineSets.end(); itr++) {
        CastleShrineSet* pShrineSet = itr->second;

        if (pShrineSet == NULL)
            continue;

        Item* pItem = addShrineToZone(pShrineSet->m_GuardShrine, pShrineSet->m_ItemType);

        if (pItem == NULL) {
            pShrineSet->setCastleSymbolItemID(0);
        } else {
            pShrineSet->setCastleSymbolItemID(pItem->getItemID());
        }

        addShrineToZone(pShrineSet->m_HolyShrine, pShrineSet->m_ItemType);
    }

    __END_CATCH
}

Item* CastleShrineInfoManager::addShrineToZone(ShrineInfo& shrineInfo, ItemType_t itemType)

{
    __BEGIN_TRY

    // Get the zone the shrine goes into.
    Zone* pZone = getZoneByZoneID(shrineInfo.getZoneID());
    Assert(pZone != NULL);

    MonsterCorpse* pShrine = new MonsterCorpse(shrineInfo.getMonsterType(), shrineInfo.getName(), 2);
    Assert(pShrine != NULL);

    pShrine->setShrine(true);
    pShrine->setZone(pZone);

    pZone->getObjectRegistry().registerObject(pShrine);
    shrineInfo.setObjectID(pShrine->getObjectID());

    cout << "AddShrine[" << (int)shrineInfo.getZoneID() << "] "
         << (shrineInfo.getShrineType() == ShrineInfo::SHRINE_GUARD ? "Guard" : "Holy")
         << ", mtype=" << shrineInfo.getMonsterType() << ", oid=" << pShrine->getObjectID() << endl;

    Item* pItem = NULL;

    TPOINT tp = pZone->addItem(pShrine, shrineInfo.getX(), shrineInfo.getY(), true);
    Assert(tp.x != -1);

    // Add the castle symbol if it needs to be added.
    if (shrineInfo.getShrineType() == ShrineInfo::SHRINE_GUARD) {
        // if ( AddBible[ itemType ] )
        {
            list<OptionType_t> optionNULL;
            pItem = de::gameContext().itemFactories().createItem(Item::ITEM_CLASS_CASTLE_SYMBOL, itemType, optionNULL);
            Assert(pItem != NULL);

            char strZoneID[10];
            sprintf(strZoneID, "%d", (int)pZone->getZoneID());

            pZone->registerObject(pItem);
            pItem->create(strZoneID, STORAGE_CORPSE, pShrine->getObjectID(), 0, 0);

            pShrine->addTreasure(pItem);
        }

        // Mark it as a guard shrine.
        pShrine->setFlag(Effect::EFFECT_CLASS_CASTLE_SHRINE_GUARD);

        // Attach the Shield Effect to every guard shrine.
        pShrine->setFlag(Effect::EFFECT_CLASS_SHRINE_SHIELD);

        EffectShrineShield* pEffect = new EffectShrineShield(pShrine);
        pEffect->setShrineID(itemType);
        pEffect->setTick(60 * 10);

        pShrine->getEffectManager().addEffect(pEffect);
    } else {
        // Mark it as a holy shrine.
        pShrine->setFlag(Effect::EFFECT_CLASS_CASTLE_SHRINE_HOLY);
    }

    // Set the shrine coordinates anew.
    shrineInfo.setX(tp.x);
    shrineInfo.setY(tp.y);

    return pItem;

    __END_CATCH
}

void CastleShrineInfoManager::addShrineSet(CastleShrineSet* pShrineSet)

{
    __BEGIN_TRY

    if (pShrineSet == NULL)
        return;

    ShrineID_t shrineID = pShrineSet->m_ShrineID;

    HashMapShrineSetItor itr = m_ShrineSets.find(shrineID);

    if (itr != m_ShrineSets.end()) {
        filelog("ShrineInfoError.log", "CastleShrineInfoManager::addShrineSet DuplicatedException : %d", (int)shrineID);
        return;
    }

    m_ShrineSets[shrineID] = pShrineSet;

    __END_CATCH
}

void CastleShrineInfoManager::deleteShrineSet(ShrineID_t shrineID)

{
    __BEGIN_TRY

    HashMapShrineSetItor itr = m_ShrineSets.find(shrineID);

    if (itr != m_ShrineSets.end()) {
        SAFE_DELETE(itr->second);
        m_ShrineSets.erase(shrineID);
    }

    __END_CATCH
}

CastleShrineSet* CastleShrineInfoManager::getShrineSet(ShrineID_t shrineID) const

{
    __BEGIN_TRY

    HashMapShrineSetConstItor itr = m_ShrineSets.find(shrineID);

    if (itr != m_ShrineSets.end()) {
        return itr->second;
    }

    return NULL;

    __END_CATCH
}

bool CastleShrineInfoManager::isMatchHolyShrine(Item* pItem, MonsterCorpse* pMonsterCorpse) const

{
    __BEGIN_TRY

    if (pItem->getItemClass() != Item::ITEM_CLASS_CASTLE_SYMBOL)
        return false;

    ItemType_t itemType = pItem->getItemType();
    ShrineID_t shrineID = itemType; // ShrineID = ItemType(of CastleSymbol)

    CastleShrineSet* pShrineSet = getShrineSet(shrineID);

    if (pShrineSet == NULL) {
        // There is no Shrine Set for this castle symbol.
        return false;
    }

    // True when the MonsterType of the holy shrine of the Shrine set for this castle
    // symbol equals the MonsterType of the MonsterCorpse passed in and the ObjectID matches too.
    return pShrineSet->m_HolyShrine.getMonsterType() == pMonsterCorpse->getMonsterType() &&
           pShrineSet->m_HolyShrine.getObjectID() == pMonsterCorpse->getObjectID();

    __END_CATCH
}

// Can this race pick up a castle symbol fragment?
bool CastleShrineInfoManager::canPickupCastleSymbol(Race_t race, CastleSymbol* pCastleSymbol) const

{
    __BEGIN_TRY

    // First find out which war this castle symbol fragment belongs to.
    CastleShrineSet* pShrineSet = getShrineSet(pCastleSymbol->getItemType());

    if (pShrineSet == NULL) {
        return false;
    }

    ZoneID_t guardZoneID = pShrineSet->m_GuardShrine.getZoneID();
    ZoneID_t castleZoneID;

    bool isCastle = de::gameContext().castleInfos().getCastleZoneID(guardZoneID, castleZoneID);
    Assert(isCastle == true);

    War* pWar = de::gameContext().warSystem().getActiveWar(castleZoneID);

    if (pWar == NULL) {
        // Unexpected state.
        filelog("WarError.log", "전쟁도 안하는데 성의 상징조각을 주울려고 한다. ItemType: %u",
                (int)pCastleSymbol->getItemType());
        return false;
    }

    if (pWar->getWarType() == WAR_GUILD) {
        CastleInfo* pCastleInfo = de::gameContext().castleInfos().getCastleInfo(castleZoneID);

        if (pCastleInfo == NULL) {
            // Unexpected state.
            filelog("WarError.log", "성이 아니다. ItemType: %u, ZoneID : %u", (int)pCastleSymbol->getItemType(),
                    (int)castleZoneID);
            return false;
        }

        return (race == pCastleInfo->getRace());
    }

    // Unexpected state.
    filelog("WarError.log", "이상한 전쟁이다. WarType : %u", (int)pWar->getWarType());

    return false;
    __END_CATCH
}

bool CastleShrineInfoManager::getMatchGuardShrinePosition(Item* pItem, ZoneItemPosition& zip) const

{
    __BEGIN_TRY

    if (pItem->getItemClass() != Item::ITEM_CLASS_CASTLE_SYMBOL)
        return false;

    ItemType_t itemType = pItem->getItemType();
    ShrineID_t shrineID = itemType; // ShrineID = ItemType(of CastleSymbol)

    CastleShrineSet* pShrineSet = getShrineSet(shrineID);

    if (pShrineSet == NULL) {
        // There is no Shrine Set for this castle symbol.
        return false;
    }

    zip.setZoneID(pShrineSet->m_GuardShrine.getZoneID());
    zip.setZoneX(pShrineSet->m_GuardShrine.getX());
    zip.setZoneY(pShrineSet->m_GuardShrine.getY());

    return true;

    __END_CATCH
}

// Called from putCastleSymbol (someone placed the symbol on the holy shrine) with bLock = false,
// and from returnAllCastleSymbol (the time ran out) with bLock = true.
// When true it is called from another thread (the one the WarSystem runs on), so it must lock internally;
// when false it runs on the zone group thread of the zone holding the shrine, so it must not lock.
// 2003. 2. 5. by Sequoia
bool CastleShrineInfoManager::returnCastleSymbol(ShrineID_t shrineID, bool bLock) const

{
    __BEGIN_TRY

    // Find the CastleSymbol related to shrineID using the DB information.
    CastleShrineSet* pShrineSet = getShrineSet(shrineID);

    if (pShrineSet == NULL)
        return false;

    Item::ItemClass ItemClass = Item::ITEM_CLASS_CASTLE_SYMBOL;
    ItemID_t ItemID = pShrineSet->getCastleSymbolItemID();

    if (ItemID == 0)
        return false;

    GlobalItemPosition* pItemPosition = GlobalItemPositionLoader::getInstance()->load(ItemClass, ItemID);

    if (pItemPosition == NULL)
        return false;

    Item* pItem = pItemPosition->popItem(bLock);

    if (pItem != NULL && pItem->getItemClass() == Item::ITEM_CLASS_CASTLE_SYMBOL) {
        Zone* pZone = pItemPosition->getZone();
        Assert(pZone != NULL);

        CastleSymbol* pCastleSymbol = dynamic_cast<CastleSymbol*>(pItem);
        Assert(pCastleSymbol != NULL);

        return returnCastleSymbol(pZone, pCastleSymbol);
    }

    return false;

    __END_CATCH
}

// Called only from the WarSystem.
bool CastleShrineInfoManager::returnAllCastleSymbol(ZoneID_t castleZoneID) const

{
    __BEGIN_TRY

    bool bReturned = false;

    HashMapShrineSetConstItor itr = m_ShrineSets.begin();

    // The shrineID for castleZoneID cannot be looked up, so compare them one by one.
    for (; itr != m_ShrineSets.end(); itr++) {
        CastleShrineSet* pShrineSet = itr->second;

        ZoneID_t guardZoneID = pShrineSet->m_GuardShrine.getZoneID();
        ZoneID_t guardCastleZoneID;

        if (de::gameContext().castleInfos().getCastleZoneID(guardZoneID, guardCastleZoneID)) {
            if (castleZoneID == guardCastleZoneID) {
                bReturned = returnCastleSymbol(pShrineSet->m_ShrineID) || bReturned;
            }
        }
    }

    return bReturned;

    __END_CATCH
}


bool CastleShrineInfoManager::returnCastleSymbol(Zone* pZone, CastleSymbol* pCastleSymbol) const

{
    __BEGIN_TRY

    Assert(pZone != NULL);
    Assert(pCastleSymbol != NULL);

    // Find the TargetZone and Shrine.
    ShrineID_t shrineID = pCastleSymbol->getItemType();
    CastleShrineSet* pShrineSet = getShrineSet(shrineID);

    if (pShrineSet == NULL)
        return false;

    ShrineInfo& GuardShrine = pShrineSet->m_GuardShrine;

    Zone* pTargetZone = getZoneByZoneID(GuardShrine.getZoneID());
    Assert(pTargetZone != NULL);

    ObjectID_t CorpseObjectID = GuardShrine.getObjectID();

    pZone->transportItemToCorpse(pCastleSymbol, pTargetZone, CorpseObjectID);

    //	StringStream msg;
    //	msg << "Castle symbol fragment (" << GuardShrine.getName() << ") returned to the guard
    // shrine (" << GuardShrine.getName() << ").";

    char msg[200];
    sprintf(msg, de::gameContext().strings().c_str(STRID_RETURN_TO_GUARD_SHRINE_CASTLE_SYMBOL),
            GuardShrine.getName().c_str(), GuardShrine.getName().c_str());

    GCSystemMessage msgPkt;
    msgPkt.setMessage(msg);

    de::gameContext().castleInfos().broadcastShrinePacket(shrineID, &msgPkt);

    return true;

    __END_CATCH
}

bool CastleShrineInfoManager::putCastleSymbol(PlayerCreature* pPC, Item* pItem, MonsterCorpse* pCorpse) const

{
    __BEGIN_TRY

    Assert(pPC != NULL);
    Assert(pItem != NULL);
    Assert(pCorpse != NULL);

    ShrineID_t shrineID = pItem->getItemType();

    filelog("WarLog.txt", "%s가 성의 상징[%u]을 성지 성단[%s]에 넣었습니다.", pPC->getName().c_str(), (uint)shrineID,
            pCorpse->getName().c_str());

    // Attach the effect showing the castle symbol flying back from the shrine it was put into.
    //	sendCastleSymbolEffect( pCorpse, Effect::EFFECT_CLASS_SHRINE_HOLY_WARP );

    // Take the castle symbol from the PC and put it inside the shrine.
    Assert(pItem->getObjectID() == pPC->getExtraInventorySlotItem()->getObjectID());
    pPC->deleteItemFromExtraInventorySlot();

    GCDeleteInventoryItem gcDeleteInventoryItem;
    gcDeleteInventoryItem.setObjectID(pItem->getObjectID());

    pPC->getPlayer()->sendPacket(&gcDeleteInventoryItem);

    deleteRelicEffect(pPC, pItem);

    pCorpse->addTreasure(pItem);

    CastleShrineSet* pShrineSet = getShrineSet(shrineID);
    if (pShrineSet == NULL)
        return false;

    ZoneID_t guardZoneID = pShrineSet->m_GuardShrine.getZoneID();
    ZoneID_t castleZoneID;

    bool isCastle = de::gameContext().castleInfos().getCastleZoneID(guardZoneID, castleZoneID);
    Assert(isCastle == true);

    // Placing it in the matching shrine ends the war and returns it to the guard shrine,
    if (isMatchHolyShrine(pItem, pCorpse) && de::gameContext().warSystem().isModifyCastleOwner(castleZoneID, pPC)) {
        de::gameContext().warSystem().endWar(pPC, castleZoneID);

        // War::executeEnd returns it when the war ends.
        //        returnCastleSymbol( shrineID, false );

        return true;
    }

    // Placed in another shrine, or with no war about to end, it simply returns to the guard shrine.
    returnCastleSymbol(shrineID, false);

    return false;

    __END_CATCH
}

// pZone is the guardZone.
bool CastleShrineInfoManager::removeShrineShield(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);
    HashMapShrineSetConstItor itr = m_ShrineSets.begin();

    ZoneID_t guardZoneID = pZone->getZoneID();

    // The shrineID for castleZoneID cannot be looked up, so compare them one by one.
    for (; itr != m_ShrineSets.end(); itr++) {
        CastleShrineSet* pShrineSet = itr->second;

        ZoneID_t currentGuardZoneID = pShrineSet->m_GuardShrine.getZoneID();

        if (guardZoneID == currentGuardZoneID) {
            Item* pItem = pZone->getItem(pShrineSet->m_GuardShrine.getObjectID());

            if (pItem != NULL && pItem->getItemClass() == Item::ITEM_CLASS_CORPSE &&
                pItem->isFlag(Effect::EFFECT_CLASS_HAS_CASTLE_SYMBOL) &&
                pItem->isFlag(Effect::EFFECT_CLASS_SHRINE_SHIELD)) {
                pItem->removeFlag(Effect::EFFECT_CLASS_SHRINE_SHIELD);

                Corpse* pCorpse = dynamic_cast<Corpse*>(pItem);

                EffectManager& EM = pItem->getEffectManager();
                EM.deleteEffect(Effect::EFFECT_CLASS_SHRINE_SHIELD);

                GCRemoveEffect gcRemoveEffect;
                gcRemoveEffect.setObjectID(pItem->getObjectID());
                gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_SHRINE_SHIELD);
                pZone->broadcastPacket(pCorpse->getX(), pCorpse->getY(), &gcRemoveEffect);
            }
        }
    }

    return true;

    __END_CATCH
}

// pZone is the guardZone.
bool CastleShrineInfoManager::addShrineShield(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    bool bAdded = false;

    __ENTER_CRITICAL_SECTION((*pZone))

    bAdded = addShrineShield_LOCKED(pZone);

    __LEAVE_CRITICAL_SECTION((*pZone))

    return bAdded;

    __END_CATCH
}

// pZone is the guardZone.
bool CastleShrineInfoManager::addShrineShield_LOCKED(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);
    HashMapShrineSetConstItor itr = m_ShrineSets.begin();

    ZoneID_t guardZoneID = pZone->getZoneID();

    // The shrineID for castleZoneID cannot be looked up, so compare them one by one.
    for (; itr != m_ShrineSets.end(); itr++) {
        CastleShrineSet* pShrineSet = itr->second;

        ZoneID_t currentGuardZoneID = pShrineSet->m_GuardShrine.getZoneID();

        if (guardZoneID == currentGuardZoneID) {
            Item* pItem = pZone->getItem(pShrineSet->m_GuardShrine.getObjectID());

            if (pItem != NULL && pItem->getItemClass() == Item::ITEM_CLASS_CORPSE &&
                !pItem->isFlag(Effect::EFFECT_CLASS_SHRINE_SHIELD)) {
                pItem->setFlag(Effect::EFFECT_CLASS_SHRINE_SHIELD);

                Corpse* pCorpse = dynamic_cast<Corpse*>(pItem);
                Assert(pCorpse != NULL);

                EffectManager& EM = pItem->getEffectManager();
                EffectShrineShield* pEffect = new EffectShrineShield(pCorpse);
                pEffect->setShrineID(pCorpse->getItemType());
                pEffect->setTick(60 * 10);
                EM.addEffect(pEffect);

                GCAddEffect gcAddEffect;
                gcAddEffect.setObjectID(pItem->getObjectID());
                gcAddEffect.setEffectID(Effect::EFFECT_CLASS_SHRINE_SHIELD);
                gcAddEffect.setDuration(65000);

                pZone->broadcastPacket(pCorpse->getX(), pCorpse->getY(), &gcAddEffect);
            }
        }
    }

    return true;

    __END_CATCH
}

string CastleShrineInfoManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "ShrineInfoManager(" << "Size:" << size() << ",(\n";

    HashMapShrineSetConstItor itr = m_ShrineSets.begin();

    for (; itr != m_ShrineSets.end(); itr++) {
        msg << "\t" << itr->second->toString() << "\n";
    }

    msg << "))";

    return msg.toString();

    __END_CATCH
}
