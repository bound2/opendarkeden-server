//////////////////////////////////////////////////////////////////////////////
// Filename    : BombMaterial.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "BombMaterial.h"

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
BombMaterialInfoManager* g_pBombMaterialInfoManager = NULL;

ItemID_t BombMaterial::m_ItemIDRegistry = 0;
Mutex BombMaterial::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
BombMaterial::BombMaterial()

    : m_ItemType(0) {
    m_Num = 1;
}

BombMaterial::BombMaterial(ItemType_t itemType, const list<OptionType_t>& optionType)

    : m_ItemType(itemType) {
    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), m_ItemType, optionType)) {
        filelog("itembug.log", "BombMaterial::BombMaterial() : Invalid item type or option type");
        throw "BombMaterial::BombMaterial() : Invalid item type or optionType";
    }
}

//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void BombMaterial::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y,
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

    defaultItemObjectRepository().insertNumOnlyItem(GEAR_BOMB_MATERIAL, m_ItemID, m_ObjectID, m_ItemType, ownerID,
                                                    (int)storage, storageID, (int)x, (int)y, (int)m_Num);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void BombMaterial::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_BOMB_MATERIAL, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void BombMaterial::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    defaultItemObjectRepository().updateNumOnlyItem(GEAR_BOMB_MATERIAL, m_ObjectID, m_ItemType, ownerID, (int)storage,
                                                    storageID, (int)x, (int)y, (int)m_Num, m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string BombMaterial::toString() const

{
    StringStream msg;

    msg << "BombMaterial(" << "ItemID:" << m_ItemID << ",ItemType:" << m_ItemType << ",Num:" << (int)m_Num << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t BombMaterial::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pBombMaterialInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t BombMaterial::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pBombMaterialInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t BombMaterial::getWeight() const

{
    __BEGIN_TRY

    return g_pBombMaterialInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string BombMaterialInfo::toString() const

{
    StringStream msg;

    msg << "BombMaterialInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void BombMaterialInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_BOMB_MATERIAL);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<BasicInfoRow> rows = defaultItemObjectRepository().loadBasicInfos(GEAR_BOMB_MATERIAL);

    for (size_t r = 0; r < rows.size(); r++) {
        BombMaterialInfo* pBombMaterialInfo = new BombMaterialInfo();

        pBombMaterialInfo->setItemType(rows[r].itemType);
        pBombMaterialInfo->setName(rows[r].name);
        pBombMaterialInfo->setEName(rows[r].ename);
        pBombMaterialInfo->setPrice(rows[r].price);
        pBombMaterialInfo->setVolumeType(rows[r].volume);
        pBombMaterialInfo->setWeight(rows[r].weight);
        pBombMaterialInfo->setRatio(rows[r].ratio);

        addItemInfo(pBombMaterialInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void BombMaterialLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<NumOnlyObjectRow> rows =
        defaultItemObjectRepository().loadNumOnlyItemOfOwner(GEAR_BOMB_MATERIAL, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            BombMaterial* pBombMaterial = new BombMaterial();

            pBombMaterial->setItemID(rows[r].itemID);
            pBombMaterial->setObjectID(rows[r].objectID);
            pBombMaterial->setItemType(rows[r].itemType);

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            pBombMaterial->setNum(rows[r].num);

            Inventory* pInventory = NULL;
            Slayer* pSlayer = NULL;
            Vampire* pVampire = NULL;
            Motorcycle* pMotorcycle = NULL;
            Inventory* pMotorInventory = NULL;
            // Item*       pItem           = NULL;
            Stash* pStash = NULL;
            // Belt*       pBelt           = NULL;
            // Inventory*  pBeltInventory  = NULL;

            if (pCreature->isSlayer()) {
                pSlayer = dynamic_cast<Slayer*>(pCreature);
                pInventory = pSlayer->getInventory();
                pStash = pSlayer->getStash();
                pMotorcycle = pSlayer->getMotorcycle();

                if (pMotorcycle)
                    pMotorInventory = pMotorcycle->getInventory();
            } else if (pCreature->isVampire()) {
                pVampire = dynamic_cast<Vampire*>(pCreature);
                pInventory = pVampire->getInventory();
                pStash = pVampire->getStash();
            } else
                throw UnsupportedError("Monster,NPC 인벤토리의 저장은 아직 지원되지 않습니다.");

            switch (storage) {
            case STORAGE_INVENTORY:
                if (pInventory->canAddingEx(x, y, pBombMaterial)) {
                    pInventory->addItemEx(x, y, pBombMaterial);
                } else {
                    processItemBugEx(pCreature, pBombMaterial);
                }
                break;

            case STORAGE_GEAR:
                processItemBugEx(pCreature, pBombMaterial);
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pBombMaterial);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pBombMaterial);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pBombMaterial);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pBombMaterial);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pBombMaterial);
                } else
                    pStash->insert(x, y, pBombMaterial);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pBombMaterial);
                break;

            default:
                throw Error("invalid storage or OwnerID must be NULL");
            }

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
void BombMaterialLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<BombZoneObjectRow> rows =
        defaultItemObjectRepository().loadBombInZone(GEAR_BOMB_MATERIAL, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        BombMaterial* pBombMaterial = new BombMaterial();

        pBombMaterial->setItemID(rows[r].itemID);
        pBombMaterial->setObjectID(rows[r].objectID);
        pBombMaterial->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pBombMaterial);
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
void BombMaterialLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

BombMaterialLoader* g_pBombMaterialLoader = NULL;
