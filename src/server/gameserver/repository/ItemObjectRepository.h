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
// and the zone loader's zone SELECT. The object statements' literals differ
// per class only in the table name and in copy-paste whitespace quirks; the
// Info SELECT comes in a handful of column shapes (GearInfoKind). So a family
// shares one method set and selects its table — and its exact literal —
// through an enum; the MySQL impl keeps every class's seven literals
// byte-for-byte.
//
// The first family: the nine slayer gear classes with a Grade column: Ring,
// Bracelet, Necklace, Coat, Trouser, Shoes, Glove, Helm, Shield. The second
// family: the eight vampire and ousters gear classes of the same shape:
// VampireRing, VampireBracelet, VampireNecklace, OustersRing, OustersCoat,
// OustersCirclet, OustersPendent, OustersBoots. The third family: six classes
// whose object statements are gear's but whose Info SELECT is not: VampireCoat
// (16 columns, no UpgradeRatio / DowngradeRatio), OustersStone (20: gear plus
// ElementalType, Elemental), VampireEarring (gear's 18 behind an
// ifnull(MAX(ItemType),0)), VampireWeapon and OustersChakram (20 weapon
// columns: minDamage, maxDamage, Speed, CriticalBonus in place of Defense,
// Protection), OustersWristlet (22: weapon plus ElementalType, Elemental).
// Each Info shape has its own row and loader; the MySQL impl records the
// shape per table and refuses a loader of another shape, so a mismatch
// throws instead of misreading the columns silently (a longer row would
// drop its extra columns; a shorter one would throw getField's
// OutOfBoundException with no hint which table).
// The fourth family, the four silver weapons: Sword, Blade, Cross, Mace —
// gear's INSERT, but a Silver column in the UPDATE (after EnchantLevel,
// before Grade) and in both loads (owner: 13 columns, zone: 12), so a second
// object shape with its own update / loads / rows; their Info SELECT has 21
// columns (weapon plus MaxSilver after maxDamage), Cross's and Mace's 22 (an
// MPBonus before MaxSilver). The spec records the object shape too, and the
// update / load methods refuse a table of the other shape.
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
// Not enclosed: the other 66 item files with SQL (later rounds) and the
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
    GEAR_OUSTERS_BOOTS,
    GEAR_VAMPIRE_COAT,
    GEAR_OUSTERS_STONE,
    GEAR_VAMPIRE_EARRING,
    GEAR_VAMPIRE_WEAPON,
    GEAR_OUSTERS_CHAKRAM,
    GEAR_OUSTERS_WRISTLET,
    GEAR_SWORD,
    GEAR_BLADE,
    GEAR_CROSS,
    GEAR_MACE
};

// The Info SELECT shapes; which loader a table's <Class>Info rows come from.
enum GearInfoKind {
    GEAR_INFO_UNSET = 0,        // a spec row that forgot its kind: every loader refuses it
    GEAR_INFO_STANDARD,         // loadGearInfos
    GEAR_INFO_NO_RATIO,         // loadGearInfosNoRatio (VampireCoat)
    GEAR_INFO_ELEMENTAL,        // loadGearInfosElemental (OustersStone)
    GEAR_INFO_WEAPON,           // loadWeaponInfos (VampireWeapon, OustersChakram)
    GEAR_INFO_WEAPON_ELEMENTAL, // loadWeaponInfosElemental (OustersWristlet)
    GEAR_INFO_SILVER_WEAPON,    // loadSilverWeaponInfos (Sword, Blade)
    GEAR_INFO_SILVER_WEAPON_MP  // loadSilverWeaponMPInfos (Cross, Mace)
};

// The object-table shapes; which update / owner-load / zone-load a table takes.
enum GearObjectKind {
    GEAR_OBJECT_UNSET = 0, // a spec row that forgot its kind: every method refuses it
    GEAR_OBJECT,           // updateGear, loadGearOfOwner, loadGearInZone
    SILVER_WEAPON_OBJECT   // updateSilverWeapon, loadSilverWeaponOfOwner, loadSilverWeaponInZone
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

// VampireCoatInfo: the standard shape without UpgradeRatio and DowngradeRatio.
struct GearInfoNoRatioRow {
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
    int upgradeCrashPercent;
    int nextOptionRatio;
    int nextItemType;
};

// OustersStoneInfo: the standard shape plus ElementalType, Elemental.
struct GearInfoElementalRow {
    GearInfoRow gear;
    int elementalType;
    int elemental;
};

// VampireWeaponInfo / OustersChakramInfo: the twenty weapon columns in SELECT order.
struct WeaponInfoRow {
    int itemType;
    std::string name;
    std::string ename;
    int price;
    int volume;
    int weight;
    int ratio;
    int durability;
    int minDamage;
    int maxDamage;
    int speed;
    std::string reqAbility;
    int itemLevel;
    int criticalBonus;
    std::string defaultOption;
    int upgradeRatio;
    int upgradeCrashPercent;
    int nextOptionRatio;
    int nextItemType;
    int downgradeRatio;
};

// OustersWristletInfo: the weapon shape plus ElementalType, Elemental.
struct WeaponInfoElementalRow {
    WeaponInfoRow weapon;
    int elementalType;
    int elemental;
};

// The silver weapons' owner SELECT: gear's twelve columns plus Silver, in
// the order Durability, EnchantLevel, Silver, Grade, ItemFlag.
struct SilverWeaponObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    std::string optionField;
    int durability;
    int enchantLevel;
    int silver;
    int grade;
    int createType; // ItemFlag
};

