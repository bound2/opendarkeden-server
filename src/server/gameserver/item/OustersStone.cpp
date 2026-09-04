//////////////////////////////////////////////////////////////////////////////
// Filename    : OustersStone.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "OustersStone.h"

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
OustersStoneInfoManager* g_pOustersStoneInfoManager = NULL;

ItemID_t OustersStone::m_ItemIDRegistry = 0;
Mutex OustersStone::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
OustersStone::OustersStone()

//: m_ItemType(0), m_Durability(0)
{
    setItemType(0);
    setDurability(0);
    // m_EnchantLevel = 0;
}

OustersStone::OustersStone(ItemType_t itemType, const list<OptionType_t>& optionType)

//: m_ItemType(itemType), m_OptionType(optionType), m_Durability(0)
{
    setItemType(itemType);
    setOptionType(optionType);
    // m_EnchantLevel = 0;

    setDurability(computeMaxDurability(this));

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "OustersStone::OustersStone() : Invalid item type or option type");
        throw "OustersStone::OustersStone() : Invalid item type or optionType";
    }
}


//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void OustersStone::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y,
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

    defaultItemObjectRepository().insertGear(GEAR_OUSTERS_STONE, m_ItemID, m_ObjectID, getItemType(), ownerID,
                                             (int)storage, storageID, (int)x, (int)y, optionField, getDurability(),
                                             getGrade(), (int)m_CreateType);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void OustersStone::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_OUSTERS_STONE, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void OustersStone::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateGear(GEAR_OUSTERS_STONE, m_ObjectID, getItemType(), ownerID, (int)storage,
                                             storageID, (int)x, (int)y, optionField, getDurability(), getGrade(),
                                             (int)getEnchantLevel(), m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string OustersStone::toString() const

{
    StringStream msg;

    msg << "OustersStone(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",Durability:" << (int)getDurability() << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}


/*//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t OustersStone::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pOustersStoneInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t OustersStone::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pOustersStoneInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t OustersStone::getWeight() const

{
    __BEGIN_TRY

    return g_pOustersStoneInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set armor's Defense Bonus
//--------------------------------------------------------------------------------
Defense_t OustersStone::getDefenseBonus() const

{
    __BEGIN_TRY

    return g_pOustersStoneInfoManager->getItemInfo(m_ItemType)->getDefenseBonus();

    __END_CATCH
}
Protection_t OustersStone::getProtectionBonus() const

{
    __BEGIN_TRY

    return g_pOustersStoneInfoManager->getItemInfo(m_ItemType)->getProtectionBonus();

    __END_CATCH
}
*/

//--------------------------------------------------------------------------------
// get Elemental Type
//--------------------------------------------------------------------------------
ElementalType OustersStone::getElementalType(void) const {
    __BEGIN_TRY

    return g_pOustersStoneInfoManager->getItemInfo(getItemType())->getElementalType();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get Elemental
//--------------------------------------------------------------------------------
Elemental_t OustersStone::getElemental(void) const {
    __BEGIN_TRY

    return g_pOustersStoneInfoManager->getItemInfo(getItemType())->getElemental();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string OustersStoneInfo::toString() const

{
    StringStream msg;

    msg << "OustersStoneInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ",DefenseBonus:" << m_DefenseBonus
        << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void OustersStoneInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_OUSTERS_STONE);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<GearInfoElementalRow> rows = defaultItemObjectRepository().loadGearInfosElemental(GEAR_OUSTERS_STONE);

    for (size_t r = 0; r < rows.size(); r++) {
        OustersStoneInfo* pOustersStoneInfo = new OustersStoneInfo();

        pOustersStoneInfo->setItemType(rows[r].gear.itemType);
        pOustersStoneInfo->setName(rows[r].gear.name);
        pOustersStoneInfo->setEName(rows[r].gear.ename);
        pOustersStoneInfo->setPrice(rows[r].gear.price);
        pOustersStoneInfo->setVolumeType(rows[r].gear.volume);
        pOustersStoneInfo->setWeight(rows[r].gear.weight);
        pOustersStoneInfo->setRatio(rows[r].gear.ratio);
        pOustersStoneInfo->setDurability(rows[r].gear.durability);
        pOustersStoneInfo->setDefenseBonus(rows[r].gear.defense);
        pOustersStoneInfo->setProtectionBonus(rows[r].gear.protection);
        pOustersStoneInfo->setReqAbility(rows[r].gear.reqAbility);
        pOustersStoneInfo->setItemLevel(rows[r].gear.itemLevel);
        pOustersStoneInfo->setDefaultOptions(rows[r].gear.defaultOption);
        pOustersStoneInfo->setUpgradeRatio(rows[r].gear.upgradeRatio);
        pOustersStoneInfo->setUpgradeCrashPercent(rows[r].gear.upgradeCrashPercent);
        pOustersStoneInfo->setNextOptionRatio(rows[r].gear.nextOptionRatio);
        pOustersStoneInfo->setNextItemType(rows[r].gear.nextItemType);
        pOustersStoneInfo->setDowngradeRatio(rows[r].gear.downgradeRatio);
        pOustersStoneInfo->setElementalType((ElementalType)rows[r].elementalType);
        pOustersStoneInfo->setElemental((Elemental_t)rows[r].elemental);

        addItemInfo(pOustersStoneInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void OustersStoneLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<GearObjectRow> rows =
        defaultItemObjectRepository().loadGearOfOwner(GEAR_OUSTERS_STONE, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            OustersStone* pOustersStone = new OustersStone();

            pOustersStone->setItemID(rows[r].itemID);
            pOustersStone->setObjectID(rows[r].objectID);
            pOustersStone->setItemType(rows[r].itemType);

            if (g_pOustersStoneInfoManager->getItemInfo(pOustersStone->getItemType())->isUnique())
                pOustersStone->setUnique();

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pOustersStone->setOptionType(optionTypes);

            pOustersStone->setDurability(rows[r].durability);
            pOustersStone->setGrade(rows[r].grade);
            pOustersStone->setEnchantLevel(rows[r].enchantLevel);
            pOustersStone->setCreateType((Item::CreateType)rows[r].createType);

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
                if (pInventory->canAddingEx(x, y, pOustersStone)) {
                    pInventory->addItemEx(x, y, pOustersStone);
                } else {
                    processItemBugEx(pCreature, pOustersStone);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer() || pCreature->isVampire()) {
                    processItemBugEx(pCreature, pOustersStone);
                } else if (pCreature->isOusters()) {
                    if (!pOusters->isWear((Ousters::WearPart)x)) {
                        pOusters->wearItem((Ousters::WearPart)x, pOustersStone);
                    } else {
                        processItemBugEx(pCreature, pOustersStone);
                    }
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pOustersStone);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pOustersStone);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pOustersStone);
                else if (pCreature->isOusters())
                    pOusters->addItemToExtraInventorySlot(pOustersStone);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pOustersStone);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pOustersStone);
                } else
                    pStash->insert(x, y, pOustersStone);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pOustersStone);
                break;

            default:
                throw Error("invalid storage or OwnerID must be NULL");
            }
        } catch (Error& error) {
            filelog("itemLoadError.txt", "[%s] %s,Owner:%s", getItemClassName().c_str(), error.toString().c_str(),
                    pCreature->getName().c_str());
            throw;
        } catch (Throwable& t) {
            filelog("itemLoadError.txt", "[%s] %s,Owner:%s", getItemClassName().c_str(), t.toString().c_str(),
                    pCreature->getName().c_str());
        }
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to zone
//--------------------------------------------------------------------------------
void OustersStoneLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<GearZoneObjectRow> rows =
        defaultItemObjectRepository().loadGearInZone(GEAR_OUSTERS_STONE, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        OustersStone* pOustersStone = new OustersStone();

        pOustersStone->setItemID(rows[r].itemID);
        pOustersStone->setObjectID(rows[r].objectID);
        pOustersStone->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        string optionField = rows[r].optionField;
        list<OptionType_t> optionTypes;
        setOptionTypeFromField(optionTypes, optionField);
        pOustersStone->setOptionType(optionTypes);

        pOustersStone->setDurability(rows[r].durability);
        pOustersStone->setEnchantLevel(rows[r].enchantLevel);
        pOustersStone->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pOustersStone);
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
void OustersStoneLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

OustersStoneLoader* g_pOustersStoneLoader = NULL;
