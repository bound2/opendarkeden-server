// MySQL-backed ItemObjectRepository (task 3.2, the item milestone). One
// method set per object shape; the table — and the class's exact literal,
// copy-paste whitespace and all — comes from the spec row the GearTable
// enum indexes. The two StringStream chains of the originals (the create
// INSERT and the zone SELECT) are format strings here; every streamed
// expression maps to the conversion StringStream used for its type
// (DWORD/WORD "%u", int "%d", text as is), so the bytes on the wire are
// the same. The tinysave and save literals keep their "%ld" for the DWORD
// ids exactly as written; AR's create INSERT was already a parameterized
// statement and is verbatim. The guns carry an eighth literal, the
// saveBullet UPDATE, and four Num-only items (Pupa, Larva, ComposMei, Potion)
// a ninth, their destroy() DELETE, and Key a tenth, setNewMotorcycle's Target
// UPDATE (NULL for every other table). The spec row also
// records which object shape and which Info shape the class's tables
// have; every loader checks them, so a call with the wrong loader fails
// loudly instead of misreading the columns silently. GEAR_INFO_UNSET and
// GEAR_OBJECT_UNSET are their enums' zeros, so a spec row that forgets a
// kind is refused by every shape-checked method rather than read as the
// standard shape (tinysaveGear consults the kind only to refuse the
// GUN_OBJECT tables; loadMaxGearType never consults it).

#include <string>
#include <vector>

#include "DB.h"
#include "repository/ItemObjectRepository.h"

using namespace std;

