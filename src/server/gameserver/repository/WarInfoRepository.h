#ifndef __WAR_INFO_REPOSITORY_H__
#define __WAR_INFO_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Persistence seam for the race-war tables (task 3.2): the shrines and
// their owning race (ShrineInfo), the castles and their guild/tax state
// (CastleInfo), the level-war sweeper bonuses (SweeperBonusInfo), the
// sweeper safes and owners (SweeperSetInfo, SweeperOwnerInfo), the
// three war histories (LevelWarHistory, GuildWarHistory and
// RaceWarHistory), the race-war entry limits (RaceWarPCLimit, both
// the rows themselves and the per-race totals one history records),
// the participant list (RaceWarPCList), the siege-war reinforcement
// registry (ReinforceRegisterInfo), the scheduled wars
// (WarScheduleInfo) and the master lairs (MasterLairInfo). Reads are
// typed to the driver getter the inline code called (getInt → int,
// getString → std::string); the writes'
// parameters are typed to the member/getter each caller streamed, so
// the varargs bytes reaching the format strings are unchanged.
//
// Other users of these tables keep their own inline SQL: the guild
// managers (gameserver and sharedserver) count and list castles by
// guild; GuildRepository::countReinforceRegistrations joins
// ReinforceRegisterInfo to WarScheduleInfo for a guild-wide count, a
// different statement from the war-scoped ones here; and
// war/WarScheduler.cpp reads the ACCEPT registrations of a war id
// without a server id. war/WarScheduler.cpp also keeps its own
// WarScheduleInfo statements — the conditional per-zone load and the
// guild-schedule cancel — which are not the ones enclosed here.
// war/RaceWar.cpp's two shrine-owner reads are no longer among them
// — they call loadShrineOwners() now.

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

// RaceWar::recordRaceWarStart's per-race totals: SUM(CurrentNum) over
// RaceWarPCLimit, grouped by race. Both columns come back through getInt,
// as the inline read took them.
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

