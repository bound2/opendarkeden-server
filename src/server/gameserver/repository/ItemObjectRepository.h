#ifndef __ITEM_OBJECT_REPOSITORY_H__
#define __ITEM_OBJECT_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// The per-class item-object tables. Each item class owns an <Class>Object
// table and a <Class>Info table and runs up to seven statements against
// them: the create INSERT, the tinysave "SET %s", the save UPDATE, the info
// manager's MAX(ItemType) and its column SELECT, the creature loader's owner
// SELECT and the zone loader's zone SELECT. Some classes have extra
// statements (the guns' saveBullet UPDATE, Key's Target UPDATE, the couple
// rings' partner count, the destroy() DELETEs, the war items' delete-by-owner
// in place of an owner SELECT, PetItem's with-info variants) and some have
// fewer (a loader with no SQL leaves its slot empty). The object statements
// differ per class only in the table name and in whitespace quirks; the Info
// SELECT comes in a handful of column shapes. So classes of one shape share
// a method set and select their table through the GearTable enum; the MySQL
// impl keeps every class's exact literal.
//
// Each table's spec row records its object shape (GearObjectKind) and its
// Info shape (GearInfoKind). Every shape-checked method refuses a table of
// another shape, so a mismatch throws instead of misreading the columns
// silently (a longer row would drop its extra columns; a shorter one would
// throw getField's OutOfBoundException with no hint which table). A table
// whose loader has no SQL carries no literal for it, and the loads refuse
// such a table rather than formatting a NULL.
//
// Reads are typed to the driver getter used for each column: the owner load
// reads ItemID/ObjectID/ItemType/StorageID through getDWORD, X/Y through
// getBYTE and the rest through getInt; the zone load reads every numeric
// column through getInt (and names no Grade column). Exceptions are noted on
// the row structs. Write parameters are typed to what each caller passes:
// DWORD/WORD through "%u", int through "%d", text as is.
//
// The motorcycle-redeem statements live here too because they are the
// MotorcycleObject table's: an existence probe on a key's Target, a
// four-column read of the row to rebuild the object from (ItemID, ItemType,
// OptionType, Durability) and, when the row is missing, an INSERT of a fresh
// row with an empty OwnerID and OptionType. The read and the insert exist in
// two spellings (the handlers' and the quest action's: "WHERE ItemID=%u"
// versus "where ItemID = %u", "INSERT INTO" versus "INSERT IGNORE INTO"), so
// they take a spelling enum; the probe has one spelling. The callers keep
// their narrowing into ItemID_t / ItemType_t / Durability_t and their
// 300-durability default for a missing row. Key::setNewMotorcycle, which
// the callers make between the probe and the read, is saveKeyTarget through
// the item class.

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
    GEAR_LEARNING_ITEM,
    GEAR_MIXING_ITEM,
    GEAR_PET_FOOD,
    GEAR_KEY,
    GEAR_OUSTERS_SUMMON_ITEM,
    GEAR_SLAYER_PORTAL_ITEM,
    GEAR_MONEY,
    GEAR_COUPLE_RING,
    GEAR_VAMPIRE_COUPLE_RING,
    GEAR_VAMPIRE_PORTAL_ITEM,
    GEAR_VAMPIRE_AMULET,
    GEAR_CORE_ZAP,
    GEAR_BELT,
    GEAR_OUSTERS_ARMSBAND,
    GEAR_MITTEN,
    GEAR_SHOULDER_ARMOR,
    GEAR_PERSONA,
    GEAR_DERMIS,
    GEAR_FASCIA,
    GEAR_CARRYING_RECEIVER,
    GEAR_BLOOD_BIBLE,
    GEAR_CASTLE_SYMBOL,
    GEAR_SWEEPER,
    GEAR_RELIC,
    GEAR_MOTORCYCLE,
    GEAR_CODE_SHEET,
    GEAR_WAR_ITEM,
    GEAR_PET_ITEM
};

