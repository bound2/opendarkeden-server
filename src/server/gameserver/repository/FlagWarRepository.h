#ifndef __FLAG_WAR_REPOSITORY_H__
#define __FLAG_WAR_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Persistence seam for the three tables the capture-the-flag system owns
// (task 3.2): the flag-pole fields loaded at boot (FlagPolePosition), the
// per-round tally of who planted which flag (FlagWarStat) and the
// per-round history rolled up from it (FlagWarHistory).
//
// Reads are typed to the driver getter the inline code called — every
// column here came back through getInt or getString — and the CASTS the
// call sites applied to those ints (ZoneID_t, ZoneCoord_t, Race_t,
// MonsterType_t) stay at the call sites, so nothing about how a row
// becomes game state moves behind the seam.

// FlagManager::init — one flag-pole field. Note race: the column is a
// MySQL ENUM('SLAYER','VAMPIRE','OUSTERS'), and the statement selects
// "Race-1", which makes MySQL yield the enum's 1-based INDEX and
// subtract one. So race is already 0/1/2 and matches Race_t, and the
// "-1" is load bearing rather than a typo.
struct FlagPoleRow {
    int zoneID;
    int centerX;
    int centerY;
    int width;
    int height;
    int race;
    int monsterType;
};

// FlagManager::recordFlagWarHistory — one row of the GROUP BY that rolls
// FlagWarStat up per player per server. flagNum is the count(*).
//
// WARNING: the statement behind this cannot run under the sql_mode
// this project requires. It groups by (Name, ServerID) while selecting
// PlayerID and Race bare; FlagWarStat has no unique key, so no
// functional dependency saves it, and CLAUDE.md's sql_mode keeps
// ONLY_FULL_GROUP_BY. MySQL refuses it with error 1055 and
// loadFlagWarStatTotals throws on every call it receives.
//
// It receives none on a default deployment: ActiveFlagWar is 0 in both
// shipped gameserver.conf files, and ClientManager only ticks
// FlagManager when it is on. So the roll-up is dead twice over —
// unreachable by configuration, and refused by the server if an
// operator or a GM turns the flag war on.
//
// And when it IS reached, the throw does not stop at the caller. The
// const char* END_DB rethrows is not a Throwable, so nothing between
// endFlagWar and main.cpp's catch (...) catches it: the first flag war
// that ends takes the gameserver process down. That is the inline
// statement's behaviour, moved unchanged and pinned by the integration
// tier; see MySQLFlagWarRepository.cpp.
struct FlagWarStatTotalRow {
    std::string playerID;
    std::string name;
    int race;
    int serverID;
    int flagNum;
};

class FlagWarRepository {
public:
    virtual ~FlagWarRepository() {}

    // FlagManager::init, at boot.
    virtual std::vector<FlagPoleRow> loadFlagPoles() = 0;

    // FlagManager::resetFlagCounts — the whole tally, unconditionally.
    // Called at round START (from startFlagWar), not between rounds as
    // an earlier draft of this comment said.
    virtual void deleteAllFlagWarStats() = 0;

    // FlagManager::recordPutFlag. The tally is keyed by (Name, ItemID)
    // only as an INDEX, not a unique constraint, so the caller checks
    // before inserting rather than relying on the database to refuse a
    // duplicate.
    virtual bool flagStatExists(const std::string& name, ItemID_t itemID) = 0;
    virtual void insertFlagStat(const std::string& playerID, const std::string& name, int race, int serverID,
                                ItemID_t itemID) = 0;

    // FlagManager::recordFlagWarHistory, at the end of a round.
    virtual std::vector<FlagWarStatTotalRow> loadFlagWarStatTotals() = 0;
    virtual void insertFlagWarHistory(const std::string& flagWarID, const std::string& playerID,
                                      const std::string& name, int race, int serverID, int flagNum) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLFlagWarRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
FlagWarRepository& defaultFlagWarRepository();

#endif
