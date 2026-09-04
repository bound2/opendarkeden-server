//////////////////////////////////////////////////////////////////////////////
// Filename    : OustersBoots.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "OustersBoots.h"

#include "Belt.h"
#include "DB.h"
#include "ItemInfoManager.h"
#include "ItemUtil.h"
#include "Motorcycle.h"
#include "Ousters.h"
#include "Slayer.h"
#include "Stash.h"
#include "Vampire.h"
#include "repository/ItemObjectRepository.h"

// global variable declaration
OustersBootsInfoManager* g_pOustersBootsInfoManager = NULL;

ItemID_t OustersBoots::m_ItemIDRegistry = 0;
Mutex OustersBoots::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
OustersBoots::OustersBoots()

//: m_ItemType(0), m_Durability(0)
{
    setItemType(0);
    setDurability(0);
    //	m_EnchantLevel = 0;
}

OustersBoots::OustersBoots(ItemType_t itemType, const list<OptionType_t>& optionType)

//: m_ItemType(itemType), m_OptionType(optionType), m_Durability(0)
{
    setItemType(itemType);
    setOptionType(optionType);
    //	m_EnchantLevel = 0;

    setDurability(computeMaxDurability(this));

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "OustersBoots::OustersBoots() : Invalid item type or option type");
        throw "OustersBoots::OustersBoots() : Invalid item type or optionType";
    }
}


