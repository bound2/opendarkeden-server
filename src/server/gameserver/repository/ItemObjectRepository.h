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
// byte-for-byte (the guns' eight: they add a saveBullet UPDATE; four Num-only
// items' nine: they override destroy() with a DELETE).
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
// The fifth family, the four guns: AR, SG, SMG, SR — a BulletCount column in
// the INSERT (before Grade), in the UPDATE (between EnchantLevel and Silver)
// and in both loads (owner: 14 columns, zone: 13), plus an eighth statement,
// the saveBullet UPDATE. SG, SMG and SR are one shape (GUN_OBJECT: their
// tinysave writes BulletCount too, and the loads name EnchantLevel,
// BulletCount, Silver); AR is another (AR_GUN_OBJECT: gear's tinysave, and
// the loads name BulletCount, Silver, EnchantLevel). The loaders read each
// column at its table's ordinal into the field it names. Their Info SELECT
// has 22 columns: the weapon shape with ToHitBonus and `Range` after
// maxDamage.
// The sixth family, ten "Num + ItemFlag" items: EventItem, EventTree, LuckyBag,
// MoonCard, EventETC, ResurrectItem, DyePotion, EventStar, EffectItem,
// PetEnchantItem — no OptionType, Durability, Grade or EnchantLevel anywhere;
// a Num column (a BYTE, cast (int) into the INSERT and UPDATE, read through
// getBYTE in both loads) and ItemFlag. Owner and zone SELECT: nine columns.
// Their Info SELECT is the seven-column basic shape (the head without
// Durability) alone or with one or two class-specific columns after Ratio:
// `Function`; ResurrectType; FunctionFlag, FunctionValue; EffectClass,
// TimeSec; `Function`, FunctionGrade.
// The seventh family, four "Num-only" items: ETC, Serum, VampireETC, Water —
// the sixth family's INSERT and loads without their ItemFlag column, its
// UPDATE unchanged: nine columns in the INSERT, eight in the UPDATE and eight
// in both loads; no create type anywhere (ETCObject's table still has an
// ItemFlag column, left at its default); tinysave is gear's. Their
// Info SELECT is the basic shape alone (ETC, Water) or with one varchar
// column after Ratio (Serum's SerumEffect, fed to parseEffect; VampireETC's
// ReqAbility).
// The eighth family, six more Num-only items whose create was already a
// parameterized statement (verbatim): HolyWater, Magazine, Pupa, Larva,
// ComposMei, Potion — the same object shape. Four of them (Pupa, Larva,
// ComposMei, Potion) override destroy() with a DELETE naming the table as a
// %s: a ninth literal, destroyItemObject. Their Info SELECT adds to the basic
// head: minDamage, maxDamage (HolyWater); ItemLevel, MaxBullets,
// MaxSilverBullets, Vivid, GunType-1 (Magazine); Effect, fed to parseEffect
// (Pupa, Larva, ComposMei — the string shape); ItemLevel, Effect (Potion).
// The ninth family, four Num-only items whose zone SELECT differs: Skull reads
// Num through getDWORD (SKULL_OBJECT); Bomb, BombMaterial and Mine name no Num
// column in it at all (BOMB_OBJECT, seven columns). Their INSERT, UPDATE,
// tinysave and owner load are the Num-only ones (Skull's create was already
// parameterized; the other three stream). Info: basic plus ItemLevel (Skull);
// minDamage, maxDamage (Bomb, Mine); basic alone (BombMaterial).
// The tenth family: four "ItemFlag-only" items — QuestItem, SMSItem, SubInventory,
// TrapItem (FLAG_OBJECT: ids, Storage, StorageID, X, Y and ItemFlag; the UPDATE
// writes no ItemFlag) — and two "plain" ones without even that column,
// EventGiftBox and LearningItem (PLAIN_OBJECT: seven columns everywhere); both
// share the seven-argument UPDATE. Info: basic plus one int column (BonusRatio,
// Charge, SkillType) or two (Width, Height; `Function`, Parameter), each fed to
// the class's own setters; EventGiftBox is basic alone.
//
// Reads are typed to the driver getter the inline code called: the owner
// load read ItemID/ObjectID/ItemType/StorageID through getDWORD, X/Y
// through getBYTE and the rest through getInt; the zone load read every
// numeric column through getInt (and the INSERT-built zone SELECT names no
// Grade column — it never did; the Num and Num-only families read Num through
// getBYTE in both loads, see the sixth and seventh families). The info load is
// getInt/getString per
// column. Write parameters are typed to what each caller streamed, so the
// varargs bytes are unchanged: the create INSERT was a StringStream chain
// (DWORD/WORD through "%u", int through "%d"; AR's was already a
// parameterized statement and is verbatim), the save UPDATE and tinysave
// keep their "%ld" for the DWORD ids exactly as written.
//
// Not enclosed: the other 28 item files with SQL (later rounds) and the
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
    GEAR_MACE,
    GEAR_AR,
    GEAR_SG,
    GEAR_SMG,
    GEAR_SR,
    GEAR_EVENT_ITEM,
    GEAR_EVENT_TREE,
    GEAR_LUCKY_BAG,
    GEAR_MOON_CARD,
    GEAR_EVENT_ETC,
    GEAR_RESURRECT_ITEM,
    GEAR_DYE_POTION,
    GEAR_EVENT_STAR,
    GEAR_EFFECT_ITEM,
    GEAR_PET_ENCHANT_ITEM,
    GEAR_ETC,
    GEAR_SERUM,
    GEAR_VAMPIRE_ETC,
    GEAR_WATER,
    GEAR_HOLY_WATER,
    GEAR_MAGAZINE,
    GEAR_PUPA,
    GEAR_LARVA,
    GEAR_COMPOS_MEI,
    GEAR_POTION,
    GEAR_SKULL,
    GEAR_BOMB,
    GEAR_BOMB_MATERIAL,
    GEAR_MINE,
    GEAR_QUEST_ITEM,
    GEAR_SMSITEM,
    GEAR_SUB_INVENTORY,
    GEAR_TRAP_ITEM,
    GEAR_EVENT_GIFT_BOX,
    GEAR_LEARNING_ITEM
};

