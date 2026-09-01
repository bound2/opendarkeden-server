#ifndef __BALANCE_INFO_REPOSITORY_H__
#define __BALANCE_INFO_REPOSITORY_H__

#include <vector>

// Read-only seam for the level/exp BALANCE tables (task 3.2): the
// per-level exp ladders the gameserver loads once at boot and indexes
// by level — STR/DEX/INT (STRBalanceInfo, DEXBalanceInfo,
// INTBalanceInfo), the vampire and ousters level ladders
// (VampEXPBalanceInfo, OustersEXPBalanceInfo), the rank ladders
// (RankEXPInfo, one per RankType), the skill-domain ladders
// (SkillDomainInfo, one per DomainType), the fame limits (FameLimitInfo,
// one per DomainType), and the pet ladders (PetExpInfo,
// PetAttrBalanceInfo, PetAttrInfo). Every field is typed to the driver
// getter the inline code called (getInt → int), so each caller's
// narrowing (Level_t, Exp_t, SkillBonus_t, PetLevel_t, ...) still
// happens at the caller on the same value.
//
// Every ladder loader first asks for MAX(<key>) to size an array, then
// reads the rows. The MAX probe is exposed as a bool: MySQL answers
// MAX() over an EMPTY table with one row holding NULL, and the driver's
// getInt would then atoi(NULL). The inline code's "no data" checks
// (getRowCount()==0 / !next()) could therefore never fire — an empty
// table crashed instead of throwing. The seam maps the NULL to "no
// maximum" so the callers' intended throws fire; see the MySQL
// implementation.
//
// The loginserver's CLCreatePCHandler reads single rows of several of
// these tables inline, and the loginserver/sharedserver load
// GameServerGroupInfo with their own code — their own extractions.

enum LevelExpTable {
    LEVEL_EXP_TABLE_STR,
    LEVEL_EXP_TABLE_DEX,
    LEVEL_EXP_TABLE_INT,
    LEVEL_EXP_TABLE_VAMP_EXP,
    LEVEL_EXP_TABLE_OUSTERS_EXP,
    LEVEL_EXP_TABLE_MAX
};

// A level's row. skillPointBonus is selected for OustersEXPBalanceInfo
// only and is 0 for the other four tables.
struct LevelExpRow {
    int level;
    int goalExp;
    int accumExp;
    int skillPointBonus;
};

// SkillDomainInfo's row.
struct DomainLevelRow {
    int domainType;
    int level;
    int goalExp;
    int accumExp;
    int bestItemType;
};

// FameLimitInfo's row.
struct FameLimitRow {
    int domainType;
    int level;
    int fame;
};

// PetExpInfo's row (PetGoalExp is not selected by the loader).
struct PetExpRow {
    int petLevel;
    int petAccumExp;
};

// PetAttrBalanceInfo's row.
struct PetAttrBalanceRow {
    int petAttr;
    int level;
    int addAttr;
    int accumAttr;
};

// PetAttrInfo's row.
struct PetAttrRatioRow {
    int petAttr;
    int enchantRatio;
};

class BalanceInfoRepository {
public:
    virtual ~BalanceInfoRepository() {}

    // MAX(Level) of a ladder; false when the table has no rows.
    virtual bool loadMaxLevel(LevelExpTable table, int& maxLevel) = 0;
    virtual std::vector<LevelExpRow> loadLevels(LevelExpTable table) = 0;

    // RankEXPInfo, one ladder per RankType.
    virtual bool loadMaxRankLevel(int rankType, int& maxLevel) = 0;
    virtual std::vector<LevelExpRow> loadRankLevels(int rankType) = 0;

    // SkillDomainInfo, one ladder per DomainType.
    virtual bool loadMaxDomainLevel(int domainType, int& maxLevel) = 0;
    virtual std::vector<DomainLevelRow> loadDomainLevels(int domainType) = 0;

    // FameLimitInfo, one ladder per DomainType.
    virtual bool loadMaxFameLevel(int domainType, int& maxLevel) = 0;
    virtual std::vector<FameLimitRow> loadFameLimits(int domainType) = 0;

    // The pet tables.
    virtual std::vector<PetExpRow> loadPetExp() = 0;
    virtual std::vector<PetAttrBalanceRow> loadPetAttrBalance() = 0;
    virtual std::vector<PetAttrRatioRow> loadPetAttrRatios() = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLBalanceInfoRepository.cpp. An accessor function rather than a
// g_p* extern: ratchet R1 counts those.
BalanceInfoRepository& defaultBalanceInfoRepository();

#endif
