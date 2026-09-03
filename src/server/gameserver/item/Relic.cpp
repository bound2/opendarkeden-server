//////////////////////////////////////////////////////////////////////////////
// Filename    : Relic.cpp
// Written By  : Changaya
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Relic.h"

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
RelicInfoManager* g_pRelicInfoManager = NULL;

ItemID_t Relic::m_ItemIDRegistry = 0;
Mutex Relic::m_Mutex;


const string RelicType2String[] = {"RELIC_TYPE_SLAYER", "RELIC_TYPE_VAMPIRE"};


//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
Relic::Relic()

    : m_ItemType(0), m_Durability(0) {
    m_EnchantLevel = 0;
}

Relic::Relic(ItemType_t itemType, const list<OptionType_t>& optionType)

    : m_ItemType(itemType), m_Durability(0) {
    try {
        m_EnchantLevel = 0;

        m_Durability = computeMaxDurability(this);

        if (!g_pItemInfoManager->isPossibleItem(getItemClass(), m_ItemType, optionType)) {
            filelog("itembug.log", "Relic::Relic() : Invalid item type or option type");
            throw("Relic::Relic() : Invalid item type or optionType");
        }
    } catch (Throwable& t) {
        cout << t.toString().c_str() << endl;
        Assert(false);
    }
}


//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void Relic::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y, ItemID_t itemID)

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

    defaultItemObjectRepository().insertWarItem(GEAR_RELIC, m_ItemID, m_ObjectID, m_ItemType, ownerID, (int)storage,
                                                storageID, (int)x, (int)y, m_Durability);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Relic::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_RELIC, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Relic::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    defaultItemObjectRepository().updateWarItem(GEAR_RELIC, m_ObjectID, m_ItemType, ownerID, (int)storage, storageID,
                                                (int)x, (int)y, m_Durability, (int)m_EnchantLevel, m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string Relic::toString() const

{
    StringStream msg;

    msg << "Relic(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)m_ItemType << ",Durability:" << (int)m_Durability
        << ",EnchantLevel:" << (int)m_EnchantLevel << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t Relic::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pRelicInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t Relic::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pRelicInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t Relic::getWeight() const

{
    __BEGIN_TRY

    return g_pRelicInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set armor's Defense Bonus
//--------------------------------------------------------------------------------
Defense_t Relic::getDefenseBonus() const

{
    __BEGIN_TRY

    return g_pRelicInfoManager->getItemInfo(m_ItemType)->getDefenseBonus();

    __END_CATCH
}
Protection_t Relic::getProtectionBonus() const

{
    __BEGIN_TRY

    return g_pRelicInfoManager->getItemInfo(m_ItemType)->getProtectionBonus();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string RelicInfo::toString() const

{
    StringStream msg;

    msg << "RelicInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ",DefenseBonus:" << m_DefenseBonus
        << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void RelicInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_RELIC);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<RelicInfoRow> rows = defaultItemObjectRepository().loadRelicInfos(GEAR_RELIC);

    for (size_t r = 0; r < rows.size(); r++) {
        RelicInfo* pRelicInfo = new RelicInfo();

        pRelicInfo->setItemType(rows[r].war.itemType);
        pRelicInfo->setName(rows[r].war.name);
        pRelicInfo->setEName(rows[r].war.ename);
        pRelicInfo->setPrice(rows[r].war.price);
        pRelicInfo->setVolumeType(rows[r].war.volume);
        pRelicInfo->setWeight(rows[r].war.weight);
        pRelicInfo->setRatio(rows[r].war.ratio);
        pRelicInfo->setDurability(rows[r].war.durability);
        pRelicInfo->setDefenseBonus(rows[r].war.defense);
        pRelicInfo->setProtectionBonus(rows[r].war.protection);
        pRelicInfo->setReqAbility(rows[r].war.reqAbility);
        pRelicInfo->setItemLevel(rows[r].war.itemLevel);
        pRelicInfo->setRelicType(rows[r].relicType);
        pRelicInfo->zoneID = rows[r].zoneID;
        pRelicInfo->x = rows[r].x;
        pRelicInfo->y = rows[r].y;
        pRelicInfo->monsterType = rows[r].monsterType;

        addItemInfo(pRelicInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void RelicLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    // Relic load할게 있다는것은..
    // 현재로서는 이전에 서버다운이 되었다는 의미이다.
    // 그래서, 지운다. by sigi
    defaultItemObjectRepository().deleteWarItemsOfOwner(GEAR_RELIC, pCreature->getName());

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to zone
//--------------------------------------------------------------------------------
void RelicLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<WarItemZoneObjectRow> rows =
        defaultItemObjectRepository().loadWarItemInZone(GEAR_RELIC, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        Relic* pRelic = new Relic();

        pRelic->setItemID(rows[r].itemID);
        pRelic->setObjectID(rows[r].objectID);
        pRelic->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        pRelic->setDurability(rows[r].durability);
        pRelic->setEnchantLevel(rows[r].enchantLevel);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pRelic);
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
void RelicLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

RelicLoader* g_pRelicLoader = NULL;
