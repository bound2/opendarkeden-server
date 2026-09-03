//////////////////////////////////////////////////////////////////////////////
// Filename    : Dermis.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Dermis.h"

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
DermisInfoManager* g_pDermisInfoManager = NULL;

ItemID_t Dermis::m_ItemIDRegistry = 0;
Mutex Dermis::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
Dermis::Dermis()

{
    setItemType(0);
}

Dermis::Dermis(ItemType_t itemType, const list<OptionType_t>& optionType)

{
    setItemType(itemType);
    setOptionType(optionType);

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "Dermis::Dermis() : Invalid item type or option type");
        throw("Dermis::Dermis() : Invalid item type or optionType");
    }
}


//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void Dermis::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y, ItemID_t itemID)

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

    defaultItemObjectRepository().insertOptionGradeItem(GEAR_DERMIS, m_ItemID, m_ObjectID, getItemType(), ownerID,
                                                        (int)storage, storageID, (int)x, (int)y, optionField,
                                                        getGrade(), (int)m_CreateType);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Dermis::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_DERMIS, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Dermis::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateAmulet(GEAR_DERMIS, m_ObjectID, getItemType(), ownerID, (int)storage, storageID,
                                               (int)x, (int)y, optionField, getGrade(), (int)getEnchantLevel(),
                                               m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string Dermis::toString() const

{
    StringStream msg;

    msg << "Dermis(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string DermisInfo::toString() const

{
    StringStream msg;

    msg << "DermisInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",DefenseBonus:" << m_DefenseBonus << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void DermisInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_DERMIS);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<GearInfoNoDurabilityRow> rows = defaultItemObjectRepository().loadGearInfosNoDurability(GEAR_DERMIS);

    for (size_t r = 0; r < rows.size(); r++) {
        DermisInfo* pDermisInfo = new DermisInfo();

        pDermisInfo->setItemType(rows[r].itemType);
        pDermisInfo->setName(rows[r].name);
        pDermisInfo->setEName(rows[r].ename);
        pDermisInfo->setPrice(rows[r].price);
        pDermisInfo->setVolumeType(rows[r].volume);
        pDermisInfo->setWeight(rows[r].weight);
        pDermisInfo->setRatio(rows[r].ratio);
        pDermisInfo->setDefenseBonus(rows[r].defense);
        pDermisInfo->setProtectionBonus(rows[r].protection);
        pDermisInfo->setReqAbility(rows[r].reqAbility);
        pDermisInfo->setItemLevel(rows[r].itemLevel);
        pDermisInfo->setDefaultOptions(rows[r].defaultOption);
        pDermisInfo->setUpgradeRatio(rows[r].upgradeRatio);
        pDermisInfo->setUpgradeCrashPercent(rows[r].upgradeCrashPercent);
        pDermisInfo->setNextOptionRatio(rows[r].nextOptionRatio);
        pDermisInfo->setNextItemType(rows[r].nextItemType);
        pDermisInfo->setDowngradeRatio(rows[r].downgradeRatio);

        addItemInfo(pDermisInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void DermisLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<OptionGradeObjectRow> rows =
        defaultItemObjectRepository().loadOptionGradeOfOwner(GEAR_DERMIS, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            Dermis* pDermis = new Dermis();

            pDermis->setItemID(rows[r].itemID);
            pDermis->setObjectID(rows[r].objectID);
            pDermis->setItemType(rows[r].itemType);

            if (g_pDermisInfoManager->getItemInfo(pDermis->getItemType())->isUnique())
                pDermis->setUnique();

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pDermis->setOptionType(optionTypes);

            pDermis->setGrade(rows[r].grade);
            pDermis->setEnchantLevel(rows[r].enchantLevel);
            pDermis->setCreateType((Item::CreateType)rows[r].createType);

            Inventory* pInventory = NULL;
            Slayer* pSlayer = NULL;
            Vampire* pVampire = NULL;
            Motorcycle* pMotorcycle = NULL;
            Inventory* pMotorInventory = NULL;
            Stash* pStash = NULL;

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
                if (pInventory->canAddingEx(x, y, pDermis)) {
                    pInventory->addItemEx(x, y, pDermis);
                } else {
                    processItemBugEx(pCreature, pDermis);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer()) {
                    processItemBugEx(pCreature, pDermis);
                } else if (pCreature->isVampire()) {
                    if (!pVampire->isWear((Vampire::WearPart)x)) {
                        pVampire->wearItem((Vampire::WearPart)x, pDermis);
                    } else {
                        processItemBugEx(pCreature, pDermis);
                    }
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pDermis);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pDermis);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pDermis);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pDermis);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pDermis);
                } else
                    pStash->insert(x, y, pDermis);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pDermis);
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
void DermisLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to inventory
//--------------------------------------------------------------------------------
void DermisLoader::load(StorageID_t storageID, Inventory* pInventory)

    {__BEGIN_TRY

         __END_CATCH}

DermisLoader* g_pDermisLoader = NULL;
