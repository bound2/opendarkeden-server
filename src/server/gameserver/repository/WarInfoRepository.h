#ifndef __WAR_INFO_REPOSITORY_H__
#define __WAR_INFO_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// The race-war tables: the shrines and their owning race (ShrineInfo),
// the castles and their guild/tax state (CastleInfo), the level-war
// sweeper bonuses (SweeperBonusInfo), the sweeper safes and owners
// (SweeperSetInfo, SweeperOwnerInfo), the three war histories
// (LevelWarHistory, GuildWarHistory and RaceWarHistory), the race-war
// entry limits (RaceWarPCLimit, both the rows themselves and the
// per-race totals one history records), the participant list
// (RaceWarPCList), the siege-war reinforcement registry
// (ReinforceRegisterInfo), the scheduled wars (WarScheduleInfo) and the
// master lairs (MasterLairInfo). All on the DARKEDEN connection. Reads
// are typed to the driver getter used (getInt → int, getString →
// std::string).

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

// CastleInfoManager::save — the mutable castle state.
struct CastleStateRecord {
    int guildID;
    std::string name;
    int race;
    int itemTaxRatio;
    int entranceFee;
    int taxBalance;
};

// WarScheduler::load's row, in SELECT order — ten columns in the live
// arm, five under __OLD_GUILD_WAR__. The macro mirrors
// war/WarScheduler.cpp, whose loop reads these columns positionally: the
// live build leaves __OLD_GUILD_WAR__ undefined (it is commented out in
// Core/Types.h), so the ten-column form is the one that compiles.
struct WarScheduleRow {
    int warID;
    std::string warType;
#ifndef __OLD_GUILD_WAR__
    int attackerCount;
    int attackGuildID[5];
#else
    int attackGuildID;
#endif
    int warFee;
    std::string startTime;
};

// RaceWar::recordRaceWarStart's per-race totals: SUM(CurrentNum) over
// RaceWarPCLimit, grouped by race. Both columns come through getInt.
struct RaceCurrentNumRow {
    int race;
    int currentNum;
};

// PCWarLimiter::load's row: the five columns in SELECT order, every one
// through getInt.
struct RaceWarLimitRow {
    int id;
    int minLevel;
    int maxLevel;
    int limitNum;
    int currentNum;
};

// RaceWarLimiter::clearPCList's row. `race` is read from column 1 — the
// Name column. See the note above loadRaceWarPCList().
struct RaceWarPCListRow {
    std::string name;
    int race;
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
    virtual void saveShrineOwner(int ownerRace, int shrineID) = 0;

    // --- castles ----------------------------------------------------------
    virtual std::vector<CastleRow> loadCastles(int serverID) = 0;
    virtual void saveCastle(int serverID, int zoneID, const CastleStateRecord& record) = 0;
    // A caller-composed "Column=value" SET fragment, spliced in as raw SQL
    // text. Returns whether a row changed.
    virtual bool tinysaveCastle(const std::string& fieldFragment, ZoneID_t zoneID, int serverID) = 0;

    // --- sweeper bonuses ----------------------------------------------------
    // MAX(Type); false when the table is empty.
    virtual bool loadMaxSweeperBonusType(int& maxType) = 0;
    virtual std::vector<SweeperBonusRow> loadSweeperBonuses() = 0;
    virtual std::vector<SweeperBonusOwnerRow> loadSweeperBonusOwners(int level) = 0;
    virtual void saveSweeperBonusOwner(Race_t ownerRace, SweeperBonusType_t type) = 0;

    // --- sweeper safes and owners --------------------------------------------
    virtual std::vector<SweeperSetRow> loadSweeperSets(ZoneID_t zoneID) = 0;
    virtual std::vector<SweeperOwnerRow> loadSweeperOwners(ZoneID_t zoneID) = 0;
    // LevelWarManager's two-column read of the same table.
    virtual std::vector<SweeperBonusOwnerRow> loadSweeperOwnerRaces(ZoneID_t zoneID) = 0;
    // Parameters in the order the UPDATE streams them.
    virtual void saveSweeperOwner(int ownerRace, int safeType, uint itemType) = 0;

    // --- level-war history ----------------------------------------------------
    virtual void insertLevelWarHistory(int level, const std::string& levelWarID, const std::string& slayerOld,
                                       const std::string& vampireOld, const std::string& oustersOld,
                                       const std::string& defaultOld) = 0;
    virtual void updateLevelWarHistory(const std::string& slayerNew, const std::string& vampireNew,
                                       const std::string& oustersNew, const std::string& defaultNew, int level,
                                       const std::string& levelWarID) = 0;


    // --- guild-war history ------------------------------------------------
    // An INSERT IGNORE: a repeated start for the same WarID is dropped.
    virtual void insertGuildWarHistory(int warID, const std::string& guildWarID, int serverID,
                                       const std::string& castleName, int defenseGuildID,
                                       const std::string& defenseGuildName, int attackGuildID,
                                       const std::string& attackGuildName) = 0;
    virtual void updateGuildWarWinner(int winnerGuildID, const std::string& winnerGuildName, int warID) = 0;

    // --- race-war history -------------------------------------------------
    // The totals RaceWar::recordRaceWarStart sums before writing its row.
    virtual std::vector<RaceCurrentNumRow> loadRaceWarCurrentNums() = 0;
    // A plain INSERT, unlike the guild war's IGNORE: a repeated start for
    // the same RaceWarID adds a second row (the table is keyless).
    virtual void insertRaceWarHistory(const std::string& raceWarID, uint slayerNum, uint vampireNum, uint oustersNum,
                                      const std::string& slayerOld, const std::string& vampireOld,
                                      const std::string& oustersOld) = 0;
    virtual void updateRaceWarBloodBibles(const std::string& slayerNew, const std::string& vampireNew,
                                          const std::string& oustersNew, const std::string& raceWarID) = 0;

