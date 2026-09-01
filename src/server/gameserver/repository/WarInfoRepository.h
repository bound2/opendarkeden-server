#ifndef __WAR_INFO_REPOSITORY_H__
#define __WAR_INFO_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Persistence seam for the race-war tables (task 3.2): the shrines and
// their owning race (ShrineInfo), the castles and their guild/tax state
// (CastleInfo), the level-war sweeper bonuses (SweeperBonusInfo), the
// sweeper safes and owners (SweeperSetInfo, SweeperOwnerInfo), the
// level-war history (LevelWarHistory) and the master lairs
// (MasterLairInfo). Reads are typed to the driver getter the inline
// code called (getInt → int, getString → std::string); the writes'
// parameters are typed to the member/getter each caller streamed, so
// the varargs bytes reaching the format strings are unchanged.
//
// Other users of these tables keep their own inline SQL: the guild
// managers (gameserver and sharedserver) count and list castles by
// guild, and war/RaceWar.cpp re-reads the shrine owners.

// ShrineInfo's row, 20 columns in SELECT order.
struct ShrineRow {
    int id;
    std::string name;
    int itemType;
    int slayerGuardZoneID;
    int slayerGuardX;
    int slayerGuardY;
    int slayerGuardMonsterType;
    int vampireGuardZoneID;
    int vampireGuardX;
    int vampireGuardY;
    int vampireGuardMonsterType;
    int oustersGuardZoneID;
    int oustersGuardX;
    int oustersGuardY;
    int oustersGuardMonsterType;
    int holyZoneID;
    int holyX;
    int holyY;
    int holyMonsterType;
    int ownerRace;
};

struct ShrineOwnerRow {
    int id;
    int ownerRace;
};

// CastleInfo's row for one server, 19 columns in SELECT order.
struct CastleRow {
    int zoneID;
    int shrineID;
    int guildID;
    std::string name;
    int race;
    int itemTaxRatio;
    int entranceFee;
    int taxBalance;
    std::string bonusOptionType;
    int firstResurrectZoneID;
    int firstResurrectX;
    int firstResurrectY;
    int secondResurrectZoneID;
    int secondResurrectX;
    int secondResurrectY;
    int thirdResurrectZoneID;
    int thirdResurrectX;
    int thirdResurrectY;
    std::string zoneIDList;
};

// CastleInfoManager::save — the mutable castle state. Typed to the
// original expressions: every numeric was cast to (int) at the call
// except ItemTaxRatio, which is an int already.
struct CastleStateRecord {
    int guildID;
    std::string name;
    int race;
    int itemTaxRatio;
    int entranceFee;
    int taxBalance;
};

struct SweeperBonusRow {
    int type;
    std::string name;
    std::string optionList;
    int ownerRace;
    int level;
};

struct SweeperBonusOwnerRow {
    int type;
    int ownerRace;
};

// SweeperSetInfo's row for one zone, 14 columns in SELECT order.
struct SweeperSetRow {
    int itemType;
    int slayerX;
    int slayerY;
    int slayerMonsterType;
    int vampireX;
    int vampireY;
    int vampireMonsterType;
    int oustersX;
    int oustersY;
    int oustersMonsterType;
    int defaultX;
    int defaultY;
    int defaultMonsterType;
    std::string name;
};

struct SweeperOwnerRow {
    int sweeperType;
    int ownerRace;
    int sweeperSafeType;
};

// MasterLairInfo's row, 25 columns in SELECT order.
struct MasterLairRow {
    int zoneID;
    int masterNotReadyMonsterType;
    int masterMonsterType;
    int masterRemainNotReady;
    int masterX;
    int masterY;
    int masterDir;
    int maxPassPlayer;
    int summonX;
    int summonY;
    int firstRegenDelay;
    int regenDelay;
    int startDelay;
    int endDelay;
    int kickOutDelay;
    int kickZoneID;
    int kickZoneX;
    int kickZoneY;
    int lairAttackTick;
    int lairAttackMinNumber;
    int lairAttackMaxNumber;
    std::string masterSummonSay;
    std::string masterDeadSlayerSay;
    std::string masterDeadVampireSay;
    std::string masterNotDeadSay;
};

class WarInfoRepository {
public:
    virtual ~WarInfoRepository() {}

    // --- shrines ----------------------------------------------------------
    virtual std::vector<ShrineRow> loadShrines() = 0;
    virtual std::vector<ShrineOwnerRow> loadShrineOwners() = 0;
    // ShrineSet::saveOwner — the two ints the original cast at the call.
    virtual void saveShrineOwner(int ownerRace, int shrineID) = 0;

    // --- castles ----------------------------------------------------------
    virtual std::vector<CastleRow> loadCastles(int serverID) = 0;
    // CastleInfoManager::save.
    virtual void saveCastle(int serverID, int zoneID, const CastleStateRecord& record) = 0;
    // CastleInfoManager::tinysave — a caller-composed "Column=value" SET
    // fragment (raw SQL text, the same quarantine as
    // CharacterRepository::tinysave). Returns whether a row changed.
    virtual bool tinysaveCastle(const std::string& fieldFragment, ZoneID_t zoneID, int serverID) = 0;

    // --- sweeper bonuses ----------------------------------------------------
    // MAX(Type); false when the table is empty (see BalanceInfoRepository.h
    // for why the inline guard could never fire).
    virtual bool loadMaxSweeperBonusType(int& maxType) = 0;
    virtual std::vector<SweeperBonusRow> loadSweeperBonuses() = 0;
    virtual std::vector<SweeperBonusOwnerRow> loadSweeperBonusOwners(int level) = 0;
    // SweeperBonus::setRace — the members as streamed.
    virtual void saveSweeperBonusOwner(Race_t ownerRace, SweeperBonusType_t type) = 0;

    // --- sweeper safes and owners --------------------------------------------
    virtual std::vector<SweeperSetRow> loadSweeperSets(ZoneID_t zoneID) = 0;
    virtual std::vector<SweeperOwnerRow> loadSweeperOwners(ZoneID_t zoneID) = 0;
    // LevelWarManager's two-column read of the same table.
    virtual std::vector<SweeperBonusOwnerRow> loadSweeperOwnerRaces(ZoneID_t zoneID) = 0;
    // SweeperSetManager::saveSweeperOwner — the parameters as declared there.
    virtual void saveSweeperOwner(int ownerRace, int safeType, uint itemType) = 0;

    // --- level-war history ----------------------------------------------------
    virtual void insertLevelWarHistory(int level, const std::string& levelWarID, const std::string& slayerOld,
                                       const std::string& vampireOld, const std::string& oustersOld,
                                       const std::string& defaultOld) = 0;
    virtual void updateLevelWarHistory(const std::string& slayerNew, const std::string& vampireNew,
                                       const std::string& oustersNew, const std::string& defaultNew, int level,
                                       const std::string& levelWarID) = 0;

    // --- master lairs -----------------------------------------------------------
    virtual std::vector<MasterLairRow> loadMasterLairs() = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLWarInfoRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
WarInfoRepository& defaultWarInfoRepository();

#endif
