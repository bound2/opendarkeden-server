//////////////////////////////////////////////////////////////////////////////
// Filename    : Bomb.cpp
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Bomb.h"

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
BombInfoManager* g_pBombInfoManager = NULL;

ItemID_t Bomb::m_ItemIDRegistry = 0;
Mutex Bomb::m_Mutex;

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
Bomb::Bomb()

    : m_ItemType(0), m_Damage(0) //, m_Dir(0)
{
    m_Num = 1;
}

Bomb::Bomb(ItemType_t itemType, const list<OptionType_t>& optionType)

    : m_ItemType(itemType) {
    if (!g_pItemInfoManager->isPossibleItem(getItemClass(), m_ItemType, optionType)) {
        filelog("itembug.log", "Bomb::Bomb() : Invalid item type or option type");
        throw("Bomb::Bomb() : Invalid item type or optionType");
    }
    m_Num = 1;
    m_Damage = 0;
}


//--------------------------------------------------------------------------------
// create item
//--------------------------------------------------------------------------------
void Bomb::create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y, ItemID_t itemID)

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

    defaultItemObjectRepository().insertNumOnlyItem(GEAR_BOMB, m_ItemID, m_ObjectID, m_ItemType, ownerID, (int)storage,
                                                    storageID, (int)x, (int)y, (int)m_Num);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Bomb::tinysave(const char* field) const

{
    __BEGIN_TRY

    defaultItemObjectRepository().tinysaveGear(GEAR_BOMB, field, m_ItemID);

    __END_CATCH
}

//--------------------------------------------------------------------------------
// save item
//--------------------------------------------------------------------------------
void Bomb::save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y)