// The Info SELECT shapes; which loader a table's <Class>Info rows come from.
enum GearInfoKind {
    GEAR_INFO_UNSET = 0,            // a spec row that forgot its kind: every loader refuses it
    GEAR_INFO_STANDARD,             // loadGearInfos
    GEAR_INFO_NO_RATIO,             // loadGearInfosNoRatio (VampireCoat)
    GEAR_INFO_ELEMENTAL,            // loadGearInfosElemental (OustersStone)
    GEAR_INFO_WEAPON,               // loadWeaponInfos (VampireWeapon, OustersChakram)
    GEAR_INFO_WEAPON_ELEMENTAL,     // loadWeaponInfosElemental (OustersWristlet)
    GEAR_INFO_SILVER_WEAPON,        // loadSilverWeaponInfos (Sword, Blade)
    GEAR_INFO_SILVER_WEAPON_MP,     // loadSilverWeaponMPInfos (Cross, Mace)
    GEAR_INFO_GUN,                  // loadGunInfos (AR, SG, SMG, SR)
    GEAR_INFO_BASIC,                // loadBasicInfos (EventItem, EventTree, LuckyBag, MoonCard, ETC, Water)
    GEAR_INFO_BASIC_FUNCTION,       // loadFunctionInfos (EventETC)
    GEAR_INFO_BASIC_RESURRECT,      // loadResurrectInfos (ResurrectItem)
    GEAR_INFO_BASIC_FUNCTION_VALUE, // loadFunctionValueInfos (DyePotion, EventStar)
    GEAR_INFO_BASIC_EFFECT,         // loadEffectInfos (EffectItem)
    GEAR_INFO_BASIC_FUNCTION_GRADE, // loadFunctionGradeInfos (PetEnchantItem)
    GEAR_INFO_BASIC_STRING,         // loadStringInfos (Serum, VampireETC, Pupa, Larva, ComposMei)
    GEAR_INFO_BASIC_DAMAGE,         // loadDamageInfos (HolyWater)
    GEAR_INFO_MAGAZINE,             // loadMagazineInfos (Magazine)
    GEAR_INFO_BASIC_LEVEL_STRING,   // loadLevelStringInfos (Potion)
    GEAR_INFO_BASIC_LEVEL,          // loadLevelInfos (Skull)
    GEAR_INFO_BASIC_INT,            // loadIntInfos (QuestItem, SMSItem, LearningItem)
    GEAR_INFO_BASIC_INT_PAIR        // loadIntPairInfos (SubInventory, TrapItem)
};

