//////////////////////////////////////////////////////////////////////////////
// Filename    : CastleSymbol.cpp
// Written By  : Changaya
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CastleSymbol.h"

#include <stdio.h>

#include "Belt.h"
#include "DB.h"
#include "ItemInfoManager.h"
#include "ItemUtil.h"
#include "Motorcycle.h"
#include "Slayer.h"
#include "Stash.h"
#include "Vampire.h"
#include "repository/ItemObjectRepository.h"

// global variable declaration
CastleSymbolInfoManager* g_pCastleSymbolInfoManager = NULL;

ItemID_t CastleSymbol::m_ItemIDRegistry = 0;
Mutex CastleSymbol::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
CastleSymbol::CastleSymbol()

    : m_ItemType(0), m_Durability(0) {
    m_EnchantLevel = 0;
}

CastleSymbol::CastleSymbol(ItemType_t itemType, const list<OptionType_t>& optionType)

    : m_ItemType(itemType), m_Durability(0) {
    try {
        m_EnchantLevel = 0;

        m_Durability = computeMaxDurability(this);

        if (!g_pItemInfoManager->isPossibleItem(getItemClass(), m_ItemType, optionType)) {
            filelog("itembug.log", "CastleSymbol::CastleSymbol() : Invalid item type or option type");
            throw("CastleSymbol::CastleSymbol() : Invalid item type or optionType");
        }
    } catch (Throwable& t) {
        cout << t.toString().c_str() << endl;
        Assert(false);
    }
}


//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void CastleSymbol::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y,
                          ItemID_t itemID)

{
    __BEGIN_TRY

    if (itemID == 0) {
        __ENTER_CRITICAL_SECTION(m_Mutex)

        m_ItemIDRegistry += g_pItemInfoManager->getItemIDSuccessor();
        m_ItemID = m_ItemIDRegistry;

        __LEAVE_CRITICAL_SECTION(m_Mutex)
    } else {
        m_ItemID = itemID;
    }

    const string sql =
        defaultItemObjectRepository().insertWarItem(GEAR_CASTLE_SYMBOL, m_ItemID, m_ObjectID, m_ItemType, ownerID,
                                                    (int)storage, storageID, (int)x, (int)y, m_Durability);
    filelog("WarLog.txt", "%s", sql.c_str());

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void CastleSymbol::tinysave(const char* field) const

{
    __BEGIN_TRY

    char query[255];

    sprintf(query, "UPDATE CastleSymbolObject SET %s WHERE ItemID=%ld", field, m_ItemID);
    defaultItemObjectRepository().tinysaveGear(GEAR_CASTLE_SYMBOL, field, m_ItemID);
    filelog("WarLog.txt", "%s", query);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void CastleSymbol::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    defaultItemObjectRepository().updateWarItem(GEAR_CASTLE_SYMBOL, m_ObjectID, m_ItemType, ownerID, (int)storage,
                                                storageID, (int)x, (int)y, m_Durability, (int)m_EnchantLevel, m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string CastleSymbol::toString() const

{
    StringStream msg;

    msg << "CastleSymbol(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)m_ItemType
        << ",Durability:" << (int)m_Durability << ",EnchantLevel:" << (int)m_EnchantLevel << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t CastleSymbol::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pCastleSymbolInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t CastleSymbol::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pCastleSymbolInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t CastleSymbol::getWeight() const

{
    __BEGIN_TRY

    return g_pCastleSymbolInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set armor's Defense Bonus
//--------------------------------------------------------------------------------
Defense_t CastleSymbol::getDefenseBonus() const

{
    __BEGIN_TRY

    return g_pCastleSymbolInfoManager->getItemInfo(m_ItemType)->getDefenseBonus();

    __END_CATCH
}
Protection_t CastleSymbol::getProtectionBonus() const

{
    __BEGIN_TRY

    return g_pCastleSymbolInfoManager->getItemInfo(m_ItemType)->getProtectionBonus();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string CastleSymbolInfo::toString() const

{
    StringStream msg;

    msg << "CastleSymbolInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ",DefenseBonus:" << m_DefenseBonus
        << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void CastleSymbolInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_CASTLE_SYMBOL);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<WarInfoRow> rows = defaultItemObjectRepository().loadWarInfos(GEAR_CASTLE_SYMBOL);

    for (size_t r = 0; r < rows.size(); r++) {
        CastleSymbolInfo* pCastleSymbolInfo = new CastleSymbolInfo();

        pCastleSymbolInfo->setItemType(rows[r].itemType);
        pCastleSymbolInfo->setName(rows[r].name);
        pCastleSymbolInfo->setEName(rows[r].ename);
        pCastleSymbolInfo->setPrice(rows[r].price);
        pCastleSymbolInfo->setVolumeType(rows[r].volume);
        pCastleSymbolInfo->setWeight(rows[r].weight);
        pCastleSymbolInfo->setRatio(rows[r].ratio);
        pCastleSymbolInfo->setDurability(rows[r].durability);
        pCastleSymbolInfo->setDefenseBonus(rows[r].defense);
        pCastleSymbolInfo->setProtectionBonus(rows[r].protection);
        pCastleSymbolInfo->setReqAbility(rows[r].reqAbility);
        pCastleSymbolInfo->setItemLevel(rows[r].itemLevel);

        addItemInfo(pCastleSymbolInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void CastleSymbolLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    // CastleSymbol load할게 있다는것은..
    // 현재로서는 이전에 서버다운이 되었다는 의미이다.
    // 그래서, 지운다. by sigi
    defaultItemObjectRepository().deleteWarItemsOfOwner(GEAR_CASTLE_SYMBOL, pCreature->getName());

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to zone
//--------------------------------------------------------------------------------
void CastleSymbolLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<WarItemZoneObjectRow> rows =
        defaultItemObjectRepository().loadWarItemInZone(GEAR_CASTLE_SYMBOL, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        CastleSymbol* pCastleSymbol = new CastleSymbol();

        pCastleSymbol->setItemID(rows[r].itemID);
        pCastleSymbol->setObjectID(rows[r].objectID);
        pCastleSymbol->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        pCastleSymbol->setDurability(rows[r].durability);
        pCastleSymbol->setEnchantLevel(rows[r].enchantLevel);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pCastleSymbol);
        } break;

        case STORAGE_STASH:
        case STORAGE_CORPSE:
            throw UnsupportedError("상자 및 시체안의 아이템의 저장은 아직 지원되지 않습니다.");

        default:
            throw Error("Storage must be STORAGE_ZONE");
        }
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to inventory
//--------------------------------------------------------------------------------
void CastleSymbolLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

CastleSymbolLoader* g_pCastleSymbolLoader = NULL;
