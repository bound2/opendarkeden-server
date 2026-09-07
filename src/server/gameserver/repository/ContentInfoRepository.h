#ifndef __CONTENT_INFO_REPOSITORY_H__
#define __CONTENT_INFO_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// The content tables the gameserver loads at boot: the monsters
// (MonsterInfo), the skill balance (SkillBalance), the NPCs of a zone
// (NPC), the NPC dialogue scripts (Script), the NPC trigger scripts
// (Triggers; the sister table ZoneTriggers is ZoneInfoRepository's), the
// monster-AI directive sets (DirectiveSet) and the tunable variables
// (AttrInfo — also written back by VariableManager::setVariable on every
// call: the GM `opset` path and the defaults set at init()/load()). Rows
// are typed to the driver getter used for each column (getInt → int,
// getBYTE → BYTE, getString → std::string).
//
// The MAX probes are exposed as bools: MAX() over an empty table is one
// NULL row, and the probe answers false rather than handing back a NULL
// field.

// MonsterInfoManager::load — the 35 columns of its SELECT, in order.
struct MonsterInfoRow {
    int monsterType;
    int spriteType;
    std::string hName;
    std::string eName;
    int level;
    int str;
    int dex;
    int inte;
    int bodySize;
    int exp;
    int mainColor;
    int subColor;
    int alignment;
    int attackOrder;
    int moral;
    int delay;
    int attackDelay;
    int sight;
    int meleeRange;
    int missileRange;
    int regenPortal;
    int regenInvisible;
    int regenBat;
    std::string moveMode;
    int aiType;
    std::string enhance;
    int unburrowChance;
    int master;
    int clanType;
    std::string defaultEffects;
    int chief;
    int normalRegen;
    int hasTreasure;
    int monsterClass;
    int skullType;
};

// MonsterInfoManager::reload — its own 32-column SELECT: MonsterSummonInfo
// inline, no Chief / HasTreasure / MonsterClass / SkullType.
struct MonsterReloadRow {
    int monsterType;
    int spriteType;
    std::string hName;
    std::string eName;
    int level;
    int str;
    int dex;
    int inte;
    int bodySize;
    int exp;
    int mainColor;
    int subColor;
    int alignment;
    int attackOrder;
    int moral;
    int delay;
    int attackDelay;
    int sight;
    int meleeRange;
    int missileRange;
    int regenPortal;
    int regenInvisible;
    int regenBat;
    std::string moveMode;
    int aiType;
    std::string enhance;
    int unburrowChance;
    int master;
    int clanType;
    std::string monsterSummonInfo;
    std::string defaultEffects;
    int normalRegen;
};

struct MonsterSummonRow {
    int monsterType;
    std::string summonInfo;
};

// SkillInfoManager::load — the 26 columns of its SELECT. Domain and
// MagicDomain came through getBYTE; the last six columns were read only
// for Ousters-domain skills (the row carries them for every skill).
struct SkillBalanceRow {
    int type;
    std::string name;
    int level;
    int minDamage;
    int maxDamage;
    int minDelay;
    int maxDelay;
    int minDuration;
    int maxDuration;
    int mana;
    int minRange;
    int maxRange;
    int target;
    int subExp;
    int point;
    BYTE domain;
    BYTE magicDomain;
    int melee;
    int magic;
    int physic;
    int skillPoint;
    int levelUpPoint;
    std::string requireSkill;
    std::string condition;
    int elementalDomain;
    int canDelete;
};

struct NPCRow {
    std::string name;
    int npcID;
    int spriteType;
    int race;
    int mainColor;
    int subColor;
    int clanType;
    int showInMinimap;
    int taxingCastleZoneID;
};

struct ScriptRow {
    int scriptID;
    std::string ownerID;
    std::string subject;
    std::string content;
};

struct DirectiveSetRow {
    int id;
    std::string name;
    std::string content;
    std::string deadContent;
};

struct VariableRow {
    int attrID;
    int attr1;
    int attr2;
};

// One Triggers row of an NPC, in SELECT order.
struct NPCTriggerRow {
    int triggerID;
    std::string triggerType;
    std::string conditions;
    std::string actions;
};

class ContentInfoRepository {
public:
    virtual ~ContentInfoRepository() {}

    // --- monsters ---------------------------------------------------------------
    virtual bool loadMaxMonsterType(int& maxType) = 0;
    virtual std::vector<MonsterInfoRow> loadMonsterInfos() = 0;
    virtual std::vector<MonsterSummonRow> loadMonsterSummonInfos() = 0;
    // reload: every row, or the one row of a type.
    virtual std::vector<MonsterReloadRow> loadMonsterInfosForReload() = 0;
    virtual std::vector<MonsterReloadRow> loadMonsterInfoForReload(MonsterType_t monsterType) = 0;

    // --- skills ------------------------------------------------------------------
    virtual bool loadMaxSkillBalanceType(int& maxType) = 0;
    virtual std::vector<SkillBalanceRow> loadSkillBalances() = 0;

    // --- NPCs of a zone (NPCManager::load; the ints the caller cast) ------------
    virtual std::vector<NPCRow> loadNPCs(int zoneID) = 0;
    virtual std::vector<NPCRow> loadNPCsOfRace(int zoneID, int race) = 0;

    // --- NPC scripts, ordered by ScriptID ----------------------------------------
    virtual std::vector<ScriptRow> loadScripts() = 0;

    // --- NPC trigger scripts (quest/TriggerManager::load(name)) ------------------
    // TriggerID through getInt, the three texts through getString, untrimmed.
    virtual std::vector<NPCTriggerRow> loadNPCTriggers(const std::string& npcName) = 0;

    // --- monster-AI directive sets -----------------------------------------------
    virtual bool loadMaxDirectiveSetID(int& maxID) = 0;
    virtual std::vector<DirectiveSetRow> loadDirectiveSets() = 0;

    // --- tunable variables (AttrInfo) ----------------------------------------------
    virtual bool loadMaxAttrID(int& maxAttrID) = 0;
    virtual std::vector<VariableRow> loadVariables() = 0;
    // VariableManager::setVariable.
    virtual void saveVariable(int value, int attrID) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLContentInfoRepository.cpp.
ContentInfoRepository& defaultContentInfoRepository();

#endif
