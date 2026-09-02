#ifndef __ITEM_OBJECT_REPOSITORY_H__
#define __ITEM_OBJECT_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Persistence seam for the per-class item-object tables (task 3.2, the item
// milestone): each item class owns an <Class>Object table and a <Class>Info
// table and runs the same seven statements against them — the create
// INSERT, the tinysave "SET %s", the save UPDATE, the info manager's
// MAX(ItemType) and its column SELECT, the creature loader's owner SELECT
// and the zone loader's zone SELECT. The literals differ per class only in the table
// name and in copy-paste whitespace quirks, so a family shares one method
// set and selects its table — and its exact literal — through an enum;
// the MySQL impl keeps every class's seven literals byte-for-byte.
//
// The first family: the nine slayer gear classes with a Grade column: Ring,
// Bracelet, Necklace, Coat, Trouser, Shoes, Glove, Helm, Shield. The second family:
// the eight vampire and ousters gear classes of the same shape: VampireRing,
// VampireBracelet, VampireNecklace, OustersRing, OustersCoat, OustersCirclet,
// OustersPendent, OustersBoots. (VampireCoat's Info SELECT has 16 columns,
// OustersStone's 20, VampireEarring guards an ifnull(MAX) — later rounds.)
//
// Reads are typed to the driver getter the inline code called: the owner
// load read ItemID/ObjectID/ItemType/StorageID through getDWORD, X/Y
// through getBYTE and the rest through getInt; the zone load read every
// numeric column through getInt (and the INSERT-built zone SELECT names no
// Grade column — it never did). The info load is getInt/getString per
// column. Write parameters are typed to what each caller streamed, so the
// varargs bytes are unchanged: the create INSERT was a StringStream chain
// (DWORD/WORD through "%u", int through "%d"), the save UPDATE and tinysave
// keep their "%ld" for the DWORD ids exactly as written.
//
// Not enclosed: the other 72 item files with SQL (later rounds) and the
// loaders' storage-placement logic (stays with the class). ItemInfoManager.cpp
// holds only the registry calls, no SQL.

enum GearTable {
    GEAR_RING,
    GEAR_BRACELET,
    GEAR_NECKLACE,
    GEAR_COAT,
    GEAR_TROUSER,
    GEAR_SHOES,
    GEAR_GLOVE,
    GEAR_HELM,
    GEAR_SHIELD,
    GEAR_VAMPIRE_RING,
    GEAR_VAMPIRE_BRACELET,
    GEAR_VAMPIRE_NECKLACE,
    GEAR_OUSTERS_RING,
    GEAR_OUSTERS_COAT,
    GEAR_OUSTERS_CIRCLET,
    GEAR_OUSTERS_PENDENT,
    GEAR_OUSTERS_BOOTS
};

// <Class>Loader::load(Creature*): the owner SELECT's twelve columns.
struct GearObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    std::string optionField;
    int durability;
    int grade;
    int enchantLevel;
    int createType; // ItemFlag
};

// <Class>Loader::load(Zone*): the zone SELECT's eleven columns (no Grade).
struct GearZoneObjectRow {
    int itemID;
    int objectID;
    int itemType;
    int storage;
    int storageID;
    int x;
    int y;
    std::string optionField;
    int durability;
    int enchantLevel;
    int createType; // ItemFlag
};

// <Class>InfoManager::load: the eighteen <Class>Info columns in SELECT order.
struct GearInfoRow {
    int itemType;
    std::string name;
    std::string ename;
    int price;
    int volume;
    int weight;
    int ratio;
    int durability;
    int defense;
    int protection;
    std::string reqAbility;
    int itemLevel;
    std::string defaultOption;
    int upgradeRatio;
    int upgradeCrashPercent;
    int nextOptionRatio;
    int nextItemType;
    int downgradeRatio;
};

class ItemObjectRepository {
public:
    virtual ~ItemObjectRepository() {}

    // <Class>::create — the INSERT with the ItemFlag column fed the create type.
    virtual void insertGear(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                            const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                            const std::string& optionField, Durability_t durability, int grade, int createType) = 0;
    // <Class>::tinysave — "SET %s": the caller's field text is the statement.
    virtual void tinysaveGear(GearTable table, const char* field, ItemID_t itemID) = 0;
    // <Class>::save.
    virtual void updateGear(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                            int storage, StorageID_t storageID, int x, int y, const std::string& optionField,
                            Durability_t durability, int grade, int enchantLevel, ItemID_t itemID) = 0;

    // <Class>InfoManager::load — MAX(ItemType) (getInt, as before: an empty
    // Info table is one NULL row and atoi(NULL) crashed there too), then the rows.
    virtual int loadMaxGearType(GearTable table) = 0;
    virtual std::vector<GearInfoRow> loadGearInfos(GearTable table) = 0;

    // <Class>Loader::load(Creature*) — the owner's rows in Storage IN(0, 1, 2, 3, 4, 9).
    virtual std::vector<GearObjectRow> loadGearOfOwner(GearTable table, const std::string& ownerName) = 0;
    // <Class>Loader::load(Zone*) — `storage` is what the caller streamed ((int)STORAGE_ZONE).
    virtual std::vector<GearZoneObjectRow> loadGearInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;
};

// The process-wide MySQL-backed instance, wired in MySQLItemObjectRepository.cpp.
// An accessor function rather than a g_p* extern: ratchet R1 counts those.
ItemObjectRepository& defaultItemObjectRepository();

#endif