{
    __BEGIN_TRY

    defaultItemObjectRepository().updateNumOnlyItem(GEAR_BOMB, m_ObjectID, m_ItemType, ownerID, (int)storage, storageID,
                                                    (int)x, (int)y, (int)m_Num, m_ItemID);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string Bomb::toString() const

{
    StringStream msg;

    msg << "Bomb(" << "ItemID:" << m_ItemID << ",ItemType:" << m_ItemType << ",Num:" << (int)m_Num << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// get width
//--------------------------------------------------------------------------------
VolumeWidth_t Bomb::getVolumeWidth() const

{
    __BEGIN_TRY

    return g_pBombInfoManager->getItemInfo(m_ItemType)->getVolumeWidth();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get height
//--------------------------------------------------------------------------------
VolumeHeight_t Bomb::getVolumeHeight() const

{
    __BEGIN_TRY

    return g_pBombInfoManager->getItemInfo(m_ItemType)->getVolumeHeight();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get weight
//--------------------------------------------------------------------------------
Weight_t Bomb::getWeight() const

{
    __BEGIN_TRY

    return g_pBombInfoManager->getItemInfo(m_ItemType)->getWeight();

    __END_CATCH
}

Damage_t Bomb::getMinDamage() const

{
    __BEGIN_TRY

    return g_pBombInfoManager->getItemInfo(m_ItemType)->getMinDamage();

    __END_CATCH
}

Damage_t Bomb::getMaxDamage() const

{
    __BEGIN_TRY

    return g_pBombInfoManager->getItemInfo(m_ItemType)->getMaxDamage();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string BombInfo::toString() const

{
    StringStream msg;

    msg << "BombInfo(" << "ItemType:" << m_ItemType << ",Name:" << m_Name << ",EName:" << m_EName
        << ",Price:" << m_Price << ",VolumeType:" << Volume2String[m_VolumeType] << ",Weight:" << m_Weight
        << ",Description:" << m_Description << ")";

    return msg.toString();
}


//--------------------------------------------------------------------------------
// load from DB
//--------------------------------------------------------------------------------
void BombInfoManager::load()

{
    __BEGIN_TRY

    m_InfoCount = defaultItemObjectRepository().loadMaxGearType(GEAR_BOMB);

    m_pItemInfos = new ItemInfo*[m_InfoCount + 1];

    for (uint i = 0; i <= m_InfoCount; i++)
        m_pItemInfos[i] = NULL;

    vector<DamageInfoRow> rows = defaultItemObjectRepository().loadDamageInfos(GEAR_BOMB);

    for (size_t r = 0; r < rows.size(); r++) {
        BombInfo* pBombInfo = new BombInfo();

        pBombInfo->setItemType(rows[r].basic.itemType);
        pBombInfo->setName(rows[r].basic.name);
        pBombInfo->setEName(rows[r].basic.ename);
        pBombInfo->setPrice(rows[r].basic.price);
        pBombInfo->setVolumeType(rows[r].basic.volume);
        pBombInfo->setWeight(rows[r].basic.weight);
        pBombInfo->setRatio(rows[r].basic.ratio);
        pBombInfo->setMinDamage(rows[r].minDamage);
        pBombInfo->setMaxDamage(rows[r].maxDamage);

        addItemInfo(pBombInfo);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// load to creature
//--------------------------------------------------------------------------------
void BombLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<NumOnlyObjectRow> rows =
        defaultItemObjectRepository().loadNumOnlyItemOfOwner(GEAR_BOMB, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        try {
            Bomb* pBomb = new Bomb();

            pBomb->setItemID(rows[r].itemID);
            pBomb->setObjectID(rows[r].objectID);
            pBomb->setItemType(rows[r].itemType);

            Storage storage = (Storage)rows[r].storage;
            StorageID_t storageID = rows[r].storageID;
            BYTE x = rows[r].x;
            BYTE y = rows[r].y;

            pBomb->setNum(rows[r].num);

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
                if (pInventory->canAddingEx(x, y, pBomb)) {
                    pInventory->addItemEx(x, y, pBomb);
                } else {
                    processItemBugEx(pCreature, pBomb);
                }
                break;

            case STORAGE_GEAR:
                processItemBugEx(pCreature, pBomb);
                break;

            case STORAGE_BELT:
                processItemBugEx(pCreature, pBomb);
                break;

            case STORAGE_EXTRASLOT:
                if (pCreature->isSlayer())
                    pSlayer->addItemToExtraInventorySlot(pBomb);
                else if (pCreature->isVampire())
                    pVampire->addItemToExtraInventorySlot(pBomb);
                break;

            case STORAGE_MOTORCYCLE:
                processItemBugEx(pCreature, pBomb);
                break;

            case STORAGE_STASH:
                if (pStash->isExist(x, y)) {
                    processItemBugEx(pCreature, pBomb);
                } else
                    pStash->insert(x, y, pBomb);
                break;

            case STORAGE_GARBAGE:
                processItemBug(pCreature, pBomb);
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
void BombLoader::load(Zone* pZone)

{
    __BEGIN_TRY

    Assert(pZone != NULL);

    vector<BombZoneObjectRow> rows =
        defaultItemObjectRepository().loadBombInZone(GEAR_BOMB, (int)STORAGE_ZONE, pZone->getZoneID());

    for (size_t r = 0; r < rows.size(); r++) {
        Bomb* pBomb = new Bomb();

        pBomb->setItemID(rows[r].itemID);
        pBomb->setObjectID(rows[r].objectID);
        pBomb->setItemType(rows[r].itemType);

        Storage storage = (Storage)rows[r].storage;
        StorageID_t storageID = rows[r].storageID;
        BYTE x = rows[r].x;
        BYTE y = rows[r].y;

        switch (storage) {
        case STORAGE_ZONE: {
            Tile& pTile = pZone->getTile(x, y);
            Assert(!pTile.hasItem());
            pTile.addItem(pBomb);
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
void BombLoader::load(StorageID_t storageID, Inventory* pInventory)

{
    __BEGIN_TRY

    Statement* pStmt;

    BEGIN_DB {}
    END_DB(pStmt)

    __END_CATCH
}

BombLoader* g_pBombLoader = NULL;
