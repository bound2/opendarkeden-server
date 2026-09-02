//////////////////////////////////////////////////////////////////////////////
// Filename    : Shield.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Shield.h"

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
ShieldInfoManager* g_pShieldInfoManager = NULL;

ItemID_t Shield::m_ItemIDRegistry = 0;
Mutex Shield::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
Shield::Shield()

//: m_ItemType(0), m_Durability(0)
{
    setItemType(0);
    setDurability(0);
    // m_EnchantLevel = 0;
}

Shield::Shield(ItemType_t itemType, const list<OptionType_t>& optionType)

//: m_ItemType(itemType), m_OptionType(optionType), m_Durability(0)
{
    setItemType(itemType);
    setOptionType(optionType);

    // m_EnchantLevel = 0;

    setDurability(computeMaxDurability(this));

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "Shield::Shield() : Invalid item type or option type");
        throw("Shield::Shield() : Invalid item type or optionType");
    }
}


//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void Shield::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y, ItemID_t itemID)

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

    defaultItemObjectRepository().insertGear(GEAR_SHIELD, m_ItemID, m_ObjectID, getItemType(), ownerID, (int)storage,
                                             storageID, (int)x, (int)y, optionField, getDurability(), getGrade(),
                                             (int)m_CreateType);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Shield::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_SHIELD, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Shield::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateGear(GEAR_SHIELD, m_ObjectID, getItemType(), ownerID, (int)storage, storageID,
                                             (int)x, (int)y, optionField, getDurability(), getGrade(),
                                             (int)getEnchantLevel(), m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string Shield::toString() const

{
    StringStream msg;

    msg << "Shield(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",Durability:" << (int)getDurability() << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}


/*//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t Shield::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pShieldInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t Shield::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pShieldInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t Shield::getWeight() const

{
    __BEGIN_TRY

    return g_pShieldInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set armor's Defense Bonus
//--------------------------------------------------------------------------------
Defense_t Shield::getDefenseBonus() const

{
    __BEGIN_TRY

    return g_pShieldInfoManager->getItemInfo(m_ItemType)->getDefenseBonus();

    __END_CATCH
}
Protection_t Shield::getProtectionBonus() const

{
    __BEGIN_TRY

    return g_pShieldInfoManager->getItemInfo(m_ItemType)->getProtectionBonus();

    __END_CATCH
}
*/

//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string ShieldInfo::toString() const

{
    StringStream msg;

    msg << "ShieldInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ",DefenseBonus:" << m_DefenseBonus
        << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void ShieldInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_SHIELD);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<GearInfoRow> rows = defaultItemObjectRepository().loadGearInfos(GEAR_SHIELD);

    for (size_t r = 0; r < rows.size(); r++) {
        ShieldInfo* pShieldInfo = new ShieldInfo();

        pShieldInfo->setItemType(rows[r].itemType);
        pShieldInfo->setName(rows[r].name);
        pShieldInfo->setEName(rows[r].ename);
        pShieldInfo->setPrice(rows[r].price);
        pShieldInfo->setVolumeType(rows[r].volume);
        pShieldInfo->setWeight(rows[r].weight);
        pShieldInfo->setRatio(rows[r].ratio);
        pShieldInfo->setDurability(rows[r].durability);
        pShieldInfo->setDefenseBonus(rows[r].defense);
        pShieldInfo->setProtectionBonus(rows[r].protection);
        pShieldInfo->setReqAbility(rows[r].reqAbility);
        pShieldInfo->setItemLevel(rows[r].itemLevel);
        pShieldInfo->setDefaultOptions(rows[r].defaultOption);
        pShieldInfo->setUpgradeRatio(rows[r].upgradeRatio);
        pShieldInfo->setUpgradeCrashPercent(rows[r].upgradeCrashPercent);
        pShieldInfo->setNextOptionRatio(rows[r].nextOptionRatio);
        pShieldInfo->setNextItemType(rows[r].nextItemType);
        pShieldInfo->setDowngradeRatio(rows[r].downgradeRatio);

        addItemInfo(pShieldInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void ShieldLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<GearObjectRow> rows = defaultItemObjectRepository().loadGearOfOwner(GEAR_SHIELD, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            Shield* pShield = new Shield();

            pShield->setItemID(rows[r].itemID);
            pShield->setObjectID(rows[r].objectID);
            pShield->setItemType(rows[r].itemType);

            if (g_pShieldInfoManager->getItemInfo(pShield->getItemType())->isUnique())
                pShield->setUnique();

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pShield->setOptionType(optionTypes);

            pShield->setDurability(rows[r].durability);
            pShield->setGrade(rows[r].grade);
            pShield->setEnchantLevel(rows[r].enchantLevel);
            pShield->setCreateType((Item::CreateType)rows[r].createType);

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
                if (pInventory->canAddingEx(x, y, pShield)) {
                    pInventory->addItemEx(x, y, pShield);
                } else {
                    processItemBugEx(pCreature, pShield);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer()) {
                    if (!pSlayer->isWear((Slayer::WearPart)x)) {
                        pSlayer->wearItem((Slayer::WearPart)x, pShield);
                    } else {
                        processItemBugEx(pCreature, pShield);
                    }
                } else if (pCreature->isVampire()) {
                    processItemBugEx(pCreature, pShield);
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pShield);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pShield);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pShield);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pShield);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pShield);
                } else
                    pStash->insert(x, y, pShield);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pShield);
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
void ShieldLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<GearZoneObjectRow> rows =
        defaultItemObjectRepository().loadGearInZone(GEAR_SHIELD, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        Shield* pShield = new Shield();

        pShield->setItemID(rows[r].itemID);
        pShield->setObjectID(rows[r].objectID);
        pShield->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        string optionField = rows[r].optionField;
        list<OptionType_t> optionTypes;
        setOptionTypeFromField(optionTypes, optionField);
        pShield->setOptionType(optionTypes);

        pShield->setDurability(rows[r].durability);
        pShield->setEnchantLevel(rows[r].enchantLevel);
        pShield->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pShield);
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
void ShieldLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

ShieldLoader* g_pShieldLoader = NULL;
