//////////////////////////////////////////////////////////////////////////////
// Filename    : SG.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

// include files
#include "SG.h"

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
SGInfoManager* g_pSGInfoManager = NULL;

ItemID_t SG::m_ItemIDRegistry = 0;
Mutex SG::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
SG::SG()

{
    setItemType(0);
    setDurability(0);
    setEnchantLevel(0);
    setBulletCount(0);
    setBonusDamage(0);
    setSilver(0);
}

SG::SG(ItemType_t itemType, const list<OptionType_t>& optionType)

//: Gun(itemType, optionType)
{
    setItemType(itemType);
    setOptionType(optionType);
    setDurability(computeMaxDurability(this));

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "SG::SG() : Invalid item type or option type");
        throw("SG::SG() : Invalid item type or optionType");
    }
}


//--------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------
SG::~SG()

{}


//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void SG::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y, ItemID_t itemID)

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

    defaultItemObjectRepository().insertGun(GEAR_SG, m_ItemID, m_ObjectID, getItemType(), ownerID, (int)storage,
                                            storageID, (int)x, (int)y, optionField, getDurability(),
                                            (int)getBulletCount(), (int)getGrade(), (int)m_CreateType);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void SG::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGun(GEAR_SG, field, (int)getBulletCount(), m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void SG::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateGun(GEAR_SG, m_ObjectID, getItemType(), ownerID, (int)storage, storageID,
                                            (int)x, (int)y, optionField, getDurability(), (int)getEnchantLevel(),
                                            (int)getBulletCount(), (int)getSilver(), (int)getGrade(), m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void SG::saveBullet() {
    __BEGIN_TRY

    defaultItemObjectRepository().saveGunBullet(GEAR_SG, getBulletCount(), m_ItemID);

    __END_CATCH
}

void SG::makePCItemInfo(PCItemInfo& result) const {
    Item::makePCItemInfo(result);
    result.setItemNum(getBulletCount());
}

//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string SG::toString() const

{
    StringStream msg;

    msg << "SG(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",Durability:" << (int)getDurability() << ",BulletCount:" << (int)getBulletCount()
        << ",Silver:" << (int)getSilver() << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}

/*//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t SG::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pSGInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t SG::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pSGInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t SG::getWeight() const

{
    __BEGIN_TRY

    return g_pSGInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set weapon's minDamage
//--------------------------------------------------------------------------------
Damage_t SG::getMinDamage() const

{
    __BEGIN_TRY

    return g_pSGInfoManager->getItemInfo(m_ItemType)->getMinDamage() + m_BonusDamage;

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set weapon's maxDamage
//--------------------------------------------------------------------------------
Damage_t SG::getMaxDamage() const

{
    __BEGIN_TRY

    return g_pSGInfoManager->getItemInfo(m_ItemType)->getMaxDamage() + m_BonusDamage;

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set weapon's range
//--------------------------------------------------------------------------------
Range_t SG::getRange() const

{
    __BEGIN_TRY

    return g_pSGInfoManager->getItemInfo(m_ItemType)->getRange();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set weapon's ToHit Bonus
//--------------------------------------------------------------------------------
ToHit_t SG::getToHitBonus() const

{
    __BEGIN_TRY

    return g_pSGInfoManager->getItemInfo(m_ItemType)->getToHitBonus();

    __END_CATCH
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
int SG::getCriticalBonus(void) const

{
    __BEGIN_TRY

    return g_pSGInfoManager->getItemInfo(m_ItemType)->getCriticalBonus();

    __END_CATCH
}
*/

//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string SGInfo::toString() const

{
    StringStream msg;

    msg << "SGInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName << ",Price:" << m_Price
        << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight << ",Description:" << m_Description
        << ",Durability:" << m_Durability << ",minDamage:" << m_MinDamage << ",maxDamage:" << m_MaxDamage
        << ",ToHitBonus:" << m_ToHitBonus << ",Range:" << (int)m_Range << ",ReqAbility:?" << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void SGInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_SG);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<GunInfoRow> rows = defaultItemObjectRepository().loadGunInfos(GEAR_SG);

    for (size_t r = 0; r < rows.size(); r++) {
        SGInfo* pSGInfo = new SGInfo();

        pSGInfo->setItemType(rows[r].itemType);
        pSGInfo->setName(rows[r].name);
        pSGInfo->setEName(rows[r].ename);
        pSGInfo->setPrice(rows[r].price);
        pSGInfo->setVolumeType(rows[r].volume);
        pSGInfo->setWeight(rows[r].weight);
        pSGInfo->setRatio(rows[r].ratio);
        pSGInfo->setDurability(rows[r].durability);
        pSGInfo->setMinDamage(rows[r].minDamage);
        pSGInfo->setMaxDamage(rows[r].maxDamage);
        pSGInfo->setToHitBonus(rows[r].toHitBonus);
        pSGInfo->setRange(rows[r].range);
        pSGInfo->setSpeed(rows[r].speed);
        pSGInfo->setReqAbility(rows[r].reqAbility);
        pSGInfo->setItemLevel(rows[r].itemLevel);
        pSGInfo->setCriticalBonus(rows[r].criticalBonus);
        pSGInfo->setDefaultOptions(rows[r].defaultOption);
        pSGInfo->setUpgradeRatio(rows[r].upgradeRatio);
        pSGInfo->setUpgradeCrashPercent(rows[r].upgradeCrashPercent);
        pSGInfo->setNextOptionRatio(rows[r].nextOptionRatio);
        pSGInfo->setNextItemType(rows[r].nextItemType);
        pSGInfo->setDowngradeRatio(rows[r].downgradeRatio);

        addItemInfo(pSGInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void SGLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<GunObjectRow> rows = defaultItemObjectRepository().loadGunOfOwner(GEAR_SG, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            SG* pSG = new SG();

            pSG->setItemID(rows[r].itemID);
            pSG->setObjectID(rows[r].objectID);
            pSG->setItemType(rows[r].itemType);

            if (g_pSGInfoManager->getItemInfo(pSG->getItemType())->isUnique())
                pSG->setUnique();

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pSG->setOptionType(optionTypes);

            pSG->setDurability(rows[r].durability);
            pSG->setEnchantLevel(rows[r].enchantLevel);
            pSG->setBulletCount(rows[r].bulletCount);
            pSG->setSilver(rows[r].silver);
            pSG->setGrade(rows[r].grade);
            pSG->setCreateType((Item::CreateType)rows[r].createType);

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
                if (pInventory->canAddingEx(x, y, pSG)) {
                    pInventory->addItemEx(x, y, pSG);
                } else {
                    processItemBugEx(pCreature, pSG);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer()) {
                    if (!pSlayer->isWear((Slayer::WearPart)x)) {
                        pSlayer->wearItem((Slayer::WearPart)x, pSG);
                    } else {
                        processItemBugEx(pCreature, pSG);
                    }
                } else if (pCreature->isVampire()) {
                    processItemBugEx(pCreature, pSG);
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pSG);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pSG);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pSG);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pSG);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pSG);
                } else
                    pStash->insert(x, y, pSG);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pSG);
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
void SGLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<GunZoneObjectRow> rows =
        defaultItemObjectRepository().loadGunInZone(GEAR_SG, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        SG* pSG = new SG();

        pSG->setItemID(rows[r].itemID);
        pSG->setObjectID(rows[r].objectID);
        pSG->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        string optionField = rows[r].optionField;
        list<OptionType_t> optionTypes;
        setOptionTypeFromField(optionTypes, optionField);
        pSG->setOptionType(optionTypes);

        pSG->setDurability(rows[r].durability);
        pSG->setEnchantLevel(rows[r].enchantLevel);
        pSG->setBulletCount(rows[r].bulletCount);
        pSG->setSilver(rows[r].silver);
        pSG->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pSG);
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
void SGLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

SGLoader* g_pSGLoader = NULL;