namespace {

struct GearSpec {
    const char* insert;        // <Class>::create
    const char* tinysave;      // <Class>::tinysave
    const char* update;        // <Class>::save
    const char* maxType;       // <Class>InfoManager::load, first statement
    const char* infos;         // <Class>InfoManager::load, second statement
    const char* ofOwner;       // <Class>Loader::load(Creature*)
    const char* inZone;        // <Class>Loader::load(Zone*)
    const char* saveBullet;    // <Class>::saveBullet — the guns only; NULL for the other tables
    const char* destroy;       // <Class>::destroy — Pupa, Larva, ComposMei, Potion only; NULL for the other tables
    const char* saveTarget;    // Key::setNewMotorcycle — Key only; NULL for the other tables
    GearInfoKind infoKind;     // which load*Infos reads `infos`
    GearObjectKind objectKind; // which update / load*OfOwner / load*InZone fit the object table
};

// Indexed by GearTable; the order is the enum's.
const GearSpec kGear[] = {
    // Ring (GEAR_RING)
    {
        "INSERT INTO RingObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE RingObject SET %s WHERE ItemID=%ld",
        "UPDATE RingObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM RingInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM RingInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM RingObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM RingObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // Bracelet (GEAR_BRACELET)
    {
        "INSERT INTO BraceletObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE BraceletObject SET %s WHERE ItemID=%ld",
        "UPDATE BraceletObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM BraceletInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, "
        "ReqAbility,ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, "
        "DowngradeRatio FROM BraceletInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM BraceletObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM BraceletObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // Necklace (GEAR_NECKLACE)
    {
        "INSERT INTO NecklaceObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE NecklaceObject SET %s WHERE ItemID=%ld",
        "UPDATE NecklaceObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM NecklaceInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM NecklaceInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y,OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM NecklaceObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM NecklaceObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // Coat (GEAR_COAT)
    {
        "INSERT INTO CoatObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u,  %d, %d)",
        "UPDATE CoatObject SET %s WHERE ItemID=%ld",
        "UPDATE CoatObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM CoatInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM CoatInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM CoatObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM CoatObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // Trouser (GEAR_TROUSER)
    {
        "INSERT INTO TrouserObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE TrouserObject SET %s WHERE ItemID=%ld",
        "UPDATE TrouserObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM TrouserInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM TrouserInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y,OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM TrouserObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM TrouserObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // Shoes (GEAR_SHOES)
    {
        "INSERT INTO ShoesObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE ShoesObject SET %s WHERE ItemID=%ld",
        "UPDATE ShoesObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM ShoesInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM ShoesInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y,OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM ShoesObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM ShoesObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // Glove (GEAR_GLOVE)
    {
        "INSERT INTO GloveObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE GloveObject SET %s WHERE ItemID=%ld",
        "UPDATE GloveObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel = %d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM GloveInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM GloveInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM GloveObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM GloveObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // Helm (GEAR_HELM)
    {
        "INSERT INTO HelmObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE HelmObject SET %s WHERE ItemID=%ld",
        "UPDATE HelmObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM HelmInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM HelmInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM HelmObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM HelmObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // Shield (GEAR_SHIELD)
    {
        "INSERT INTO ShieldObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE ShieldObject SET %s WHERE ItemID=%ld",
        "UPDATE ShieldObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM ShieldInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM ShieldInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM ShieldObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM ShieldObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // VampireRing (GEAR_VAMPIRE_RING)
    {
        "INSERT INTO VampireRingObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE VampireRingObject SET %s WHERE ItemID=%ld",
        "UPDATE VampireRingObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM VampireRingInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM VampireRingInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y,OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM VampireRingObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM VampireRingObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // VampireBracelet (GEAR_VAMPIRE_BRACELET)
    {
        "INSERT INTO VampireBraceletObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, "
        "OptionType, Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE VampireBraceletObject SET %s WHERE ItemID=%ld",
        "UPDATE VampireBraceletObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, "
        "Y=%d, OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM VampireBraceletInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM VampireBraceletInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM VampireBraceletObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM VampireBraceletObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // VampireNecklace (GEAR_VAMPIRE_NECKLACE)
    {
        "INSERT INTO VampireNecklaceObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, "
        "OptionType, Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE VampireNecklaceObject SET %s WHERE ItemID=%ld",
        "UPDATE VampireNecklaceObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, "
        "Y=%d, OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM VampireNecklaceInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM VampireNecklaceInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y,OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM VampireNecklaceObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM VampireNecklaceObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // OustersRing (GEAR_OUSTERS_RING)
    {
        "INSERT INTO OustersRingObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE OustersRingObject SET %s WHERE ItemID=%ld",
        "UPDATE OustersRingObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM OustersRingInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM OustersRingInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM OustersRingObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM OustersRingObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // OustersCoat (GEAR_OUSTERS_COAT)
    {
        "INSERT INTO OustersCoatObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE OustersCoatObject SET %s WHERE ItemID=%ld",
        "UPDATE OustersCoatObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM OustersCoatInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM OustersCoatInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM OustersCoatObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM OustersCoatObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // OustersCirclet (GEAR_OUSTERS_CIRCLET)
    {
        "INSERT INTO OustersCircletObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, "
        "OptionType, Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE OustersCircletObject SET %s WHERE ItemID=%ld",
        "UPDATE OustersCircletObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, "
        "Y=%d, OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM OustersCircletInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM OustersCircletInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM OustersCircletObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM OustersCircletObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // OustersPendent (GEAR_OUSTERS_PENDENT)
    {
        "INSERT INTO OustersPendentObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, "
        "OptionType, Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE OustersPendentObject SET %s WHERE ItemID=%ld",
        "UPDATE OustersPendentObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, "
        "Y=%d, OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM OustersPendentInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM OustersPendentInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM OustersPendentObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM OustersPendentObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // OustersBoots (GEAR_OUSTERS_BOOTS)
    {
        "INSERT INTO OustersBootsObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE OustersBootsObject SET %s WHERE ItemID=%ld",
        "UPDATE OustersBootsObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM OustersBootsInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM OustersBootsInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM OustersBootsObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM OustersBootsObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // VampireCoat (GEAR_VAMPIRE_COAT)
    {
        "INSERT INTO VampireCoatObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE VampireCoatObject SET %s WHERE ItemID=%ld",
        "UPDATE VampireCoatObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM VampireCoatInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeCrashPercent, NextOptionRatio, NextItemType FROM VampireCoatInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM VampireCoatObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM VampireCoatObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_NO_RATIO,
        GEAR_OBJECT,
    },
    // OustersStone (GEAR_OUSTERS_STONE)
    {
        "INSERT INTO OustersStoneObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE OustersStoneObject SET %s WHERE ItemID=%ld",
        "UPDATE OustersStoneObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM OustersStoneInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio, "
        "ElementalType, Elemental FROM OustersStoneInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM OustersStoneObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM OustersStoneObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_ELEMENTAL,
        GEAR_OBJECT,
    },
    // VampireEarring (GEAR_VAMPIRE_EARRING)
    {
        "INSERT INTO VampireEarringObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, "
        "OptionType, Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE VampireEarringObject SET %s WHERE ItemID=%ld",
        "UPDATE VampireEarringObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, "
        "Y=%d, OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT ifnull(MAX(ItemType),0) FROM VampireEarringInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, Defense, Protection, ReqAbility, "
        "ItemLevel, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, DowngradeRatio "
        "FROM VampireEarringInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y,OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM VampireEarringObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM VampireEarringObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_STANDARD,
        GEAR_OBJECT,
    },
    // VampireWeapon (GEAR_VAMPIRE_WEAPON)
    {
        "INSERT INTO VampireWeaponObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE VampireWeaponObject SET %s WHERE ItemID=%ld",
        "UPDATE VampireWeaponObject SET ObjectID=%ld, ItemType=%d, OwnerID= '%s', Storage=%d, StorageID=%ld, X=%d, "
        "Y=%d, OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM VampireWeaponInfo",
        "SELECT "
        "ItemType,Name,EName,Price,Volume,Weight,Ratio,Durability,minDamage,maxDamage,Speed,ReqAbility,ItemLevel, "
        "CriticalBonus, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, "
        "DowngradeRatio FROM VampireWeaponInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM VampireWeaponObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM VampireWeaponObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_WEAPON,
        GEAR_OBJECT,
    },
    // OustersChakram (GEAR_OUSTERS_CHAKRAM)
    {
        "INSERT INTO OustersChakramObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, "
        "OptionType, Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE OustersChakramObject SET %s WHERE ItemID=%ld",
        "UPDATE OustersChakramObject SET ObjectID=%ld, ItemType=%d, OwnerID= '%s', Storage=%d, StorageID=%ld, X=%d, "
        "Y=%d, OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM OustersChakramInfo",
        "SELECT "
        "ItemType,Name,EName,Price,Volume,Weight,Ratio,Durability,minDamage,maxDamage,Speed,ReqAbility,ItemLevel, "
        "CriticalBonus, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, "
        "DowngradeRatio FROM OustersChakramInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade, EnchantLevel, "
        "ItemFlag FROM OustersChakramObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM OustersChakramObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_WEAPON,
        GEAR_OBJECT,
    },
    // OustersWristlet (GEAR_OUSTERS_WRISTLET)
    {
        "INSERT INTO OustersWristletObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, "
        "OptionType, Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE OustersWristletObject SET %s WHERE ItemID=%ld",
        "UPDATE OustersWristletObject SET ObjectID=%ld, ItemType=%d, OwnerID= '%s', Storage=%d, StorageID=%ld, X=%d, "
        "Y=%d, OptionType='%s', Durability=%d, Grade=%d, EnchantLevel=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM OustersWristletInfo",
        "SELECT "
        "ItemType,Name,EName,Price,Volume,Weight,Ratio,Durability,minDamage,maxDamage,Speed,ReqAbility,ItemLevel, "
        "CriticalBonus, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, "
        "DowngradeRatio, ElementalType, Elemental FROM OustersWristletInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, Grade,EnchantLevel, "
        "ItemFlag FROM OustersWristletObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, ItemFlag "
        "FROM OustersWristletObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_WEAPON_ELEMENTAL,
        GEAR_OBJECT,
    },
    // Sword (GEAR_SWORD)
    {
        "INSERT INTO SwordObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE SwordObject SET %s WHERE ItemID=%ld",
        "UPDATE SwordObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, EnchantLevel=%d, Silver=%d, Grade=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM SwordInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, minDamage, maxDamage, MaxSilver, "
        "Speed, ReqAbility, ItemLevel, CriticalBonus, DefaultOption, UpgradeRatio, UpgradeCrashPercent, "
        "NextOptionRatio, NextItemType, DowngradeRatio FROM SwordInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y,OptionType, Durability, EnchantLevel, Silver, "
        "Grade, ItemFlag FROM SwordObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, Silver, "
        "ItemFlag FROM SwordObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_SILVER_WEAPON,
        SILVER_WEAPON_OBJECT,
    },
    // Blade (GEAR_BLADE)
    {
        "INSERT INTO BladeObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE BladeObject SET %s WHERE ItemID=%ld",
        "UPDATE BladeObject SET ObjectID=%ld, ItemType=%d, OwnerID= '%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, EnchantLevel=%d, Silver=%d, Grade=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM BladeInfo",
        "SELECT "
        "ItemType,Name,EName,Price,Volume,Weight,Ratio,Durability,minDamage,maxDamage,MaxSilver,Speed,ReqAbility,"
        "ItemLevel, CriticalBonus, DefaultOption, UpgradeRatio, UpgradeCrashPercent, NextOptionRatio, NextItemType, "
        "DowngradeRatio FROM BladeInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, Silver, "
        "Grade, ItemFlag FROM BladeObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, Silver, "
        "ItemFlag FROM BladeObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_SILVER_WEAPON,
        SILVER_WEAPON_OBJECT,
    },
    // Cross (GEAR_CROSS)
    {
        "INSERT INTO CrossObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE CrossObject SET %s WHERE ItemID=%ld",
        "UPDATE CrossObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%d, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, EnchantLevel=%d, Silver=%d, Grade=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM CrossInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, minDamage, maxDamage, MPBonus, "
        "MaxSilver, Speed, ReqAbility, ItemLevel, CriticalBonus, DefaultOption, UpgradeRatio, UpgradeCrashPercent, "
        "NextOptionRatio, NextItemType, DowngradeRatio FROM CrossInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, Silver, "
        "Grade, ItemFlag FROM CrossObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, Silver, "
        "ItemFlag FROM CrossObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_SILVER_WEAPON_MP,
        SILVER_WEAPON_OBJECT,
    },
    // Mace (GEAR_MACE)
    {
        "INSERT INTO MaceObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d)",
        "UPDATE MaceObject SET %s WHERE ItemID=%ld",
        "UPDATE MaceObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, EnchantLevel=%d, Silver=%d, Grade=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM MaceInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, minDamage, maxDamage, MPBonus, "
        "MaxSilver, Speed, ReqAbility, ItemLevel, CriticalBonus, DefaultOption, UpgradeRatio, UpgradeCrashPercent, "
        "NextOptionRatio, NextItemType, DowngradeRatio FROM MaceInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, Silver, "
        "Grade, ItemFlag FROM MaceObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, Silver, "
        "ItemFlag FROM MaceObject WHERE Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_SILVER_WEAPON_MP,
        SILVER_WEAPON_OBJECT,
    },
    // AR (GEAR_AR)
    {
        "INSERT INTO ARObject (ItemID, ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, OptionType, Durability, "
        "BulletCount, Grade, ItemFlag) VALUES(%ld, %ld, %d, '%s', %d, %ld, %d, %d, '%s', %d, %d, %d, %d)",
        "UPDATE ARObject SET %s WHERE ItemID=%ld",
        "UPDATE ARObject SET ObjectID = %ld, ItemType = %d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, EnchantLevel=%d, BulletCount=%d, Silver=%d, Grade=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM ARInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, minDamage, maxDamage, ToHitBonus, "
        "`Range`, Speed, ReqAbility, ItemLevel, CriticalBonus, DefaultOption, UpgradeRatio, UpgradeCrashPercent, "
        "NextOptionRatio, NextItemType, DowngradeRatio FROM ARInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, BulletCount, Silver, "
        "EnchantLevel, Grade, ItemFlag FROM ARObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, BulletCount, Silver, "
        "EnchantLevel, ItemFlag FROM ARObject WHERE Storage = %d AND StorageID = %u",
        "UPDATE ARObject SET BulletCount = %d WHERE ItemID = %d",
        NULL,
        NULL,
        GEAR_INFO_GUN,
        AR_GUN_OBJECT,
    },
    // SG (GEAR_SG)
    {
        "INSERT INTO SGObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, BulletCount, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d, %d)",
        "UPDATE SGObject SET %s, BulletCount=%d WHERE ItemID=%ld",
        "UPDATE SGObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, EnchantLevel=%d, BulletCount=%d, Silver=%d, Grade=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM SGInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, minDamage, maxDamage, ToHitBonus, "
        "`Range`, Speed, ReqAbility, ItemLevel, CriticalBonus, DefaultOption, UpgradeRatio, UpgradeCrashPercent, "
        "NextOptionRatio, NextItemType, DowngradeRatio FROM SGInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y,OptionType, Durability, EnchantLevel, "
        "BulletCount, Silver, Grade, ItemFlag FROM SGObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, OptionType, Durability, EnchantLevel, "
        "BulletCount, Silver, ItemFlag FROM SGObject WHERE Storage = %d AND StorageID = %u",
        "UPDATE SGObject SET BulletCount = %d WHERE ItemID = %d",
        NULL,
        NULL,
        GEAR_INFO_GUN,
        GUN_OBJECT,
    },
    // SMG (GEAR_SMG)
    {
        "INSERT INTO SMGObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, BulletCount, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d, %d)",
        "UPDATE SMGObject SET %s, BulletCount=%d WHERE ItemID=%ld",
        "UPDATE SMGObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, EnchantLevel=%d, BulletCount=%d, Silver=%d, Grade=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM SMGInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, minDamage, maxDamage, ToHitBonus, "
        "`Range`, Speed, ReqAbility, ItemLevel, CriticalBonus, DefaultOption, UpgradeRatio, UpgradeCrashPercent, "
        "NextOptionRatio, NextItemType, DowngradeRatio FROM SMGInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y,OptionType, Durability, EnchantLevel, "
        "BulletCount, Silver, Grade, ItemFlag FROM SMGObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y,OptionType, Durability, EnchantLevel, "
        "BulletCount, Silver, ItemFlag FROM SMGObject WHERE Storage = %d AND StorageID = %u",
        "UPDATE SMGObject SET BulletCount = %d WHERE ItemID = %ld",
        NULL,
        NULL,
        GEAR_INFO_GUN,
        GUN_OBJECT,
    },
    // SR (GEAR_SR)
    {
        "INSERT INTO SRObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID , X, Y, OptionType, "
        "Durability, BulletCount, Grade, ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, '%s', %u, %d, %d,  %d)",
        "UPDATE SRObject SET %s, BulletCount=%d WHERE ItemID=%ld",
        "UPDATE SRObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "OptionType='%s', Durability=%d, EnchantLevel=%d, BulletCount=%d, Silver=%d, Grade=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM SRInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Durability, minDamage, maxDamage, ToHitBonus, "
        "`Range`, Speed, ReqAbility, ItemLevel, CriticalBonus, DefaultOption, UpgradeRatio, UpgradeCrashPercent, "
        "NextOptionRatio, NextItemType, DowngradeRatio FROM SRInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y,OptionType, Durability, EnchantLevel, "
        "BulletCount, Silver, Grade, ItemFlag FROM SRObject WHERE OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y,OptionType, Durability, EnchantLevel, "
        "BulletCount, Silver, ItemFlag FROM SRObject WHERE Storage = %d AND StorageID = %u",
        "UPDATE SRObject SET BulletCount = %d WHERE ItemID = %d",
        NULL,
        NULL,
        GEAR_INFO_GUN,
        GUN_OBJECT,
    },
    // EventItem (GEAR_EVENT_ITEM)
    {
        "INSERT INTO EventItemObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num, ItemFlag) "
        "VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d, %d)",
        "UPDATE EventItemObject SET %s WHERE ItemID=%ld",
        "UPDATE EventItemObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM EventItemInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio FROM EventItemInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM EventItemObject WHERE OwnerID "
        "= '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM EventItemObject WHERE Storage "
        "= %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC,
        NUM_OBJECT,
    },
    // EventTree (GEAR_EVENT_TREE)
    {
        "INSERT INTO EventTreeObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num, ItemFlag) "
        "VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d, %d)",
        "UPDATE EventTreeObject SET %s WHERE ItemID=%ld",
        "UPDATE EventTreeObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM EventTreeInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio FROM EventTreeInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM EventTreeObject WHERE OwnerID "
        "= '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM EventTreeObject WHERE Storage "
        "= %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC,
        NUM_OBJECT,
    },
    // LuckyBag (GEAR_LUCKY_BAG)
    {
        "INSERT INTO LuckyBagObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num, ItemFlag) "
        "VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d, %d)",
        "UPDATE LuckyBagObject SET %s WHERE ItemID=%ld",
        "UPDATE LuckyBagObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM LuckyBagInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio FROM LuckyBagInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM LuckyBagObject WHERE OwnerID "
        "= '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM LuckyBagObject WHERE Storage "
        "= %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC,
        NUM_OBJECT,
    },
    // MoonCard (GEAR_MOON_CARD)
    {
        "INSERT INTO MoonCardObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num, ItemFlag) "
        "VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d, %d)",
        "UPDATE MoonCardObject SET %s WHERE ItemID=%ld",
        "UPDATE MoonCardObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM MoonCardInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio FROM MoonCardInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM MoonCardObject WHERE OwnerID "
        "= '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM MoonCardObject WHERE Storage "
        "= %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC,
        NUM_OBJECT,
    },
    // EventETC (GEAR_EVENT_ETC)
    {
        "INSERT INTO EventETCObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num, ItemFlag) "
        "VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d, %d)",
        "UPDATE EventETCObject SET %s WHERE ItemID=%ld",
        "UPDATE EventETCObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM EventETCInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, `Function` FROM EventETCInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM EventETCObject WHERE OwnerID "
        "= '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM EventETCObject WHERE Storage "
        "= %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_FUNCTION,
        NUM_OBJECT,
    },
    // ResurrectItem (GEAR_RESURRECT_ITEM)
    {
        "INSERT INTO ResurrectItemObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num, "
        "ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d, %d)",
        "UPDATE ResurrectItemObject SET %s WHERE ItemID=%ld",
        "UPDATE ResurrectItemObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, "
        "Y=%d, Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM ResurrectItemInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, ResurrectType FROM ResurrectItemInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM ResurrectItemObject WHERE "
        "OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM ResurrectItemObject WHERE "
        "Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_RESURRECT,
        NUM_OBJECT,
    },
    // DyePotion (GEAR_DYE_POTION)
    {
        "INSERT INTO DyePotionObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num, ItemFlag) "
        "VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d, %d)",
        "UPDATE DyePotionObject SET %s WHERE ItemID=%ld",
        "UPDATE DyePotionObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM DyePotionInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, FunctionFlag, FunctionValue FROM DyePotionInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM DyePotionObject WHERE OwnerID "
        "= '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM DyePotionObject WHERE Storage "
        "= %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_FUNCTION_VALUE,
        NUM_OBJECT,
    },
    // EventStar (GEAR_EVENT_STAR)
    {
        "INSERT INTO EventStarObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num, ItemFlag) "
        "VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d, %d)",
        "UPDATE EventStarObject SET %s WHERE ItemID=%ld",
        "UPDATE EventStarObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM EventStarInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, FunctionFlag, FunctionValue FROM EventStarInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM EventStarObject WHERE OwnerID "
        "= '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM EventStarObject WHERE Storage "
        "= %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_FUNCTION_VALUE,
        NUM_OBJECT,
    },
    // EffectItem (GEAR_EFFECT_ITEM)
    {
        "INSERT INTO EffectItemObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num, ItemFlag) "
        "VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d, %d)",
        "UPDATE EffectItemObject SET %s WHERE ItemID=%ld",
        "UPDATE EffectItemObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM EffectItemInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, EffectClass, TimeSec FROM EffectItemInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM EffectItemObject WHERE "
        "OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM EffectItemObject WHERE "
        "Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_EFFECT,
        NUM_OBJECT,
    },
    // PetEnchantItem (GEAR_PET_ENCHANT_ITEM)
    {
        "INSERT INTO PetEnchantItemObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num, "
        "ItemFlag) VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d, %d)",
        "UPDATE PetEnchantItemObject SET %s WHERE ItemID=%ld",
        "UPDATE PetEnchantItemObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, "
        "Y=%d, Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM PetEnchantItemInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, `Function`, FunctionGrade FROM PetEnchantItemInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM PetEnchantItemObject WHERE "
        "OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM PetEnchantItemObject WHERE "
        "Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_FUNCTION_GRADE,
        NUM_OBJECT,
    },
    // ETC (GEAR_ETC)
    {
        "INSERT INTO ETCObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) VALUES(%u, %u, "
        "%u, '%s', %d, %u, %d, %d,%d)",
        "UPDATE ETCObject SET %s WHERE ItemID=%ld",
        "UPDATE ETCObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, Num=%d  "
        "WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM ETCInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio FROM ETCInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM ETCObject WHERE OwnerID = '%s' AND "
        "Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM ETCObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC,
        NUM_ONLY_OBJECT,
    },
    // Serum (GEAR_SERUM)
    {
        "INSERT INTO SerumObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) VALUES "
        "(%u,%u,%u,'%s',%d, %u, %d,%d,%d)",
        "UPDATE SerumObject SET %s WHERE ItemID=%ld",
        "UPDATE SerumObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, Num=%d "
        "WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM SerumInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, SerumEffect FROM SerumInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM SerumObject WHERE OwnerID = '%s' AND "
        "Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM SerumObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_STRING,
        NUM_ONLY_OBJECT,
    },
    // VampireETC (GEAR_VAMPIRE_ETC)
    {
        "INSERT INTO VampireETCObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) VALUES "
        "(%u,%u,%u,'%s',%d, %u, %d,%d,%d)",
        "UPDATE VampireETCObject SET %s WHERE ItemID=%ld",
        "UPDATE VampireETCObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM VampireETCInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, ReqAbility FROM VampireETCInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM VampireETCObject WHERE OwnerID = '%s' "
        "AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM VampireETCObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_STRING,
        NUM_ONLY_OBJECT,
    },
    // Water (GEAR_WATER)
    {
        "INSERT INTO WaterObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) VALUES "
        "(%u,%u,%u,'%s',%d, %u, %d,%d,%d)",
        "UPDATE WaterObject SET %s WHERE ItemID=%ld",
        "UPDATE WaterObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, Num=%d "
        "WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM WaterInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio FROM WaterInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM WaterObject WHERE OwnerID = '%s' AND "
        "Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM WaterObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC,
        NUM_ONLY_OBJECT,
    },
    // HolyWater (GEAR_HOLY_WATER)
    {
        "INSERT INTO HolyWaterObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) VALUES "
        "(%ld, %ld, %d, '%s', %d, %ld, %d, %d, %d)",
        "UPDATE HolyWaterObject SET %s WHERE ItemID=%ld",
        "UPDATE HolyWaterObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld ,X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM HolyWaterInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, minDamage, maxDamage FROM HolyWaterInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM HolyWaterObject WHERE OwnerID = '%s' "
        "AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM HolyWaterObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_DAMAGE,
        NUM_ONLY_OBJECT,
    },
    // Magazine (GEAR_MAGAZINE)
    {
        "INSERT INTO MagazineObject (ItemID, ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) VALUES(%ld, "
        "%ld, %d, '%s', %d, %ld, %d, %d, %d)",
        "UPDATE MagazineObject SET %s WHERE ItemID=%ld",
        "UPDATE MagazineObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM MagazineInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, ItemLevel, MaxBullets, MaxSilverBullets, Vivid, "
        "GunType-1 FROM MagazineInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM MagazineObject WHERE OwnerID = '%s' AND "
        "Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM MagazineObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_MAGAZINE,
        NUM_ONLY_OBJECT,
    },
    // Pupa (GEAR_PUPA)
    {
        "INSERT INTO PupaObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) VALUES(%ld, %ld, "
        "%d, '%s', %d, %ld, %d, %d, %d)",
        "UPDATE PupaObject SET %s WHERE ItemID=%ld",
        "UPDATE PupaObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, Num=%d "
        "WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM PupaInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Effect FROM PupaInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM PupaObject WHERE OwnerID = '%s' AND "
        "Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM PupaObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        "DELETE FROM %s WHERE ItemID = %ld",
        NULL,
        GEAR_INFO_BASIC_STRING,
        NUM_ONLY_OBJECT,
    },
    // Larva (GEAR_LARVA)
    {
        "INSERT INTO LarvaObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) VALUES(%ld, "
        "%ld, %d, '%s', %d, %ld, %d, %d, %d)",
        "UPDATE LarvaObject SET %s WHERE ItemID=%ld",
        "UPDATE LarvaObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, Num=%d "
        "WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM LarvaInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Effect FROM LarvaInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM LarvaObject WHERE OwnerID = '%s' AND "
        "Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM LarvaObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        "DELETE FROM %s WHERE ItemID = %ld",
        NULL,
        GEAR_INFO_BASIC_STRING,
        NUM_ONLY_OBJECT,
    },
    // ComposMei (GEAR_COMPOS_MEI)
    {
        "INSERT INTO ComposMeiObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) VALUES(%ld, "
        "%ld, %d, '%s', %d, %ld, %d, %d, %d)",
        "UPDATE ComposMeiObject SET %s WHERE ItemID=%ld",
        "UPDATE ComposMeiObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM ComposMeiInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Effect FROM ComposMeiInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM ComposMeiObject WHERE OwnerID = '%s' "
        "AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM ComposMeiObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        "DELETE FROM %s WHERE ItemID = %ld",
        NULL,
        GEAR_INFO_BASIC_STRING,
        NUM_ONLY_OBJECT,
    },
    // Potion (GEAR_POTION)
    {
        "INSERT INTO PotionObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) VALUES(%ld, "
        "%ld, %d, '%s', %d, %ld, %d, %d, %d)",
        "UPDATE PotionObject SET %s WHERE ItemID=%ld",
        "UPDATE PotionObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM PotionInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, ItemLevel, Effect FROM PotionInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM PotionObject WHERE OwnerID = '%s' AND "
        "Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM PotionObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        "DELETE FROM %s WHERE ItemID = %ld",
        NULL,
        GEAR_INFO_BASIC_LEVEL_STRING,
        NUM_ONLY_OBJECT,
    },
    // Skull (GEAR_SKULL)
    {
        "INSERT INTO SkullObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) VALUES (%ld, "
        "%ld, %d, '%s', %d, %ld, %d, %d, %d)",
        "UPDATE SkullObject SET %s WHERE ItemID=%ld",
        "UPDATE SkullObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, Num=%d "
        "WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM SkullInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, ItemLevel FROM SkullInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM SkullObject WHERE OwnerID = '%s' AND "
        "Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM SkullObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_LEVEL,
        SKULL_OBJECT,
    },
    // Bomb (GEAR_BOMB)
    {
        "INSERT INTO BombObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) VALUES(%u, %u, "
        "%u, '%s', %d, %u, %d, %d,%d)",
        "UPDATE BombObject SET %s WHERE ItemID=%ld",
        "UPDATE BombObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, Num=%d "
        "WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM BombInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, minDamage, maxDamage FROM BombInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM BombObject WHERE OwnerID = '%s' AND "
        "Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y FROM BombObject WHERE Storage = %d AND StorageID "
        "= %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_DAMAGE,
        BOMB_OBJECT,
    },
    // BombMaterial (GEAR_BOMB_MATERIAL)
    {
        "INSERT INTO BombMaterialObject (ItemID, ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) VALUES "
        "(%u, %u, %u, '%s', %d, %u, %d, %d,%d)",
        "UPDATE BombMaterialObject SET %s WHERE ItemID=%ld",
        "UPDATE BombMaterialObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM BombMaterialInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio FROM BombMaterialInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM BombMaterialObject WHERE OwnerID = '%s' "
        "AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y FROM BombMaterialObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC,
        BOMB_OBJECT,
    },
    // Mine (GEAR_MINE)
    {
        "INSERT INTO MineObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) VALUES(%u, %u, "
        "%u, '%s', %d, %u, %d, %d,%d)",
        "UPDATE MineObject SET %s WHERE ItemID=%ld",
        "UPDATE MineObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, Num=%d "
        "WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM MineInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, minDamage, maxDamage FROM MineInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num FROM MineObject WHERE OwnerID = '%s' AND "
        "Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y FROM MineObject WHERE Storage = %d AND StorageID "
        "= %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_DAMAGE,
        BOMB_OBJECT,
    },
    // QuestItem (GEAR_QUEST_ITEM)
    {
        "INSERT INTO QuestItemObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, ItemFlag) "
        "VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d)",
        "UPDATE QuestItemObject SET %s WHERE ItemID=%ld",
        "UPDATE QuestItemObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d "
        "WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM QuestItemInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, BonusRatio FROM QuestItemInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, ItemFlag FROM QuestItemObject WHERE OwnerID = "
        "'%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, ItemFlag FROM QuestItemObject WHERE Storage = %d "
        "AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_INT,
        FLAG_OBJECT,
    },
    // SMSItem (GEAR_SMSITEM)
    {
        "INSERT INTO SMSItemObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, ItemFlag) "
        "VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d)",
        "UPDATE SMSItemObject SET %s WHERE ItemID=%ld",
        "UPDATE SMSItemObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d WHERE "
        "ItemID=%ld",
        "SELECT MAX(ItemType) FROM SMSItemInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Charge FROM SMSItemInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, ItemFlag FROM SMSItemObject WHERE OwnerID = '%s' "
        "AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, ItemFlag FROM SMSItemObject WHERE Storage = %d "
        "AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_INT,
        FLAG_OBJECT,
    },
    // SubInventory (GEAR_SUB_INVENTORY)
    {
        "INSERT INTO SubInventoryObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, ItemFlag) "
        "VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d)",
        "UPDATE SubInventoryObject SET %s WHERE ItemID=%ld",
        "UPDATE SubInventoryObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d "
        "WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM SubInventoryInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Width, Height FROM SubInventoryInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, ItemFlag FROM SubInventoryObject WHERE OwnerID = "
        "'%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, ItemFlag FROM SubInventoryObject WHERE Storage = "
        "%d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_INT_PAIR,
        FLAG_OBJECT,
    },
    // TrapItem (GEAR_TRAP_ITEM)
    {
        "INSERT INTO TrapItemObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, ItemFlag) "
        "VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d)",
        "UPDATE TrapItemObject SET %s WHERE ItemID=%ld",
        "UPDATE TrapItemObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d "
        "WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM TrapItemInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, `Function`, Parameter FROM TrapItemInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, ItemFlag FROM TrapItemObject WHERE OwnerID = "
        "'%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, ItemFlag FROM TrapItemObject WHERE Storage = %d "
        "AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_INT_PAIR,
        FLAG_OBJECT,
    },
    // EventGiftBox (GEAR_EVENT_GIFT_BOX)
    {
        "INSERT INTO EventGiftBoxObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y) VALUES(%u, "
        "%u, %u, '%s', %d, %u, %d, %d)",
        "UPDATE EventGiftBoxObject SET %s WHERE ItemID=%ld",
        "UPDATE EventGiftBoxObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d "
        "WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM EventGiftBoxInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio FROM EventGiftBoxInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y FROM EventGiftBoxObject WHERE OwnerID = '%s' AND "
        "Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y FROM EventGiftBoxObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC,
        PLAIN_OBJECT,
    },
    // LearningItem (GEAR_LEARNING_ITEM)
    {
        "INSERT INTO LearningItemObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y) VALUES(%u, "
        "%u, %u, '%s', %d, %u, %d, %d)",
        "UPDATE LearningItemObject SET %s WHERE ItemID=%ld",
        "UPDATE LearningItemObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%s, StorageID=%ld, X=%d, Y=%d "
        "WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM LearningItemInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, SkillType FROM LearningItemInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y FROM LearningItemObject WHERE OwnerID = '%s' AND "
        "Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y FROM LearningItemObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_INT,
        PLAIN_OBJECT,
    },
    // MixingItem (GEAR_MIXING_ITEM)
    {
        "INSERT INTO MixingItemObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num, ItemFlag) "
        "VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d, %d)",
        "UPDATE MixingItemObject SET %s WHERE ItemID=%ld",
        "UPDATE MixingItemObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM MixingItemInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Target-1, Type-1, SlayerLevel, VampireLevel, "
        "OustersLevel FROM MixingItemInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM MixingItemObject WHERE "
        "OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM MixingItemObject WHERE "
        "Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_MIXING_ITEM,
        MIXING_ITEM_OBJECT,
    },
    // PetFood (GEAR_PET_FOOD)
    {
        "INSERT INTO PetFoodObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num, ItemFlag) "
        "VALUES(%u, %u, %u, '%s', %d, %u, %d, %d, %d, %d)",
        "UPDATE PetFoodObject SET %s WHERE ItemID=%ld",
        "UPDATE PetFoodObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Num=%u WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM PetFoodInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, Target, PetHP, TameRatio FROM PetFoodInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Num, ItemFlag FROM PetFoodObject WHERE OwnerID = "
        "'%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, ItemFlag FROM PetFoodObject WHERE Storage = %d "
        "AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_INT_TRIPLE,
        PET_FOOD_OBJECT,
    },
    // Key (GEAR_KEY)
    {
        "INSERT INTO KeyObject (ItemID,  ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Target) VALUES(%u, %u, "
        "%u, '%s', %d, %u, %d, %d, %u)",
        "UPDATE KeyObject SET %s WHERE ItemID=%ld",
        "UPDATE KeyObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, Y=%d, "
        "Target=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM KeyInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, OptionType, TargetType FROM KeyInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Target FROM KeyObject WHERE OwnerID = '%s' AND "
        "Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Target FROM KeyObject WHERE Storage = %d AND "
        "StorageID = %u",
        NULL,
        NULL,
        "UPDATE KeyObject SET Target=%lu WHERE ItemID=%lu",
        GEAR_INFO_BASIC_INT_PAIR,
        KEY_OBJECT,
    },
    // OustersSummonItem (GEAR_OUSTERS_SUMMON_ITEM)
    {
        "INSERT INTO OustersSummonItemObject (ItemID,ObjectID,ItemType,OwnerID, Storage,StorageID,X,Y, Charge) VALUES "
        "(%u,%u,%u,'%s',%d,%u,%d,%d,%d)",
        "UPDATE OustersSummonItemObject SET %s WHERE ItemID=%ld",
        "UPDATE OustersSummonItemObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, "
        "Y=%d, Charge=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM OustersSummonItemInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, MaxCharge, Effect FROM OustersSummonItemInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Charge FROM OustersSummonItemObject WHERE "
        "OwnerID = '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Charge FROM OustersSummonItemObject WHERE "
        "Storage = %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_SUMMON_ITEM,
        CHARGE_OBJECT,
    },
    // SlayerPortalItem (GEAR_SLAYER_PORTAL_ITEM)
    {
        "INSERT INTO SlayerPortalItemObject (ItemID,ObjectID,ItemType,OwnerID, Storage,StorageID,X,Y, Charge) VALUES "
        "(%u,%u,%u,'%s',%d,%u,%d,%d,%d)",
        "UPDATE SlayerPortalItemObject SET %s WHERE ItemID=%ld",
        "UPDATE SlayerPortalItemObject SET ObjectID=%ld, ItemType=%d, OwnerID='%s', Storage=%d, StorageID=%ld, X=%d, "
        "Y=%d, Charge=%d WHERE ItemID=%ld",
        "SELECT MAX(ItemType) FROM SlayerPortalItemInfo",
        "SELECT ItemType, Name, EName, Price, Volume, Weight, Ratio, MaxCharge, ReqAbility FROM SlayerPortalItemInfo",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Charge FROM SlayerPortalItemObject WHERE OwnerID "
        "= '%s' AND Storage IN(0, 1, 2, 3, 4, 9)",
        "SELECT ItemID, ObjectID, ItemType, Storage, StorageID, X, Y, Charge FROM SlayerPortalItemObject WHERE Storage "
        "= %d AND StorageID = %u",
        NULL,
        NULL,
        NULL,
        GEAR_INFO_BASIC_LEVEL_STRING,
        CHARGE_OBJECT,
    },
};

static_assert(sizeof(kGear) / sizeof(kGear[0]) == GEAR_SLAYER_PORTAL_ITEM + 1, "kGear must cover every GearTable");

const GearSpec& spec(GearTable table) {
    return kGear[table];
}

void requireInfoKind(GearTable table, GearInfoKind kind, const char* loader) {
    if (spec(table).infoKind != kind) {
        throw Error(string("ItemObjectRepository: ") + loader + " called for a table with another Info shape");
    }
}

void requireObjectKind(GearTable table, GearObjectKind kind, const char* method) {
    if (spec(table).objectKind != kind) {
        throw Error(string("ItemObjectRepository: ") + method + " called for a table with another object shape");
    }
}

// The gun methods serve both gun shapes; the two differ only in the tinysave
// literal and in the order the loads read BulletCount, Silver and EnchantLevel.
void requireGunObject(GearTable table, const char* method) {
    GearObjectKind kind = spec(table).objectKind;
    if (kind != GUN_OBJECT && kind != AR_GUN_OBJECT) {
        throw Error(string("ItemObjectRepository: ") + method + " called for a table that is not a gun");
    }
}

// insertGear's twelve varargs fit gear's and the silver weapons' INSERT
// literals; every other shape's INSERT takes a different count, so insertGear
// refuses those tables (the gun INSERTs take thirteen).
void requireGearInsert(GearTable table, const char* method) {
    GearObjectKind kind = spec(table).objectKind;
    if (kind != GEAR_OBJECT && kind != SILVER_WEAPON_OBJECT) {
        throw Error(string("ItemObjectRepository: ") + method +
                    " called for a table whose INSERT takes other arguments");
    }
}

// tinysaveGear's literal takes (field, itemID); the GUN_OBJECT tables' takes
// (field, bulletCount, itemID). Either literal fed the other's arguments would
// format the wrong varargs, so each method refuses the other's tables.
void requireTinysaveShape(GearTable table, bool withBullet, const char* method) {
    if ((spec(table).objectKind == GUN_OBJECT) != withBullet) {
        throw Error(string("ItemObjectRepository: ") + method + " called for a table whose tinysave takes " +
                    (withBullet ? "no BulletCount" : "a BulletCount"));
    }
}

// The Num-only INSERT, UPDATE and owner load serve the three Num-only zone
// variants; only the zone loads differ (Skull reads Num through getDWORD; the
// Bomb tables' zone SELECT has no Num column), so each zone load checks its own kind.
void requireNumOnlyObject(GearTable table, const char* method) {
    GearObjectKind kind = spec(table).objectKind;
    if (kind != NUM_ONLY_OBJECT && kind != SKULL_OBJECT && kind != BOMB_OBJECT) {
        throw Error(string("ItemObjectRepository: ") + method + " called for a table that is not a Num-only item");
    }
}

// The Num + ItemFlag INSERT and UPDATE serve MixingItem and PetFood too: their
// tables have the same columns, only their loads read Num through getInt.
void requireNumObject(GearTable table, const char* method) {
    GearObjectKind kind = spec(table).objectKind;
    if (kind != NUM_OBJECT && kind != MIXING_ITEM_OBJECT && kind != PET_FOOD_OBJECT) {
        throw Error(string("ItemObjectRepository: ") + method +
                    " called for a table that is not a Num + ItemFlag item");
    }
}

// PetFood's zone SELECT names no Num column: it is the ItemFlag-only zone shape.
void requireFlagZone(GearTable table, const char* method) {
    GearObjectKind kind = spec(table).objectKind;
    if (kind != FLAG_OBJECT && kind != PET_FOOD_OBJECT) {
        throw Error(string("ItemObjectRepository: ") + method + " called for a table with another zone shape");
    }
}

// The eight columns every Info shape but the basic one starts with, read in SELECT order.
template <class Row> void readInfoHead(Result* pResult, uint& i, Row& row) {
    row.itemType = pResult->getInt(++i);
    row.name = pResult->getString(++i);
    row.ename = pResult->getString(++i);
    row.price = pResult->getInt(++i);
    row.volume = pResult->getInt(++i);
    row.weight = pResult->getInt(++i);
    row.ratio = pResult->getInt(++i);
    row.durability = pResult->getInt(++i);
}

// The ten columns after the head in the standard gear Info shape.
void readGearInfoTail(Result* pResult, uint& i, GearInfoRow& row) {
    row.defense = pResult->getInt(++i);
    row.protection = pResult->getInt(++i);
    row.reqAbility = pResult->getString(++i);
    row.itemLevel = pResult->getInt(++i);
    row.defaultOption = pResult->getString(++i);
    row.upgradeRatio = pResult->getInt(++i);
    row.upgradeCrashPercent = pResult->getInt(++i);
    row.nextOptionRatio = pResult->getInt(++i);
    row.nextItemType = pResult->getInt(++i);
    row.downgradeRatio = pResult->getInt(++i);
}

// The twelve columns after the head in the weapon Info shape.
void readWeaponInfoTail(Result* pResult, uint& i, WeaponInfoRow& row) {
    row.minDamage = pResult->getInt(++i);
    row.maxDamage = pResult->getInt(++i);
    row.speed = pResult->getInt(++i);
    row.reqAbility = pResult->getString(++i);
    row.itemLevel = pResult->getInt(++i);
    row.criticalBonus = pResult->getInt(++i);
    row.defaultOption = pResult->getString(++i);
    row.upgradeRatio = pResult->getInt(++i);
    row.upgradeCrashPercent = pResult->getInt(++i);
    row.nextOptionRatio = pResult->getInt(++i);
    row.nextItemType = pResult->getInt(++i);
    row.downgradeRatio = pResult->getInt(++i);
}

// The fourteen columns after the head in the gun Info shape: the weapon
// columns with ToHitBonus and `Range` after maxDamage.
void readGunInfoTail(Result* pResult, uint& i, GunInfoRow& row) {
    row.minDamage = pResult->getInt(++i);
    row.maxDamage = pResult->getInt(++i);
    row.toHitBonus = pResult->getInt(++i);
    row.range = pResult->getInt(++i);
    row.speed = pResult->getInt(++i);
    row.reqAbility = pResult->getString(++i);
    row.itemLevel = pResult->getInt(++i);
    row.criticalBonus = pResult->getInt(++i);
    row.defaultOption = pResult->getString(++i);
    row.upgradeRatio = pResult->getInt(++i);
    row.upgradeCrashPercent = pResult->getInt(++i);
    row.nextOptionRatio = pResult->getInt(++i);
    row.nextItemType = pResult->getInt(++i);
    row.downgradeRatio = pResult->getInt(++i);
}

// The seven columns of the basic Info shape (the head without Durability).
void readBasicInfo(Result* pResult, uint& i, BasicInfoRow& row) {
    row.itemType = pResult->getInt(++i);
    row.name = pResult->getString(++i);
    row.ename = pResult->getString(++i);
    row.price = pResult->getInt(++i);
    row.volume = pResult->getInt(++i);
    row.weight = pResult->getInt(++i);
    row.ratio = pResult->getInt(++i);
}

// The six columns every Info SELECT starts with: the basic shape without Ratio
// (MixingItemInfo, OustersSummonItemInfo).
void readHeadInfo(Result* pResult, uint& i, HeadInfoRow& row) {
    row.itemType = pResult->getInt(++i);
    row.name = pResult->getString(++i);
    row.ename = pResult->getString(++i);
    row.price = pResult->getInt(++i);
    row.volume = pResult->getInt(++i);
    row.weight = pResult->getInt(++i);
}

// The ten columns after MaxSilver in the silver-weapon Info shapes.
template <class Row> void readSilverWeaponInfoTail(Result* pResult, uint& i, Row& row) {
    row.speed = pResult->getInt(++i);
    row.reqAbility = pResult->getString(++i);
    row.itemLevel = pResult->getInt(++i);
    row.criticalBonus = pResult->getInt(++i);
    row.defaultOption = pResult->getString(++i);
    row.upgradeRatio = pResult->getInt(++i);
    row.upgradeCrashPercent = pResult->getInt(++i);
    row.nextOptionRatio = pResult->getInt(++i);
    row.nextItemType = pResult->getInt(++i);
    row.downgradeRatio = pResult->getInt(++i);
}

class MySQLItemObjectRepository : public ItemObjectRepository {
public:
    void insertGear(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType, const string& ownerID,
                    int storage, StorageID_t storageID, int x, int y, const string& optionField,
                    Durability_t durability, int grade, int createType) {
        requireGearInsert(table, "insertGear");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).insert, itemID, objectID, itemType, ownerID.c_str(), storage, storageID, x,
                                y, optionField.c_str(), durability, grade, createType);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void tinysaveGear(GearTable table, const char* field, ItemID_t itemID) {
        requireTinysaveShape(table, false, "tinysaveGear");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).tinysave, field, itemID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertGun(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType, const string& ownerID,
                   int storage, StorageID_t storageID, int x, int y, const string& optionField, Durability_t durability,
                   int bulletCount, int grade, int createType) {
        requireGunObject(table, "insertGun");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).insert, itemID, objectID, itemType, ownerID.c_str(), storage, storageID, x,
                                y, optionField.c_str(), durability, bulletCount, grade, createType);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void tinysaveGun(GearTable table, const char* field, int bulletCount, ItemID_t itemID) {
        requireTinysaveShape(table, true, "tinysaveGun");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).tinysave, field, bulletCount, itemID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateGun(GearTable table, ObjectID_t objectID, ItemType_t itemType, const string& ownerID, int storage,
                   StorageID_t storageID, int x, int y, const string& optionField, Durability_t durability,
                   int enchantLevel, int bulletCount, int silver, int grade, ItemID_t itemID) {
        requireGunObject(table, "updateGun");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).update, objectID, itemType, ownerID.c_str(), storage, storageID, x, y,
                                optionField.c_str(), durability, enchantLevel, bulletCount, silver, grade, itemID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void saveGunBullet(GearTable table, BYTE bulletCount, ItemID_t itemID) {
        requireGunObject(table, "saveGunBullet");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).saveBullet, bulletCount, itemID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateGear(GearTable table, ObjectID_t objectID, ItemType_t itemType, const string& ownerID, int storage,
                    StorageID_t storageID, int x, int y, const string& optionField, Durability_t durability, int grade,
                    int enchantLevel, ItemID_t itemID) {
        requireObjectKind(table, GEAR_OBJECT, "updateGear");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).update, objectID, itemType, ownerID.c_str(), storage, storageID, x, y,
                                optionField.c_str(), durability, grade, enchantLevel, itemID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateSilverWeapon(GearTable table, ObjectID_t objectID, ItemType_t itemType, const string& ownerID,
                            int storage, StorageID_t storageID, int x, int y, const string& optionField,
                            Durability_t durability, int enchantLevel, int silver, int grade, ItemID_t itemID) {
        requireObjectKind(table, SILVER_WEAPON_OBJECT, "updateSilverWeapon");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).update, objectID, itemType, ownerID.c_str(), storage, storageID, x, y,
                                optionField.c_str(), durability, enchantLevel, silver, grade, itemID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    int loadMaxGearType(GearTable table) {
        int maxType = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).maxType);
            pResult->next();
            maxType = pResult->getInt(1);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return maxType;
    }

    vector<GearInfoRow> loadGearInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_STANDARD, "loadGearInfos");
        vector<GearInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                GearInfoRow row;
                readInfoHead(pResult, i, row);
                readGearInfoTail(pResult, i, row);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<GearInfoNoRatioRow> loadGearInfosNoRatio(GearTable table) {
        requireInfoKind(table, GEAR_INFO_NO_RATIO, "loadGearInfosNoRatio");
        vector<GearInfoNoRatioRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                GearInfoNoRatioRow row;
                readInfoHead(pResult, i, row);
                row.defense = pResult->getInt(++i);
                row.protection = pResult->getInt(++i);
                row.reqAbility = pResult->getString(++i);
                row.itemLevel = pResult->getInt(++i);
                row.defaultOption = pResult->getString(++i);
                row.upgradeCrashPercent = pResult->getInt(++i);
                row.nextOptionRatio = pResult->getInt(++i);
                row.nextItemType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<GearInfoElementalRow> loadGearInfosElemental(GearTable table) {
        requireInfoKind(table, GEAR_INFO_ELEMENTAL, "loadGearInfosElemental");
        vector<GearInfoElementalRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                GearInfoElementalRow row;
                readInfoHead(pResult, i, row.gear);
                readGearInfoTail(pResult, i, row.gear);
                row.elementalType = pResult->getInt(++i);
                row.elemental = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<WeaponInfoRow> loadWeaponInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_WEAPON, "loadWeaponInfos");
        vector<WeaponInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                WeaponInfoRow row;
                readInfoHead(pResult, i, row);
                readWeaponInfoTail(pResult, i, row);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<WeaponInfoElementalRow> loadWeaponInfosElemental(GearTable table) {
        requireInfoKind(table, GEAR_INFO_WEAPON_ELEMENTAL, "loadWeaponInfosElemental");
        vector<WeaponInfoElementalRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                WeaponInfoElementalRow row;
                readInfoHead(pResult, i, row.weapon);
                readWeaponInfoTail(pResult, i, row.weapon);
                row.elementalType = pResult->getInt(++i);
                row.elemental = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<SilverWeaponInfoRow> loadSilverWeaponInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_SILVER_WEAPON, "loadSilverWeaponInfos");
        vector<SilverWeaponInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                SilverWeaponInfoRow row;
                readInfoHead(pResult, i, row);
                row.minDamage = pResult->getInt(++i);
                row.maxDamage = pResult->getInt(++i);
                row.maxSilver = pResult->getInt(++i);
                readSilverWeaponInfoTail(pResult, i, row);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<SilverWeaponMPInfoRow> loadSilverWeaponMPInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_SILVER_WEAPON_MP, "loadSilverWeaponMPInfos");
        vector<SilverWeaponMPInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                SilverWeaponMPInfoRow row;
                readInfoHead(pResult, i, row);
                row.minDamage = pResult->getInt(++i);
                row.maxDamage = pResult->getInt(++i);
                row.mpBonus = pResult->getInt(++i);
                row.maxSilver = pResult->getInt(++i);
                readSilverWeaponInfoTail(pResult, i, row);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<GunInfoRow> loadGunInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_GUN, "loadGunInfos");
        vector<GunInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                GunInfoRow row;
                readInfoHead(pResult, i, row);
                readGunInfoTail(pResult, i, row);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    // The two gun shapes name BulletCount, Silver and EnchantLevel in different
    // orders; each value is read at its table's ordinal into the field its
    // column names, as the inline setters did.
    template <class Row> void readGunTail(GearTable table, Result* pResult, uint& i, Row& row) {
        if (spec(table).objectKind == AR_GUN_OBJECT) {
            row.bulletCount = pResult->getInt(++i);
            row.silver = pResult->getInt(++i);
            row.enchantLevel = pResult->getInt(++i);
        } else {
            row.enchantLevel = pResult->getInt(++i);
            row.bulletCount = pResult->getInt(++i);
            row.silver = pResult->getInt(++i);
        }
    }

    vector<GunObjectRow> loadGunOfOwner(GearTable table, const string& ownerName) {
        requireGunObject(table, "loadGunOfOwner");
        vector<GunObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).ofOwner, ownerName.c_str());

            while (pResult->next()) {
                uint i = 0;
                GunObjectRow row;
                row.itemID = pResult->getDWORD(++i);
                row.objectID = pResult->getDWORD(++i);
                row.itemType = pResult->getDWORD(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getDWORD(++i);
                row.x = pResult->getBYTE(++i);
                row.y = pResult->getBYTE(++i);
                row.optionField = pResult->getString(++i);
                row.durability = pResult->getInt(++i);
                readGunTail(table, pResult, i, row);
                row.grade = pResult->getInt(++i);
                row.createType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<GunZoneObjectRow> loadGunInZone(GearTable table, int storage, ZoneID_t zoneID) {
        requireGunObject(table, "loadGunInZone");
        vector<GunZoneObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).inZone, storage, zoneID);

            while (pResult->next()) {
                uint i = 0;
                GunZoneObjectRow row;
                row.itemID = pResult->getInt(++i);
                row.objectID = pResult->getInt(++i);
                row.itemType = pResult->getInt(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getInt(++i);
                row.x = pResult->getInt(++i);
                row.y = pResult->getInt(++i);
                row.optionField = pResult->getString(++i);
                row.durability = pResult->getInt(++i);
                readGunTail(table, pResult, i, row);
                row.createType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    // The Num + ItemFlag items: no OptionType, Durability, Grade or EnchantLevel.
    void insertNumItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                       const string& ownerID, int storage, StorageID_t storageID, int x, int y, int num,
                       int createType) {
        requireNumObject(table, "insertNumItem");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).insert, itemID, objectID, itemType, ownerID.c_str(), storage, storageID, x,
                                y, num, createType);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateNumItem(GearTable table, ObjectID_t objectID, ItemType_t itemType, const string& ownerID, int storage,
                       StorageID_t storageID, int x, int y, int num, ItemID_t itemID) {
        requireNumObject(table, "updateNumItem");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).update, objectID, itemType, ownerID.c_str(), storage, storageID, x, y, num,
                                itemID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<BasicInfoRow> loadBasicInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_BASIC, "loadBasicInfos");
        vector<BasicInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                BasicInfoRow row;
                readBasicInfo(pResult, i, row);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<FunctionInfoRow> loadFunctionInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_BASIC_FUNCTION, "loadFunctionInfos");
        vector<FunctionInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                FunctionInfoRow row;
                readBasicInfo(pResult, i, row.basic);
                row.function = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ResurrectInfoRow> loadResurrectInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_BASIC_RESURRECT, "loadResurrectInfos");
        vector<ResurrectInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                ResurrectInfoRow row;
                readBasicInfo(pResult, i, row.basic);
                row.resurrectType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<FunctionValueInfoRow> loadFunctionValueInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_BASIC_FUNCTION_VALUE, "loadFunctionValueInfos");
        vector<FunctionValueInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                FunctionValueInfoRow row;
                readBasicInfo(pResult, i, row.basic);
                row.functionFlag = pResult->getInt(++i);
                row.functionValue = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<EffectInfoRow> loadEffectInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_BASIC_EFFECT, "loadEffectInfos");
        vector<EffectInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                EffectInfoRow row;
                readBasicInfo(pResult, i, row.basic);
                row.effectClass = pResult->getInt(++i);
                row.timeSec = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<FunctionGradeInfoRow> loadFunctionGradeInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_BASIC_FUNCTION_GRADE, "loadFunctionGradeInfos");
        vector<FunctionGradeInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                FunctionGradeInfoRow row;
                readBasicInfo(pResult, i, row.basic);
                row.function = pResult->getInt(++i);
                row.functionGrade = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<NumObjectRow> loadNumItemOfOwner(GearTable table, const string& ownerName) {
        requireObjectKind(table, NUM_OBJECT, "loadNumItemOfOwner");
        vector<NumObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).ofOwner, ownerName.c_str());

            while (pResult->next()) {
                uint i = 0;
                NumObjectRow row;
                row.itemID = pResult->getDWORD(++i);
                row.objectID = pResult->getDWORD(++i);
                row.itemType = pResult->getDWORD(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getDWORD(++i);
                row.x = pResult->getBYTE(++i);
                row.y = pResult->getBYTE(++i);
                row.num = pResult->getBYTE(++i);
                row.createType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<NumZoneObjectRow> loadNumItemInZone(GearTable table, int storage, ZoneID_t zoneID) {
        requireObjectKind(table, NUM_OBJECT, "loadNumItemInZone");
        vector<NumZoneObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).inZone, storage, zoneID);

            while (pResult->next()) {
                uint i = 0;
                NumZoneObjectRow row;
                row.itemID = pResult->getInt(++i);
                row.objectID = pResult->getInt(++i);
                row.itemType = pResult->getInt(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getInt(++i);
                row.x = pResult->getInt(++i);
                row.y = pResult->getInt(++i);
                row.num = pResult->getBYTE(++i);
                row.createType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    // The Num-only items: the Num + ItemFlag shape without ItemFlag — eight
    // columns in the INSERT, the UPDATE and both loads; no create type anywhere.
    void insertNumOnlyItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                           const string& ownerID, int storage, StorageID_t storageID, int x, int y, int num) {
        requireNumOnlyObject(table, "insertNumOnlyItem");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).insert, itemID, objectID, itemType, ownerID.c_str(), storage, storageID, x,
                                y, num);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateNumOnlyItem(GearTable table, ObjectID_t objectID, ItemType_t itemType, const string& ownerID,
                           int storage, StorageID_t storageID, int x, int y, int num, ItemID_t itemID) {
        requireNumOnlyObject(table, "updateNumOnlyItem");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).update, objectID, itemType, ownerID.c_str(), storage, storageID, x, y, num,
                                itemID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<StringInfoRow> loadStringInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_BASIC_STRING, "loadStringInfos");
        vector<StringInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                StringInfoRow row;
                readBasicInfo(pResult, i, row.basic);
                row.value = pResult->getString(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    // The four Num-only items with their own destroy(): the DELETE with the
    // class's table name as its %s. The original returned false when no row
    // went and true otherwise, including after a caught DB error.
    bool destroyItemObject(GearTable table, const string& objectTableName, ItemID_t itemID) {
        if (spec(table).destroy == NULL) {
            throw Error("ItemObjectRepository: destroyItemObject called for a table without a destroy literal");
        }
        bool deleted = true;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).destroy, objectTableName.c_str(), itemID);
            deleted = pStmt->getAffectedRowCount() != 0;
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return deleted;
    }

    vector<DamageInfoRow> loadDamageInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_BASIC_DAMAGE, "loadDamageInfos");
        vector<DamageInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                DamageInfoRow row;
                readBasicInfo(pResult, i, row.basic);
                row.minDamage = pResult->getInt(++i);
                row.maxDamage = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<MagazineInfoRow> loadMagazineInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_MAGAZINE, "loadMagazineInfos");
        vector<MagazineInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                MagazineInfoRow row;
                readBasicInfo(pResult, i, row.basic);
                row.itemLevel = pResult->getInt(++i);
                row.maxBullets = pResult->getInt(++i);
                row.maxSilverBullets = pResult->getInt(++i);
                row.vivid = pResult->getInt(++i);
                row.gunType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<LevelStringInfoRow> loadLevelStringInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_BASIC_LEVEL_STRING, "loadLevelStringInfos");
        vector<LevelStringInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                LevelStringInfoRow row;
                readBasicInfo(pResult, i, row.basic);
                row.itemLevel = pResult->getInt(++i);
                row.value = pResult->getString(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<LevelInfoRow> loadLevelInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_BASIC_LEVEL, "loadLevelInfos");
        vector<LevelInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                LevelInfoRow row;
                readBasicInfo(pResult, i, row.basic);
                row.itemLevel = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    // Skull's zone load: the Num-only columns, Num through getDWORD.
    vector<SkullZoneObjectRow> loadSkullInZone(GearTable table, int storage, ZoneID_t zoneID) {
        requireObjectKind(table, SKULL_OBJECT, "loadSkullInZone");
        vector<SkullZoneObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).inZone, storage, zoneID);

