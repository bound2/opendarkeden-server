#ifndef __CHARACTER_REPOSITORY_H__
#define __CHARACTER_REPOSITORY_H__

#include <string>

#include "CharacterRace.h"
#include "Types.h"

// The character row: the load() SELECT each race class runs at login, the
// periodic vitals/position save, the exp/fame/rank tail save, and the
// caller-composed tinysave fragments. The race tables carry different
// columns, so the records are per-race. Load fields are typed to the
// driver getter used for the column (int for getInt, BYTE for getBYTE,
// std::string for getString); any narrowing happens in the race class
// when it hands the value to a setter. Write fields carry the member type
// the caller streams, or int where the caller casts.

// Slayer load(): the 54 columns of the Slayer row, in SELECT order. Every
// selected column is surfaced, including ones the loader then overrides
// or ignores: sight (overridden to 13 right after it is applied) and
// reward (selected, never acted on). phone is the raw varchar(7); the
// loader atoi()s it.
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
// stashNum, competence and competenceShape are read through getBYTE (the
// slayer reads Competence through getInt); the columns are tinyint
// unsigned, so nothing is lost, and the loader's ">= 4 → 3" clamp
// compares the BYTE. reward: selected, never acted on. No MP columns;
// SilverDamage instead.
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

// Vampire saveExps(): SilverDamage is written ONLY when non-zero.
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

// Two callers send loadSlayerPlayerID's statement with different bytes
// ("WHERE Name='%s'" for the whisper lookup, lower-case "where" for the GM
// ban); the enum selects which text is sent.
enum SlayerPlayerIDSpelling { PLAYERID_SPELLING_WHISPER, PLAYERID_SPELLING_OPDENY, PLAYERID_SPELLING_MAX };

// The GM guild-master check: Fame, BladeLevel, SwordLevel, GunLevel,
// HealLevel, EnchantLevel of one Slayer row, every column through getInt.
struct SlayerMasterStatsRow {
    int fame;
    int bladeLevel;
    int swordLevel;
    int gunLevel;
    int healLevel;
    int enchantLevel;
};

class CharacterRepository {
public:
    virtual ~CharacterRepository() {}

    // The login-time load: the character's ACTIVE row from its own race
    // table. False when there is none (the name has no row, or the row is
    // INACTIVE — the login server may have deleted the character while it
    // was handed over); on true, record carries every selected column.
    virtual bool loadSlayer(const std::string& ownerName, SlayerLoadRecord& record) = 0;
    // The connect-time probe that takes the character's race from the
    // database rather than the type the client sent. False unless EXACTLY
    // one row; the caller treats none and several alike (logs to
    // connectDB_BUG.txt and throws ProtocolException). Every character has
    // a Slayer row whatever its race, so one probe covers all three.
    virtual bool loadSlayerAccount(const std::string& name, std::string& playerID, std::string& race) = 0;
    // Name-to-account lookup (the whisper spelling). Answers on the FIRST
    // row rather than requiring exactly one, so on a duplicate name this
    // returns a value where loadSlayerAccount returns false.
    virtual bool loadSlayerPlayerID(const std::string& name, std::string& playerID) = 0;
    // The same lookup with the spelling chosen; the overload above is the
    // whisper spelling and delegates here.
    virtual bool loadSlayerPlayerID(SlayerPlayerIDSpelling spelling, const std::string& name,
                                    std::string& playerID) = 0;
    // The GM guild-master checks: the six Slayer columns, and one race's
    // Level, each through getInt (the caller narrows into Fame_t /
    // SkillLevel_t / Level_t); false when the name has no row in that
    // table.
    virtual bool loadSlayerMasterStats(const std::string& name, SlayerMasterStatsRow& row) = 0;
    virtual bool loadVampireLevel(const std::string& name, int& level) = 0;
    virtual bool loadOustersLevel(const std::string& name, int& level) = 0;
    virtual bool loadVampire(const std::string& ownerName, VampireLoadRecord& record) = 0;
    virtual bool loadOusters(const std::string& ownerName, OustersLoadRecord& record) = 0;

    // The vampire's attribute-redistribution counter, a column the load
    // SELECT does not name. The read is false when the vampire has no row;
    // the write stores the int the caller computed.
    virtual bool loadVampireRedistributeAttr(const std::string& name, int& redistributeAttr) = 0;
    virtual void saveVampireRedistributeAttr(int redistributeAttr, const std::string& name) = 0;

    // Slayer.Race as the text getString returns ('SLAYER' / 'VAMPIRE' /
    // ...); false when the name has no Slayer row. Reads the SLAYER table
    // for every race: that table is the character index.
    virtual bool loadSlayerRaceText(const std::string& name, std::string& raceText) = 0;
    // GuildID from the race's table, through getInt; the caller casts to
    // GuildID_t and applies its 0 / 99 / 66 rule. False when no row.
    virtual bool loadGuildID(const std::string& name, CharacterRace race, int& guildID) = 0;
    // "UPDATE Slayer SET SEX='%s' WHERE Name='%s'" then the same on
    // Vampire, on one Statement. The Ousters table has no statement here;
    // the caller returns before reaching this for anything but a slayer or
    // a vampire. The text is the caller's Sex2String entry.
    virtual void saveSex(const std::string& name, const std::string& sexText) = 0;

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

    // tinysave: applies a caller-composed "Column=value, ..." SET fragment
    // to the character's own race table. The fragment is raw SQL text
    // built by dozens of call sites (sprintf into char[80] buffers) and is
    // interpolated unescaped.
    virtual void tinysave(const std::string& ownerName, CharacterRace race, const std::string& fieldFragment) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLCharacterRepository.cpp.
CharacterRepository& defaultCharacterRepository();

#endif
