//////////////////////////////////////////////////////////////////////////////
// Filename    : Motorcycle.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Motorcycle.h"

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
MotorcycleInfoManager* g_pMotorcycleInfoManager = NULL;

ItemID_t Motorcycle::m_ItemIDRegistry = 0;
Mutex Motorcycle::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
Motorcycle::Motorcycle()

    : m_ItemType(0), m_Durability(0), m_pInventory(NULL) {}

Motorcycle::Motorcycle(ItemType_t itemType, const list<OptionType_t>& optionType)

    : m_ItemType(itemType), m_OptionType(optionType), m_Durability(0), m_pInventory(NULL) {
    __BEGIN_TRY

    // 모터사이클은 타입에 따라 인벤토리의 크기가 다르다.
    switch (itemType) {
    case 0:
        m_pInventory = new Inventory(10, 6);
        break;
    case 1:
        m_pInventory = new Inventory(10, 6);
        break;
    case 2:
        m_pInventory = new Inventory(10, 6);
        break;
    case 3:
        m_pInventory = new Inventory(10, 6);
        break;
    case 4:
        m_pInventory = new Inventory(10, 6);
        break;
    case 5:
        m_pInventory = new Inventory(10, 6);
        break;
    default:
        m_pInventory = new Inventory(10, 6);
        break;
    }

    m_Durability = computeMaxDurability(this);

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), m_ItemType, optionType)) {
        filelog("itembug.log", "Motorcycle::Motorcycle() : Invalid item type or option type");
        throw "Motorcycle::Motorcycle() : Invalid item type or optionType";
    }

    __END_CATCH
}

Motorcycle::~Motorcycle()

{
    SAFE_DELETE(m_pInventory);
}


//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void Motorcycle::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y, ItemID_t itemID)

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

    string optionField;
    setOptionTypeToField(m_OptionType, optionField);

    defaultItemObjectRepository().insertMotorcycle(GEAR_MOTORCYCLE, m_ItemID, m_ObjectID, m_ItemType, ownerID,
                                                   (int)storage, storageID, (int)x, (int)y, optionField, m_Durability);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Motorcycle::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_MOTORCYCLE, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Motorcycle::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(m_OptionType, optionField);

    defaultItemObjectRepository().updateMotorcycle(GEAR_MOTORCYCLE, m_ObjectID, m_ItemType, ownerID, (int)storage,
                                                   storageID, (int)x, (int)y, optionField, m_Durability, m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string Motorcycle::toString() const

{
    StringStream msg;

    msg << "Motorcycle(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)m_ItemType
        << ",OptionType:" << getOptionTypeToString(m_OptionType).c_str() << ",Durability:" << (int)m_Durability << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t Motorcycle::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pMotorcycleInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t Motorcycle::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pMotorcycleInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t Motorcycle::getWeight() const

{
    __BEGIN_TRY

    return g_pMotorcycleInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string MotorcycleInfo::toString() const

{
    StringStream msg;

    msg << "MotorcycleInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void MotorcycleInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_MOTORCYCLE);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<DurabilityInfoRow> rows = defaultItemObjectRepository().loadDurabilityInfos(GEAR_MOTORCYCLE);

    for (size_t r = 0; r < rows.size(); r++) {
        MotorcycleInfo* pMotorcycleInfo = new MotorcycleInfo();

        pMotorcycleInfo->setItemType(rows[r].itemType);
        pMotorcycleInfo->setName(rows[r].name);
        pMotorcycleInfo->setEName(rows[r].ename);
        pMotorcycleInfo->setPrice(rows[r].price);
        pMotorcycleInfo->setVolumeType(rows[r].volume);
        pMotorcycleInfo->setWeight(rows[r].weight);
        pMotorcycleInfo->setRatio(rows[r].ratio);
        pMotorcycleInfo->setDurability(rows[r].durability);

        addItemInfo(pMotorcycleInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void MotorcycleLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<MotorcycleObjectRow> rows =
        defaultItemObjectRepository().loadMotorcycleOfOwner(GEAR_MOTORCYCLE, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            Motorcycle* pMotorcycle = new Motorcycle();

            pMotorcycle->setItemID(rows[r].itemID);
            pMotorcycle->setObjectID(rows[r].objectID);
            pMotorcycle->setItemType(rows[r].itemType);

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pMotorcycle->setOptionType(optionTypes);

            pMotorcycle->setDurability(rows[r].durability);

            /*
                switch(storage)
                {
                    case STORAGE_INVENTORY:
                    case STORAGE_GEAR:
                    case STORAGE_BELT :
                    case STORAGE_EXTRASLOT :
                    case STORAGE_MOTORCYCLE:
                    case STORAGE_STASH:
                        // 모터 사이클 안에 모터 사이클을 보관할 수가 있나
                        Assert(false);
                        pMotorcycle->destroy();
                        SAFE_DELETE(pMotorcycle);
                        break;

                    default :
                        SAFE_DELETE(pStmt);	// by sigi
                        throw Error("invalid storage or OwnerID must be NULL");
                }
                */

        } catch (Error& error) {
            filelog("itemLoadError.txt", "[%s] %s", getItemClassName().c_str(), error.toString().c_str());
            throw;
        } catch (Throwable& t) {
            filelog("itemLoadError.txt", "[%s] %s", getItemClassName().c_str(), t.toString().c_str());
        }
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to zone
//--------------------------------------------------------------------------------
void MotorcycleLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<MotorcycleZoneObjectRow> rows =
        defaultItemObjectRepository().loadMotorcycleInZone(GEAR_MOTORCYCLE, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        Motorcycle* pMotorcycle = new Motorcycle();

        pMotorcycle->setItemID(rows[r].itemID);
        pMotorcycle->setObjectID(rows[r].objectID);
        pMotorcycle->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        pMotorcycle->setDurability(rows[r].durability);

        switch (storage) {
        case STORAGE_ZONE: {
            //						Tile & pTile = pZone->getTile(x,y);
            //						Assert(!pTile.hasItem());
            pZone->addItem(pMotorcycle, x, y);
            //						pTile.addItem(pMotorcycle);
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
void MotorcycleLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

MotorcycleLoader* g_pMotorcycleLoader = NULL;
