//////////////////////////////////////////////////////////////////////////////
// Filename    : Bracelet.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Bracelet.h"

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
BraceletInfoManager* g_pBraceletInfoManager = NULL;

ItemID_t Bracelet::m_ItemIDRegistry = 0;
Mutex Bracelet::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
Bracelet::Bracelet()

//: m_ItemType(0), m_Durability(0)
{
    setItemType(0);
    setDurability(0);
    //	m_EnchantLevel = 0;
}

Bracelet::Bracelet(ItemType_t itemType, const list<OptionType_t>& optionType)

//: m_ItemType(itemType), m_OptionType(optionType), m_Durability(0)
{
    setItemType(itemType);
    setOptionType(optionType);
    //	m_EnchantLevel = 0;
    setDurability(computeMaxDurability(this));

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "Bracelet::Bracelet() : Invalid item type or option type");
        cerr << "Bracelet::Bracelet() : Invalid item type or optionType" << endl;
        throw "Bracelet::Bracelet() : Invalid item type or optionType";
    }
}

//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void Bracelet::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y, ItemID_t itemID)

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

    defaultItemObjectRepository().insertGear(GEAR_BRACELET, m_ItemID, m_ObjectID, getItemType(), ownerID, (int)storage,
                                             storageID, (int)x, (int)y, optionField, getDurability(), getGrade(),
                                             (int)m_CreateType);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Bracelet::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_BRACELET, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Bracelet::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateGear(GEAR_BRACELET, m_ObjectID, getItemType(), ownerID, (int)storage, storageID,
                                             (int)x, (int)y, optionField, getDurability(), getGrade(),
                                             (int)getEnchantLevel(), m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string Bracelet::toString() const

{
    StringStream msg;

    msg << "Bracelet(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",Durability:" << (int)getDurability() << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}


/*//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t Bracelet::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pBraceletInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t Bracelet::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pBraceletInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t Bracelet::getWeight() const

{
    __BEGIN_TRY

    return g_pBraceletInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set armor's Defense Bonus
//--------------------------------------------------------------------------------
Defense_t Bracelet::getDefenseBonus() const

{
    __BEGIN_TRY

    return g_pBraceletInfoManager->getItemInfo(m_ItemType)->getDefenseBonus();

    __END_CATCH
}
Protection_t Bracelet::getProtectionBonus() const

{
    __BEGIN_TRY

    return g_pBraceletInfoManager->getItemInfo(m_ItemType)->getProtectionBonus();

    __END_CATCH
}

*/
//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string BraceletInfo::toString() const

{
    StringStream msg;

    msg << "BraceletInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ",DefenseBonus:" << m_DefenseBonus
        << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void BraceletInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_BRACELET);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<GearInfoRow> rows = defaultItemObjectRepository().loadGearInfos(GEAR_BRACELET);

    for (size_t r = 0; r < rows.size(); r++) {
        BraceletInfo* pBraceletInfo = new BraceletInfo();

        pBraceletInfo->setItemType(rows[r].itemType);
        pBraceletInfo->setName(rows[r].name);
        pBraceletInfo->setEName(rows[r].ename);
        pBraceletInfo->setPrice(rows[r].price);
        pBraceletInfo->setVolumeType(rows[r].volume);
        pBraceletInfo->setWeight(rows[r].weight);
        pBraceletInfo->setRatio(rows[r].ratio);
        pBraceletInfo->setDurability(rows[r].durability);
        pBraceletInfo->setDefenseBonus(rows[r].defense);
        pBraceletInfo->setProtectionBonus(rows[r].protection);
        pBraceletInfo->setReqAbility(rows[r].reqAbility);
        pBraceletInfo->setItemLevel(rows[r].itemLevel);
        pBraceletInfo->setDefaultOptions(rows[r].defaultOption);
        pBraceletInfo->setUpgradeRatio(rows[r].upgradeRatio);
        pBraceletInfo->setUpgradeCrashPercent(rows[r].upgradeCrashPercent);
        pBraceletInfo->setNextOptionRatio(rows[r].nextOptionRatio);
        pBraceletInfo->setNextItemType(rows[r].nextItemType);
        pBraceletInfo->setDowngradeRatio(rows[r].downgradeRatio);

        addItemInfo(pBraceletInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void BraceletLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<GearObjectRow> rows = defaultItemObjectRepository().loadGearOfOwner(GEAR_BRACELET, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            Bracelet* pBracelet = new Bracelet();

            pBracelet->setItemID(rows[r].itemID);
            pBracelet->setObjectID(rows[r].objectID);
            pBracelet->setItemType(rows[r].itemType);

            if (g_pBraceletInfoManager->getItemInfo(pBracelet->getItemType())->isUnique())
                pBracelet->setUnique();

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pBracelet->setOptionType(optionTypes);

            pBracelet->setDurability(rows[r].durability);
            pBracelet->setGrade(rows[r].grade);
            pBracelet->setEnchantLevel(rows[r].enchantLevel);
            pBracelet->setCreateType((Item::CreateType)rows[r].createType);

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
                if (pInventory->canAddingEx(x, y, pBracelet)) {
                    pInventory->addItemEx(x, y, pBracelet);
                } else {
                    processItemBugEx(pCreature, pBracelet);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer()) {
                    if (!pSlayer->isWear((Slayer::WearPart)x)) {
                        pSlayer->wearItem((Slayer::WearPart)x, pBracelet);
                    } else {
                        processItemBugEx(pCreature, pBracelet);
                    }
                } else if (pCreature->isVampire()) {
                    processItemBugEx(pCreature, pBracelet);
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pBracelet);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pBracelet);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pBracelet);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pBracelet);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pBracelet);
                } else
                    pStash->insert(x, y, pBracelet);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pBracelet);
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
void BraceletLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<GearZoneObjectRow> rows =
        defaultItemObjectRepository().loadGearInZone(GEAR_BRACELET, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        Bracelet* pBracelet = new Bracelet();

        pBracelet->setItemID(rows[r].itemID);
        pBracelet->setObjectID(rows[r].objectID);
        pBracelet->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        string optionField = rows[r].optionField;
        list<OptionType_t> optionTypes;
        setOptionTypeFromField(optionTypes, optionField);
        pBracelet->setOptionType(optionTypes);

        pBracelet->setDurability(rows[r].durability);
        pBracelet->setEnchantLevel(rows[r].enchantLevel);
        pBracelet->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pBracelet);
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
void BraceletLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)


    __END_CATCH
}

BraceletLoader* g_pBraceletLoader = NULL;