            while (pResult->next()) {
                uint i = 0;
                SkullZoneObjectRow row;
                row.itemID = pResult->getInt(++i);
                row.objectID = pResult->getInt(++i);
                row.itemType = pResult->getInt(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getInt(++i);
                row.x = pResult->getInt(++i);
                row.y = pResult->getInt(++i);
                row.num = pResult->getDWORD(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    // Bomb, BombMaterial, Mine: their zone SELECT names no Num column (seven).
    vector<BombZoneObjectRow> loadBombInZone(GearTable table, int storage, ZoneID_t zoneID) {
        requireObjectKind(table, BOMB_OBJECT, "loadBombInZone");
        vector<BombZoneObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).inZone, storage, zoneID);

            while (pResult->next()) {
                uint i = 0;
                BombZoneObjectRow row;
                row.itemID = pResult->getInt(++i);
                row.objectID = pResult->getInt(++i);
                row.itemType = pResult->getInt(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getInt(++i);
                row.x = pResult->getInt(++i);
                row.y = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<NumOnlyObjectRow> loadNumOnlyItemOfOwner(GearTable table, const string& ownerName) {
        requireNumOnlyObject(table, "loadNumOnlyItemOfOwner");
        vector<NumOnlyObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).ofOwner, ownerName.c_str());

            while (pResult->next()) {
                uint i = 0;
                NumOnlyObjectRow row;
                row.itemID = pResult->getDWORD(++i);
                row.objectID = pResult->getDWORD(++i);
                row.itemType = pResult->getDWORD(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getDWORD(++i);
                row.x = pResult->getBYTE(++i);
                row.y = pResult->getBYTE(++i);
                row.num = pResult->getBYTE(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<NumOnlyZoneObjectRow> loadNumOnlyItemInZone(GearTable table, int storage, ZoneID_t zoneID) {
        requireObjectKind(table, NUM_ONLY_OBJECT, "loadNumOnlyItemInZone");
        vector<NumOnlyZoneObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).inZone, storage, zoneID);

            while (pResult->next()) {
                uint i = 0;
                NumOnlyZoneObjectRow row;
                row.itemID = pResult->getInt(++i);
                row.objectID = pResult->getInt(++i);
                row.itemType = pResult->getInt(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getInt(++i);
                row.x = pResult->getInt(++i);
                row.y = pResult->getInt(++i);
                row.num = pResult->getBYTE(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    // The ItemFlag-only items (FLAG_OBJECT) and the plain ones (PLAIN_OBJECT):
    // neither has Num, OptionType, Durability, Grade or EnchantLevel; the plain
    // ones have no ItemFlag either. Both share the seven-argument UPDATE.
    void insertFlagItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                        const string& ownerID, int storage, StorageID_t storageID, int x, int y, int createType) {
        requireObjectKind(table, FLAG_OBJECT, "insertFlagItem");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).insert, itemID, objectID, itemType, ownerID.c_str(), storage, storageID, x,
                                y, createType);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertPlainItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                         const string& ownerID, int storage, StorageID_t storageID, int x, int y) {
        requireObjectKind(table, PLAIN_OBJECT, "insertPlainItem");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).insert, itemID, objectID, itemType, ownerID.c_str(), storage, storageID, x,
                                y);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updatePlainItem(GearTable table, ObjectID_t objectID, ItemType_t itemType, const string& ownerID, int storage,
                         StorageID_t storageID, int x, int y, ItemID_t itemID) {
        GearObjectKind kind = spec(table).objectKind;
        if (kind != FLAG_OBJECT && kind != PLAIN_OBJECT) {
            throw Error("ItemObjectRepository: updatePlainItem called for a table with another object shape");
        }
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).update, objectID, itemType, ownerID.c_str(), storage, storageID, x, y,
                                itemID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<IntInfoRow> loadIntInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_BASIC_INT, "loadIntInfos");
        vector<IntInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                IntInfoRow row;
                readBasicInfo(pResult, i, row.basic);
                row.value = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<IntPairInfoRow> loadIntPairInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_BASIC_INT_PAIR, "loadIntPairInfos");
        vector<IntPairInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                IntPairInfoRow row;
                readBasicInfo(pResult, i, row.basic);
                row.first = pResult->getInt(++i);
                row.second = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<FlagObjectRow> loadFlagItemOfOwner(GearTable table, const string& ownerName) {
        requireObjectKind(table, FLAG_OBJECT, "loadFlagItemOfOwner");
        vector<FlagObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).ofOwner, ownerName.c_str());

            while (pResult->next()) {
                uint i = 0;
                FlagObjectRow row;
                row.itemID = pResult->getDWORD(++i);
                row.objectID = pResult->getDWORD(++i);
                row.itemType = pResult->getDWORD(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getDWORD(++i);
                row.x = pResult->getBYTE(++i);
                row.y = pResult->getBYTE(++i);
                row.createType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<FlagZoneObjectRow> loadFlagItemInZone(GearTable table, int storage, ZoneID_t zoneID) {
        requireFlagZone(table, "loadFlagItemInZone");
        vector<FlagZoneObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).inZone, storage, zoneID);

            while (pResult->next()) {
                uint i = 0;
                FlagZoneObjectRow row;
                row.itemID = pResult->getInt(++i);
                row.objectID = pResult->getInt(++i);
                row.itemType = pResult->getInt(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getInt(++i);
                row.x = pResult->getInt(++i);
                row.y = pResult->getInt(++i);
                row.createType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<PlainObjectRow> loadPlainItemOfOwner(GearTable table, const string& ownerName) {
        requireObjectKind(table, PLAIN_OBJECT, "loadPlainItemOfOwner");
        vector<PlainObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).ofOwner, ownerName.c_str());

            while (pResult->next()) {
                uint i = 0;
                PlainObjectRow row;
                row.itemID = pResult->getDWORD(++i);
                row.objectID = pResult->getDWORD(++i);
                row.itemType = pResult->getDWORD(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getDWORD(++i);
                row.x = pResult->getBYTE(++i);
                row.y = pResult->getBYTE(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<PlainZoneObjectRow> loadPlainItemInZone(GearTable table, int storage, ZoneID_t zoneID) {
        requireObjectKind(table, PLAIN_OBJECT, "loadPlainItemInZone");
        vector<PlainZoneObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).inZone, storage, zoneID);

            while (pResult->next()) {
                uint i = 0;
                PlainZoneObjectRow row;
                row.itemID = pResult->getInt(++i);
                row.objectID = pResult->getInt(++i);
                row.itemType = pResult->getInt(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getInt(++i);
                row.x = pResult->getInt(++i);
                row.y = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    // MixingItem and PetFood: the Num + ItemFlag INSERT, UPDATE and tinysave, but
    // both loads read Num through getInt; PetFood's zone SELECT names no Num
    // column at all (loadFlagItemInZone serves it).
    vector<NumIntObjectRow> loadNumIntItemOfOwner(GearTable table, const string& ownerName) {
        GearObjectKind kind = spec(table).objectKind;
        if (kind != MIXING_ITEM_OBJECT && kind != PET_FOOD_OBJECT) {
            throw Error("ItemObjectRepository: loadNumIntItemOfOwner called for a table with another object shape");
        }
        vector<NumIntObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).ofOwner, ownerName.c_str());

            while (pResult->next()) {
                uint i = 0;
                NumIntObjectRow row;
                row.itemID = pResult->getDWORD(++i);
                row.objectID = pResult->getDWORD(++i);
                row.itemType = pResult->getDWORD(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getDWORD(++i);
                row.x = pResult->getBYTE(++i);
                row.y = pResult->getBYTE(++i);
                row.num = pResult->getInt(++i);
                row.createType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<NumIntZoneObjectRow> loadNumIntItemInZone(GearTable table, int storage, ZoneID_t zoneID) {
        requireObjectKind(table, MIXING_ITEM_OBJECT, "loadNumIntItemInZone");
        vector<NumIntZoneObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).inZone, storage, zoneID);

            while (pResult->next()) {
                uint i = 0;
                NumIntZoneObjectRow row;
                row.itemID = pResult->getInt(++i);
                row.objectID = pResult->getInt(++i);
                row.itemType = pResult->getInt(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getInt(++i);
                row.x = pResult->getInt(++i);
                row.y = pResult->getInt(++i);
                row.num = pResult->getInt(++i);
                row.createType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    // Key: the plain columns plus Target (an ItemID_t; "%u" in the INSERT as the
    // chain streamed it, "%d" in the UPDATE as written; getDWORD in both loads).
    void insertKey(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType, const string& ownerID,
                   int storage, StorageID_t storageID, int x, int y, ItemID_t target) {
        requireObjectKind(table, KEY_OBJECT, "insertKey");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).insert, itemID, objectID, itemType, ownerID.c_str(), storage, storageID, x,
                                y, target);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateKey(GearTable table, ObjectID_t objectID, ItemType_t itemType, const string& ownerID, int storage,
                   StorageID_t storageID, int x, int y, ItemID_t target, ItemID_t itemID) {
        requireObjectKind(table, KEY_OBJECT, "updateKey");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).update, objectID, itemType, ownerID.c_str(), storage, storageID, x, y,
                                target, itemID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<KeyObjectRow> loadKeyOfOwner(GearTable table, const string& ownerName) {
        requireObjectKind(table, KEY_OBJECT, "loadKeyOfOwner");
        vector<KeyObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).ofOwner, ownerName.c_str());

            while (pResult->next()) {
                uint i = 0;
                KeyObjectRow row;
                row.itemID = pResult->getDWORD(++i);
                row.objectID = pResult->getDWORD(++i);
                row.itemType = pResult->getDWORD(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getDWORD(++i);
                row.x = pResult->getBYTE(++i);
                row.y = pResult->getBYTE(++i);
                row.target = pResult->getDWORD(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<KeyZoneObjectRow> loadKeyInZone(GearTable table, int storage, ZoneID_t zoneID) {
        requireObjectKind(table, KEY_OBJECT, "loadKeyInZone");
        vector<KeyZoneObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).inZone, storage, zoneID);

            while (pResult->next()) {
                uint i = 0;
                KeyZoneObjectRow row;
                row.itemID = pResult->getInt(++i);
                row.objectID = pResult->getInt(++i);
                row.itemType = pResult->getInt(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getInt(++i);
                row.x = pResult->getInt(++i);
                row.y = pResult->getInt(++i);
                row.target = pResult->getDWORD(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    // Key::setNewMotorcycle — the Target UPDATE with the new motorcycle's id ("%lu"
    // for both DWORDs, as written; the original discarded the Result). Refuses
    // tables without the literal.
    void saveKeyTarget(GearTable table, ItemID_t targetID, ItemID_t itemID) {
        if (spec(table).saveTarget == NULL) {
            throw Error("ItemObjectRepository: saveKeyTarget called for a table without a Target literal");
        }
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).saveTarget, targetID, itemID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    // OustersSummonItem and SlayerPortalItem: the plain columns plus Charge (an
    // int). Both loads read the same getters — the zone one too reads the ids
    // through getDWORD and X, Y through getBYTE — so one row serves both.
    void insertChargeItem(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType,
                          const string& ownerID, int storage, StorageID_t storageID, int x, int y, int charge) {
        requireObjectKind(table, CHARGE_OBJECT, "insertChargeItem");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).insert, itemID, objectID, itemType, ownerID.c_str(), storage, storageID, x,
                                y, charge);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateChargeItem(GearTable table, ObjectID_t objectID, ItemType_t itemType, const string& ownerID, int storage,
                          StorageID_t storageID, int x, int y, int charge, ItemID_t itemID) {
        requireObjectKind(table, CHARGE_OBJECT, "updateChargeItem");
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).update, objectID, itemType, ownerID.c_str(), storage, storageID, x, y,
                                charge, itemID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void readChargeRow(Result* pResult, uint& i, ChargeObjectRow& row) {
        row.itemID = pResult->getDWORD(++i);
        row.objectID = pResult->getDWORD(++i);
        row.itemType = pResult->getDWORD(++i);
        row.storage = pResult->getInt(++i);
        row.storageID = pResult->getDWORD(++i);
        row.x = pResult->getBYTE(++i);
        row.y = pResult->getBYTE(++i);
        row.charge = pResult->getInt(++i);
    }

    vector<ChargeObjectRow> loadChargeItemOfOwner(GearTable table, const string& ownerName) {
        requireObjectKind(table, CHARGE_OBJECT, "loadChargeItemOfOwner");
        vector<ChargeObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).ofOwner, ownerName.c_str());

            while (pResult->next()) {
                uint i = 0;
                ChargeObjectRow row;
                readChargeRow(pResult, i, row);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ChargeObjectRow> loadChargeItemInZone(GearTable table, int storage, ZoneID_t zoneID) {
        requireObjectKind(table, CHARGE_OBJECT, "loadChargeItemInZone");
        vector<ChargeObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).inZone, storage, zoneID);

            while (pResult->next()) {
                uint i = 0;
                ChargeObjectRow row;
                readChargeRow(pResult, i, row);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<MixingItemInfoRow> loadMixingItemInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_MIXING_ITEM, "loadMixingItemInfos");
        vector<MixingItemInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                MixingItemInfoRow row;
                readHeadInfo(pResult, i, row.head);
                row.target = pResult->getInt(++i);
                row.type = pResult->getInt(++i);
                row.slayerLevel = pResult->getInt(++i);
                row.vampireLevel = pResult->getInt(++i);
                row.oustersLevel = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<IntTripleInfoRow> loadIntTripleInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_BASIC_INT_TRIPLE, "loadIntTripleInfos");
        vector<IntTripleInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                IntTripleInfoRow row;
                readBasicInfo(pResult, i, row.basic);
                row.first = pResult->getInt(++i);
                row.second = pResult->getInt(++i);
                row.third = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<SummonItemInfoRow> loadSummonItemInfos(GearTable table) {
        requireInfoKind(table, GEAR_INFO_SUMMON_ITEM, "loadSummonItemInfos");
        vector<SummonItemInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                SummonItemInfoRow row;
                readHeadInfo(pResult, i, row.head);
                row.maxCharge = pResult->getInt(++i);
                row.effectID = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<GearObjectRow> loadGearOfOwner(GearTable table, const string& ownerName) {
        requireObjectKind(table, GEAR_OBJECT, "loadGearOfOwner");
        vector<GearObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).ofOwner, ownerName.c_str());

            while (pResult->next()) {
                uint i = 0;
                GearObjectRow row;
                row.itemID = pResult->getDWORD(++i);
                row.objectID = pResult->getDWORD(++i);
                row.itemType = pResult->getDWORD(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getDWORD(++i);
                row.x = pResult->getBYTE(++i);
                row.y = pResult->getBYTE(++i);
                row.optionField = pResult->getString(++i);
                row.durability = pResult->getInt(++i);
                row.grade = pResult->getInt(++i);
                row.enchantLevel = pResult->getInt(++i);
                row.createType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<GearZoneObjectRow> loadGearInZone(GearTable table, int storage, ZoneID_t zoneID) {
        requireObjectKind(table, GEAR_OBJECT, "loadGearInZone");
        vector<GearZoneObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).inZone, storage, zoneID);

            while (pResult->next()) {
                uint i = 0;
                GearZoneObjectRow row;
                row.itemID = pResult->getInt(++i);
                row.objectID = pResult->getInt(++i);
                row.itemType = pResult->getInt(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getInt(++i);
                row.x = pResult->getInt(++i);
                row.y = pResult->getInt(++i);
                row.optionField = pResult->getString(++i);
                row.durability = pResult->getInt(++i);
                row.enchantLevel = pResult->getInt(++i);
                row.createType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<SilverWeaponObjectRow> loadSilverWeaponOfOwner(GearTable table, const string& ownerName) {
        requireObjectKind(table, SILVER_WEAPON_OBJECT, "loadSilverWeaponOfOwner");
        vector<SilverWeaponObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).ofOwner, ownerName.c_str());

            while (pResult->next()) {
                uint i = 0;
                SilverWeaponObjectRow row;
                row.itemID = pResult->getDWORD(++i);
                row.objectID = pResult->getDWORD(++i);
                row.itemType = pResult->getDWORD(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getDWORD(++i);
                row.x = pResult->getBYTE(++i);
                row.y = pResult->getBYTE(++i);
                row.optionField = pResult->getString(++i);
                row.durability = pResult->getInt(++i);
                row.enchantLevel = pResult->getInt(++i);
                row.silver = pResult->getInt(++i);
                row.grade = pResult->getInt(++i);
                row.createType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<SilverWeaponZoneObjectRow> loadSilverWeaponInZone(GearTable table, int storage, ZoneID_t zoneID) {
        requireObjectKind(table, SILVER_WEAPON_OBJECT, "loadSilverWeaponInZone");
        vector<SilverWeaponZoneObjectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).inZone, storage, zoneID);

            while (pResult->next()) {
                uint i = 0;
                SilverWeaponZoneObjectRow row;
                row.itemID = pResult->getInt(++i);
                row.objectID = pResult->getInt(++i);
                row.itemType = pResult->getInt(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getInt(++i);
                row.x = pResult->getInt(++i);
                row.y = pResult->getInt(++i);
                row.optionField = pResult->getString(++i);
                row.durability = pResult->getInt(++i);
                row.enchantLevel = pResult->getInt(++i);
                row.silver = pResult->getInt(++i);
                row.createType = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }
};

} // namespace

ItemObjectRepository& defaultItemObjectRepository() {
    static MySQLItemObjectRepository instance;
    return instance;
}