// The object-table shapes; which update / owner-load / zone-load a table takes.
enum GearObjectKind {
    GEAR_OBJECT_UNSET = 0, // a spec row that forgot its kind: every shape-checked method refuses it
                           // (loadMaxGearType never consults the kind; tinysaveGear checks it only
                           // to refuse the GUN_OBJECT tables; insertGear refuses every shape but
                           // gear's and the silver weapons', whose INSERT takes its twelve varargs)
    GEAR_OBJECT,           // updateGear, loadGearOfOwner, loadGearInZone
    SILVER_WEAPON_OBJECT,  // updateSilverWeapon, loadSilverWeaponOfOwner, loadSilverWeaponInZone
    GUN_OBJECT,    // SG, SMG, SR: insertGun, tinysaveGun, updateGun, saveGunBullet, loadGunOfOwner, loadGunInZone
    AR_GUN_OBJECT, // AR: the same, but tinysaveGear, and the loads name BulletCount, Silver, EnchantLevel
    NUM_OBJECT, // the Num + ItemFlag items: insertNumItem, tinysaveGear, updateNumItem, loadNumItemOfOwner, loadNumItemInZone
    NUM_ONLY_OBJECT, // the Num-only items: insertNumOnlyItem, tinysaveGear, updateNumOnlyItem, loadNumOnlyItemOfOwner, loadNumOnlyItemInZone, destroyItemObject (Pupa, Larva, ComposMei, Potion)
    SKULL_OBJECT, // Skull: the Num-only INSERT, tinysave, UPDATE and owner load, but loadSkullInZone (Num through getDWORD)
    BOMB_OBJECT, // Bomb, BombMaterial, Mine: the same, but loadBombInZone (no Num column in the zone SELECT)
    FLAG_OBJECT, // the ItemFlag-only items: insertFlagItem, tinysaveGear, updatePlainItem, loadFlagItemOfOwner, loadFlagItemInZone
    PLAIN_OBJECT // the plain items: insertPlainItem, tinysaveGear, updatePlainItem, loadPlainItemOfOwner, loadPlainItemInZone
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

// The guns' owner SELECT: gear's twelve columns plus BulletCount and Silver
// (14). SG, SMG and SR name them EnchantLevel, BulletCount, Silver; AR names
// BulletCount, Silver, EnchantLevel — the loader reads in the table's order,
// so each value lands in the field its column names.
struct GunObjectRow {
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
    int bulletCount;
    int silver;
    int grade;
    int createType; // ItemFlag
};

// The guns' zone SELECT: gear's eleven columns plus BulletCount and Silver (13, no Grade), same two orders.
struct GunZoneObjectRow {
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
    int bulletCount;
    int silver;
    int createType; // ItemFlag
};

// ARInfo / SGInfo / SMGInfo / SRInfo: the weapon shape with ToHitBonus and `Range` after maxDamage (22 columns).
struct GunInfoRow {
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
    int toHitBonus;
    int range;
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

// The Num + ItemFlag items' owner SELECT: nine columns — the ids (getDWORD),
// Storage (getInt), StorageID (getDWORD), X, Y and Num (getBYTE), ItemFlag (getInt).
struct NumObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    BYTE num;
    int createType; // ItemFlag
};

// Their zone SELECT: the same nine columns, everything but Num through getInt.
struct NumZoneObjectRow {
    int itemID;
    int objectID;
    int itemType;
    int storage;
    int storageID;
    int x;
    int y;
    BYTE num;
    int createType; // ItemFlag
};

// The basic Info shape: seven columns, the head without Durability
// (EventItemInfo, EventTreeInfo, LuckyBagInfo, MoonCardInfo).
struct BasicInfoRow {
    int itemType;
    std::string name;
    std::string ename;
    int price;
    int volume;
    int weight;
    int ratio;
};

// EventETCInfo: basic plus `Function`.
struct FunctionInfoRow {
    BasicInfoRow basic;
    int function;
};

// ResurrectItemInfo: basic plus ResurrectType.
struct ResurrectInfoRow {
    BasicInfoRow basic;
    int resurrectType;
};

// DyePotionInfo, EventStarInfo: basic plus FunctionFlag, FunctionValue.
struct FunctionValueInfoRow {
    BasicInfoRow basic;
    int functionFlag;
    int functionValue;
};

// EffectItemInfo: basic plus EffectClass, TimeSec (TimeSec feeds setDuration).
struct EffectInfoRow {
    BasicInfoRow basic;
    int effectClass;
    int timeSec;
};

// PetEnchantItemInfo: basic plus `Function`, FunctionGrade.
struct FunctionGradeInfoRow {
    BasicInfoRow basic;
    int function;
    int functionGrade;
};

// SerumInfo, VampireETCInfo: basic plus one varchar column (SerumEffect, ReqAbility).
struct StringInfoRow {
    BasicInfoRow basic;
    std::string value;
};

// The Num-only items' owner SELECT: the Num + ItemFlag columns without ItemFlag
// (eight) — the ids (getDWORD), Storage (getInt), StorageID (getDWORD), X, Y and Num (getBYTE).
struct NumOnlyObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    BYTE num;
};

