#ifndef __CHARACTER_REPOSITORY_H__
#define __CHARACTER_REPOSITORY_H__

#include <string>

#include "CharacterRace.h"
#include "Types.h"

// Persistence seam for the character row's non-load writes (task 3.2):
// the periodic vitals/position save, the exp/fame/rank tail save, and
// the caller-composed tinysave fragments. The race tables carry
// different columns, so the records are per-race; each field mirrors the
// ORIGINAL EXPRESSION's type — the member/getter type, or int where the
// inline SQL applied an explicit (int) cast — so the varargs bytes
// reaching the format strings are unchanged. The load() SELECTs stay
// inline in the race classes for now — they are the next extraction
// round.

// Slayer save(): CurrentHP/HP/CurrentMP/MP/ZoneID/XCoord/YCoord.
struct SlayerVitalsRecord {
    HP_t currentHP;
    HP_t maxHP;
    MP_t currentMP;
    MP_t maxMP;
    ZoneID_t zoneID;
    int x;
    int y;
};

// Vampire save(): no MP columns; SilverDamage rides along instead.
struct VampireVitalsRecord {
    int currentHP;
    int maxHP;
    int silverDamage;
    int zoneID;
    int x;
    int y;
};

// Ousters save(): same column set as the slayer's, streamed as ints.
struct OustersVitalsRecord {
    int currentHP;
    int maxHP;
    int currentMP;
    int maxMP;
    int zoneID;
    int x;
    int y;
};

// Slayer saveExps(): the attr/domain goal-exp tail plus fame, rank and
// the advancement block.
struct SlayerExpsRecord {
    Exp_t strGoalExp;
    Exp_t dexGoalExp;
    Exp_t intGoalExp;
    Exp_t bladeGoalExp;
    Exp_t swordGoalExp;
    Exp_t gunGoalExp;
    Exp_t enchantGoalExp;
    Exp_t healGoalExp;
    Exp_t etcGoalExp;
    Alignment_t alignment;
    Fame_t fame;
    Rank_t rank;
    RankExp_t rankGoalExp;
    Level_t advancementClass;
    Exp_t advancementGoalExp;
    Attr_t advancedSTR;
    Attr_t advancedDEX;
    Attr_t advancedINT;
    Attr_t advancedAttrBonus;
};

// Vampire saveExps(): SilverDamage is written ONLY when non-zero — the
// implementation preserves the conditional-fragment quirk.
struct VampireExpsRecord {
    Alignment_t alignment;
    Fame_t fame;
    Exp_t goalExp;
    Silver_t silverDamage;
    Rank_t rank;
    RankExp_t rankGoalExp;
    Level_t advancementClass;
    Exp_t advancementGoalExp;
};

// Ousters saveExps(): SilverDamage is written unconditionally.
struct OustersExpsRecord {
    Alignment_t alignment;
    Fame_t fame;
    Exp_t goalExp;
    Silver_t silverDamage;
    Rank_t rank;
    RankExp_t rankGoalExp;
    Level_t advancementClass;
    Exp_t advancementGoalExp;
};

class CharacterRepository {
public:
    virtual ~CharacterRepository() {}

    // The periodic save() row update — vitals and position.
    virtual void saveSlayerVitals(const std::string& ownerName, const SlayerVitalsRecord& record) = 0;
    virtual void saveVampireVitals(const std::string& ownerName, const VampireVitalsRecord& record) = 0;
    virtual void saveOustersVitals(const std::string& ownerName, const OustersVitalsRecord& record) = 0;

    // The saveExps() tail — flushed on logout so the sub-threshold exp
    // the handlers batch up (they persist only every 10th tick) is not
    // lost.
    virtual void saveSlayerExps(const std::string& ownerName, const SlayerExpsRecord& record) = 0;
    virtual void saveVampireExps(const std::string& ownerName, const VampireExpsRecord& record) = 0;
    virtual void saveOustersExps(const std::string& ownerName, const OustersExpsRecord& record) = 0;

    // tinysave: applies a caller-composed "Column=value, ..." SET
    // fragment to the character's own race table. The fragment is raw
    // SQL text built by dozens of call sites (sprintf into char[80]
    // buffers) — a legacy quirk this seam quarantines but cannot yet
    // retire; narrowing it to typed columns is later work.
    virtual void tinysave(const std::string& ownerName, CharacterRace race, const std::string& fieldFragment) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLCharacterRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
CharacterRepository& defaultCharacterRepository();

#endif