// The Info SELECT shapes; which loader a table's <Class>Info rows come from.
enum GearInfoKind {
    GEAR_INFO_UNSET = 0,        // a spec row that forgot its kind: every loader refuses it
    GEAR_INFO_STANDARD,         // loadGearInfos (the gear classes, VampireEarring, VampireAmulet, Mitten,
                                // ShoulderArmor)
    GEAR_INFO_NO_RATIO,         // loadGearInfosNoRatio (VampireCoat, Persona)
    GEAR_INFO_ELEMENTAL,        // loadGearInfosElemental (OustersStone)
    GEAR_INFO_WEAPON,           // loadWeaponInfos (VampireWeapon, OustersChakram)
    GEAR_INFO_WEAPON_ELEMENTAL, // loadWeaponInfosElemental (OustersWristlet)
    GEAR_INFO_SILVER_WEAPON,    // loadSilverWeaponInfos (Sword, Blade)
    GEAR_INFO_SILVER_WEAPON_MP, // loadSilverWeaponMPInfos (Cross, Mace)
    GEAR_INFO_GUN,              // loadGunInfos (AR, SG, SMG, SR)
    GEAR_INFO_BASIC,            // loadBasicInfos (EventItem, EventTree, LuckyBag, MoonCard, ETC, Water, BombMaterial,
                                // EventGiftBox, Money, CoupleRing, VampireCoupleRing, WarItem, PetItem)
    GEAR_INFO_BASIC_FUNCTION,   // loadFunctionInfos (EventETC)
    GEAR_INFO_BASIC_RESURRECT,  // loadResurrectInfos (ResurrectItem)
    GEAR_INFO_BASIC_FUNCTION_VALUE, // loadFunctionValueInfos (DyePotion, EventStar)
    GEAR_INFO_BASIC_EFFECT,         // loadEffectInfos (EffectItem)
    GEAR_INFO_BASIC_FUNCTION_GRADE, // loadFunctionGradeInfos (PetEnchantItem)
    GEAR_INFO_BASIC_STRING,         // loadStringInfos (Serum, VampireETC, Pupa, Larva, ComposMei)
    GEAR_INFO_BASIC_DAMAGE,         // loadDamageInfos (HolyWater)
    GEAR_INFO_MAGAZINE,             // loadMagazineInfos (Magazine)
    GEAR_INFO_BASIC_LEVEL_STRING,   // loadLevelStringInfos (Potion, SlayerPortalItem, VampirePortalItem)
    GEAR_INFO_BASIC_LEVEL,          // loadLevelInfos (Skull)
    GEAR_INFO_BASIC_INT,            // loadIntInfos (QuestItem, SMSItem, LearningItem, CoreZap)
    GEAR_INFO_BASIC_INT_PAIR,       // loadIntPairInfos (SubInventory, TrapItem, Key)
    GEAR_INFO_BASIC_INT_TRIPLE,     // loadIntTripleInfos (PetFood)
    GEAR_INFO_MIXING_ITEM,          // loadMixingItemInfos (MixingItem)
    GEAR_INFO_SUMMON_ITEM,          // loadSummonItemInfos (OustersSummonItem)
    GEAR_INFO_POCKET,               // loadPocketInfos (OustersArmsband: PocketCount, ItemLevel through getInt)
    GEAR_INFO_POCKET_BYTE,          // loadPocketInfos (Belt: PocketCount, ItemLevel through getBYTE)
    GEAR_INFO_NO_DURABILITY,        // loadGearInfosNoDurability (Dermis, Fascia, CarryingReceiver)
    GEAR_INFO_WAR,                  // loadWarInfos (BloodBible, CastleSymbol, Sweeper)
    GEAR_INFO_RELIC,                // loadRelicInfos (Relic)
    GEAR_INFO_DURABILITY,           // loadDurabilityInfos (Motorcycle: the eight head columns alone)
    GEAR_INFO_HEAD                  // loadHeadInfos (CodeSheet: the six-column head alone)
};

