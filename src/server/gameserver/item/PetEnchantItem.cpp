//////////////////////////////////////////////////////////////////////////////
// Filename    : PetEnchantItem.cpp
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "PetEnchantItem.h"

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

PetEnchantItemInfoManager* g_pPetEnchantItemInfoManager = NULL;

ItemID_t PetEnchantItem::m_ItemIDRegistry = 0;
Mutex PetEnchantItem::m_Mutex;

//////////////////////////////////////////////////////////////////////////////
// class PetEnchantItem member methods
//////////////////////////////////////////////////////////////////////////////

PetEnchantItem::PetEnchantItem()

{
    m_ItemType = 0;
}

PetEnchantItem::PetEnchantItem(ItemType_t itemType, const list<OptionType_t>& optionType, ItemNum_t Num)

{
    m_ItemType = itemType;
    m_Num = Num;

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), m_ItemType, optionType)) {
        filelog("itembug.log", "PetEnchantItem::PetEnchantItem() : Invalid item type or option type");
        throw "PetEnchantItem::PetEnchantItem() : Invalid item type or optionType";
    }
}

void PetEnchantItem::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y,
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

    defaultItemObjectRepository().insertNumItem(GEAR_PET_ENCHANT_ITEM, m_ItemID, m_ObjectID, m_ItemType, ownerID,
                                                (int)storage, storageID, (int)x, (int)y, (int)m_Num, (int)m_CreateType);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void PetEnchantItem::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_PET_ENCHANT_ITEM, field, m_ItemID);

    __END_CATCH
}

void PetEnchantItem::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    defaultItemObjectRepository().updateNumItem(GEAR_PET_ENCHANT_ITEM, m_ObjectID, m_ItemType, ownerID, (int)storage,
                                                storageID, (int)x, (int)y, (int)m_Num, m_ItemID);

    __END_CATCH
}

string PetEnchantItem::toString() const

{
    StringStream msg;

    msg << "PetEnchantItem(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)m_ItemType << ",Num:" << (int)m_Num
        << ")";

    return msg.toString();
}

VolumeWidth_t PetEnchantItem::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pPetEnchantItemInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}

VolumeHeight_t PetEnchantItem::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pPetEnchantItemInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}

Weight_t PetEnchantItem::getWeight() const

{
    __BEGIN_TRY

    return g_pPetEnchantItemInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// class PetEnchantItemInfo member methods
//////////////////////////////////////////////////////////////////////////////

string PetEnchantItemInfo::toString() const

{
    StringStream msg;
    msg << "PetEnchantItemInfo(" << "ItemType:" << (int)m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << (int)m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << (int)m_Weight
        << ",Description:" << m_Description << ")";
    return msg.toString();
}

void PetEnchantItemInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_PET_ENCHANT_ITEM);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<FunctionGradeInfoRow> rows = defaultItemObjectRepository().loadFunctionGradeInfos(GEAR_PET_ENCHANT_ITEM);

    for (size_t r = 0; r < rows.size(); r++) {
        PetEnchantItemInfo* pPetEnchantItemInfo = new PetEnchantItemInfo();

        pPetEnchantItemInfo->setItemType(rows[r].basic.itemType);
        pPetEnchantItemInfo->setName(rows[r].basic.name);
        pPetEnchantItemInfo->setEName(rows[r].basic.ename);
        pPetEnchantItemInfo->setPrice(rows[r].basic.price);
        pPetEnchantItemInfo->setVolumeType(rows[r].basic.volume);
        pPetEnchantItemInfo->setWeight(rows[r].basic.weight);
        pPetEnchantItemInfo->setRatio(rows[r].basic.ratio);
        pPetEnchantItemInfo->setFunction(rows[r].function);
        pPetEnchantItemInfo->setFunctionGrade(rows[r].functionGrade);

        addItemInfo(pPetEnchantItemInfo);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// class PetEnchantItemLoader member methods
//////////////////////////////////////////////////////////////////////////////

void PetEnchantItemLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<NumObjectRow> rows =
        defaultItemObjectRepository().loadNumItemOfOwner(GEAR_PET_ENCHANT_ITEM, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            PetEnchantItem* pPetEnchantItem = new PetEnchantItem();

            pPetEnchantItem->setItemID(rows[r].itemID);
            pPetEnchantItem->setObjectID(rows[r].objectID);
            pPetEnchantItem->setItemType(rows[r].itemType);

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            pPetEnchantItem->setNum(rows[r].num);
            pPetEnchantItem->setCreateType((Item::CreateType)rows[r].createType);

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
                if (pInventory->canAddingEx(x, y, pPetEnchantItem)) {
                    pInventory->addItemEx(x, y, pPetEnchantItem);
                } else {
                    processItemBugEx(pCreature, pPetEnchantItem);
                }
                break;

            case STORAGE_GEAR:
                processItemBugEx(pCreature, pPetEnchantItem);
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pPetEnchantItem);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pPetEnchantItem);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pPetEnchantItem);
                else if (pCreature->isOusters())
                    pOusters->addItemToExtraInventorySlot(pPetEnchantItem);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pPetEnchantItem);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pPetEnchantItem);
                } else
                    pStash->insert(x, y, pPetEnchantItem);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pPetEnchantItem);
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

void PetEnchantItemLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<NumZoneObjectRow> rows =
        defaultItemObjectRepository().loadNumItemInZone(GEAR_PET_ENCHANT_ITEM, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        PetEnchantItem* pPetEnchantItem = new PetEnchantItem();

        pPetEnchantItem->setItemID(rows[r].itemID);
        pPetEnchantItem->setObjectID(rows[r].objectID);
        pPetEnchantItem->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        pPetEnchantItem->setNum(rows[r].num);
        pPetEnchantItem->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pPetEnchantItem);
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

void PetEnchantItemLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

PetEnchantItemLoader* g_pPetEnchantItemLoader = NULL;