// RaceWarLimiter::clearPCList's row. `race` is deliberately read from
// column 1 — the Name column — because that is the column the inline
// loop passed to getInt. See the note above loadRaceWarPCList().
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
    // ShrineSet::saveBloodBibleOwner — the two ints the original cast at
    // the call.
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
    // SweeperSetManager::saveSweeperOwner declares (uint itemType, int
    // safeType, int ownerRace); the seam takes the same three types in the
    // order the UPDATE streams them.
    virtual void saveSweeperOwner(int ownerRace, int safeType, uint itemType) = 0;

    // --- level-war history ----------------------------------------------------
    virtual void insertLevelWarHistory(int level, const std::string& levelWarID, const std::string& slayerOld,
                                       const std::string& vampireOld, const std::string& oustersOld,
                                       const std::string& defaultOld) = 0;
    virtual void updateLevelWarHistory(const std::string& slayerNew, const std::string& vampireNew,
                                       const std::string& oustersNew, const std::string& defaultNew, int level,
                                       const std::string& levelWarID) = 0;


    // --- guild-war history ------------------------------------------------
    // GuildWar::recordGuildWarStart / recordGuildWarEnd. Every argument is
    // the (int) cast or c_str() the caller already applied; the INSERT is an
    // INSERT IGNORE, so a repeated start for the same WarID is dropped.
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
    // PCWarLimiter's three statements. `tableName` is the caller's
    // getTableName(), spliced through "%s" as raw SQL text — the same
    // quarantine as tinysaveCastle. The hook is polymorphic in form only:
    // all three overrides (Slayer, Vampire, Ousters) return
    // "RaceWarPCLimit".
    virtual std::vector<RaceWarLimitRow> loadRaceWarLimits(const std::string& tableName, int race) = 0;
    // PCWarLimiter::clearCurrent — every row of the table, not just this
    // limiter's race.
    virtual void clearRaceWarCurrentNums(const std::string& tableName) = 0;
    // PCWarLimiter::saveCurrent, keyed on the row's own ID.
    virtual void saveRaceWarCurrentNum(const std::string& tableName, int currentNum, int id) = 0;

    // --- race-war participants (RaceWarPCList) ------------------------------
    // RaceWarLimiter::clearPCList reads the list, logs it to a file and then
    // empties the table. The two statements are separate here; the caller's
    // log loop still sits between them, so the order on the wire is
    // unchanged.
    //
    // WARNING — the row's `race` is what the inline loop actually read:
    // getInt(COLUMN 1), which is Name, not Race. getInt is atoi, so the
    // value is 0 for any name that does not begin with a digit or sign,
    // and the caller's per-race tally is wrong. The caller indexes a
    // three-element array with it: a name parsing to 0, 1 or 2 lands in
    // the wrong bucket, and one parsing to 3 or more — or to a negative,
    // atoi honouring a leading sign — writes outside the array
    // altogether. Preserved here byte-for-byte rather than quietly
    // corrected: the fix is a behaviour change and wants its own round.
    virtual std::vector<RaceWarPCListRow> loadRaceWarPCList() = 0;
    virtual void deleteRaceWarPCList() = 0;
    // RaceWarLimiter::addPCList — an INSERT IGNORE, Name being the table's
    // PRIMARY KEY, so re-joining is dropped rather than failing.
    virtual void insertRaceWarPCListEntry(const std::string& name, int race) = 0;
    // RaceWarLimiter::isInPCList. A COUNT(*) always answers with exactly one
    // row, so the inline "if (pResult->next())" guard could never fail; the
    // caller compares the count against 0 as it did.
    virtual int countRaceWarPCListEntries(const std::string& name) = 0;
    virtual void deleteRaceWarPCListEntry(const std::string& name) = 0;

    // --- siege-war reinforcement registry ----------------------------------
    // SiegeWar's six statements, all scoped to (WarID, ServerID). serverID is
    // g_pConfig->getPropertyInt("ServerID"), an int the format strings render
    // through "%u" as before.
    // The two counts a COUNT(*) always answers with exactly one row, so the
    // inline "if (pResult->next())" guards could never fail (the same
    // reasoning BalanceInfoRepository.h records for its MAX read).
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
    // War::initWarIDRegistry's two probes. Both call next() without checking
    // it and read column 1 through getDWORD, as the inline code did: a
    // COUNT(*) always answers with one row, and the caller only asks for the
    // MAX after the count came back non-zero, so the NULL an empty table
    // would yield never reaches getDWORD.
    virtual int countWarSchedules() = 0;
    virtual DWORD loadMaxWarID() = 0;
    // WarSchedule::create and WarSchedule::save. Both report whether a row
    // actually changed (getAffectedRowCount() > 0); the callers log to
    // WarError.log and give up when nothing did. Every numeric is the (int)
    // the caller cast at the call and still renders through "%u"; the two
    // literals keep the tab run a backslash-continued source line left in
    // them, before VALUES.
    virtual bool insertWarSchedule(int warID, int serverID, int zoneID, const std::string& warType, int attackGuildID,
                                   int warFee, const std::string& startTime, const std::string& status) = 0;
    virtual bool replaceWarSchedule(int warID, int serverID, int zoneID, const std::string& warType, int attackerCount,
                                    int attackGuildID, int attackGuildID2, int attackGuildID3, int attackGuildID4,
                                    int attackGuildID5, int warFee, const std::string& startTime,
                                    const std::string& status) = 0;
    // WarSchedule::tinysave — a caller-composed "Column=value" SET fragment
    // (raw SQL text, the same quarantine as CastleInfoManager::tinysave).
    // The DWORD war id goes through "%d", as before.
    virtual void tinysaveWarSchedule(const std::string& fieldFragment, WarID_t warID, int serverID) = 0;

    // --- master lairs -----------------------------------------------------------
    virtual std::vector<MasterLairRow> loadMasterLairs() = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLWarInfoRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
WarInfoRepository& defaultWarInfoRepository();

#endif