// Their zone SELECT: the same eight columns, everything but Num through getInt.
struct NumOnlyZoneObjectRow {
    int itemID;
    int objectID;
    int itemType;
    int storage;
    int storageID;
    int x;
    int y;
    BYTE num;
};

// HolyWaterInfo: basic plus minDamage, maxDamage.
struct DamageInfoRow {
    BasicInfoRow basic;
    int minDamage;
    int maxDamage;
};

// MagazineInfo: basic plus ItemLevel, MaxBullets, MaxSilverBullets, Vivid, GunType-1
// (the caller keeps its `!= 0` on Vivid and its (MagazineInfo::GunType) cast).
struct MagazineInfoRow {
    BasicInfoRow basic;
    int itemLevel;
    int maxBullets;
    int maxSilverBullets;
    int vivid;
    int gunType;
};

// PotionInfo: basic plus ItemLevel and the Effect varchar (fed to parseEffect).
struct LevelStringInfoRow {
    BasicInfoRow basic;
    int itemLevel;
    std::string value;
};

// SkullInfo: basic plus ItemLevel.
struct LevelInfoRow {
    BasicInfoRow basic;
    int itemLevel;
};

// Skull's zone SELECT: the Num-only eight, Num through getDWORD.
struct SkullZoneObjectRow {
    int itemID;
    int objectID;
    int itemType;
    int storage;
    int storageID;
    int x;
    int y;
    DWORD num;
};

// Bomb, BombMaterial, Mine: their zone SELECT names no Num column (seven, all getInt).
struct BombZoneObjectRow {
    int itemID;
    int objectID;
    int itemType;
    int storage;
    int storageID;
    int x;
    int y;
};

