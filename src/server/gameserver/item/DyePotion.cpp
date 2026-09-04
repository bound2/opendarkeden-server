//////////////////////////////////////////////////////////////////////////////
// Filename    : DyePotion.cpp
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "DyePotion.h"

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

DyePotionInfoManager* g_pDyePotionInfoManager = NULL;

ItemID_t DyePotion::m_ItemIDRegistry = 0;
Mutex DyePotion::m_Mutex;

//////////////////////////////////////////////////////////////////////////////
// class DyePotion member methods
//////////////////////////////////////////////////////////////////////////////

DyePotion::DyePotion()

{
    setItemType(0);
}

DyePotion::DyePotion(ItemType_t itemType, const list<OptionType_t>& optionType, ItemNum_t Num)

{
    setItemType(itemType);
    setNum(Num);

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "DyePotion::DyePotion() : Invalid item type or option type");
        throw "DyePotion::DyePotion() : Invalid item type or optionType";
    }
}

void DyePotion::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y, ItemID_t itemID)

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

    defaultItemObjectRepository().insertNumItem(GEAR_DYE_POTION, m_ItemID, m_ObjectID, getItemType(), ownerID,
                                                (int)storage, storageID, (int)x, (int)y, (int)getNum(),
                                                (int)m_CreateType);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void DyePotion::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_DYE_POTION, field, m_ItemID);

    __END_CATCH
}

void DyePotion::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    defaultItemObjectRepository().updateNumItem(GEAR_DYE_POTION, m_ObjectID, getItemType(), ownerID, (int)storage,
                                                storageID, (int)x, (int)y, (int)getNum(), m_ItemID);

    __END_CATCH
}

string DyePotion::toString() const

{
    StringStream msg;

    msg << "DyePotion(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType() << ",Num:" << (int)getNum()
        << ")";

    return msg.toString();
}

/*VolumeWidth_t DyePotion::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pDyePotionInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}

VolumeHeight_t DyePotion::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pDyePotionInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}

Weight_t DyePotion::getWeight() const

{
    __BEGIN_TRY

    return g_pDyePotionInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}
*/
//////////////////////////////////////////////////////////////////////////////
// class DyePotionInfo member methods
//////////////////////////////////////////////////////////////////////////////

string DyePotionInfo::toString() const

{
    StringStream msg;
    msg << "DyePotionInfo(" << "ItemType:" << (int)m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << (int)m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << (int)m_Weight
        << ",Function:" << (int)m_fFunction << ",FunctionValue:" << (int)m_FunctionValue
        << ",Description:" << m_Description << ")";
    return msg.toString();
}

void DyePotionInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_DYE_POTION);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<FunctionValueInfoRow> rows = defaultItemObjectRepository().loadFunctionValueInfos(GEAR_DYE_POTION);

    for (size_t r = 0; r < rows.size(); r++) {
        DyePotionInfo* pDyePotionInfo = new DyePotionInfo();

        pDyePotionInfo->setItemType(rows[r].basic.itemType);
        pDyePotionInfo->setName(rows[r].basic.name);
        pDyePotionInfo->setEName(rows[r].basic.ename);
        pDyePotionInfo->setPrice(rows[r].basic.price);
        pDyePotionInfo->setVolumeType(rows[r].basic.volume);
        pDyePotionInfo->setWeight(rows[r].basic.weight);
        pDyePotionInfo->setRatio(rows[r].basic.ratio);
        pDyePotionInfo->setFunctionFlag(rows[r].functionFlag);
        pDyePotionInfo->setFunctionValue(rows[r].functionValue);

        addItemInfo(pDyePotionInfo);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// class DyePotionLoader member methods
//////////////////////////////////////////////////////////////////////////////

void DyePotionLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<NumObjectRow> rows = defaultItemObjectRepository().loadNumItemOfOwner(GEAR_DYE_POTION, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            DyePotion* pDyePotion = new DyePotion();

            pDyePotion->setItemID(rows[r].itemID);
            pDyePotion->setObjectID(rows[r].objectID);
            pDyePotion->setItemType(rows[r].itemType);

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            pDyePotion->setNum(rows[r].num);
            pDyePotion->setCreateType((Item::CreateType)rows[r].createType);

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
                if (pInventory->canAddingEx(x, y, pDyePotion)) {
                    pInventory->addItemEx(x, y, pDyePotion);
                } else {
                    processItemBugEx(pCreature, pDyePotion);
                }
                break;

            case STORAGE_GEAR:
                processItemBugEx(pCreature, pDyePotion);
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pDyePotion);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pDyePotion);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pDyePotion);
                else if (pCreature->isOusters())
                    pOusters->addItemToExtraInventorySlot(pDyePotion);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pDyePotion);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pDyePotion);
                } else
                    pStash->insert(x, y, pDyePotion);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pDyePotion);
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

void DyePotionLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<NumZoneObjectRow> rows =
        defaultItemObjectRepository().loadNumItemInZone(GEAR_DYE_POTION, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        DyePotion* pDyePotion = new DyePotion();

        pDyePotion->setItemID(rows[r].itemID);
        pDyePotion->setObjectID(rows[r].objectID);
        pDyePotion->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        pDyePotion->setNum(rows[r].num);
        pDyePotion->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pDyePotion);
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

void DyePotionLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

DyePotionLoader* g_pDyePotionLoader = NULL;
