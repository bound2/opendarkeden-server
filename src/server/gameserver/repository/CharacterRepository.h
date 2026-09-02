#ifndef __CHARACTER_REPOSITORY_H__
#define __CHARACTER_REPOSITORY_H__

#include <string>

#include "CharacterRace.h"
#include "Types.h"

// Persistence seam for the character row (task 3.2): the load() SELECT
// each race class runs at login, the periodic vitals/position save, the
// exp/fame/rank tail save, and the caller-composed tinysave fragments.
// The race tables carry different columns, so the records are per-race;
// each field mirrors the ORIGINAL EXPRESSION's type. For the writes that
// is the member/getter type, or int where the inline SQL applied an
// explicit (int) cast — so the varargs bytes reaching the format strings
// are unchanged. For the loads it is the driver getter the inline code
// called on that column — int for getInt, BYTE for getBYTE, std::string
// for getString — so every narrowing the race class performed when it
// handed the value to a setter still happens THERE, on the same value,
// and the record adds no conversion of its own.

// Slayer load(): the 54 columns of the Slayer row, in SELECT order.
// Every column the SELECT names is surfaced, including the ones the
// loader then overrides or ignores: sight (overridden to 13 right after
// it is applied) and reward (the reward flow that consumed it is dead —
// the column is selected, never acted on). phone is the raw varchar(7);
// the loader atoi()s it, as it always did.
struct SlayerLoadRecord {
    std::string name;
    int advancementClass;
    int advancementGoalExp;
    int competence;
    int competenceShape;
    std::string sex;
    int masterEffectColor;
    std::string hairStyle;
    int hairColor;
    int skinColor;
    std::string phone;
    int str;
    int strGoalExp;
    int dex;
    int dexGoalExp;
    int inte;
    int intGoalExp;
    int advancedSTR;
    int advancedDEX;
    int advancedINT;
    int bonus;
    int rank;
    int rankGoalExp;
    int currentHP;
    int maxHP;
    int currentMP;
    int maxMP;
    int fame;
    int gold;
    int guildID;
    int bladeLevel;
    int bladeGoalExp;
    int swordLevel;
    int swordGoalExp;
    int gunLevel;
    int gunGoalExp;
    int enchantLevel;
    int enchantGoalExp;
    int healLevel;
    int healGoalExp;
    int etcLevel;
    int etcGoalExp;
    int zoneID;
    int x;
    int y;
    int sight;
    int gunBonusExp;
    int rifleBonusExp;
    int alignment;
    int stashGold;
    BYTE stashNum;
    int resurrectZone;
    int reward;
    int smsCharge;
};

// Vampire load(): the 33 columns of the Vampire row, in SELECT order.
// stashNum, competence and competenceShape were read through getBYTE
// (the slayer reads Competence through getInt); the columns are tinyint
// unsigned, so the narrower getter loses nothing — the type is kept so
// the loader's ">= 4 → 3" clamp compares what it always compared.
// reward: dead flow, as for the slayer. No MP columns; SilverDamage
// instead.
struct VampireLoadRecord {
    std::string name;
    int advancementClass;
    int advancementGoalExp;
    std::string sex;
    int masterEffectColor;
    int batColor;
    int skinColor;
    int str;
    int dex;
    int inte;
    int maxHP;
    int currentHP;
    int fame;
    int goalExp;
    int level;
    int bonus;
    int gold;
    int guildID;
    int zoneID;
    int x;
    int y;
    int sight;
    int alignment;
    int stashGold;
    BYTE stashNum;
    BYTE competence;
    BYTE competenceShape;
    int resurrectZone;
    int silverDamage;
    int reward;
    int smsCharge;
    int rank;
    int rankGoalExp;
};

// Ousters load(): the 34 columns of the Ousters row, in SELECT order —
// the vampire's shape plus MP, SkillBonus and HairColor, minus Reward.
struct OustersLoadRecord {
    std::string name;
    int advancementClass;
    int advancementGoalExp;
    std::string sex;
    int masterEffectColor;
    int str;
    int dex;
    int inte;
    int maxHP;
    int currentHP;
    int maxMP;
    int currentMP;
    int fame;
    int goalExp;
    int level;
    int bonus;
    int skillBonus;
    int gold;
    int guildID;
    int zoneID;
    int x;
    int y;
    int sight;
    int alignment;
    int stashGold;
    BYTE stashNum;
    BYTE competence;
    BYTE competenceShape;
    int resurrectZone;
    int silverDamage;
    int smsCharge;
    int rank;
    int rankGoalExp;
    int hairColor;
};

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

    // The login-time load: the character's ACTIVE row from its own race
    // table. Returns false when there is none (the name has no row, or
    // the row is INACTIVE — the login server may have deleted the
    // character while it was handed over); on true, record carries
    // every selected column.
    virtual bool loadSlayer(const std::string& ownerName, SlayerLoadRecord& record) = 0;
    virtual bool loadVampire(const std::string& ownerName, VampireLoadRecord& record) = 0;
    virtual bool loadOusters(const std::string& ownerName, OustersLoadRecord& record) = 0;

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
