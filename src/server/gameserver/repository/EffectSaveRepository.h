#ifndef __EFFECT_SAVE_REPOSITORY_H__
#define __EFFECT_SAVE_REPOSITORY_H__

#include <string>
#include <vector>

#include <sys/time.h>

#include "Types.h"

// Persistence seam for the effects that survive a logout (task 3.2): the
// thirteen one-table-per-effect saves the Effect*.cpp classes run from
// their create()/destroy()/save() overrides and their *Loader::load().
// Four shapes of table:
//  - DEADLINE tables (YearTime, DayTime): EffectAftermath,
//    EffectKillAftermath, EffectMute, CanEnterGDRLair, EffectRestore.
//    DayTime is the absolute expiry (m_Deadline.tv_sec, a Unix time);
//    YearTime is the write's own timestamp in "year time" and is never
//    read back.
//  - REMAIN tables (RemainTime): the three force scrolls. RemainTime is
//    the REMAINING duration in turns at the moment of the write, so a
//    loaded scroll restarts with the time it had left — logged-out time
//    does not count against it, unlike the deadline effects.
//  - EnemyErase: a deadline table with an EnemyName column and one row
//    per enemy (the DELETE keys on OwnerID AND EnemyName; the UPDATE
//    does not, and rewrites every row of the owner — see the MySQL
//    implementation).
//  - PER-CREATURE tables: the deadline core plus their own columns —
//    EffectBloodDrain (Level), EffectFlare and EffectLight (OldSight),
//    EffectYellowPoisonToCreature (Level and OldSight). Their loaders
//    read BOTH timestamps and recompute the remaining turns from the
//    year-time difference as well as the day-time one, where the
//    deadline loaders read DayTime alone — except EffectBloodDrain's,
//    which reads DayTime alone too.
// Every table is keyless (an OwnerID index only) EXCEPT
// EffectKillAftermath, which has OwnerID as its primary key — a second
// create() for the same owner accumulates a duplicate row on the twelve
// keyless tables and raises ER_DUP_ENTRY on that one. EffectLight is
// keyless in the strongest sense: its CREATE TABLE carries no index at
// all, not even the OwnerID one the other twelve have.
//
// The per-character purges in CreatureUtil.cpp and the loginserver's
// CLDeletePCHandler DELETE from only THREE of these tables
// (EffectAftermath, EffectMute, EnemyErase) as part of their multi-table
// character deletion — not enclosed here. Nothing purges the other five:
// an EffectKillAftermath, CanEnterGDRLair or force-scroll row outlives
// its character, and a name-reuser inherits it on first login (the same
// quirk BloodBibleSignRepository documents for its table).

enum DeadlineEffectTable {
    EFFECT_TABLE_AFTERMATH,
    EFFECT_TABLE_KILL_AFTERMATH,
    EFFECT_TABLE_MUTE,
    EFFECT_TABLE_CAN_ENTER_GDR_LAIR,
    EFFECT_TABLE_RESTORE,
    DEADLINE_EFFECT_TABLE_MAX
};

enum RemainEffectTable {
    EFFECT_TABLE_SAFE_FORCE_SCROLL,
    EFFECT_TABLE_BEHEMOTH_FORCE_SCROLL,
    EFFECT_TABLE_CARNELIAN_FORCE_SCROLL,
    REMAIN_EFFECT_TABLE_MAX
};

// What loadEnemyErases() returns: dayTime as the driver's getDWORD
// returned it, enemyName as getString did.
struct EnemyEraseRow {
    DWORD dayTime;
    std::string enemyName;
};

// The four per-creature effect tables: the deadline core plus their own
// columns. All four are keyless, so a second create() for the same owner
// leaves a duplicate row behind, and every load consumes whatever rows it
// finds — the loaders build one effect object per row.
enum CreatureEffectTable {
    CREATURE_EFFECT_BLOOD_DRAIN,
    CREATURE_EFFECT_FLARE,
    CREATURE_EFFECT_LIGHT,
    CREATURE_EFFECT_YELLOW_POISON_TO_CREATURE,
    CREATURE_EFFECT_TABLE_MAX
};

