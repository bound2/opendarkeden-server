#ifndef __SKILL_SAVE_REPOSITORY_H__
#define __SKILL_SAVE_REPOSITORY_H__

#include <string>
#include <vector>

#include <sys/time.h>

#include "Types.h"

// Persistence seam for the three learned-skill tables (task 3.2):
// SkillSave (slayer), VampireSkillSave and OustersSkillSave. One row per
// (OwnerID, SkillType); the tables are KEYLESS — a non-unique index only
// — so nothing stops a second row for the same skill, and the loaders
// have to cope (the vampire/ousters loaders skip a type they already
// hold; the slayer loader does not).
//
// Two record families per table, on purpose:
//  - the *Row structs are what load() returns: every field typed to the
//    driver getter the inline code called (getInt → int), so the race
//    class performs the same narrowing it always did when it hands the
//    value to a slot setter;
//  - the *Record structs are what the slot classes persist: every field
//    typed to the slot MEMBER it came from, so the varargs bytes reaching
//    the format strings are unchanged.
//
// This seam does not enclose the per-character purges: CreatureUtil.cpp
// and the loginserver's CLDeletePCHandler DELETE all three tables inline
// as part of their multi-table character deletion — that flow is
// extracted with its own repository.

// --- what load() returns ---------------------------------------------------

// SkillSave: nextTime is selected but no loader has ever consumed it —
// the slot's run time is recomputed from "now + delay" on load. Surfaced
// so the row is complete; ignore it like the loaders do.
struct SlayerSkillRow {
    int skillType;
    int skillLevel;
    int skillExp;
    int delay;
    int castingTime;
    int nextTime;
};

// VampireSkillSave: no level or exp — vampire skills do not level.
struct VampireSkillRow {
    int skillType;
    int delay;
    int castingTime;
    int nextTime;
};

// OustersSkillSave: a level but no exp.
struct OustersSkillRow {
    int skillType;
    int skillLevel;
    int delay;
    int castingTime;
    int nextTime;
};

// --- what the slot classes persist ------------------------------------------

// nextTime is the slot's m_runTime.tv_sec — a time_t (8 bytes on the
// deployed x86-64 build) that the format strings have always read
// through %d. That reads the low half of the stack/register slot, which
// holds the whole value for any timestamp below 2^31 — fine until 2038,
// and preserved as-is (the widening/narrowing conversions in this DB
// layer are a follow-up, see MySQLCharacterRepository.cpp).
struct SlayerSkillRecord {
    SkillType_t skillType;
    ExpLevel_t skillLevel;
    Exp_t skillExp;
    Turn_t delay;
    Turn_t castingTime;
    time_t nextTime;
};

struct VampireSkillRecord {
    SkillType_t skillType;
    Turn_t delay;
    Turn_t castingTime;
    time_t nextTime;
};

struct OustersSkillRecord {
    SkillType_t skillType;
    ExpLevel_t skillLevel;
    Turn_t delay;
    Turn_t castingTime;
    time_t nextTime;
};

class SkillSaveRepository {
public:
    virtual ~SkillSaveRepository() {}

    // Every row the owner has, in the order the ORDER-BY-less SELECT
    // returns them — see the MySQL implementation for what that order
    // actually is.
    virtual std::vector<SlayerSkillRow> loadSlayerSkills(const std::string& ownerName) = 0;
    virtual std::vector<VampireSkillRow> loadVampireSkills(const std::string& ownerName) = 0;
    virtual std::vector<OustersSkillRow> loadOustersSkills(const std::string& ownerName) = 0;

    // The slot's create(): a new row. Nothing checks for an existing one
    // (keyless table), exactly like the inline INSERT.
    virtual void insertSlayerSkill(const std::string& ownerName, const SlayerSkillRecord& record) = 0;
    virtual void insertVampireSkill(const std::string& ownerName, const VampireSkillRecord& record) = 0;
    virtual void insertOustersSkill(const std::string& ownerName, const OustersSkillRecord& record) = 0;

    // The slot's save(): the columns that change after learning — level,
    // exp and delay for a slayer skill, delay alone for a vampire skill,
    // level and delay for an ousters skill. CastingTime and NextTime are
    // written once, at insert, and never updated.
    virtual void updateSlayerSkill(const std::string& ownerName, SkillType_t skillType, ExpLevel_t skillLevel,
                                   Exp_t skillExp, Turn_t delay) = 0;
    virtual void updateVampireSkill(const std::string& ownerName, SkillType_t skillType, Turn_t delay) = 0;
    virtual void updateOustersSkill(const std::string& ownerName, SkillType_t skillType, ExpLevel_t skillLevel,
                                    Turn_t delay) = 0;

    // OustersSkillSlot::destroy — the only race whose slots can be
    // unlearned one at a time. Removes every row of that type (keyless
    // table: duplicates go together).
    virtual void deleteOustersSkill(const std::string& ownerName, SkillType_t skillType) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLSkillSaveRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
SkillSaveRepository& defaultSkillSaveRepository();

#endif
