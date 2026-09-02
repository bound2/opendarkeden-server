// MySQL-backed ItemObjectRepository (task 3.2, the item milestone). One
// method set for the gear classes; the table — and the class's exact
// literal, copy-paste whitespace and all — comes from the spec row the
// GearTable enum indexes. The two StringStream chains of the originals
// (the create INSERT and the zone SELECT) are format strings here; every
// streamed expression maps to the conversion StringStream used for its
// type (DWORD/WORD "%u", int "%d", text as is), so the bytes on the wire
// are the same. The tinysave and save literals keep their "%ld" for the
// DWORD ids exactly as written.

#include <string>
#include <vector>

#include "DB.h"
#include "repository/ItemObjectRepository.h"

using namespace std;

namespace {

struct GearSpec {
    const char* insert;   // <Class>::create
    const char* tinysave; // <Class>::tinysave
    const char* update;   // <Class>::save
    const char* maxType;  // <Class>InfoManager::load, first statement
    const char* infos;    // <Class>InfoManager::load, second statement
    const char* ofOwner;  // <Class>Loader::load(Creature*)
    const char* inZone;   // <Class>Loader::load(Zone*)
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
    },
};

static_assert(sizeof(kGear) / sizeof(kGear[0]) == GEAR_OUSTERS_BOOTS + 1, "kGear must cover every GearTable");

const GearSpec& spec(GearTable table) {
    return kGear[table];
}

class MySQLItemObjectRepository : public ItemObjectRepository {
public:
    void insertGear(GearTable table, ItemID_t itemID, ObjectID_t objectID, ItemType_t itemType, const string& ownerID,
                    int storage, StorageID_t storageID, int x, int y, const string& optionField,
                    Durability_t durability, int grade, int createType) {
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
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).tinysave, field, itemID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateGear(GearTable table, ObjectID_t objectID, ItemType_t itemType, const string& ownerID, int storage,
                    StorageID_t storageID, int x, int y, const string& optionField, Durability_t durability, int grade,
                    int enchantLevel, ItemID_t itemID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(spec(table).update, objectID, itemType, ownerID.c_str(), storage, storageID, x, y,
                                optionField.c_str(), durability, grade, enchantLevel, itemID);
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
        vector<GearInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(spec(table).infos);

            while (pResult->next()) {
                uint i = 0;
                GearInfoRow row;
                row.itemType = pResult->getInt(++i);
                row.name = pResult->getString(++i);
                row.ename = pResult->getString(++i);
                row.price = pResult->getInt(++i);
                row.volume = pResult->getInt(++i);
                row.weight = pResult->getInt(++i);
                row.ratio = pResult->getInt(++i);
                row.durability = pResult->getInt(++i);
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
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<GearObjectRow> loadGearOfOwner(GearTable table, const string& ownerName) {
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
};

} // namespace

ItemObjectRepository& defaultItemObjectRepository() {
    static MySQLItemObjectRepository instance;
    return instance;
}