    // --- race-war entry limits (RaceWarPCLimit) -----------------------------
    // `tableName` is the caller's getTableName(), spliced through "%s" as
    // raw SQL text; every override (Slayer, Vampire, Ousters) returns
    // "RaceWarPCLimit".
    virtual std::vector<RaceWarLimitRow> loadRaceWarLimits(const std::string& tableName, int race) = 0;
    // Every row of the table, not just one race's.
    virtual void clearRaceWarCurrentNums(const std::string& tableName) = 0;
    // Keyed on the row's own ID.
    virtual void saveRaceWarCurrentNum(const std::string& tableName, int currentNum, int id) = 0;

    // --- race-war participants (RaceWarPCList) ------------------------------
    // RaceWarLimiter::clearPCList reads the list, logs it to a file and then
    // empties the table; the two statements are separate here and the
    // caller's log loop sits between them.
    //
    // WARNING — the row's `race` is getInt(COLUMN 1), which is Name, not
    // Race. getInt is atoi, so the value is 0 for any name that does not
    // begin with a digit or sign, and the caller's per-race tally is
    // wrong. The caller indexes a three-element array with it: a name
    // parsing to 0, 1 or 2 lands in the wrong bucket, and one parsing to
    // 3 or more — or to a negative, atoi honouring a leading sign — writes
    // outside the array altogether.
    virtual std::vector<RaceWarPCListRow> loadRaceWarPCList() = 0;
    virtual void deleteRaceWarPCList() = 0;
    // An INSERT IGNORE, Name being the table's PRIMARY KEY, so re-joining
    // is dropped rather than failing.
    virtual void insertRaceWarPCListEntry(const std::string& name, int race) = 0;
    // COUNT(*) of rows for the name; the caller compares against 0.
    virtual int countRaceWarPCListEntries(const std::string& name) = 0;
    virtual void deleteRaceWarPCListEntry(const std::string& name) = 0;

    // --- siege-war reinforcement registry ----------------------------------
    // All scoped to (WarID, ServerID). serverID is
    // g_pConfig->getPropertyInt("ServerID"), an int rendered through "%u".
    virtual int countWaitingReinforceRegistrations(WarID_t warID, int serverID) = 0;
    virtual int countDeniedReinforceRegistrations(WarID_t warID, int serverID, GuildID_t guildID) = 0;
    // The first WAIT registration's guild; false when there is none and the
    // caller keeps its own 0.
    virtual bool loadWaitingReinforceGuild(WarID_t warID, int serverID, GuildID_t& guildID) = 0;
    virtual void insertReinforceRegistration(WarID_t warID, int serverID, GuildID_t guildID) = 0;
    // Both return whether a row actually changed (getAffectedRowCount() > 0).
    virtual bool acceptReinforceRegistration(WarID_t warID, int serverID, GuildID_t guildID) = 0;
    virtual bool denyReinforceRegistration(WarID_t warID, int serverID, GuildID_t guildID) = 0;
    virtual void deleteReinforceRegistrations(WarID_t warID, int serverID) = 0;

    // --- scheduled wars ----------------------------------------------------
    // War::initWarIDRegistry's two probes. Both read column 1 through
    // getDWORD without checking next(): a COUNT(*) always answers with one
    // row, and the caller only asks for the MAX after the count came back
    // non-zero, so the NULL an empty table would yield never reaches
    // getDWORD.
    virtual int countWarSchedules() = 0;
    virtual DWORD loadMaxWarID() = 0;
    // WarSchedule::create and WarSchedule::save. Both report whether a row
    // actually changed (getAffectedRowCount() > 0); the callers log to
    // WarError.log and give up when nothing did. Every numeric renders
    // through "%u"; the two literals carry a tab run before VALUES.
    virtual bool insertWarSchedule(int warID, int serverID, int zoneID, const std::string& warType, int attackGuildID,
                                   int warFee, const std::string& startTime, const std::string& status) = 0;
    virtual bool replaceWarSchedule(int warID, int serverID, int zoneID, const std::string& warType, int attackerCount,
                                    int attackGuildID, int attackGuildID2, int attackGuildID3, int attackGuildID4,
                                    int attackGuildID5, int warFee, const std::string& startTime,
                                    const std::string& status) = 0;
    // A caller-composed "Column=value" SET fragment, spliced in as raw SQL
    // text. The DWORD war id goes through "%d".
    virtual void tinysaveWarSchedule(const std::string& fieldFragment, WarID_t warID, int serverID) = 0;

    // The WAIT and START schedules of one zone, in StartTime order.
    // serverID is g_pConfig->getPropertyInt("ServerID"); both ints render
    // through "%u".
    virtual std::vector<WarScheduleRow> loadWarSchedules(int serverID, int zoneID) = 0;
    // The first ACCEPT registration of a war id, with NO server id — unlike
    // loadWaitingReinforceGuild, which is server-scoped and reads
    // Status='WAIT'. False when there is none, and the caller then leaves
    // the war's reinforce guild alone.
    virtual bool loadAcceptedReinforceGuild(WarID_t warID, int& guildID) = 0;
    // The literal carries four tabs after the zone id, and the zone id goes
    // through "%d" here where the load uses "%u".
    virtual void cancelGuildWarSchedules(int serverID, int zoneID) = 0;

    // --- master lairs -----------------------------------------------------------
    virtual std::vector<MasterLairRow> loadMasterLairs() = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLWarInfoRepository.cpp.
WarInfoRepository& defaultWarInfoRepository();

#endif
