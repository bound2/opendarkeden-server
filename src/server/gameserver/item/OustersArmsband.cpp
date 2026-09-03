//////////////////////////////////////////////////////////////////////////////
// Filename    : OustersArmsband.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "OustersArmsband.h"

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
OustersArmsbandInfoManager* g_pOustersArmsbandInfoManager = NULL;

ItemID_t OustersArmsband::m_ItemIDRegistry = 0;
Mutex OustersArmsband::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
OustersArmsband::OustersArmsband()

//: m_ItemType(0), m_Durability(0), m_pInventory(NULL)
{
    setItemType(0);
    setDurability(0);
    m_pInventory = NULL;
    //	m_EnchantLevel = 0;
}

OustersArmsband::OustersArmsband(ItemType_t itemType, const list<OptionType_t>& optionType)

//: m_ItemType(itemType), m_OptionType(optionType), m_Durability(0), m_pInventory(NULL)
{
    setItemType(itemType);
    setOptionType(optionType);
    OustersArmsbandInfo* pOustersArmsbandInfo =
        dynamic_cast<OustersArmsbandInfo*>(g_pOustersArmsbandInfoManager->getItemInfo(getItemType()));

    m_pInventory = new Inventory(pOustersArmsbandInfo->getPocketCount(), 1);

    //	m_EnchantLevel = 0;

    setDurability(computeMaxDurability(this));

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "OustersArmsband::OustersArmsband() : Invalid item type or option type");
        throw("OustersArmsband::OustersArmsband() : Invalid item type or optionType");
    }
}

//--------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------
OustersArmsband::~OustersArmsband()

{
    SAFE_DELETE(m_pInventory);
}

//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void OustersArmsband::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y,
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

    defaultItemObjectRepository().insertGear(GEAR_OUSTERS_ARMSBAND, m_ItemID, m_ObjectID, getItemType(), ownerID,
                                             (int)storage, storageID, (int)x, (int)y, optionField, getDurability(),
                                             getGrade(), (int)m_CreateType);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// destroy item
//--------------------------------------------------------------------------------
bool OustersArmsband::destroy()

{
    __BEGIN_TRY

    //-------------------------------------------------------
    // 암스밴드에 남아있는 아이템이 있다면 안에 있는 아이템들도
    // destroy 해줘야 한다.
    // 암스밴드 같은 경우는 위에서 delete하면서 아이템을 삭제
    // 하기 때문에 여기서는 delete해주지 않기로 한다...
    // 쓸모가 없다면 위에서 필히 벨트를 지워야 한다.
    //-------------------------------------------------------
    for (int i = 0; i < m_pInventory->getHeight(); i++) {
        for (int j = 0; j < m_pInventory->getWidth(); j++) {
            Item* pItem = m_pInventory->getItem(j, i);
            if (pItem != NULL) {
                pItem->destroy();
            }
        }
    }

    if (!defaultItemObjectRepository().destroyGearObject(GEAR_OUSTERS_ARMSBAND, m_ItemID))
        return false;

    __END_CATCH

    return true;
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void OustersArmsband::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_OUSTERS_ARMSBAND, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void OustersArmsband::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateGear(GEAR_OUSTERS_ARMSBAND, m_ObjectID, getItemType(), ownerID, (int)storage,
                                             storageID, (int)x, (int)y, optionField, getDurability(), getGrade(),
                                             (int)getEnchantLevel(), m_ItemID);

    // 일일이 아이템을 하나씩 꺼내서 바로 UPDATE 하도록 한다.
    for (int i = 0; i < m_pInventory->getHeight(); i++) {
        for (int j = 0; j < m_pInventory->getWidth(); j++) {
            Item* pItem = m_pInventory->getItem(j, 0);
            if (pItem != NULL) {
                pItem->save(ownerID, STORAGE_BELT, m_ItemID, j, 0);
            }
        }
    }

    __END_CATCH
}