// QuestItemInfo (BonusRatio), SMSItemInfo (Charge), LearningItemInfo (SkillType): basic plus one int.
struct IntInfoRow {
    BasicInfoRow basic;
    int value;
};

// SubInventoryInfo (Width, Height), TrapItemInfo (`Function`, Parameter): basic plus two ints in SELECT order.
struct IntPairInfoRow {
    BasicInfoRow basic;
    int first;
    int second;
};

// The ItemFlag-only items' owner SELECT: the ids (getDWORD), Storage (getInt),
// StorageID (getDWORD), X, Y (getBYTE), ItemFlag (getInt).
struct FlagObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    int createType; // ItemFlag
};

// Their zone SELECT: the same eight columns through getInt.
struct FlagZoneObjectRow {
    int itemID;
    int objectID;
    int itemType;
    int storage;
    int storageID;
    int x;
    int y;
    int createType; // ItemFlag
};

// The plain items' owner SELECT: the ItemFlag-only columns without ItemFlag (seven).
struct PlainObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
};

// Their zone SELECT: the same seven columns through getInt.
struct PlainZoneObjectRow {
    int itemID;
    int objectID;
    int itemType;
    int storage;
    int storageID;
    int x;
    int y;
};

class ItemObjectRepository {
public:
    virtual ~ItemObjectRepository() {}

    // <Class>::create — the INSERT with the ItemFlag column fed the create type.
    // Refuses tables whose INSERT takes other arguments (the guns' thirteen, the
    // Num + ItemFlag items' ten, the Num-only items' nine).
    virtual void insertGear(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                            const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                            const std::string& optionField, Durability_t durability, int grade, int createType) = 0;
    // <Class>::tinysave — "SET %s": the caller's field text is the statement.
    // Refuses the GUN_OBJECT tables, whose tinysave literal takes a BulletCount too.
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
    virtual std::vector<GunInfoRow> loadGunInfos(GearTable table) = 0;
    virtual std::vector<BasicInfoRow> loadBasicInfos(GearTable table) = 0;
    virtual std::vector<FunctionInfoRow> loadFunctionInfos(GearTable table) = 0;
    virtual std::vector<ResurrectInfoRow> loadResurrectInfos(GearTable table) = 0;
    virtual std::vector<FunctionValueInfoRow> loadFunctionValueInfos(GearTable table) = 0;
    virtual std::vector<EffectInfoRow> loadEffectInfos(GearTable table) = 0;
    virtual std::vector<FunctionGradeInfoRow> loadFunctionGradeInfos(GearTable table) = 0;
    virtual std::vector<StringInfoRow> loadStringInfos(GearTable table) = 0;
    virtual std::vector<DamageInfoRow> loadDamageInfos(GearTable table) = 0;
    virtual std::vector<MagazineInfoRow> loadMagazineInfos(GearTable table) = 0;
    virtual std::vector<LevelStringInfoRow> loadLevelStringInfos(GearTable table) = 0;
    virtual std::vector<LevelInfoRow> loadLevelInfos(GearTable table) = 0;
    virtual std::vector<IntInfoRow> loadIntInfos(GearTable table) = 0;
    virtual std::vector<IntPairInfoRow> loadIntPairInfos(GearTable table) = 0;

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

    // The guns (see GearObjectKind; every method takes both gun shapes unless
    // noted): <Class>::create with BulletCount before Grade; SG / SMG / SR's
    // tinysave "SET %s, BulletCount=%d" (GUN_OBJECT only; AR's is gear's);
    // <Class>::save with EnchantLevel, BulletCount, Silver, Grade in that order;
    // <Class>::saveBullet (the caller passed its BYTE BulletCount uncast); and
    // the two loads with their BulletCount and Silver columns.
    virtual void insertGun(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                           const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                           const std::string& optionField, Durability_t durability, int bulletCount, int grade,
                           int createType) = 0;
    virtual void tinysaveGun(GearTable table, const char* field, int bulletCount, ItemID_t itemID) = 0;
    virtual void updateGun(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                           int storage, StorageID_t storageID, int x, int y, const std::string& optionField,
                           Durability_t durability, int enchantLevel, int bulletCount, int silver, int grade,
                           ItemID_t itemID) = 0;
    virtual void saveGunBullet(GearTable table, BYTE bulletCount, ItemID_t itemID) = 0;
    virtual std::vector<GunObjectRow> loadGunOfOwner(GearTable table, const std::string& ownerName) = 0;
    virtual std::vector<GunZoneObjectRow> loadGunInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;

