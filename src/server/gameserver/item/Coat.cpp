//////////////////////////////////////////////////////////////////////////////
// Filename    : Coat.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Coat.h"

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
CoatInfoManager* g_pCoatInfoManager = NULL;

ItemID_t Coat::m_ItemIDRegistry = 0;
Mutex Coat::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
Coat::Coat()

//: m_ItemType(0), m_Durability(0)
{
    setItemType(0);
    setDurability(0);
    //	m_EnchantLevel = 0;
}

Coat::Coat(ItemType_t itemType, const list<OptionType_t>& optionType)

//: m_ItemType(itemType), m_OptionType(optionType), m_Durability(0)
{
    setItemType(itemType);
    setOptionType(optionType);
    //	m_EnchantLevel = 0;
    setDurability(computeMaxDurability(this));

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "Coat::Coat() : Invalid item type or option type");
        throw("Coat::Coat() : Invalid item type or optionType");
    }
}


//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void Coat::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y, ItemID_t itemID)

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

    defaultItemObjectRepository().insertGear(GEAR_COAT, m_ItemID, m_ObjectID, getItemType(), ownerID, (int)storage,
                                             storageID, (int)x, (int)y, optionField, getDurability(), (int)getGrade(),
                                             (int)m_CreateType);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Coat::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_COAT, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Coat::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateGear(GEAR_COAT, m_ObjectID, getItemType(), ownerID, (int)storage, storageID,
                                             (int)x, (int)y, optionField, getDurability(), (int)getGrade(),
                                             (int)getEnchantLevel(), m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string Coat::toString() const

{
    StringStream msg;

    msg << "Coat(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",Durability:" << (int)getDurability() << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}


/*//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t Coat::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pCoatInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t Coat::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pCoatInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t Coat::getWeight() const

{
    __BEGIN_TRY

    return g_pCoatInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set armor's Defense Bonus
//--------------------------------------------------------------------------------
Defense_t Coat::getDefenseBonus() const

{
    __BEGIN_TRY

    return g_pCoatInfoManager->getItemInfo(m_ItemType)->getDefenseBonus();

    __END_CATCH
}
Protection_t Coat::getProtectionBonus() const

{
    __BEGIN_TRY

    return g_pCoatInfoManager->getItemInfo(m_ItemType)->getProtectionBonus();

    __END_CATCH
}
*/

//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string CoatInfo::toString() const

{
    StringStream msg;

    msg << "CoatInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ",DefenseBonus:" << m_DefenseBonus
        << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void CoatInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_COAT);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<GearInfoRow> rows = defaultItemObjectRepository().loadGearInfos(GEAR_COAT);

    for (size_t r = 0; r < rows.size(); r++) {
        CoatInfo* pCoatInfo = new CoatInfo();

        pCoatInfo->setItemType(rows[r].itemType);
        pCoatInfo->setName(rows[r].name);
        pCoatInfo->setEName(rows[r].ename);
        pCoatInfo->setPrice(rows[r].price);
        pCoatInfo->setVolumeType(rows[r].volume);
        pCoatInfo->setWeight(rows[r].weight);
        pCoatInfo->setRatio(rows[r].ratio);
        pCoatInfo->setDurability(rows[r].durability);
        pCoatInfo->setDefenseBonus(rows[r].defense);
        pCoatInfo->setProtectionBonus(rows[r].protection);
        pCoatInfo->setReqAbility(rows[r].reqAbility);
        pCoatInfo->setItemLevel(rows[r].itemLevel);
        pCoatInfo->setDefaultOptions(rows[r].defaultOption);
        pCoatInfo->setUpgradeRatio(rows[r].upgradeRatio);
        pCoatInfo->setUpgradeCrashPercent(rows[r].upgradeCrashPercent);
        pCoatInfo->setNextOptionRatio(rows[r].nextOptionRatio);
        pCoatInfo->setNextItemType(rows[r].nextItemType);
        pCoatInfo->setDowngradeRatio(rows[r].downgradeRatio);

        addItemInfo(pCoatInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void CoatLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<GearObjectRow> rows = defaultItemObjectRepository().loadGearOfOwner(GEAR_COAT, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            Coat* pCoat = new Coat();

            pCoat->setItemID(rows[r].itemID);
            pCoat->setObjectID(rows[r].objectID);
            pCoat->setItemType(rows[r].itemType);

            if (g_pCoatInfoManager->getItemInfo(pCoat->getItemType())->isUnique())
                pCoat->setUnique();

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pCoat->setOptionType(optionTypes);

            pCoat->setDurability(rows[r].durability);
            pCoat->setGrade(rows[r].grade);
            pCoat->setEnchantLevel(rows[r].enchantLevel);
            pCoat->setCreateType((Item::CreateType)rows[r].createType);

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
                if (pInventory->canAddingEx(x, y, pCoat)) {
                    pInventory->addItemEx(x, y, pCoat);
                } else {
                    processItemBugEx(pCreature, pCoat);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer()) {
                    if (!pSlayer->isWear((Slayer::WearPart)x)) {
                        pSlayer->wearItem((Slayer::WearPart)x, pCoat);
                    } else {
                        processItemBugEx(pCreature, pCoat);
                    }
                } else if (pCreature->isVampire()) {
                    processItemBugEx(pCreature, pCoat);
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pCoat);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pCoat);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pCoat);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pCoat);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pCoat);
                } else
                    pStash->insert(x, y, pCoat);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pCoat);
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
void CoatLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<GearZoneObjectRow> rows =
        defaultItemObjectRepository().loadGearInZone(GEAR_COAT, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        Coat* pCoat = new Coat();

        pCoat->setItemID(rows[r].itemID);
        pCoat->setObjectID(rows[r].objectID);
        pCoat->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        string optionField = rows[r].optionField;
        list<OptionType_t> optionTypes;
        setOptionTypeFromField(optionTypes, optionField);
        pCoat->setOptionType(optionTypes);

        pCoat->setDurability(rows[r].durability);
        pCoat->setEnchantLevel(rows[r].enchantLevel);
        pCoat->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pCoat);
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
void CoatLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

CoatLoader* g_pCoatLoader = NULL;
