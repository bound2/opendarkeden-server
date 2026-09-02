//////////////////////////////////////////////////////////////////////////////
// Filename    : SubInventory.cpp
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "SubInventory.h"

#include "Belt.h"
#include "DB.h"
#include "ItemInfoManager.h"
#include "ItemUtil.h"
#include "Motorcycle.h"
#include "Ousters.h"
#include "Slayer.h"
#include "Stash.h"
#include "Utility.h"
#include "Vampire.h"
#include "repository/ItemObjectRepository.h"

SubInventoryInfoManager* g_pSubInventoryInfoManager = NULL;

ItemID_t SubInventory::m_ItemIDRegistry = 0;
Mutex SubInventory::m_Mutex;

//////////////////////////////////////////////////////////////////////////////
// class SubInventory member methods
//////////////////////////////////////////////////////////////////////////////

SubInventory::SubInventory()

{
    m_ItemType = 0;
    m_pInventory = NULL;
}

SubInventory::SubInventory(ItemType_t itemType, const list<OptionType_t>& optionType)

{
    m_ItemType = itemType;

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), m_ItemType, optionType)) {
        filelog("itembug.log", "SubInventory::SubInventory() : Invalid item type or option type");
        throw("SubInventory::SubInventory() : Invalid item type or optionType");
    }

    SubInventoryInfo* pInfo = dynamic_cast<SubInventoryInfo*>(g_pSubInventoryInfoManager->getItemInfo(itemType));
    m_pInventory = new Inventory(pInfo->getWidth(), pInfo->getHeight());
}

SubInventory::~SubInventory() {
    SAFE_DELETE(m_pInventory);
}

void SubInventory::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y,
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

    defaultItemObjectRepository().insertFlagItem(GEAR_SUB_INVENTORY, m_ItemID, m_ObjectID, m_ItemType, ownerID,
                                                 (int)storage, storageID, (int)x, (int)y, (int)m_CreateType);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void SubInventory::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_SUB_INVENTORY, field, m_ItemID);

    __END_CATCH
}

void SubInventory::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    defaultItemObjectRepository().updatePlainItem(GEAR_SUB_INVENTORY, m_ObjectID, m_ItemType, ownerID, (int)storage,
                                                  storageID, (int)x, (int)y, m_ItemID);

    __END_CATCH
}

string SubInventory::toString() const

{
    StringStream msg;

    msg << "SubInventory(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)m_ItemType << ")";

    return msg.toString();
}

VolumeWidth_t SubInventory::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pSubInventoryInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}

VolumeHeight_t SubInventory::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pSubInventoryInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}

Weight_t SubInventory::getWeight() const

{
    __BEGIN_TRY

    return g_pSubInventoryInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}

void SubInventory::makePCItemInfo(PCItemInfo& result) const {
    Item::makePCItemInfo(result);
    result.setEnchantLevel(m_pInventory->getItemNum());
}

//////////////////////////////////////////////////////////////////////////////
// class SubInventoryInfo member methods
//////////////////////////////////////////////////////////////////////////////

string SubInventoryInfo::toString() const

{
    StringStream msg;
    msg << "SubInventoryInfo(" << "ItemType:" << (int)m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << (int)m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << (int)m_Weight
        << ",Description:" << m_Description << ")";
    return msg.toString();
}

void SubInventoryInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_SUB_INVENTORY);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<IntPairInfoRow> rows = defaultItemObjectRepository().loadIntPairInfos(GEAR_SUB_INVENTORY);

    for (size_t r = 0; r < rows.size(); r++) {
        SubInventoryInfo* pSubInventoryInfo = new SubInventoryInfo();

        pSubInventoryInfo->setItemType(rows[r].basic.itemType);
        pSubInventoryInfo->setName(rows[r].basic.name);
        pSubInventoryInfo->setEName(rows[r].basic.ename);
        pSubInventoryInfo->setPrice(rows[r].basic.price);
        pSubInventoryInfo->setVolumeType(rows[r].basic.volume);
        pSubInventoryInfo->setWeight(rows[r].basic.weight);
        pSubInventoryInfo->setRatio(rows[r].basic.ratio);
        pSubInventoryInfo->setWidth(rows[r].first);
        pSubInventoryInfo->setHeight(rows[r].second);

        addItemInfo(pSubInventoryInfo);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// class SubInventoryLoader member methods
//////////////////////////////////////////////////////////////////////////////

void SubInventoryLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<FlagObjectRow> rows =
        defaultItemObjectRepository().loadFlagItemOfOwner(GEAR_SUB_INVENTORY, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            SubInventory* pSubInventory = new SubInventory();

            pSubInventory->setItemID(rows[r].itemID);
            pSubInventory->setObjectID(rows[r].objectID);
            pSubInventory->setItemType(rows[r].itemType);

            SubInventoryInfo* pInfo =
                dynamic_cast<SubInventoryInfo*>(g_pSubInventoryInfoManager->getItemInfo(pSubInventory->getItemType()));
            pSubInventory->setInventory(new Inventory(pInfo->getWidth(), pInfo->getHeight()));

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            pSubInventory->setCreateType((Item::CreateType)rows[r].createType);

            Inventory* pInventory = NULL;
            Slayer* pSlayer = NULL;
            Vampire* pVampire = NULL;
            Ousters* pOusters = NULL;
            Motorcycle* pMotorcycle = NULL;
            Inventory* pMotorInventory = NULL;
            Item* pItem = NULL;
            Stash* pStash = NULL;
            Belt* pBelt = NULL;
            Inventory* pBeltInventory = NULL;

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
            } else if (pCreature->isOusters()) {
                pOusters = dynamic_cast<Ousters*>(pCreature);
                pInventory = pOusters->getInventory();
                pStash = pOusters->getStash();
            } else
                throw UnsupportedError("Monster,NPC 인벤토리의 저장은 아직 지원되지 않습니다.");

            switch (storage) {
            case STORAGE_INVENTORY:
                if (storageID != 0) {
                    processItemBugEx(pCreature, pSubInventory);
                    break;
                }

                if (pInventory->canAddingEx(x, y, pSubInventory)) {
                    pInventory->addItemEx(x, y, pSubInventory);
                } else {
                    processItemBugEx(pCreature, pSubInventory);
                }
                break;

            case STORAGE_GEAR:
                processItemBugEx(pCreature, pSubInventory);
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pSubInventory);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pSubInventory);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pSubInventory);
                else if (pCreature->isOusters())
                    pOusters->addItemToExtraInventorySlot(pSubInventory);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pSubInventory);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pSubInventory);
                } else
                    pStash->insert(x, y, pSubInventory);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pSubInventory);
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

void SubInventoryLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<FlagZoneObjectRow> rows =
        defaultItemObjectRepository().loadFlagItemInZone(GEAR_SUB_INVENTORY, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        SubInventory* pSubInventory = new SubInventory();

        pSubInventory->setItemID(rows[r].itemID);
        pSubInventory->setObjectID(rows[r].objectID);
        pSubInventory->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        pSubInventory->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pSubInventory);
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

void SubInventoryLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

SubInventoryLoader* g_pSubInventoryLoader = NULL;