//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void OustersBoots::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y,
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

    defaultItemObjectRepository().insertGear(GEAR_OUSTERS_BOOTS, m_ItemID, m_ObjectID, getItemType(), ownerID,
                                             (int)storage, storageID, (int)x, (int)y, optionField, getDurability(),
                                             getGrade(), (int)m_CreateType);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void OustersBoots::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_OUSTERS_BOOTS, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void OustersBoots::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateGear(GEAR_OUSTERS_BOOTS, m_ObjectID, getItemType(), ownerID, (int)storage,
                                             storageID, (int)x, (int)y, optionField, getDurability(), getGrade(),
                                             (int)getEnchantLevel(), m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string OustersBoots::toString() const

{
    StringStream msg;

    msg << "OustersBoots(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",Durability:" << (int)getDurability() << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}


/*//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t OustersBoots::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pOustersBootsInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t OustersBoots::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pOustersBootsInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t OustersBoots::getWeight() const

{
    __BEGIN_TRY

    return g_pOustersBootsInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set armor's Defense Bonus
//--------------------------------------------------------------------------------
Defense_t OustersBoots::getDefenseBonus() const

{
    __BEGIN_TRY

    return g_pOustersBootsInfoManager->getItemInfo(m_ItemType)->getDefenseBonus();

    __END_CATCH
}
Protection_t OustersBoots::getProtectionBonus() const

{
    __BEGIN_TRY

    return g_pOustersBootsInfoManager->getItemInfo(m_ItemType)->getProtectionBonus();

    __END_CATCH
}
*/

//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string OustersBootsInfo::toString() const

{
    StringStream msg;

    msg << "OustersBootsInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ",DefenseBonus:" << m_DefenseBonus
        << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void OustersBootsInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_OUSTERS_BOOTS);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<GearInfoRow> rows = defaultItemObjectRepository().loadGearInfos(GEAR_OUSTERS_BOOTS);

    for (size_t r = 0; r < rows.size(); r++) {
        OustersBootsInfo* pOustersBootsInfo = new OustersBootsInfo();

        pOustersBootsInfo->setItemType(rows[r].itemType);
        pOustersBootsInfo->setName(rows[r].name);
        pOustersBootsInfo->setEName(rows[r].ename);
        pOustersBootsInfo->setPrice(rows[r].price);
        pOustersBootsInfo->setVolumeType(rows[r].volume);
        pOustersBootsInfo->setWeight(rows[r].weight);
        pOustersBootsInfo->setRatio(rows[r].ratio);
        pOustersBootsInfo->setDurability(rows[r].durability);
        pOustersBootsInfo->setDefenseBonus(rows[r].defense);
        pOustersBootsInfo->setProtectionBonus(rows[r].protection);
        pOustersBootsInfo->setReqAbility(rows[r].reqAbility);
        pOustersBootsInfo->setItemLevel(rows[r].itemLevel);
        pOustersBootsInfo->setDefaultOptions(rows[r].defaultOption);
        pOustersBootsInfo->setUpgradeRatio(rows[r].upgradeRatio);
        pOustersBootsInfo->setUpgradeCrashPercent(rows[r].upgradeCrashPercent);
        pOustersBootsInfo->setNextOptionRatio(rows[r].nextOptionRatio);
        pOustersBootsInfo->setNextItemType(rows[r].nextItemType);
        pOustersBootsInfo->setDowngradeRatio(rows[r].downgradeRatio);

        addItemInfo(pOustersBootsInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void OustersBootsLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<GearObjectRow> rows =
        defaultItemObjectRepository().loadGearOfOwner(GEAR_OUSTERS_BOOTS, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            OustersBoots* pOustersBoots = new OustersBoots();

            pOustersBoots->setItemID(rows[r].itemID);
            pOustersBoots->setObjectID(rows[r].objectID);
            pOustersBoots->setItemType(rows[r].itemType);

            if (g_pOustersBootsInfoManager->getItemInfo(pOustersBoots->getItemType())->isUnique())
                pOustersBoots->setUnique();

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pOustersBoots->setOptionType(optionTypes);

            pOustersBoots->setDurability(rows[r].durability);
            pOustersBoots->setGrade(rows[r].grade);
            pOustersBoots->setEnchantLevel(rows[r].enchantLevel);
            pOustersBoots->setCreateType((Item::CreateType)rows[r].createType);

            Inventory* pInventory = NULL;
            Slayer* pSlayer = NULL;
            Vampire* pVampire = NULL;
            Ousters* pOusters = NULL;
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
            } else if (pCreature->isOusters()) {
                pOusters = dynamic_cast<Ousters*>(pCreature);
                pInventory = pOusters->getInventory();
                pStash = pOusters->getStash();
            } else
                throw UnsupportedError("Monster,NPC 인벤토리의 저장은 아직 지원되지 않습니다.");

            switch (storage) {
            case STORAGE_INVENTORY:
                if (pInventory->canAddingEx(x, y, pOustersBoots)) {
                    pInventory->addItemEx(x, y, pOustersBoots);
                } else {
                    processItemBugEx(pCreature, pOustersBoots);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer() || pCreature->isVampire()) {
                    processItemBugEx(pCreature, pOustersBoots);
                } else if (pCreature->isOusters()) {
                    if (!pOusters->isWear((Ousters::WearPart)x)) {
                        pOusters->wearItem((Ousters::WearPart)x, pOustersBoots);
                    } else {
                        processItemBugEx(pCreature, pOustersBoots);
                    }
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pOustersBoots);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pOustersBoots);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pOustersBoots);
                else if (pCreature->isOusters())
                    pOusters->addItemToExtraInventorySlot(pOustersBoots);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pOustersBoots);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pOustersBoots);
                } else
                    pStash->insert(x, y, pOustersBoots);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pOustersBoots);
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
void OustersBootsLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<GearZoneObjectRow> rows =
        defaultItemObjectRepository().loadGearInZone(GEAR_OUSTERS_BOOTS, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        OustersBoots* pOustersBoots = new OustersBoots();

        pOustersBoots->setItemID(rows[r].itemID);
        pOustersBoots->setObjectID(rows[r].objectID);
        pOustersBoots->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        string optionField = rows[r].optionField;
        list<OptionType_t> optionTypes;
        setOptionTypeFromField(optionTypes, optionField);
        pOustersBoots->setOptionType(optionTypes);

        pOustersBoots->setDurability(rows[r].durability);
        pOustersBoots->setEnchantLevel(rows[r].enchantLevel);
        pOustersBoots->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pOustersBoots);
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
void OustersBootsLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

OustersBootsLoader* g_pOustersBootsLoader = NULL;
