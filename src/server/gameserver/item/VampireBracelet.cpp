//////////////////////////////////////////////////////////////////////////////
// Filename    : VampireBracelet.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "VampireBracelet.h"

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
VampireBraceletInfoManager* g_pVampireBraceletInfoManager = NULL;

ItemID_t VampireBracelet::m_ItemIDRegistry = 0;
Mutex VampireBracelet::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
VampireBracelet::VampireBracelet()

//: m_ItemType(0), m_Durability(0)
{
    setItemType(0);
    setDurability(0);
    // m_EnchantLevel = 0;
}

VampireBracelet::VampireBracelet(ItemType_t itemType, const list<OptionType_t>& optionType)

//: m_ItemType(itemType), m_OptionType(optionType), m_Durability(0)
{
    setItemType(itemType);
    setOptionType(optionType);
    // m_EnchantLevel = 0;

    setDurability(computeMaxDurability(this));

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "VampireBracelet::VampireBracelet() : Invalid item type or option type");
        throw("VampireBracelet::VampireBracelet() : Invalid item type or optionType");
    }
}


//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void VampireBracelet::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y,
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

    defaultItemObjectRepository().insertGear(GEAR_VAMPIRE_BRACELET, m_ItemID, m_ObjectID, getItemType(), ownerID,
                                             (int)storage, storageID, (int)x, (int)y, optionField, getDurability(),
                                             getGrade(), (int)m_CreateType);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void VampireBracelet::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_VAMPIRE_BRACELET, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void VampireBracelet::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateGear(GEAR_VAMPIRE_BRACELET, m_ObjectID, getItemType(), ownerID, (int)storage,
                                             storageID, (int)x, (int)y, optionField, getDurability(), getGrade(),
                                             (int)getEnchantLevel(), m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string VampireBracelet::toString() const

{
    StringStream msg;

    msg << "VampireBracelet(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",Durability:" << (int)getDurability() << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}


/*//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t VampireBracelet::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pVampireBraceletInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t VampireBracelet::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pVampireBraceletInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t VampireBracelet::getWeight() const

{
    __BEGIN_TRY

    return g_pVampireBraceletInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set armor's Defense Bonus
//--------------------------------------------------------------------------------
Defense_t VampireBracelet::getDefenseBonus() const

{
    __BEGIN_TRY

    return g_pVampireBraceletInfoManager->getItemInfo(m_ItemType)->getDefenseBonus();

    __END_CATCH
}
Protection_t VampireBracelet::getProtectionBonus() const

{
    __BEGIN_TRY

    return g_pVampireBraceletInfoManager->getItemInfo(m_ItemType)->getProtectionBonus();

    __END_CATCH
}
*/

//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string VampireBraceletInfo::toString() const

{
    StringStream msg;

    msg << "VampireBraceletInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ",DefenseBonus:" << m_DefenseBonus
        << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void VampireBraceletInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_VAMPIRE_BRACELET);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<GearInfoRow> rows = defaultItemObjectRepository().loadGearInfos(GEAR_VAMPIRE_BRACELET);

    for (size_t r = 0; r < rows.size(); r++) {
        VampireBraceletInfo* pVampireBraceletInfo = new VampireBraceletInfo();

        pVampireBraceletInfo->setItemType(rows[r].itemType);
        pVampireBraceletInfo->setName(rows[r].name);
        pVampireBraceletInfo->setEName(rows[r].ename);
        pVampireBraceletInfo->setPrice(rows[r].price);
        pVampireBraceletInfo->setVolumeType(rows[r].volume);
        pVampireBraceletInfo->setWeight(rows[r].weight);
        pVampireBraceletInfo->setRatio(rows[r].ratio);
        pVampireBraceletInfo->setDurability(rows[r].durability);
        pVampireBraceletInfo->setDefenseBonus(rows[r].defense);
        pVampireBraceletInfo->setProtectionBonus(rows[r].protection);
        pVampireBraceletInfo->setReqAbility(rows[r].reqAbility);
        pVampireBraceletInfo->setItemLevel(rows[r].itemLevel);
        pVampireBraceletInfo->setDefaultOptions(rows[r].defaultOption);
        pVampireBraceletInfo->setUpgradeRatio(rows[r].upgradeRatio);
        pVampireBraceletInfo->setUpgradeCrashPercent(rows[r].upgradeCrashPercent);
        pVampireBraceletInfo->setNextOptionRatio(rows[r].nextOptionRatio);
        pVampireBraceletInfo->setNextItemType(rows[r].nextItemType);
        pVampireBraceletInfo->setDowngradeRatio(rows[r].downgradeRatio);

        addItemInfo(pVampireBraceletInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void VampireBraceletLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<GearObjectRow> rows =
        defaultItemObjectRepository().loadGearOfOwner(GEAR_VAMPIRE_BRACELET, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            VampireBracelet* pVampireBracelet = new VampireBracelet();

            pVampireBracelet->setItemID(rows[r].itemID);
            pVampireBracelet->setObjectID(rows[r].objectID);
            pVampireBracelet->setItemType(rows[r].itemType);

            if (g_pVampireBraceletInfoManager->getItemInfo(pVampireBracelet->getItemType())->isUnique())
                pVampireBracelet->setUnique();

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pVampireBracelet->setOptionType(optionTypes);

            pVampireBracelet->setDurability(rows[r].durability);
            pVampireBracelet->setGrade(rows[r].grade);
            pVampireBracelet->setEnchantLevel(rows[r].enchantLevel);
            pVampireBracelet->setCreateType((Item::CreateType)rows[r].createType);

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
                if (pInventory->canAddingEx(x, y, pVampireBracelet)) {
                    pInventory->addItemEx(x, y, pVampireBracelet);
                } else {
                    processItemBugEx(pCreature, pVampireBracelet);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer()) {
                    processItemBugEx(pCreature, pVampireBracelet);
                } else if (pCreature->isVampire()) {
                    if (!pVampire->isWear((Vampire::WearPart)x)) {
                        pVampire->wearItem((Vampire::WearPart)x, pVampireBracelet);
                    } else {
                        processItemBugEx(pCreature, pVampireBracelet);
                    }
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pVampireBracelet);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pVampireBracelet);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pVampireBracelet);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pVampireBracelet);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pVampireBracelet);
                } else
                    pStash->insert(x, y, pVampireBracelet);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pVampireBracelet);
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
void VampireBraceletLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<GearZoneObjectRow> rows =
        defaultItemObjectRepository().loadGearInZone(GEAR_VAMPIRE_BRACELET, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        VampireBracelet* pVampireBracelet = new VampireBracelet();

        pVampireBracelet->setItemID(rows[r].itemID);
        pVampireBracelet->setObjectID(rows[r].objectID);
        pVampireBracelet->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        string optionField = rows[r].optionField;
        list<OptionType_t> optionTypes;
        setOptionTypeFromField(optionTypes, optionField);
        pVampireBracelet->setOptionType(optionTypes);

        pVampireBracelet->setDurability(rows[r].durability);
        pVampireBracelet->setEnchantLevel(rows[r].enchantLevel);
        pVampireBracelet->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pVampireBracelet);
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
void VampireBraceletLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

VampireBraceletLoader* g_pVampireBraceletLoader = NULL;