    // The Num + ItemFlag items (see GearObjectKind): <Class>::create and save
    // with Num (the callers cast their BYTE (int)) and no OptionType, Durability,
    // Grade or EnchantLevel; tinysave is gear's; the two loads read Num as a BYTE.
    virtual void insertNumItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                               const std::string& ownerID, int storage, StorageID_t storageID, int x, int y, int num,
                               int createType) = 0;
    virtual void updateNumItem(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                               int storage, StorageID_t storageID, int x, int y, int num, ItemID_t itemID) = 0;
    virtual std::vector<NumObjectRow> loadNumItemOfOwner(GearTable table, const std::string& ownerName) = 0;
    virtual std::vector<NumZoneObjectRow> loadNumItemInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;

    // The Num-only items (see GearObjectKind): the Num + ItemFlag methods without
    // the create type; tinysave is gear's; the two loads read Num as a BYTE. The
    // INSERT, UPDATE and owner load serve the SKULL_OBJECT and BOMB_OBJECT tables
    // too; each zone load takes exactly its own kind.
    virtual void insertNumOnlyItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                                   const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                   int num) = 0;
    virtual void updateNumOnlyItem(GearTable table, ObjectID_t objectID, ItemType_t itemType,
                                   const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                   int num, ItemID_t itemID) = 0;
    virtual std::vector<NumOnlyObjectRow> loadNumOnlyItemOfOwner(GearTable table, const std::string& ownerName) = 0;
    virtual std::vector<NumOnlyZoneObjectRow> loadNumOnlyItemInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;
    // <Class>::destroy of Pupa, Larva, ComposMei and Potion — "DELETE FROM %s" with the
    // class's object table name; false when no row went, true otherwise (also after
    // a caught DB error, as the original fell through). Refuses tables without the literal.
    virtual bool destroyItemObject(GearTable table, const std::string& objectTableName, ItemID_t itemID) = 0;
    // Skull's zone load (Num through getDWORD) and the Bomb tables' (no Num column).
    virtual std::vector<SkullZoneObjectRow> loadSkullInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;
    virtual std::vector<BombZoneObjectRow> loadBombInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;

    // The ItemFlag-only and plain items (see GearObjectKind): <Class>::create with
    // or without the ItemFlag column, one seven-argument UPDATE for both, tinysave
    // gear's, and each shape's two loads.
    virtual void insertFlagItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                                const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                int createType) = 0;
    virtual void insertPlainItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                                 const std::string& ownerID, int storage, StorageID_t storageID, int x, int y) = 0;
    virtual void updatePlainItem(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                                 int storage, StorageID_t storageID, int x, int y, ItemID_t itemID) = 0;
    virtual std::vector<FlagObjectRow> loadFlagItemOfOwner(GearTable table, const std::string& ownerName) = 0;
    virtual std::vector<FlagZoneObjectRow> loadFlagItemInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;
    virtual std::vector<PlainObjectRow> loadPlainItemOfOwner(GearTable table, const std::string& ownerName) = 0;
    virtual std::vector<PlainZoneObjectRow> loadPlainItemInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;
};

// The process-wide MySQL-backed instance, wired in MySQLItemObjectRepository.cpp.
// An accessor function rather than a g_p* extern: ratchet R1 counts those.
ItemObjectRepository& defaultItemObjectRepository();

#endif