void OustersArmsband::makePCItemInfo(PCItemInfo& result) const {
    Item::makePCItemInfo(result);

    BYTE SubItemCount = 0;

    // 포켓의 숫자만큼 아이템의 정보를 읽어 들인다.
    for (int i = 0; i < getPocketCount(); i++) {
        Item* pOustersArmsbandItem = getInventory()->getItem(i, 0);

        if (pOustersArmsbandItem != NULL) {
            SubItemInfo* pSubItemInfo = new SubItemInfo();
            pSubItemInfo->setObjectID(pOustersArmsbandItem->getObjectID());
            pSubItemInfo->setItemClass(pOustersArmsbandItem->getItemClass());
            pSubItemInfo->setItemType(pOustersArmsbandItem->getItemType());
            pSubItemInfo->setItemNum(pOustersArmsbandItem->getNum());
            pSubItemInfo->setSlotID(i);

            result.addListElement(pSubItemInfo);

            SubItemCount++;
        }
    }

    result.setListNum(SubItemCount);
}

//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string OustersArmsband::toString() const

{
    StringStream msg;

    msg << "OustersArmsband(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",Durability:" << (int)getDurability() << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}


/*//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t OustersArmsband::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pOustersArmsbandInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t OustersArmsband::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pOustersArmsbandInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t OustersArmsband::getWeight() const

{
    __BEGIN_TRY

    return g_pOustersArmsbandInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}
*/

//--------------------------------------------------------------------------------
// get pocket count
//--------------------------------------------------------------------------------
PocketNum_t OustersArmsband::getPocketCount(void) const

{
    __BEGIN_TRY

    OustersArmsbandInfo* pOustersArmsbandInfo =
        dynamic_cast<OustersArmsbandInfo*>(g_pOustersArmsbandInfoManager->getItemInfo(getItemType()));
    Assert(pOustersArmsbandInfo != NULL);
    return pOustersArmsbandInfo->getPocketCount();

    __END_CATCH
}
/*
//--------------------------------------------------------------------------------
// get/set armor's Defense Bonus
//--------------------------------------------------------------------------------
Defense_t OustersArmsband::getDefenseBonus() const

{
    __BEGIN_TRY

    return g_pOustersArmsbandInfoManager->getItemInfo(m_ItemType)->getDefenseBonus();

    __END_CATCH
}
Protection_t OustersArmsband::getProtectionBonus() const

{
    __BEGIN_TRY

    return g_pOustersArmsbandInfoManager->getItemInfo(m_ItemType)->getProtectionBonus();

    __END_CATCH
}

*/
//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string OustersArmsbandInfo::toString() const

{
    StringStream msg;

    msg << "OustersArmsbandInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ",DefenseBonus:" << m_DefenseBonus
        << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void OustersArmsbandInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_OUSTERS_ARMSBAND);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<PocketInfoRow> rows = defaultItemObjectRepository().loadPocketInfos(GEAR_OUSTERS_ARMSBAND);

    for (size_t r = 0; r < rows.size(); r++) {
        OustersArmsbandInfo* pOustersArmsbandInfo = new OustersArmsbandInfo();

        pOustersArmsbandInfo->setItemType(rows[r].itemType);
        pOustersArmsbandInfo->setName(rows[r].name);
        pOustersArmsbandInfo->setEName(rows[r].ename);
        pOustersArmsbandInfo->setPrice(rows[r].price);
        pOustersArmsbandInfo->setVolumeType(rows[r].volume);
        pOustersArmsbandInfo->setWeight(rows[r].weight);
        pOustersArmsbandInfo->setRatio(rows[r].ratio);
        pOustersArmsbandInfo->setDurability(rows[r].durability);
        pOustersArmsbandInfo->setDefenseBonus(rows[r].defense);
        pOustersArmsbandInfo->setProtectionBonus(rows[r].protection);
        pOustersArmsbandInfo->setPocketCount(rows[r].pocketCount);
        pOustersArmsbandInfo->setReqAbility(rows[r].reqAbility);
        pOustersArmsbandInfo->setItemLevel(rows[r].itemLevel);
        pOustersArmsbandInfo->setDefaultOptions(rows[r].defaultOption);
        pOustersArmsbandInfo->setUpgradeRatio(rows[r].upgradeRatio);
        pOustersArmsbandInfo->setUpgradeCrashPercent(rows[r].upgradeCrashPercent);
        pOustersArmsbandInfo->setNextOptionRatio(rows[r].nextOptionRatio);
        pOustersArmsbandInfo->setNextItemType(rows[r].nextItemType);
        pOustersArmsbandInfo->setDowngradeRatio(rows[r].downgradeRatio);

        addItemInfo(pOustersArmsbandInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void OustersArmsbandLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<GearObjectRow> rows =
        defaultItemObjectRepository().loadGearOfOwner(GEAR_OUSTERS_ARMSBAND, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            OustersArmsband* pOustersArmsband = new OustersArmsband();

            pOustersArmsband->setItemID(rows[r].itemID);
            pOustersArmsband->setObjectID(rows[r].objectID);
            pOustersArmsband->setItemType(rows[r].itemType);

            if (g_pOustersArmsbandInfoManager->getItemInfo(pOustersArmsband->getItemType())->isUnique())
                pOustersArmsband->setUnique();

            OustersArmsbandInfo* pOustersArmsbandInfo = dynamic_cast<OustersArmsbandInfo*>(
                g_pOustersArmsbandInfoManager->getItemInfo(pOustersArmsband->getItemType()));
            Inventory* pOustersArmsbandInventory = new Inventory(pOustersArmsbandInfo->getPocketCount(), 1);

            pOustersArmsband->setInventory(pOustersArmsbandInventory);

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pOustersArmsband->setOptionType(optionTypes);

            pOustersArmsband->setDurability(rows[r].durability);
            pOustersArmsband->setGrade(rows[r].grade);
            pOustersArmsband->setEnchantLevel(rows[r].enchantLevel);
            pOustersArmsband->setCreateType((Item::CreateType)rows[r].createType);

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
                if (pInventory->canAddingEx(x, y, pOustersArmsband)) {
                    pInventory->addItemEx(x, y, pOustersArmsband);
                } else {
                    processItemBugEx(pCreature, pOustersArmsband);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer() || pCreature->isVampire()) {
                    processItemBugEx(pCreature, pOustersArmsband);
                } else if (pCreature->isOusters()) {
                    if (!pOusters->isWear((Ousters::WearPart)x)) {
                        pOusters->wearItem((Ousters::WearPart)x, pOustersArmsband);
                    } else {
                        processItemBugEx(pCreature, pOustersArmsband);
                    }
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pOustersArmsband);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pOustersArmsband);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pOustersArmsband);
                else if (pCreature->isOusters())
                    pOusters->addItemToExtraInventorySlot(pOustersArmsband);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pOustersArmsband);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pOustersArmsband);
                } else
                    pStash->insert(x, y, pOustersArmsband);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pOustersArmsband);
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
void OustersArmsbandLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<GearZoneObjectRow> rows =
        defaultItemObjectRepository().loadGearInZone(GEAR_OUSTERS_ARMSBAND, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        OustersArmsband* pOustersArmsband = new OustersArmsband();

        pOustersArmsband->setItemID(rows[r].itemID);
        pOustersArmsband->setObjectID(rows[r].objectID);
        pOustersArmsband->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        string optionField = rows[r].optionField;
        list<OptionType_t> optionTypes;
        setOptionTypeFromField(optionTypes, optionField);
        pOustersArmsband->setOptionType(optionTypes);

        pOustersArmsband->setDurability(rows[r].durability);
        pOustersArmsband->setEnchantLevel(rows[r].enchantLevel);
        pOustersArmsband->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pOustersArmsband);
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
void OustersArmsbandLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

OustersArmsbandLoader* g_pOustersArmsbandLoader = NULL;