// The object-table shapes; which update / owner-load / zone-load a table takes.
enum GearObjectKind {
    GEAR_OBJECT_UNSET = 0, // a spec row that forgot its kind: every shape-checked method refuses it
                           // (loadMaxGearType never consults the kind; tinysaveGear checks it only
                           // to refuse the GUN_OBJECT tables; insertGear refuses every shape but
                           // gear's and the silver weapons', whose INSERT takes its twelve varargs)
    GEAR_OBJECT,           // updateGear, loadGearOfOwner, loadGearInZone (Mitten, ShoulderArmor and Persona
                           // have no zone literal: loadGearInZone refuses them)
    SILVER_WEAPON_OBJECT,  // updateSilverWeapon, loadSilverWeaponOfOwner, loadSilverWeaponInZone
    GUN_OBJECT,    // SG, SMG, SR: insertGun, tinysaveGun, updateGun, saveGunBullet, loadGunOfOwner, loadGunInZone
    AR_GUN_OBJECT, // AR: the same, but tinysaveGear, and the loads name BulletCount, Silver, EnchantLevel
    NUM_OBJECT, // the Num + ItemFlag items: insertNumItem, tinysaveGear, updateNumItem, loadNumItemOfOwner, loadNumItemInZone
    NUM_ONLY_OBJECT, // the Num-only items: insertNumOnlyItem, tinysaveGear, updateNumOnlyItem, loadNumOnlyItemOfOwner, loadNumOnlyItemInZone, destroyItemObject (Pupa, Larva, ComposMei, Potion)
    SKULL_OBJECT, // Skull: the Num-only INSERT, tinysave, UPDATE and owner load, but loadSkullInZone (Num through getDWORD)
    BOMB_OBJECT, // Bomb, BombMaterial, Mine: the same, but loadBombInZone (no Num column in the zone SELECT)
    FLAG_OBJECT, // the ItemFlag-only items: insertFlagItem, tinysaveGear, updatePlainItem, loadFlagItemOfOwner, loadFlagItemInZone
    PLAIN_OBJECT, // the plain items: insertPlainItem, tinysaveGear, updatePlainItem, loadPlainItemOfOwner, loadPlainItemInZone (WarItem has neither literal, so both loads refuse it)
    MIXING_ITEM_OBJECT, // MixingItem: insertNumItem, tinysaveGear, updateNumItem, loadNumIntItemOfOwner, loadNumIntItemInZone
    PET_FOOD_OBJECT, // PetFood: the same, but loadFlagItemInZone (no Num column in the zone SELECT)
    KEY_OBJECT,      // Key: insertKey, tinysaveGear, updateKey, loadKeyOfOwner, loadKeyInZone
    CHARGE_OBJECT, // OustersSummonItem, SlayerPortalItem: insertChargeItem, tinysaveGear, updateChargeItem, loadChargeItemOfOwner, loadChargeItemInZone
    MONEY_OBJECT,  // Money: insertMoney, tinysaveMoney, updateMoney, loadMoneyOfOwner, loadMoneyInZone
    COUPLE_RING_OBJECT, // CoupleRing, VampireCoupleRing: insertCoupleRing, tinysaveGear, updateCoupleRing, loadCoupleRingOfOwner, loadPlainItemInZone, loadCoupleRingPartnerCount
    VAMPIRE_PORTAL_OBJECT, // VampirePortalItem: insertVampirePortal, tinysaveGear, updateVampirePortal, loadVampirePortalOfOwner, loadVampirePortalInZone
    AMULET_OBJECT, // VampireAmulet: insertOptionGradeItem, tinysaveGear, updateAmulet, loadGearOfOwner, loadGearInZone
    CORE_ZAP_OBJECT, // CoreZap: insertOptionGradeItem, tinysaveGear, updateCoreZap, loadCoreZapOfOwner, loadCoreZapInZone
    OPTION_GRADE_OBJECT, // Dermis, Fascia, CarryingReceiver: insertOptionGradeItem, tinysaveGear, updateAmulet, loadOptionGradeOfOwner (no zone load)
    WAR_ITEM_OBJECT, // BloodBible, CastleSymbol, Sweeper, Relic: insertWarItem, tinysaveGear, updateWarItem, deleteWarItemsOfOwner (in place of an owner load), loadWarItemInZone
    MOTORCYCLE_OBJECT, // Motorcycle: insertMotorcycle, tinysaveGear, updateMotorcycle, loadMotorcycleOfOwner, loadMotorcycleInZone
    CODE_SHEET_OBJECT, // CodeSheet: insertCodeSheet, tinysaveGear, updateCodeSheet, loadCodeSheetOfOwner, loadGearInZone (its zone SELECT is gear's)
    PET_ITEM_OBJECT // PetItem: insertPetItem or insertPetItemWithInfo, tinysaveGear, updatePetItem or updatePetItemWithInfo, savePetItemInfo, loadPetItemOfOwner, loadFlagItemInZone (its zone SELECT is the ItemFlag-only eight)
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

// The six columns every Info SELECT starts with (the basic shape without Ratio).
struct HeadInfoRow {
    int itemType;
    std::string name;
    std::string ename;
    int price;
    int volume;
    int weight;
};

// MixingItemInfo: the head plus Target-1, Type-1, SlayerLevel, VampireLevel, OustersLevel
// (the caller keeps its (MixingItemInfo::Target) and (MixingItemInfo::Type) casts).
struct MixingItemInfoRow {
    HeadInfoRow head;
    int target;
    int type;
    int slayerLevel;
    int vampireLevel;
    int oustersLevel;
};

// PetFoodInfo (Target, PetHP, TameRatio): basic plus three ints in SELECT order.
struct IntTripleInfoRow {
    BasicInfoRow basic;
    int first;
    int second;
    int third;
};

// OustersSummonItemInfo: the head plus MaxCharge and Effect (fed to setEffectID).
struct SummonItemInfoRow {
    HeadInfoRow head;
    int maxCharge;
    int effectID;
};

// MixingItem's and PetFood's owner SELECT: the Num + ItemFlag columns, Num through getInt.
struct NumIntObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    int num;
    int createType; // ItemFlag
};

// MixingItem's zone SELECT: the same nine columns, all through getInt.
struct NumIntZoneObjectRow {
    int itemID;
    int objectID;
    int itemType;
    int storage;
    int storageID;
    int x;
    int y;
    int num;
    int createType; // ItemFlag
};

