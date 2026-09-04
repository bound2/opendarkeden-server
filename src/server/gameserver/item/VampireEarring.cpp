//////////////////////////////////////////////////////////////////////////////
// Filename    : VampireEarring.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "VampireEarring.h"

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
VampireEarringInfoManager* g_pVampireEarringInfoManager = NULL;

ItemID_t VampireEarring::m_ItemIDRegistry = 0;
Mutex VampireEarring::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
VampireEarring::VampireEarring()

//: m_ItemType(0), m_Durability(0)
{
    setItemType(0);
    setDurability(0);
    // m_EnchantLevel = 0;
}

VampireEarring::VampireEarring(ItemType_t itemType, const list<OptionType_t>& optionType)

//: m_ItemType(itemType), m_OptionType(optionType), m_Durability(0)
{
    setItemType(itemType);
    setOptionType(optionType);
    // m_EnchantLevel = 0;

    setDurability(computeMaxDurability(this));

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "VampireEarring::VampireEarring() : Invalid item type or option type");
        throw "VampireEarring::VampireEarring() : Invalid item type or optionType";
    }
}


//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void VampireEarring::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y,
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

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().insertGear(GEAR_VAMPIRE_EARRING, m_ItemID, m_ObjectID, getItemType(), ownerID,
                                             (int)storage, storageID, (int)x, (int)y, optionField, getDurability(),
                                             getGrade(), (int)m_CreateType);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void VampireEarring::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_VAMPIRE_EARRING, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void VampireEarring::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateGear(GEAR_VAMPIRE_EARRING, m_ObjectID, getItemType(), ownerID, (int)storage,
                                             storageID, (int)x, (int)y, optionField, getDurability(), getGrade(),
                                             (int)getEnchantLevel(), m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string VampireEarring::toString() const

{
    StringStream msg;

    msg << "VampireEarring(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",Durability:" << (int)getDurability() << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}

/*
//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t VampireEarring::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pVampireEarringInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t VampireEarring::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pVampireEarringInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t VampireEarring::getWeight() const

{
    __BEGIN_TRY

    return g_pVampireEarringInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set armor's Defense Bonus
//--------------------------------------------------------------------------------
Defense_t VampireEarring::getDefenseBonus() const

{
    __BEGIN_TRY

    return g_pVampireEarringInfoManager->getItemInfo(m_ItemType)->getDefenseBonus();

    __END_CATCH
}
Protection_t VampireEarring::getProtectionBonus() const

{
    __BEGIN_TRY

    return g_pVampireEarringInfoManager->getItemInfo(m_ItemType)->getProtectionBonus();

    __END_CATCH
}
*/

//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string VampireEarringInfo::toString() const

{
    StringStream msg;

    msg << "VampireEarringInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ",DefenseBonus:" << m_DefenseBonus
        << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void VampireEarringInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_VAMPIRE_EARRING);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<GearInfoRow> rows = defaultItemObjectRepository().loadGearInfos(GEAR_VAMPIRE_EARRING);

    for (size_t r = 0; r < rows.size(); r++) {
        VampireEarringInfo* pVampireEarringInfo = new VampireEarringInfo();

        pVampireEarringInfo->setItemType(rows[r].itemType);
        pVampireEarringInfo->setName(rows[r].name);
        pVampireEarringInfo->setEName(rows[r].ename);
        pVampireEarringInfo->setPrice(rows[r].price);
        pVampireEarringInfo->setVolumeType(rows[r].volume);
        pVampireEarringInfo->setWeight(rows[r].weight);
        pVampireEarringInfo->setRatio(rows[r].ratio);
        pVampireEarringInfo->setDurability(rows[r].durability);
        pVampireEarringInfo->setDefenseBonus(rows[r].defense);
        pVampireEarringInfo->setProtectionBonus(rows[r].protection);
        pVampireEarringInfo->setReqAbility(rows[r].reqAbility);
        pVampireEarringInfo->setItemLevel(rows[r].itemLevel);
        pVampireEarringInfo->setDefaultOptions(rows[r].defaultOption);
        pVampireEarringInfo->setUpgradeRatio(rows[r].upgradeRatio);
        pVampireEarringInfo->setUpgradeCrashPercent(rows[r].upgradeCrashPercent);
        pVampireEarringInfo->setNextOptionRatio(rows[r].nextOptionRatio);
        pVampireEarringInfo->setNextItemType(rows[r].nextItemType);
        pVampireEarringInfo->setDowngradeRatio(rows[r].downgradeRatio);

        addItemInfo(pVampireEarringInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void VampireEarringLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<GearObjectRow> rows =
        defaultItemObjectRepository().loadGearOfOwner(GEAR_VAMPIRE_EARRING, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            VampireEarring* pVampireEarring = new VampireEarring();

            pVampireEarring->setItemID(rows[r].itemID);
            pVampireEarring->setObjectID(rows[r].objectID);
            pVampireEarring->setItemType(rows[r].itemType);

            if (g_pVampireEarringInfoManager->getItemInfo(pVampireEarring->getItemType())->isUnique())
                pVampireEarring->setUnique();

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pVampireEarring->setOptionType(optionTypes);

            pVampireEarring->setDurability(rows[r].durability);
            pVampireEarring->setGrade(rows[r].grade);
            pVampireEarring->setEnchantLevel(rows[r].enchantLevel);
            pVampireEarring->setCreateType((Item::CreateType)rows[r].createType);

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
                if (pInventory->canAddingEx(x, y, pVampireEarring)) {
                    pInventory->addItemEx(x, y, pVampireEarring);
                } else {
                    processItemBugEx(pCreature, pVampireEarring);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer()) {
                    processItemBugEx(pCreature, pVampireEarring);
                } else if (pCreature->isVampire()) {
                    if (!pVampire->isWear((Vampire::WearPart)x)) {
                        pVampire->wearItem((Vampire::WearPart)x, pVampireEarring);
                    } else {
                        processItemBugEx(pCreature, pVampireEarring);
                    }
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pVampireEarring);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pVampireEarring);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pVampireEarring);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pVampireEarring);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pVampireEarring);
                } else
                    pStash->insert(x, y, pVampireEarring);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pVampireEarring);
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
void VampireEarringLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<GearZoneObjectRow> rows =
        defaultItemObjectRepository().loadGearInZone(GEAR_VAMPIRE_EARRING, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        VampireEarring* pVampireEarring = new VampireEarring();

        pVampireEarring->setItemID(rows[r].itemID);
        pVampireEarring->setObjectID(rows[r].objectID);
        pVampireEarring->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        string optionField = rows[r].optionField;
        list<OptionType_t> optionTypes;
        setOptionTypeFromField(optionTypes, optionField);
        pVampireEarring->setOptionType(optionTypes);

        pVampireEarring->setDurability(rows[r].durability);
        pVampireEarring->setEnchantLevel(rows[r].enchantLevel);
        pVampireEarring->setCreateType((Item::CreateType)rows[r].createType);


        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pVampireEarring);
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
void VampireEarringLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

VampireEarringLoader* g_pVampireEarringLoader = NULL;
