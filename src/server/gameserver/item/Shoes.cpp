//////////////////////////////////////////////////////////////////////////////
// Filename    : Shoes.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Shoes.h"

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
ShoesInfoManager* g_pShoesInfoManager = NULL;

ItemID_t Shoes::m_ItemIDRegistry = 0;
Mutex Shoes::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
Shoes::Shoes()

{
    setItemType(0);
    setDurability(0);
    // m_EnchantLevel = 0;
}

Shoes::Shoes(ItemType_t itemType, const list<OptionType_t>& optionType)

{
    setItemType(itemType);
    setOptionType(optionType);
    // m_EnchantLevel = 0;
    setDurability(computeMaxDurability(this));

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "Shoes::Shoes() : Invalid item type or option type");
        throw("Shoes::Shoes() : Invalid item type or optionType");
    }
}

//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void Shoes::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y, ItemID_t itemID)

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

    defaultItemObjectRepository().insertGear(GEAR_SHOES, m_ItemID, m_ObjectID, getItemType(), ownerID, (int)storage,
                                             storageID, (int)x, (int)y, optionField, getDurability(), getGrade(),
                                             (int)m_CreateType);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Shoes::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_SHOES, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Shoes::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateGear(GEAR_SHOES, m_ObjectID, getItemType(), ownerID, (int)storage, storageID,
                                             (int)x, (int)y, optionField, getDurability(), getGrade(),
                                             (int)getEnchantLevel(), m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string Shoes::toString() const

{
    StringStream msg;

    msg << "Shoes(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",Durability:" << (int)getDurability() << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}


/*//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t Shoes::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pShoesInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t Shoes::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pShoesInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t Shoes::getWeight() const

{
    __BEGIN_TRY

    return g_pShoesInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set armor's Defense Bonus
//--------------------------------------------------------------------------------
Defense_t Shoes::getDefenseBonus() const

{
    __BEGIN_TRY

    return g_pShoesInfoManager->getItemInfo(m_ItemType)->getDefenseBonus();

    __END_CATCH
}

Defense_t Shoes::getProtectionBonus() const

{
    __BEGIN_TRY

    return g_pShoesInfoManager->getItemInfo(m_ItemType)->getProtectionBonus();

    __END_CATCH
}
*/

//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string ShoesInfo::toString() const

{
    StringStream msg;

    msg << "ShoesInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ",DefenseBonus:" << m_DefenseBonus
        << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void ShoesInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_SHOES);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<GearInfoRow> rows = defaultItemObjectRepository().loadGearInfos(GEAR_SHOES);

    for (size_t r = 0; r < rows.size(); r++) {
        ShoesInfo* pShoesInfo = new ShoesInfo();

        pShoesInfo->setItemType(rows[r].itemType);
        pShoesInfo->setName(rows[r].name);
        pShoesInfo->setEName(rows[r].ename);
        pShoesInfo->setPrice(rows[r].price);
        pShoesInfo->setVolumeType(rows[r].volume);
        pShoesInfo->setWeight(rows[r].weight);
        pShoesInfo->setRatio(rows[r].ratio);
        pShoesInfo->setDurability(rows[r].durability);
        pShoesInfo->setDefenseBonus(rows[r].defense);
        pShoesInfo->setProtectionBonus(rows[r].protection);
        pShoesInfo->setReqAbility(rows[r].reqAbility);
        pShoesInfo->setItemLevel(rows[r].itemLevel);
        pShoesInfo->setDefaultOptions(rows[r].defaultOption);
        pShoesInfo->setUpgradeRatio(rows[r].upgradeRatio);
        pShoesInfo->setUpgradeCrashPercent(rows[r].upgradeCrashPercent);
        pShoesInfo->setNextOptionRatio(rows[r].nextOptionRatio);
        pShoesInfo->setNextItemType(rows[r].nextItemType);
        pShoesInfo->setDowngradeRatio(rows[r].downgradeRatio);

        addItemInfo(pShoesInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void ShoesLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<GearObjectRow> rows = defaultItemObjectRepository().loadGearOfOwner(GEAR_SHOES, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            Shoes* pShoes = new Shoes();

            pShoes->setItemID(rows[r].itemID);
            pShoes->setObjectID(rows[r].objectID);
            pShoes->setItemType(rows[r].itemType);

            if (g_pShoesInfoManager->getItemInfo(pShoes->getItemType())->isUnique())
                pShoes->setUnique();

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pShoes->setOptionType(optionTypes);

            pShoes->setDurability(rows[r].durability);
            pShoes->setGrade(rows[r].grade);
            pShoes->setEnchantLevel(rows[r].enchantLevel);
            pShoes->setCreateType((Item::CreateType)rows[r].createType);

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
                if (pInventory->canAddingEx(x, y, pShoes)) {
                    pInventory->addItemEx(x, y, pShoes);
                } else {
                    processItemBugEx(pCreature, pShoes);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer()) {
                    if (!pSlayer->isWear((Slayer::WearPart)x)) {
                        pSlayer->wearItem((Slayer::WearPart)x, pShoes);
                    } else {
                        processItemBugEx(pCreature, pShoes);
                    }
                } else if (pCreature->isVampire()) {
                    processItemBugEx(pCreature, pShoes);
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pShoes);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pShoes);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pShoes);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pShoes);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pShoes);
                } else
                    pStash->insert(x, y, pShoes);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pShoes);
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
void ShoesLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<GearZoneObjectRow> rows =
        defaultItemObjectRepository().loadGearInZone(GEAR_SHOES, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        Shoes* pShoes = new Shoes();

        pShoes->setItemID(rows[r].itemID);
        pShoes->setObjectID(rows[r].objectID);
        pShoes->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        string optionField = rows[r].optionField;
        list<OptionType_t> optionTypes;
        setOptionTypeFromField(optionTypes, optionField);
        pShoes->setOptionType(optionTypes);

        pShoes->setDurability(rows[r].durability);
        pShoes->setEnchantLevel(rows[r].enchantLevel);
        pShoes->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pShoes);
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
void ShoesLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

ShoesLoader* g_pShoesLoader = NULL;