// Key's owner SELECT: the plain columns plus Target (getDWORD).
struct KeyObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    DWORD target;
};

// Key's zone SELECT: the plain columns through getInt, Target still through getDWORD.
struct KeyZoneObjectRow {
    int itemID;
    int objectID;
    int itemType;
    int storage;
    int storageID;
    int x;
    int y;
    DWORD target;
};

// The charge items' owner AND zone SELECT: the plain columns (ids getDWORD, Storage
// getInt, StorageID getDWORD, X, Y getBYTE) plus Charge (getInt) — both loads read
// the same getters, so one row serves both.
struct ChargeObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    int charge;
};

// Money's owner SELECT: the plain columns plus Amount (getDWORD) and Num (getBYTE).
struct MoneyObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    DWORD amount;
    BYTE num;
};

// Money's zone SELECT: the plain columns through getInt plus Amount (getDWORD); no Num.
struct MoneyZoneObjectRow {
    int itemID;
    int objectID;
    int itemType;
    int storage;
    int storageID;
    int x;
    int y;
    DWORD amount;
};

// The couple rings' owner SELECT: the plain columns plus OptionType and Name (getString)
// and PartnerItemID (getDWORD).
struct CoupleRingObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    std::string optionField;
    std::string name;
    DWORD partnerItemID;
};

// VampirePortalItem's owner SELECT — and what its zone load reads, see
// loadVampirePortalInZone: the charge columns plus TargetZID, TargetX, TargetY (getWORD).
struct VampirePortalObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    int charge;
    WORD targetZoneID;
    WORD targetX;
    WORD targetY;
};

// BeltInfo / OustersArmsbandInfo: the standard shape with PocketCount after Protection
// (nineteen columns); pocketCount and itemLevel hold what getBYTE (Belt) or getInt
// (OustersArmsband) returned.
struct PocketInfoRow {
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
    int pocketCount;
    std::string reqAbility;
    int itemLevel;
    std::string defaultOption;
    int upgradeRatio;
    int upgradeCrashPercent;
    int nextOptionRatio;
    int nextItemType;
    int downgradeRatio;
};

// CoreZap's owner SELECT: the plain columns plus OptionType (getString), Grade and ItemFlag (getInt).
struct CoreZapObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    std::string optionField;
    int grade;
    int createType; // ItemFlag
};

// CoreZap's zone SELECT: the plain columns through getInt plus OptionType and ItemFlag (no Grade).
struct CoreZapZoneObjectRow {
    int itemID;
    int objectID;
    int itemType;
    int storage;
    int storageID;
    int x;
    int y;
    std::string optionField;
    int createType; // ItemFlag
};

// Dermis, Fascia and CarryingReceiver's owner SELECT: gear's twelve columns
// without Durability, through gear's getters.
struct OptionGradeObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    std::string optionField;
    int grade;
    int enchantLevel;
    int createType; // ItemFlag
};

// DermisInfo / FasciaInfo / CarryingReceiverInfo: the standard shape without
// Durability (seventeen columns).
struct GearInfoNoDurabilityRow {
    int itemType;
    std::string name;
    std::string ename;
    int price;
    int volume;
    int weight;
    int ratio;
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

// The war items' zone SELECT: nine columns, every one through getInt.
struct WarItemZoneObjectRow {
    int itemID;
    int objectID;
    int itemType;
    int storage;
    int storageID;
    int x;
    int y;
    int durability;
    int enchantLevel;
};

// BloodBibleInfo / CastleSymbolInfo / SweeperInfo: the eight head columns and
// Defense, Protection, ReqAbility, ItemLevel — twelve, with no upgrade tail.
struct WarInfoRow {
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
};

// RelicInfo: those twelve plus RelicType and the four the InfoManager assigns
// to the info's own members (zoneID, x, y, monsterType).
struct RelicInfoRow {
    WarInfoRow war;
    std::string relicType;
    int zoneID;
    int x;
    int y;
    int monsterType;
};

// Motorcycle's owner SELECT: nine columns, gear's getters without Grade,
// EnchantLevel and ItemFlag.
struct MotorcycleObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    std::string optionField;
    int durability;
};

// Motorcycle's zone SELECT: eight columns, every one through getInt (no OptionType).
struct MotorcycleZoneObjectRow {
    int itemID;
    int objectID;
    int itemType;
    int storage;
    int storageID;
    int x;
    int y;
    int durability;
};

// CodeSheet's owner SELECT: the plain columns plus OptionType (getString).
struct CodeSheetObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    std::string optionField;
};

// The motorcycle-redeem read: the four columns a parked motorcycle is rebuilt
// from (getInt for the numbers, getString for OptionType, which the callers
// parse into option types themselves).
struct MotorcycleRedeemRow {
    int itemID;
    int itemType;
    std::string optionField;
    int durability;
};

