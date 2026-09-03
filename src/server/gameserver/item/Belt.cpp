//////////////////////////////////////////////////////////////////////////////
// Filename    : Belt.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Belt.h"

#include "DB.h"
#include "ItemInfoManager.h"
#include "ItemUtil.h"
#include "Motorcycle.h"
#include "PCItemInfo.h"
#include "Slayer.h"
#include "Stash.h"
#include "Vampire.h"
#include "repository/ItemObjectRepository.h"

// global variable declaration
BeltInfoManager* g_pBeltInfoManager = NULL;

ItemID_t Belt::m_ItemIDRegistry = 0;
Mutex Belt::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
Belt::Belt()

//: m_ItemType(0), m_Durability(0), m_pInventory(NULL)
{
    setItemType(0);
    setDurability(0);
    m_pInventory = NULL;
    //	m_EnchantLevel = 0;
}

Belt::Belt(ItemType_t itemType, const list<OptionType_t>& optionType)

//: m_ItemType(itemType), m_OptionType(optionType), m_Durability(0), m_pInventory(NULL)
{
    __BEGIN_TRY

    setItemType(itemType);
    setOptionType(optionType);

    BeltInfo* pBeltInfo = dynamic_cast<BeltInfo*>(g_pBeltInfoManager->getItemInfo(getItemType()));

    m_pInventory = new Inventory(pBeltInfo->getPocketCount(), 1);

    //	m_EnchantLevel = 0;

    setDurability(computeMaxDurability(this));

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "Belt::Belt() : Invalid item type or option type");
        throw("Belt::Belt() : Invalid item type or optionType");
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------
Belt::~Belt()

{
    SAFE_DELETE(m_pInventory);
}

//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void Belt::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y, ItemID_t itemID)

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

    defaultItemObjectRepository().insertGear(GEAR_BELT, m_ItemID, m_ObjectID, getItemType(), ownerID, (int)storage,
                                             storageID, (int)x, (int)y, optionField, getDurability(), getGrade(),
                                             (int)m_CreateType);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// destroy item
//--------------------------------------------------------------------------------
bool Belt::destroy()

