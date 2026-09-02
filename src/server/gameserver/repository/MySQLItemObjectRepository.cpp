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
// saveBullet UPDATE (NULL for every other table). The spec row also
// records which object shape and which Info shape the class's tables
// have; every loader checks them, so a call with the wrong loader fails
// loudly instead of misreading the columns silently. GEAR_INFO_UNSET and
// GEAR_OBJECT_UNSET are their enums' zeros, so a spec row that forgets a
// kind is refused by every method rather than read as the standard shape.

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
        GEAR_INFO_GUN,
        GUN_OBJECT,
    },
};

static_assert(sizeof(kGear) / sizeof(kGear[0]) == GEAR_SR + 1, "kGear must cover every GearTable");

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

// tinysaveGear's literal takes (field, itemID); the GUN_OBJECT tables' takes
// (field, bulletCount, itemID). Either literal fed the other's arguments would
// format the wrong varargs, so each method refuses the other's tables.
void requireTinysaveShape(GearTable table, bool withBullet, const char* method) {
    if ((spec(table).objectKind == GUN_OBJECT) != withBullet) {
        throw Error(string("ItemObjectRepository: ") + method + " called for a table whose tinysave takes " +
                    (withBullet ? "no BulletCount" : "a BulletCount"));
    }
}

// The eight columns every Info shape starts with, read in SELECT order.
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