// Which text the redeem read and insert send: the handlers' ("WHERE
// ItemID=%u"; "INSERT INTO") or the quest action's ("where ItemID = %u";
// "INSERT IGNORE INTO").
enum MotorcycleRedeemSpelling { REDEEM_SPELLING_HANDLER, REDEEM_SPELLING_QUEST_ACTION, REDEEM_SPELLING_MAX };

// MotorcycleInfo: the eight head columns alone.
struct DurabilityInfoRow {
    int itemType;
    std::string name;
    std::string ename;
    int price;
    int volume;
    int weight;
    int ratio;
    int durability;
};

// PetItem's owner SELECT: the eight ItemFlag-only columns and the pet's thirteen
// (eleven read through getInt, LastFeedTime and Nickname as text).
struct PetItemObjectRow {
    DWORD itemID;
    DWORD objectID;
    DWORD itemType;
    int storage;
    DWORD storageID;
    BYTE x;
    BYTE y;
    int createType; // ItemFlag
    int petCreatureType;
    int petLevel;
    int petExp;
    int petHP;
    int petAttr;
    int petAttrLevel;
    int petOption;
    int foodType;
    int canGamble;
    int canCutHead;
    int canAttack;
    std::string lastFeedTime;
    std::string nickname;
};

// EventShutdown's egg-dummy-DB teardown seeds one sentinel row in each
// of three object tables. These are NOT the named-column inserts the
// rest of this repository carries: they are POSITIONAL, name no columns,
// and quote every value including the numeric ones, so they depend on
// each table's exact column order and count. The id is INT_MAX in both
// the ItemID and ObjectID positions — a sentinel, not a real object.
//
// That path does not compile into the shipped gameserver: it sits
// behind the "#else" of a
// "#if !defined(__THAILAND_SERVER__) && !defined(__CHINA_SERVER__)",
// and the build defines neither.
enum DummyObjectTable { DUMMY_OBJECT_LARVA, DUMMY_OBJECT_SKULL, DUMMY_OBJECT_POTION, DUMMY_OBJECT_TABLE_MAX };

class ItemObjectRepository {
public:
    virtual ~ItemObjectRepository() {}

    // EventShutdown's egg-dummy-DB teardown; see DummyObjectTable above
    // for why these three are positional and why they never compile into
    // the shipped gameserver.
    virtual void insertDummySentinelRow(DummyObjectTable table) = 0;

    // <Class>::create — the INSERT with the ItemFlag column fed the create type.
    // Refuses tables whose INSERT takes other arguments (the guns' thirteen, the
    // Num + ItemFlag items' ten, the Num-only items' nine).
    virtual void insertGear(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                            const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                            const std::string& optionField, Durability_t durability, int grade, int createType) = 0;
    // <Class>::tinysave — "SET %s": the caller's field text is the statement.
    // Refuses the GUN_OBJECT tables, whose tinysave literal takes a BulletCount too,
    // and MONEY_OBJECT, whose takes an Amount.
    virtual void tinysaveGear(GearTable table, const char* field, ItemID_t itemID) = 0;
    // <Class>::save.
    virtual void updateGear(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                            int storage, StorageID_t storageID, int x, int y, const std::string& optionField,
                            Durability_t durability, int grade, int enchantLevel, ItemID_t itemID) = 0;

    // <Class>InfoManager::load — MAX(ItemType) through getInt (an empty Info
    // table is one NULL row, and atoi(NULL) crashes; VampireEarring's literal is
    // ifnull(MAX(ItemType),0), so it reads 0 instead), then the rows.
    virtual int loadMaxGearType(GearTable table) = 0;
    virtual std::vector<GearInfoRow> loadGearInfos(GearTable table) = 0;
    // The other Info shapes (see GearInfoKind); each refuses a table of another shape.
    virtual std::vector<GearInfoNoRatioRow> loadGearInfosNoRatio(GearTable table) = 0;
    virtual std::vector<GearInfoNoDurabilityRow> loadGearInfosNoDurability(GearTable table) = 0;
    virtual std::vector<WarInfoRow> loadWarInfos(GearTable table) = 0;
    virtual std::vector<RelicInfoRow> loadRelicInfos(GearTable table) = 0;
    virtual std::vector<DurabilityInfoRow> loadDurabilityInfos(GearTable table) = 0;
    virtual std::vector<HeadInfoRow> loadHeadInfos(GearTable table) = 0;
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
    virtual std::vector<IntTripleInfoRow> loadIntTripleInfos(GearTable table) = 0;
    virtual std::vector<MixingItemInfoRow> loadMixingItemInfos(GearTable table) = 0;
    virtual std::vector<SummonItemInfoRow> loadSummonItemInfos(GearTable table) = 0;
    virtual std::vector<PocketInfoRow> loadPocketInfos(GearTable table) = 0; // both pocket kinds