{
    __BEGIN_TRY

    //-------------------------------------------------------
    // 벨트에 남아있는 아이템이 있다면 안에 있는 아이템들도
    // destroy 해줘야 한다.
    // 벨트 같은 경우는 위에서 delete하면서 아이템을 삭제
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

    if (!defaultItemObjectRepository().destroyGearObject(GEAR_BELT, m_ItemID))
        return false;

    __END_CATCH

    return true;
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Belt::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_BELT, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Belt::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateGear(GEAR_BELT, m_ObjectID, getItemType(), ownerID, (int)storage, storageID,
                                             (int)x, (int)y, optionField, getDurability(), getGrade(),
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

void Belt::makePCItemInfo(PCItemInfo& result) const {
    Item::makePCItemInfo(result);

    BYTE SubItemCount = 0;

    // 포켓의 숫자만큼 아이템의 정보를 읽어 들인다.
    for (int i = 0; i < getPocketCount(); i++) {
        Item* pBeltItem = getInventory()->getItem(i, 0);
        if (pBeltItem != NULL) {
            SubItemInfo* pSubItemInfo = new SubItemInfo();
            pSubItemInfo->setObjectID(pBeltItem->getObjectID());
            pSubItemInfo->setItemClass(pBeltItem->getItemClass());
            pSubItemInfo->setItemType(pBeltItem->getItemType());
            pSubItemInfo->setItemNum(pBeltItem->getNum());
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
string Belt::toString() const

{
    StringStream msg;

    msg << "Belt(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",Durability:" << (int)getDurability() << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}


/*//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t Belt::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pBeltInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t Belt::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pBeltInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t Belt::getWeight() const

{
    __BEGIN_TRY

    return g_pBeltInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}
*/
//--------------------------------------------------------------------------------
// get pocket count
//--------------------------------------------------------------------------------
PocketNum_t Belt::getPocketCount(void) const

{
    __BEGIN_TRY

    BeltInfo* pBeltInfo = dynamic_cast<BeltInfo*>(g_pBeltInfoManager->getItemInfo(getItemType()));
    Assert(pBeltInfo != NULL);
    return pBeltInfo->getPocketCount();

    __END_CATCH
}

/*Defense_t Belt::getDefenseBonus() const

{
    __BEGIN_TRY

    return g_pBeltInfoManager->getItemInfo(m_ItemType)->getDefenseBonus();

    __END_CATCH
}

Protection_t Belt::getProtectionBonus() const

{
    __BEGIN_TRY

    return g_pBeltInfoManager->getItemInfo(m_ItemType)->getProtectionBonus();

    __END_CATCH
}
*/


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string BeltInfo::toString() const

{
    StringStream msg;

    msg << "BeltInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void BeltInfoManager::load()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_BELT);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<PocketInfoRow> rows = defaultItemObjectRepository().loadPocketInfos(GEAR_BELT);

    for (size_t r = 0; r < rows.size(); r++) {
        BeltInfo* pBeltInfo = new BeltInfo();

        pBeltInfo->setItemType(rows[r].itemType);
        pBeltInfo->setName(rows[r].name);
        pBeltInfo->setEName(rows[r].ename);
        pBeltInfo->setPrice(rows[r].price);
        pBeltInfo->setVolumeType(rows[r].volume);
        pBeltInfo->setWeight(rows[r].weight);
        pBeltInfo->setRatio(rows[r].ratio);
        pBeltInfo->setDurability(rows[r].durability);
        pBeltInfo->setDefenseBonus(rows[r].defense);
        pBeltInfo->setProtectionBonus(rows[r].protection);
        pBeltInfo->setPocketCount(rows[r].pocketCount);
        pBeltInfo->setReqAbility(rows[r].reqAbility);
        pBeltInfo->setItemLevel(rows[r].itemLevel);
        pBeltInfo->setDefaultOptions(rows[r].defaultOption);
        pBeltInfo->setUpgradeRatio(rows[r].upgradeRatio);
        pBeltInfo->setUpgradeCrashPercent(rows[r].upgradeCrashPercent);
        pBeltInfo->setNextOptionRatio(rows[r].nextOptionRatio);
        pBeltInfo->setNextItemType(rows[r].nextItemType);
        pBeltInfo->setDowngradeRatio(rows[r].downgradeRatio);

        addItemInfo(pBeltInfo);
    }

    __END_DEBUG
    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void BeltLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<GearObjectRow> rows = defaultItemObjectRepository().loadGearOfOwner(GEAR_BELT, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            Belt* pBelt = new Belt();

            pBelt->setItemID(rows[r].itemID);
            pBelt->setObjectID(rows[r].objectID);
            pBelt->setItemType(rows[r].itemType);

            if (g_pBeltInfoManager->getItemInfo(pBelt->getItemType())->isUnique())
                pBelt->setUnique();

            BeltInfo* pBeltInfo = dynamic_cast<BeltInfo*>(g_pBeltInfoManager->getItemInfo(pBelt->getItemType()));
            Inventory* pBeltInventory = new Inventory(pBeltInfo->getPocketCount(), 1);

            pBelt->setInventory(pBeltInventory);

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pBelt->setOptionType(optionTypes);

            pBelt->setDurability(rows[r].durability);
            pBelt->setGrade(rows[r].grade);
            pBelt->setEnchantLevel(rows[r].enchantLevel);
            pBelt->setCreateType((Item::CreateType)rows[r].createType);

            Inventory* pInventory = NULL;
            Slayer* pSlayer = NULL;
            Vampire* pVampire = NULL;
            Motorcycle* pMotorcycle = NULL;
            Inventory* pMotorInventory = NULL;
            // Item*       pItem           = NULL;
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
                if (pInventory->canAddingEx(x, y, pBelt)) {
                    pInventory->addItemEx(x, y, pBelt);
                } else {
                    processItemBugEx(pCreature, pBelt);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer()) {
                    if (!pSlayer->isWear((Slayer::WearPart)x)) {
                        pSlayer->wearItem((Slayer::WearPart)x, pBelt);
                    } else {
                        processItemBugEx(pCreature, pBelt);
                    }
                } else if (pCreature->isVampire()) {
                    processItemBugEx(pCreature, pBelt);
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pBelt);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pBelt);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pBelt);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pBelt);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pBelt);
                } else {
                    pStash->insert(x, y, pBelt);
                }
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pBelt);
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
void BeltLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<GearZoneObjectRow> rows =
        defaultItemObjectRepository().loadGearInZone(GEAR_BELT, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        Belt* pBelt = new Belt();

        pBelt->setItemID(rows[r].itemID);
        pBelt->setObjectID(rows[r].objectID);
        pBelt->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        string optionField = rows[r].optionField;
        list<OptionType_t> optionTypes;
        setOptionTypeFromField(optionTypes, optionField);
        pBelt->setOptionType(optionTypes);

        pBelt->setDurability(rows[r].durability);
        pBelt->setEnchantLevel(rows[r].enchantLevel);
        pBelt->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pBelt);
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
void BeltLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

// global variable definition
BeltLoader* g_pBeltLoader = NULL;