// The silver weapons' zone SELECT: gear's eleven columns plus Silver (no Grade).
struct SilverWeaponZoneObjectRow {
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
    int silver;
    int createType; // ItemFlag
};

// SwordInfo / BladeInfo: the weapon shape with MaxSilver after maxDamage (21 columns).
struct SilverWeaponInfoRow {
    int itemType;
    std::string name;
    std::string ename;
    int price;
    int volume;
    int weight;
    int ratio;
    int durability;
    int minDamage;
    int maxDamage;
    int maxSilver;
    int speed;
    std::string reqAbility;
    int itemLevel;
    int criticalBonus;
    std::string defaultOption;
    int upgradeRatio;
    int upgradeCrashPercent;
    int nextOptionRatio;
    int nextItemType;
    int downgradeRatio;
};

// CrossInfo / MaceInfo: the silver-weapon shape with an MPBonus before MaxSilver (22 columns).
struct SilverWeaponMPInfoRow {
    int itemType;
    std::string name;
    std::string ename;
    int price;
    int volume;
    int weight;
    int ratio;
    int durability;
    int minDamage;
    int maxDamage;
    int mpBonus;
    int maxSilver;
    int speed;
    std::string reqAbility;
    int itemLevel;
    int criticalBonus;
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
    // Info table is one NULL row and atoi(NULL) crashed there too; VampireEarring's
    // literal is ifnull(MAX(ItemType),0), so it reads 0 instead), then the rows.
    virtual int loadMaxGearType(GearTable table) = 0;
    virtual std::vector<GearInfoRow> loadGearInfos(GearTable table) = 0;
    // The other Info shapes (see GearInfoKind); each refuses a table of another shape.
    virtual std::vector<GearInfoNoRatioRow> loadGearInfosNoRatio(GearTable table) = 0;
    virtual std::vector<GearInfoElementalRow> loadGearInfosElemental(GearTable table) = 0;
    virtual std::vector<WeaponInfoRow> loadWeaponInfos(GearTable table) = 0;
    virtual std::vector<WeaponInfoElementalRow> loadWeaponInfosElemental(GearTable table) = 0;
    virtual std::vector<SilverWeaponInfoRow> loadSilverWeaponInfos(GearTable table) = 0;
    virtual std::vector<SilverWeaponMPInfoRow> loadSilverWeaponMPInfos(GearTable table) = 0;

    // <Class>Loader::load(Creature*) — the owner's rows in Storage IN(0, 1, 2, 3, 4, 9).
    virtual std::vector<GearObjectRow> loadGearOfOwner(GearTable table, const std::string& ownerName) = 0;
    // <Class>Loader::load(Zone*) — `storage` is what the caller streamed ((int)STORAGE_ZONE).
    virtual std::vector<GearZoneObjectRow> loadGearInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;

    // The silver weapons (see GearObjectKind): <Class>::save with EnchantLevel,
    // Silver, Grade in that order, and the two loads with their Silver column.
    virtual void updateSilverWeapon(GearTable table, ObjectID_t objectID, ItemType_t itemType,
                                    const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                    const std::string& optionField, Durability_t durability, int enchantLevel,
                                    int silver, int grade, ItemID_t itemID) = 0;
    virtual std::vector<SilverWeaponObjectRow> loadSilverWeaponOfOwner(GearTable table,
                                                                       const std::string& ownerName) = 0;
    virtual std::vector<SilverWeaponZoneObjectRow> loadSilverWeaponInZone(GearTable table, int storage,
                                                                          ZoneID_t zoneID) = 0;
};

// The process-wide MySQL-backed instance, wired in MySQLItemObjectRepository.cpp.
// An accessor function rather than a g_p* extern: ratchet R1 counts those.
ItemObjectRepository& defaultItemObjectRepository();

#endif