// What loadCreatureEffects() returns. Each table's SELECT names its own
// columns, so a field its statement does not ask for stays 0:
//  - EffectBloodDrain selects (DayTime, Level): yearTime stays 0.
//  - EffectFlare and EffectLight select (YearTime, DayTime, OldSight):
//    level stays 0.
//  - EffectYellowPoisonToCreature selects all four.
// level carries the driver call its loader made — getBYTE for
// EffectBloodDrain, getInt for EffectYellowPoisonToCreature, which differ
// above 255. Kept per table, not unified.
// OldSight is named by three of the four SELECTs and read by NONE of the
// loaders: all three hard-code the restored sight to 13 instead. The
// column stays in the statements byte for byte, and the seam does not
// fetch it — that read is a driver call the originals never made.
struct CreatureEffectRow {
    DWORD yearTime;
    DWORD dayTime;
    int level;
};

class EffectSaveRepository {
public:
    virtual ~EffectSaveRepository() {}

    // --- deadline tables ------------------------------------------------
    // yearTime is the Turn_t getCurrentYearTime() produced, dayTime the
    // effect's m_Deadline.tv_sec — the same expressions the inline SQL
    // streamed, in the same types.
    virtual void insertDeadline(DeadlineEffectTable table, const std::string& ownerName, Turn_t yearTime,
                                time_t dayTime) = 0;
    virtual void deleteDeadline(DeadlineEffectTable table, const std::string& ownerName) = 0;
    virtual void updateDeadline(DeadlineEffectTable table, const std::string& ownerName, Turn_t yearTime,
                                time_t dayTime) = 0;
    // Every DayTime row the owner has (keyless tables: possibly several),
    // as the driver's getDWORD returned them.
    virtual std::vector<DWORD> loadDeadlines(DeadlineEffectTable table, const std::string& ownerName) = 0;

    // --- remain tables --------------------------------------------------
    virtual void insertRemain(RemainEffectTable table, const std::string& ownerName, Turn_t remainTurn) = 0;
    virtual void deleteRemain(RemainEffectTable table, const std::string& ownerName) = 0;
    virtual void updateRemain(RemainEffectTable table, const std::string& ownerName, Turn_t remainTurn) = 0;
    // The FIRST row's RemainTime (the loaders only ever read one row).
    // Returns false when the owner has none.
    virtual bool loadRemain(RemainEffectTable table, const std::string& ownerName, DWORD& remainTurn) = 0;

    // --- EnemyErase -----------------------------------------------------
    virtual void insertEnemyErase(const std::string& ownerName, Turn_t yearTime, time_t dayTime,
                                  const std::string& enemyName) = 0;
    virtual void deleteEnemyErase(const std::string& ownerName, const std::string& enemyName) = 0;
    virtual void updateEnemyErase(const std::string& ownerName, Turn_t yearTime, time_t dayTime,
                                  const std::string& enemyName) = 0;
    virtual std::vector<EnemyEraseRow> loadEnemyErases(const std::string& ownerName) = 0;

    // --- per-creature tables --------------------------------------------
    // level and oldSight are the (int) casts each caller streamed; a table
    // whose statements name no such column ignores that argument. The
    // callers pass 0 there, as the seam's own tests do.
    virtual void insertCreatureEffect(CreatureEffectTable table, const std::string& ownerName, Turn_t yearTime,
                                      time_t dayTime, int level, int oldSight) = 0;
    virtual void deleteCreatureEffect(CreatureEffectTable table, const std::string& ownerName) = 0;
    virtual void updateCreatureEffect(CreatureEffectTable table, const std::string& ownerName, Turn_t yearTime,
                                      time_t dayTime, int level, int oldSight) = 0;
    // Every row the owner has, in the driver's order (no ORDER BY, as
    // before); the loaders build one effect object per row.
    virtual std::vector<CreatureEffectRow> loadCreatureEffects(CreatureEffectTable table,
                                                               const std::string& ownerName) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLEffectSaveRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
EffectSaveRepository& defaultEffectSaveRepository();

#endif
