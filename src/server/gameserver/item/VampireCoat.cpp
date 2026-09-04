//////////////////////////////////////////////////////////////////////////////
// Filename    : VampireCoat.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "VampireCoat.h"

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
VampireCoatInfoManager* g_pVampireCoatInfoManager = NULL;

ItemID_t VampireCoat::m_ItemIDRegistry = 0;
Mutex VampireCoat::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
VampireCoat::VampireCoat()

//: m_ItemType(0), m_Durability(0)
{
    setItemType(0);
    setDurability(0);
    // m_EnchantLevel = 0;
}

VampireCoat::VampireCoat(ItemType_t itemType, const list<OptionType_t>& optionType)

//: m_ItemType(itemType), m_OptionType(optionType), m_Durability(0)
{
    setItemType(itemType);
    setOptionType(optionType);
    // m_EnchantLevel = 0;

    setDurability(computeMaxDurability(this));

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "VampireCoat::VampireCoat() : Invalid item type or option type");
        throw "VampireCoat::VampireCoat() : Invalid item type or optionType";
    }
}


//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void VampireCoat::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y, ItemID_t itemID)

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
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().insertGear(GEAR_VAMPIRE_COAT, m_ItemID, m_ObjectID, getItemType(), ownerID,
                                             (int)storage, storageID, (int)x, (int)y, optionField, getDurability(),
                                             getGrade(), (int)m_CreateType);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void VampireCoat::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_VAMPIRE_COAT, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void VampireCoat::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateGear(GEAR_VAMPIRE_COAT, m_ObjectID, getItemType(), ownerID, (int)storage,
                                             storageID, (int)x, (int)y, optionField, getDurability(), getGrade(),
                                             (int)getEnchantLevel(), m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string VampireCoat::toString() const

{
    StringStream msg;

    msg << "VampireCoat(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",Durability:" << (int)getDurability() << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}

/*
//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t VampireCoat::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pVampireCoatInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t VampireCoat::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pVampireCoatInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t VampireCoat::getWeight() const

{
    __BEGIN_TRY

    return g_pVampireCoatInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set armor's Defense Bonus
//--------------------------------------------------------------------------------
Defense_t VampireCoat::getDefenseBonus() const

{
    __BEGIN_TRY

    return g_pVampireCoatInfoManager->getItemInfo(m_ItemType)->getDefenseBonus();

    __END_CATCH
}
Protection_t VampireCoat::getProtectionBonus() const

{
    __BEGIN_TRY

    return g_pVampireCoatInfoManager->getItemInfo(m_ItemType)->getProtectionBonus();

    __END_CATCH
}
*/

//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string VampireCoatInfo::toString() const

{
    StringStream msg;

    msg << "VampireCoatInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ",DefenseBonus:" << m_DefenseBonus
        << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void VampireCoatInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_VAMPIRE_COAT);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<GearInfoNoRatioRow> rows = defaultItemObjectRepository().loadGearInfosNoRatio(GEAR_VAMPIRE_COAT);

    for (size_t r = 0; r < rows.size(); r++) {
        VampireCoatInfo* pVampireCoatInfo = new VampireCoatInfo();

        pVampireCoatInfo->setItemType(rows[r].itemType);
        pVampireCoatInfo->setName(rows[r].name);
        pVampireCoatInfo->setEName(rows[r].ename);
        pVampireCoatInfo->setPrice(rows[r].price);
        pVampireCoatInfo->setVolumeType(rows[r].volume);
        pVampireCoatInfo->setWeight(rows[r].weight);
        pVampireCoatInfo->setRatio(rows[r].ratio);
        pVampireCoatInfo->setDurability(rows[r].durability);
        pVampireCoatInfo->setDefenseBonus(rows[r].defense);
        pVampireCoatInfo->setProtectionBonus(rows[r].protection);
        pVampireCoatInfo->setReqAbility(rows[r].reqAbility);
        pVampireCoatInfo->setItemLevel(rows[r].itemLevel);
        pVampireCoatInfo->setDefaultOptions(rows[r].defaultOption);
        pVampireCoatInfo->setUpgradeCrashPercent(rows[r].upgradeCrashPercent);
        pVampireCoatInfo->setNextOptionRatio(rows[r].nextOptionRatio);
        pVampireCoatInfo->setNextItemType(rows[r].nextItemType);

        addItemInfo(pVampireCoatInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void VampireCoatLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<GearObjectRow> rows = defaultItemObjectRepository().loadGearOfOwner(GEAR_VAMPIRE_COAT, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            VampireCoat* pVampireCoat = new VampireCoat();

            pVampireCoat->setItemID(rows[r].itemID);
            pVampireCoat->setObjectID(rows[r].objectID);
            pVampireCoat->setItemType(rows[r].itemType);

            if (g_pVampireCoatInfoManager->getItemInfo(pVampireCoat->getItemType())->isUnique())
                pVampireCoat->setUnique();

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;


            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pVampireCoat->setOptionType(optionTypes);

            pVampireCoat->setDurability(rows[r].durability);
            pVampireCoat->setGrade(rows[r].grade);
            pVampireCoat->setEnchantLevel(rows[r].enchantLevel);
            pVampireCoat->setCreateType((Item::CreateType)rows[r].createType);

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
                if (pInventory->canAddingEx(x, y, pVampireCoat)) {
                    pInventory->addItemEx(x, y, pVampireCoat);
                } else {
                    processItemBugEx(pCreature, pVampireCoat);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer()) {
                    processItemBugEx(pCreature, pVampireCoat);
                } else if (pCreature->isVampire()) {
                    if (!pVampire->isWear((Vampire::WearPart)x)) {
                        pVampire->wearItem((Vampire::WearPart)x, pVampireCoat);
                    } else {
                        processItemBugEx(pCreature, pVampireCoat);
                    }
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pVampireCoat);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pVampireCoat);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pVampireCoat);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pVampireCoat);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pVampireCoat);
                } else
                    pStash->insert(x, y, pVampireCoat);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pVampireCoat);
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
void VampireCoatLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<GearZoneObjectRow> rows =
        defaultItemObjectRepository().loadGearInZone(GEAR_VAMPIRE_COAT, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        VampireCoat* pVampireCoat = new VampireCoat();

        pVampireCoat->setItemID(rows[r].itemID);
        pVampireCoat->setObjectID(rows[r].objectID);
        pVampireCoat->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        string optionField = rows[r].optionField;
        list<OptionType_t> optionTypes;
        setOptionTypeFromField(optionTypes, optionField);
        pVampireCoat->setOptionType(optionTypes);

        pVampireCoat->setDurability(rows[r].durability);
        pVampireCoat->setEnchantLevel(rows[r].enchantLevel);
        pVampireCoat->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pVampireCoat);
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
void VampireCoatLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

VampireCoatLoader* g_pVampireCoatLoader = NULL;
