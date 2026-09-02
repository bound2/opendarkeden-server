//////////////////////////////////////////////////////////////////////////////
// Filename    : OustersWristlet.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

// include files
#include "OustersWristlet.h"

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
OustersWristletInfoManager* g_pOustersWristletInfoManager = NULL;

ItemID_t OustersWristlet::m_ItemIDRegistry = 0;
Mutex OustersWristlet::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
OustersWristlet::OustersWristlet()

{
    setItemType(0);
    setDurability(0);
    setBonusDamage(0);
}

OustersWristlet::OustersWristlet(ItemType_t itemType, const list<OptionType_t>& optionType)

//: m_OptionType( optionType )
{
    setItemType(itemType);
    setOptionType(optionType);
    setBonusDamage(0);

    // m_EnchantLevel = 0;
    setDurability(computeMaxDurability(this));

    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), getItemType(), getOptionTypeList())) {
        filelog("itembug.log", "OustersWristlet::OustersWristlet() : Invalid item type or option type");
        throw("OustersWristlet::OustersWristlet() : Invalid item type or optionType");
    }
}

//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void OustersWristlet::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y,
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

    defaultItemObjectRepository().insertGear(GEAR_OUSTERS_WRISTLET, m_ItemID, m_ObjectID, getItemType(), ownerID,
                                             (int)storage, storageID, (int)x, (int)y, optionField, getDurability(),
                                             getGrade(), (int)m_CreateType);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void OustersWristlet::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_OUSTERS_WRISTLET, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void OustersWristlet::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    string optionField;
    setOptionTypeToField(getOptionTypeList(), optionField);

    defaultItemObjectRepository().updateGear(GEAR_OUSTERS_WRISTLET, m_ObjectID, getItemType(), ownerID, (int)storage,
                                             storageID, (int)x, (int)y, optionField, getDurability(), getGrade(),
                                             (int)getEnchantLevel(), m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string OustersWristlet::toString() const

{
    StringStream msg;

    msg << "OustersWristlet(" << "ItemID:" << m_ItemID << ",ItemType:" << (int)getItemType()
        << ",OptionType:" << getOptionTypeToString(getOptionTypeList()).c_str()
        << ",Durability:" << (int)getDurability() << ",EnchantLevel:" << (int)getEnchantLevel() << ")";

    return msg.toString();
}


/*//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t OustersWristlet::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pOustersWristletInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t OustersWristlet::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pOustersWristletInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t OustersWristlet::getWeight() const

{
    __BEGIN_TRY

    return g_pOustersWristletInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set weapon's minDamage
//--------------------------------------------------------------------------------
Damage_t OustersWristlet::getMinDamage() const

{
    __BEGIN_TRY

    return g_pOustersWristletInfoManager->getItemInfo(m_ItemType)->getMinDamage() + m_BonusDamage;

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get/set weapon's maxDamage
//--------------------------------------------------------------------------------
Damage_t OustersWristlet::getMaxDamage() const

{
    __BEGIN_TRY

    return g_pOustersWristletInfoManager->getItemInfo(m_ItemType)->getMaxDamage() + m_BonusDamage;

    __END_CATCH
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
int OustersWristlet::getCriticalBonus(void) const

{
    __BEGIN_TRY

    return g_pOustersWristletInfoManager->getItemInfo(m_ItemType)->getCriticalBonus();

    __END_CATCH
}
*/

//--------------------------------------------------------------------------------
// get Elemental Type
//--------------------------------------------------------------------------------
ElementalType OustersWristlet::getElementalType(void) const {
    __BEGIN_TRY

    return g_pOustersWristletInfoManager->getItemInfo(getItemType())->getElementalType();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get Elemental
//--------------------------------------------------------------------------------
Elemental_t OustersWristlet::getElemental(void) const {
    __BEGIN_TRY

    return g_pOustersWristletInfoManager->getItemInfo(getItemType())->getElemental();

    __END_CATCH
}

//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string OustersWristletInfo::toString() const

{
    StringStream msg;

    msg << "OustersWristletInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ",Durability:" << m_Durability << ",minDamage:" << m_MinDamage
        << ",maxDamage:" << m_MaxDamage << ",ReqAbility:" << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void OustersWristletInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_OUSTERS_WRISTLET);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<WeaponInfoElementalRow> rows = defaultItemObjectRepository().loadWeaponInfosElemental(GEAR_OUSTERS_WRISTLET);

    for (size_t r = 0; r < rows.size(); r++) {
        OustersWristletInfo* pOustersWristletInfo = new OustersWristletInfo();

        pOustersWristletInfo->setItemType(rows[r].weapon.itemType);
        pOustersWristletInfo->setName(rows[r].weapon.name);
        pOustersWristletInfo->setEName(rows[r].weapon.ename);
        pOustersWristletInfo->setPrice(rows[r].weapon.price);
        pOustersWristletInfo->setVolumeType(rows[r].weapon.volume);
        pOustersWristletInfo->setWeight(rows[r].weapon.weight);
        pOustersWristletInfo->setRatio(rows[r].weapon.ratio);
        pOustersWristletInfo->setDurability(rows[r].weapon.durability);
        pOustersWristletInfo->setMinDamage(rows[r].weapon.minDamage);
        pOustersWristletInfo->setMaxDamage(rows[r].weapon.maxDamage);
        pOustersWristletInfo->setSpeed(rows[r].weapon.speed);
        pOustersWristletInfo->setReqAbility(rows[r].weapon.reqAbility);
        pOustersWristletInfo->setItemLevel(rows[r].weapon.itemLevel);
        pOustersWristletInfo->setCriticalBonus(rows[r].weapon.criticalBonus);
        pOustersWristletInfo->setDefaultOptions(rows[r].weapon.defaultOption);
        pOustersWristletInfo->setUpgradeRatio(rows[r].weapon.upgradeRatio);
        pOustersWristletInfo->setUpgradeCrashPercent(rows[r].weapon.upgradeCrashPercent);
        pOustersWristletInfo->setNextOptionRatio(rows[r].weapon.nextOptionRatio);
        pOustersWristletInfo->setNextItemType(rows[r].weapon.nextItemType);
        pOustersWristletInfo->setDowngradeRatio(rows[r].weapon.downgradeRatio);
        pOustersWristletInfo->setElementalType((ElementalType)rows[r].elementalType);
        pOustersWristletInfo->setElemental((Elemental_t)rows[r].elemental);

        addItemInfo(pOustersWristletInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void OustersWristletLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<GearObjectRow> rows =
        defaultItemObjectRepository().loadGearOfOwner(GEAR_OUSTERS_WRISTLET, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            OustersWristlet* pOustersWristlet = new OustersWristlet();

            pOustersWristlet->setItemID(rows[r].itemID);
            pOustersWristlet->setObjectID(rows[r].objectID);
            pOustersWristlet->setItemType(rows[r].itemType);

            if (g_pOustersWristletInfoManager->getItemInfo(pOustersWristlet->getItemType())->isUnique())
                pOustersWristlet->setUnique();

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            string optionField = rows[r].optionField;
            list<OptionType_t> optionTypes;
            setOptionTypeFromField(optionTypes, optionField);
            pOustersWristlet->setOptionType(optionTypes);

            pOustersWristlet->setDurability(rows[r].durability);
            pOustersWristlet->setGrade(rows[r].grade);
            pOustersWristlet->setEnchantLevel(rows[r].enchantLevel);
            pOustersWristlet->setCreateType((Item::CreateType)rows[r].createType);

            Inventory* pInventory = NULL;
            Slayer* pSlayer = NULL;
            Vampire* pVampire = NULL;
            Ousters* pOusters = NULL;
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
            } else if (pCreature->isOusters()) {
                pOusters = dynamic_cast<Ousters*>(pCreature);
                pInventory = pOusters->getInventory();
                pStash = pOusters->getStash();
            } else
                throw UnsupportedError("Monster,NPC 인벤토리의 저장은 아직 지원되지 않습니다.");

            switch (storage) {
            case STORAGE_INVENTORY:
                if (pInventory->canAddingEx(x, y, pOustersWristlet)) {
                    pInventory->addItemEx(x, y, pOustersWristlet);
                } else {
                    processItemBugEx(pCreature, pOustersWristlet);
                }
                break;

            case STORAGE_GEAR:
                if (pCreature->isSlayer() || pCreature->isVampire()) {
                    processItemBugEx(pCreature, pOustersWristlet);
                } else if (pCreature->isOusters()) {
                    if (!pOusters->isWear((Ousters::WearPart)x)) {
                        pOusters->wearItem((Ousters::WearPart)x, pOustersWristlet);
                    } else {
                        processItemBugEx(pCreature, pOustersWristlet);
                    }
                }
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pOustersWristlet);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pOustersWristlet);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pOustersWristlet);
                else if (pCreature->isOusters())
                    pOusters->addItemToExtraInventorySlot(pOustersWristlet);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pOustersWristlet);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pOustersWristlet);
                } else {
                    pStash->insert(x, y, pOustersWristlet);
                }
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pOustersWristlet);
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
void OustersWristletLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<GearZoneObjectRow> rows =
        defaultItemObjectRepository().loadGearInZone(GEAR_OUSTERS_WRISTLET, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        OustersWristlet* pOustersWristlet = new OustersWristlet();

        pOustersWristlet->setItemID(rows[r].itemID);
        pOustersWristlet->setObjectID(rows[r].objectID);
        pOustersWristlet->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        string optionField = rows[r].optionField;
        list<OptionType_t> optionTypes;
        setOptionTypeFromField(optionTypes, optionField);
        pOustersWristlet->setOptionType(optionTypes);

        pOustersWristlet->setDurability(rows[r].durability);
        pOustersWristlet->setEnchantLevel(rows[r].enchantLevel);
        pOustersWristlet->setCreateType((Item::CreateType)rows[r].createType);

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pOustersWristlet);
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
void OustersWristletLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

OustersWristletLoader* g_pOustersWristletLoader = NULL;