    // <Class>Loader::load(Creature*) — the owner's rows in Storage IN(0, 1, 2, 3, 4, 9).
    // Both gear loads serve the AMULET_OBJECT table too: its SELECTs are gear's.
    virtual std::vector<GearObjectRow> loadGearOfOwner(GearTable table, const std::string& ownerName) = 0;
    // <Class>Loader::load(Zone*) — `storage` is the caller's (int)STORAGE_ZONE.
    // Serves CodeSheet too: its zone SELECT is gear's eleven columns. Refuses the
    // gear tables that carry no zone literal because their zone loader holds no SQL
    // (Mitten, ShoulderArmor, Persona); other shapes it refuses anyway.
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
    // The INSERT and UPDATE serve the MIXING_ITEM_OBJECT and PET_FOOD_OBJECT tables
    // too; the two loads take NUM_OBJECT alone.
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
    // a caught DB error). Refuses tables without the literal.
    virtual bool destroyItemObject(GearTable table, const std::string& objectTableName, ItemID_t itemID) = 0;
    // Skull's zone load (Num through getDWORD) and the Bomb tables' (no Num column).
    virtual std::vector<SkullZoneObjectRow> loadSkullInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;
    virtual std::vector<BombZoneObjectRow> loadBombInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;

    // The ItemFlag-only and plain items (see GearObjectKind): <Class>::create with
    // or without the ItemFlag column, one seven-argument UPDATE for both, tinysave
    // gear's, and each shape's two loads (loadFlagItemInZone serves the
    // PET_FOOD_OBJECT table too: its zone SELECT is the flag shape; loadPlainItemInZone
    // the COUPLE_RING_OBJECT tables: theirs is the plain shape).
    virtual void insertFlagItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                                const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                int createType) = 0;
    virtual void insertPlainItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                                 const std::string& ownerID, int storage, StorageID_t storageID, int x, int y) = 0;
    virtual void updatePlainItem(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                                 int storage, StorageID_t storageID, int x, int y, ItemID_t itemID) = 0;
    virtual std::vector<FlagObjectRow> loadFlagItemOfOwner(GearTable table, const std::string& ownerName) = 0;
    // Serves PetFood and PetItem too: their zone SELECTs name the same eight columns.
    virtual std::vector<FlagZoneObjectRow> loadFlagItemInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;
    // insertPlainItemLogged returns the statement it ran: WarItem's create logs it to
    // WarLog.txt. The other plain tables use insertPlainItem.
    virtual std::string insertPlainItemLogged(GearTable table, ItemID_t itemID, ObjectID_t objectID,
                                              ItemType_t itemType, const std::string& ownerID, int storage,
                                              StorageID_t storageID, int x, int y) = 0;
    // Both plain loads refuse a table without the literal (WarItem, whose three
    // Loader::load overloads hold no SQL).
    virtual std::vector<PlainObjectRow> loadPlainItemOfOwner(GearTable table, const std::string& ownerName) = 0;
    virtual std::vector<PlainZoneObjectRow> loadPlainItemInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;

    // MixingItem and PetFood (see GearObjectKind): the Num + ItemFlag writes, but
    // the loads read Num through getInt; MixingItem's zone load is its own, PetFood's
    // is loadFlagItemInZone.
    virtual std::vector<NumIntObjectRow> loadNumIntItemOfOwner(GearTable table, const std::string& ownerName) = 0;
    virtual std::vector<NumIntZoneObjectRow> loadNumIntItemInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;

    // Key (see GearObjectKind): the plain columns plus Target — an ItemID_t, "%u" in
    // the INSERT, "%d" in the UPDATE.
    virtual void insertKey(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                           const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                           ItemID_t target) = 0;
    virtual void updateKey(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                           int storage, StorageID_t storageID, int x, int y, ItemID_t target, ItemID_t itemID) = 0;
    virtual std::vector<KeyObjectRow> loadKeyOfOwner(GearTable table, const std::string& ownerName) = 0;
    virtual std::vector<KeyZoneObjectRow> loadKeyInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;
    // Key::setNewMotorcycle — "UPDATE KeyObject SET Target=%u WHERE ItemID=%u" with the
    // new motorcycle's id. Refuses other tables.
    virtual void saveKeyTarget(GearTable table, ItemID_t targetID, ItemID_t itemID) = 0;

    // OustersSummonItem and SlayerPortalItem (see GearObjectKind): the plain columns
    // plus Charge (an int); one row for both loads.
    virtual void insertChargeItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                                  const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                  int charge) = 0;
    virtual void updateChargeItem(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                                  int storage, StorageID_t storageID, int x, int y, int charge, ItemID_t itemID) = 0;
    virtual std::vector<ChargeObjectRow> loadChargeItemOfOwner(GearTable table, const std::string& ownerName) = 0;
    virtual std::vector<ChargeObjectRow> loadChargeItemInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;

    // Money (see GearObjectKind): the plain columns plus Amount (a DWORD) and Num;
    // tinysaveMoney is "SET %s, Amount=%u".
    virtual void insertMoney(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                             const std::string& ownerID, int storage, StorageID_t storageID, int x, int y, DWORD amount,
                             int num) = 0;
    virtual void tinysaveMoney(GearTable table, const char* field, DWORD amount, ItemID_t itemID) = 0;
    virtual void updateMoney(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                             int storage, StorageID_t storageID, int x, int y, DWORD amount, int num,
                             ItemID_t itemID) = 0;
    virtual std::vector<MoneyObjectRow> loadMoneyOfOwner(GearTable table, const std::string& ownerName) = 0;
    virtual std::vector<MoneyZoneObjectRow> loadMoneyInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;

    // The couple rings (see GearObjectKind): the plain columns plus OptionType, Name
    // and PartnerItemID in the INSERT, Name and PartnerItemID in the UPDATE; the
    // owner load; and hasPartnerItem's count(*) — true with the count when a row
    // came back, false otherwise. The zone load is loadPlainItemInZone.
    virtual void insertCoupleRing(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                                  const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                  const std::string& optionField, const std::string& name, ItemID_t partnerItemID) = 0;
    virtual void updateCoupleRing(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                                  int storage, StorageID_t storageID, int x, int y, const std::string& name,
                                  ItemID_t partnerItemID, ItemID_t itemID) = 0;
    virtual std::vector<CoupleRingObjectRow> loadCoupleRingOfOwner(GearTable table, const std::string& ownerName) = 0;
    virtual bool loadCoupleRingPartnerCount(GearTable table, ItemID_t partnerItemID, int& count) = 0;

    // VampirePortalItem (see GearObjectKind): the charge columns plus the target
    // zone and coordinates (the callers cast their WORDs (int)); one row for both
    // loads. loadVampirePortalInZone reads the row's eleven getters over an
    // eight-column zone SELECT: with any row present it throws
    // OutOfBoundException from the ninth getter.
    virtual void insertVampirePortal(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                                     const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                     int charge, int targetZoneID, int targetX, int targetY) = 0;
    virtual void updateVampirePortal(GearTable table, ObjectID_t objectID, ItemType_t itemType,
                                     const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                     int charge, int targetZoneID, int targetX, int targetY, ItemID_t itemID) = 0;
    virtual std::vector<VampirePortalObjectRow> loadVampirePortalOfOwner(GearTable table,
                                                                         const std::string& ownerName) = 0;
    virtual std::vector<VampirePortalObjectRow> loadVampirePortalInZone(GearTable table, int storage,
                                                                        ZoneID_t zoneID) = 0;

    // VampireAmulet, CoreZap, Dermis, Fascia and CarryingReceiver (see
    // GearObjectKind): the gear INSERT without Durability, VampireAmulet's UPDATE
    // with Grade and EnchantLevel — Dermis's, Fascia's and CarryingReceiver's too —
    // CoreZap's with Grade alone and its two loads, and the owner load the three
    // OPTION_GRADE_OBJECT tables share.
    virtual void insertOptionGradeItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                                       const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                       const std::string& optionField, int grade, int createType) = 0;
    virtual void updateAmulet(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                              int storage, StorageID_t storageID, int x, int y, const std::string& optionField,
                              int grade, int enchantLevel, ItemID_t itemID) = 0;
    virtual void updateCoreZap(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                               int storage, StorageID_t storageID, int x, int y, const std::string& optionField,
                               int grade, ItemID_t itemID) = 0;
    virtual std::vector<CoreZapObjectRow> loadCoreZapOfOwner(GearTable table, const std::string& ownerName) = 0;
    virtual std::vector<CoreZapZoneObjectRow> loadCoreZapInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;
    virtual std::vector<OptionGradeObjectRow> loadOptionGradeOfOwner(GearTable table, const std::string& ownerName) = 0;
    // PetItem (see GearObjectKind): create and save each run one of two statements
    // (with or without the pet's columns), and savePetInfo writes the pet columns
    // alone. The ids and PetExp are unsigned; the byte- and word-wide pet fields
    // are promoted to int.
    virtual void insertPetItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                               const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                               int createType) = 0;
    virtual void insertPetItemWithInfo(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                                       const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                       int createType, int petCreatureType, int petLevel, DWORD petExp, int petHP,
                                       int petAttr, int petAttrLevel, int petOption, int foodType, int canGamble,
                                       int canCutHead, int canAttack, const std::string& lastFeedTime) = 0;
    virtual void updatePetItem(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                               int storage, StorageID_t storageID, int x, int y, ItemID_t itemID) = 0;
    virtual void updatePetItemWithInfo(GearTable table, ObjectID_t objectID, ItemType_t itemType,
                                       const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                       int petCreatureType, int petLevel, int petAttr, int petAttrLevel, DWORD petExp,
                                       int petHP, int foodType, int canGamble, int canCutHead, int canAttack,
                                       const std::string& lastFeedTime, const std::string& nickname,
                                       ItemID_t itemID) = 0;
    virtual void savePetItemInfo(GearTable table, int petCreatureType, int petLevel, int petAttr, int petAttrLevel,
                                 DWORD petExp, int petHP, int foodType, int canGamble, int canCutHead, int canAttack,
                                 const std::string& lastFeedTime, const std::string& nickname, ItemID_t itemID) = 0;
    virtual std::vector<PetItemObjectRow> loadPetItemOfOwner(GearTable table, const std::string& ownerName) = 0;

    // Motorcycle (see GearObjectKind): the gear INSERT and UPDATE without Grade and
    // ItemFlag, an owner load of nine columns and a zone load of eight.
    virtual void insertMotorcycle(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                                  const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                  const std::string& optionField, Durability_t durability) = 0;
    virtual void updateMotorcycle(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                                  int storage, StorageID_t storageID, int x, int y, const std::string& optionField,
                                  Durability_t durability, ItemID_t itemID) = 0;
    virtual std::vector<MotorcycleObjectRow> loadMotorcycleOfOwner(GearTable table, const std::string& ownerName) = 0;
    virtual std::vector<MotorcycleZoneObjectRow> loadMotorcycleInZone(GearTable table, int storage,
                                                                      ZoneID_t zoneID) = 0;
    // The motorcycle-redeem statements (see the header comment). These name
    // MotorcycleObject alone, so they take no table.
    // "SELECT ItemID FROM MotorcycleObject WHERE ItemID=%u" — true when a row
    // came back.
    virtual bool motorcycleExists(ItemID_t itemID) = 0;
    // The four-column read; true and the row when it exists, false otherwise.
    virtual bool loadMotorcycleForRedeem(MotorcycleRedeemSpelling spelling, ItemID_t itemID,
                                         MotorcycleRedeemRow& row) = 0;
    // The fresh-row INSERT (empty OwnerID and OptionType in the literal). Every
    // value goes through "%d", the three DWORDs included.
    virtual void insertRedeemedMotorcycle(MotorcycleRedeemSpelling spelling, ItemID_t itemID, ObjectID_t objectID,
                                          ItemType_t itemType, int storage, ZoneID_t zoneID, int x, int y,
                                          Durability_t durability) = 0;

    // CodeSheet (see GearObjectKind): the plain INSERT and UPDATE plus OptionType and
    // an owner load of eight columns; its zone SELECT is gear's, so loadGearInZone
    // serves it while loadGearOfOwner refuses it.
    virtual void insertCodeSheet(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                                 const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                 const std::string& optionField) = 0;
    virtual void updateCodeSheet(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                                 int storage, StorageID_t storageID, int x, int y, const std::string& optionField,
                                 ItemID_t itemID) = 0;
    virtual std::vector<CodeSheetObjectRow> loadCodeSheetOfOwner(GearTable table, const std::string& ownerName) = 0;

    // The war items (see GearObjectKind): a nine-column INSERT with Durability last
    // and no OptionType, Grade or ItemFlag, a nine-column UPDATE, the DELETE their
    // creature loader runs in place of an owner SELECT, and a nine-column zone load.
    // insertWarItem returns the statement it ran: BloodBible, CastleSymbol and
    // Sweeper log it to WarLog.txt.
    virtual std::string insertWarItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                                      const std::string& ownerID, int storage, StorageID_t storageID, int x, int y,
                                      Durability_t durability) = 0;
    virtual void updateWarItem(GearTable table, ObjectID_t objectID, ItemType_t itemType, const std::string& ownerID,
                               int storage, StorageID_t storageID, int x, int y, Durability_t durability,
                               int enchantLevel, ItemID_t itemID) = 0;
    virtual void deleteWarItemsOfOwner(GearTable table, const std::string& ownerName) = 0;
    virtual std::vector<WarItemZoneObjectRow> loadWarItemInZone(GearTable table, int storage, ZoneID_t zoneID) = 0;

    // Belt::destroy and OustersArmsband::destroy — "DELETE FROM <Class>Object WHERE
    // ItemID = %u"; false when no row went, true otherwise. Refuses tables without
    // the literal.
    virtual bool destroyGearObject(GearTable table, ItemID_t itemID) = 0;
};

// The process-wide MySQL-backed instance, wired in MySQLItemObjectRepository.cpp.
ItemObjectRepository& defaultItemObjectRepository();

#endif
