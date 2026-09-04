//////////////////////////////////////////////////////////////////////////////
// Filename    : ShoulderArmor.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "ShoulderArmor.h"

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
ShoulderArmorInfoManager* g_pShoulderArmorInfoManager = NULL;

ItemID_t ShoulderArmor::m_ItemIDRegistry = 0;
Mutex ShoulderArmor::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
ShoulderArmor::ShoulderArmor()

{
    setItemType(0);
    setDurability(0);
}

ShoulderArmor::ShoulderArmor(ItemType_t itemType, const list<OptionType_t>& optionType)

{
    setItemType(itemType);
    setOptionType(optionType);
    setDurability(computeMaxDurability(this));

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "ShoulderArmor::ShoulderArmor() : Invalid item type or option type");
        throw "ShoulderArmor::ShoulderArmor() : Invalid item type or optionType";
    }
}


//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void ShoulderArmor::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y,
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

    defaultItemObjectRepository().insertGear(GEAR_SHOULDER_ARMOR, m_ItemID, m_ObjectID, getItemType(), ownerID,
                                             (int)storage, storageID, (int)x, (int)y, optionField, getDurability(),
                                             (int)getGrade(), (int)m_CreateType);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void ShoulderArmor::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_SHOULDER_ARMOR, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void ShoulderArmor::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateGear(GEAR_SHOULDER_ARMOR, m_ObjectID, getItemType(), ownerID, (int)storage,
                                             storageID, (int)x, (int)y, optionField, getDurability(), (int)getGrade(),
                                             (int)getEnchantLevel(), m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string ShoulderArmor::toString() const

{
    StringStream msg;

    msg << "ShoulderArmor(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",Durability:" << (int)getDurability() << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string ShoulderArmorInfo::toString() const

{
    StringStream msg;

    msg << "ShoulderArmorInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ",DefenseBonus:" << m_DefenseBonus
        << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void ShoulderArmorInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_SHOULDER_ARMOR);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<GearInfoRow> rows = defaultItemObjectRepository().loadGearInfos(GEAR_SHOULDER_ARMOR);

    for (size_t r = 0; r < rows.size(); r++) {
        ShoulderArmorInfo* pShoulderArmorInfo = new ShoulderArmorInfo();

        pShoulderArmorInfo->setItemType(rows[r].itemType);
        pShoulderArmorInfo->setName(rows[r].name);
        pShoulderArmorInfo->setEName(rows[r].ename);
        pShoulderArmorInfo->setPrice(rows[r].price);
        pShoulderArmorInfo->setVolumeType(rows[r].volume);
        pShoulderArmorInfo->setWeight(rows[r].weight);
        pShoulderArmorInfo->setRatio(rows[r].ratio);
        pShoulderArmorInfo->setDurability(rows[r].durability);
        pShoulderArmorInfo->setDefenseBonus(rows[r].defense);
        pShoulderArmorInfo->setProtectionBonus(rows[r].protection);
        pShoulderArmorInfo->setReqAbility(rows[r].reqAbility);
        pShoulderArmorInfo->setItemLevel(rows[r].itemLevel);
        pShoulderArmorInfo->setDefaultOptions(rows[r].defaultOption);
        pShoulderArmorInfo->setUpgradeRatio(rows[r].upgradeRatio);
        pShoulderArmorInfo->setUpgradeCrashPercent(rows[r].upgradeCrashPercent);
        pShoulderArmorInfo->setNextOptionRatio(rows[r].nextOptionRatio);
        pShoulderArmorInfo->setNextItemType(rows[r].nextItemType);
        pShoulderArmorInfo->setDowngradeRatio(rows[r].downgradeRatio);

        addItemInfo(pShoulderArmorInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void ShoulderArmorLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<GearObjectRow> rows =
        defaultItemObjectRepository().loadGearOfOwner(GEAR_SHOULDER_ARMOR, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            ShoulderArmor* pShoulderArmor = new ShoulderArmor();

            pShoulderArmor->setItemID(rows[r].itemID);
            pShoulderArmor->setObjectID(rows[r].objectID);
            pShoulderArmor->setItemType(rows[r].itemType);

            if (g_pShoulderArmorInfoManager->getItemInfo(pShoulderArmor->getItemType())->isUnique())
                pShoulderArmor->setUnique();

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pShoulderArmor->setOptionType(optionTypes);

            pShoulderArmor->setDurability(rows[r].durability);
            pShoulderArmor->setGrade(rows[r].grade);
            pShoulderArmor->setEnchantLevel(rows[r].enchantLevel);
            pShoulderArmor->setCreateType((Item::CreateType)rows[r].createType);

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
                if (pInventory->canAddingEx(x, y, pShoulderArmor)) {
                    pInventory->addItemEx(x, y, pShoulderArmor);
                } else {
                    processItemBugEx(pCreature, pShoulderArmor);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer()) {
                    if (!pSlayer->isWear((Slayer::WearPart)x)) {
                        pSlayer->wearItem((Slayer::WearPart)x, pShoulderArmor);
                    } else {
                        processItemBugEx(pCreature, pShoulderArmor);
                    }
                } else if (pCreature->isVampire()) {
                    processItemBugEx(pCreature, pShoulderArmor);
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pShoulderArmor);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pShoulderArmor);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pShoulderArmor);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pShoulderArmor);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pShoulderArmor);
                } else
                    pStash->insert(x, y, pShoulderArmor);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pShoulderArmor);
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
void ShoulderArmorLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to inventory
//--------------------------------------------------------------------------------
void ShoulderArmorLoader::load(StorageID_t storageID, Inventory* pInventory)

    {__BEGIN_TRY

         __END_CATCH}

ShoulderArmorLoader* g_pShoulderArmorLoader = NULL;
