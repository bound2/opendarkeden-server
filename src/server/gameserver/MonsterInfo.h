//////////////////////////////////////////////////////////////////////////////
// Filename    : MonsterInfo.h
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __MONSTERINFO_H__
#define __MONSTERINFO_H__

#include <list>
#include <vector>

#include <unordered_map>

#include "Assert.h"
#include "Creature.h"
#include "Exception.h"
#include "MonsterInfoTypes.h"
#include "MonsterSummonInfo.h"
#include "Treasure.h"
#include "Types.h"

// Maximum number of monster sprite types
// add by viva
const int MAX_SPRITE_TYPE = 1000;

//////////////////////////////////////////////////////////////////////////////
// Maximum number of enemies a monster can remember, and the index into the enemy list
//////////////////////////////////////////////////////////////////////////////
enum EnemyPriority {
    ENEMY_PRIMARY = 1,
    ENEMY_SECONDARY,
    ENEMY_THIRD,
    ENEMY_FOURTH,
    ENEMY_FIFTH,
    ENEMY_SIXTH,
    ENEMY_SEVENTH,
    ENEMY_EIGHTH,
    ENEMY_MAX
};


//////////////////////////////////////////////////////////////////////////////
// When several enemies are present, which one is attacked first?
//////////////////////////////////////////////////////////////////////////////
enum AttackOrder {
    ATTACK_FIRST,     // Attack the one who struck first
    ATTACK_LAST,      // Attack the one who struck most recently
    ATTACK_WEAKEST,   // Attack the weakest one first
    ATTACK_STRONGEST, // Attack the strongest one first
    ATTACK_CLOSEST,   // Attack the closest one first
    ATTACK_FAREST,    // Attack the farthest one first
    ATTACK_FIGHTER,   // Attack fighters first
    ATTACK_PRIEST,    // Attack priests first
    ATTACK_GUNNER,    // Attack gunners first
    ATTACK_MAX
};

const string AttackOrder2String[] = {"ATTACK_FIRST",   "ATTACK_LAST",   "ATTACK_WEAKEST", "ATTACK_STRONGEST",
                                     "ATTACK_CLOSEST", "ATTACK_FAREST", "ATTACK_FIGHTER", "ATTACK_PRIEST",
                                     "ATTACK_GUNNER",  "ATTACK_BOMBER"};


//////////////////////////////////////////////////////////////////////////////
// Monster alignment -- how the monster reacts on seeing a PC
//////////////////////////////////////////////////////////////////////////////
enum MAlignment { ALIGNMENT_FRIENDLY, ALIGNMENT_NEUTRAL, ALIGNMENT_AGGRESSIVE };

const string MAlignment2String[] = {"ALIGNMENT_FRIENDLY", "ALIGNMENT_NEUTRAL", "ALIGNMENT_AGGRESSIVE"};


//////////////////////////////////////////////////////////////////////////////
// AI Level -- how smart the monster is
//////////////////////////////////////////////////////////////////////////////
enum AILevel {
    AI_VERY_LOW, //   1 - 50
    AI_LOW,      //  51 - 100
    AI_MEDIUM,   // 101 - 150
    AI_HIGH,     // 151 - 200
    AI_VERY_HIGH // 201 - 250
};

//////////////////////////////////////////////////////////////////////////////
// Body Size -- the monster's body size
//////////////////////////////////////////////////////////////////////////////
enum BodySize { BODYSIZE_SMALL = 0, BODYSIZE_MEDIUM, BODYSIZE_LARGE };

//////////////////////////////////////////////////////////////////////////////
// class MonsterInfo
//////////////////////////////////////////////////////////////////////////////

class MonsterInfo {
public:
    MonsterInfo();
    ~MonsterInfo();

public:
    MonsterType_t getMonsterType() const {
        return m_MonsterType;
    }
    void setMonsterType(MonsterType_t spriteType) {
        m_MonsterType = spriteType;
    }

    SpriteType_t getSpriteType() const {
        return m_SpriteType;
    }
    void setSpriteType(SpriteType_t spriteType) {
        m_SpriteType = spriteType;
    }

    string getHName() const {
        return m_HName;
    }
    void setHName(const string& name) {
        m_HName = name;
    }

    string getEName() const {
        return m_EName;
    }
    void setEName(const string& name) {
        m_EName = name;
    }

    Level_t getLevel() const {
        return m_Level;
    }
    void setLevel(Level_t level) {
        m_Level = level;
    }

    Attr_t getSTR() const {
        return m_STR;
    }
    void setSTR(Attr_t str) {
        m_STR = str;
    }

    Attr_t getDEX() const {
        return m_DEX;
    }
    void setDEX(Attr_t dex) {
        m_DEX = dex;
    }

    Attr_t getINT() const {
        return m_INT;
    }
    void setINT(Attr_t inte) {
        m_INT = inte;
    }

    uint getBodySize() const {
        return m_BodySize;
    }
    void setBodySize(uint size) {
        m_BodySize = size;
    }

    HP_t getHP() const {
        return m_HP;
    }
    void setHP(HP_t hp) {
        m_HP = hp;
    }

    Exp_t getExp() const {
        return m_Exp;
    }
    void setExp(Exp_t exp) {
        m_Exp = exp;
    }

    Color_t getMainColor() const {
        return m_MainColor;
    }
    void setMainColor(Color_t mainColor) {
        m_MainColor = mainColor;
    }

    Color_t getSubColor() const {
        return m_SubColor;
    }
    void setSubColor(Color_t subColor) {
        m_SubColor = subColor;
    }

    MAlignment getAlignment() const {
        return m_Alignment;
    }
    void setAlignment(MAlignment alignment) {
        m_Alignment = alignment;
    }

    AttackOrder getAttackOrder() const {
        return m_AttackOrder;
    }
    void setAttackOrder(AttackOrder attackOrder) {
        m_AttackOrder = attackOrder;
    }

    Moral_t getMoral() const {
        return m_Moral;
    }
    void setMoral(Moral_t moral) {
        m_Moral = moral;
    }

    Turn_t getDelay() const {
        return m_Delay;
    }
    void setDelay(Turn_t delay) {
        m_Delay = delay;
    }

    Turn_t getAttackDelay() const {
        return m_AttackDelay;
    }
    void setAttackDelay(Turn_t delay) {
        m_AttackDelay = delay;
    }

    Sight_t getSight() const {
        return m_Sight;
    }
    void setSight(Sight_t sight) {
        m_Sight = sight;
    }

    int getMeleeRange(void) const {
        return m_MeleeRange;
    }
    void setMeleeRange(int range) {
        m_MeleeRange = range;
    }

    int getMissileRange(void) const {
        return m_MissileRange;
    }
    void setMissileRange(int range) {
        m_MissileRange = range;
    }

    Creature::MoveMode getMoveMode() const {
        return m_MoveMode;
    }
    void setMoveMode(Creature::MoveMode moveMode) {
        m_MoveMode = moveMode;
    }
    void setMoveMode(const string& moveMode);

    uint getAIType(void) const {
        return m_AIType;
    }
    void setAIType(uint aitype) {
        m_AIType = aitype;
    }

    int getEnhanceHP(void) const {
        return m_EnhanceHP;
    }
    int getEnhanceToHit(void) const {
        return m_EnhanceToHit;
    }
    int getEnhanceDefense(void) const {
        return m_EnhanceDefense;
    }
    int getEnhanceProtection(void) const {
        return m_EnhanceProtection;
    }
    int getEnhanceMinDamage(void) const {
        return m_EnhanceMinDamage;
    }
    int getEnhanceMaxDamage(void) const {
        return m_EnhanceMaxDamage;
    }
    void parseEnhanceAttr(const string& enhance);

    void parseSlayerTreasureString(const string& text);
    TreasureList* getSlayerTreasureList(void) const {
        return m_pSlayerTreasureList;
    }
    TreasureList* getSlayerTreasureList(void) {
        return m_pSlayerTreasureList;
    }
    void setSlayerTreasureList(TreasureList* pTreasureList);

    void parseVampireTreasureString(const string& text);
    TreasureList* getVampireTreasureList(void) const {
        return m_pVampireTreasureList;
    }
    TreasureList* getVampireTreasureList(void) {
        return m_pVampireTreasureList;
    }
    void setVampireTreasureList(TreasureList* pTreasureList);

    void parseOustersTreasureString(const string& text);
    TreasureList* getOustersTreasureList(void) const {
        return m_pOustersTreasureList;
    }
    TreasureList* getOustersTreasureList(void) {
        return m_pOustersTreasureList;
    }
    void setOustersTreasureList(TreasureList* pTreasureList);

    RegenType selectRegenType() const;
    int getRegenType(RegenType rt) const {
        return m_RegenType[rt];
    }
    void setRegenType(RegenType rt, int percent);

    int getUnburrowChance(void) const {
        return m_UnburrowChance;
    }
    void setUnburrowChance(uint uc) {
        m_UnburrowChance = uc;
    } // 0~128

    int isMaster(void) const {
        return m_bMaster;
    }
    void setMaster(bool bMaster = true) {
        m_bMaster = bMaster;
    }

    int getClanType(void) const {
        return m_ClanType;
    }
    void setClanType(int clanType) {
        m_ClanType = clanType;
    }

    void setMonsterSummonInfo(const string& text);
    bool getMonsterSummonInfo(int step, SUMMON_INFO2& summonInfo) const;
    bool hasNextMonsterSummonInfo(int step) const;

    void setDefaultEffects(const string& text);
    const list<Effect::EffectClass>& getDefaultEffects() const {
        return m_DefaultEffects;
    }
    void addDefaultEffects(Creature* pCreature) const;

    bool isChief(void) const {
        return m_bChief;
    }
    void setChief(bool flag) {
        m_bChief = flag;
    }

    bool isNormalRegen(void) const {
        return m_bNormalRegen;
    }
    void setNormalRegen(bool bNormalRegen = true) {
        m_bNormalRegen = bNormalRegen;
    }

    bool hasTreasure(void) const {
        return m_bHasTreasure;
    }
    void setHasTreasure(bool bHasTreasure = true) {
        m_bHasTreasure = bHasTreasure;
    }

    int getMonsterClass(void) const {
        return m_MonsterClass;
    }
    void setMonsterClass(int mClass) {
        m_MonsterClass = mClass;
    }

    int getSkullType(void) const {
        return m_SkullType;
    }
    void setSkullType(int skullType) {
        m_SkullType = skullType;
    }

    string toString() const;

private:
    MonsterType_t m_MonsterType;                // Monster type
    SpriteType_t m_SpriteType;                  // Sprite type used
    string m_HName;                             // Monster Korean name
    string m_EName;                             // Monster English name
    Level_t m_Level;                            // Monster level
    Attr_t m_STR;                               // Base STR
    Attr_t m_DEX;                               // Base DEX
    Attr_t m_INT;                               // Base INT
    uint m_BodySize;                            // Monster body size
    HP_t m_HP;                                  // Hit points
    Exp_t m_Exp;                                // Exp a PC vampire gains for killing it (not used)
    Color_t m_MainColor;                        // Main Color (not used)
    Color_t m_SubColor;                         // Sub Color (not used)
    MAlignment m_Alignment;                     // Alignment
    AttackOrder m_AttackOrder;                  // Alignment governing the attack order
    Moral_t m_Moral;                            // Morale
    Turn_t m_Delay;                             // Delay before the next action.
    Turn_t m_AttackDelay;                       // Delay before the next attack.
    Sight_t m_Sight;                            // Sight range
    int m_MeleeRange;                           // Melee range
    int m_MissileRange;                         // Missile range
    Creature::MoveMode m_MoveMode;              // Move mode
    uint m_AIType;                              // AI type
    int m_EnhanceHP;                            // HP enhancement multiplier
    int m_EnhanceToHit;                         // ToHit enhancement multiplier
    int m_EnhanceDefense;                       // Defense enhancement multiplier
    int m_EnhanceProtection;                    // Protection enhancement multiplier
    int m_EnhanceMinDamage;                     // Damage enhancement multiplier
    int m_EnhanceMaxDamage;                     // Damage enhancement multiplier
    TreasureList* m_pSlayerTreasureList;        // Treasure list for slayers
    TreasureList* m_pVampireTreasureList;       // Treasure list for vampires
    TreasureList* m_pOustersTreasureList;       // Treasure list for ousters
    int m_RegenType[REGENTYPE_MAX];             // Probability of each regen method
    int m_UnburrowChance;                       // Chance of surfacing after being spawned
    bool m_bMaster;                             // Whether this is a vampire master
    int m_ClanType;                             // Which clan the monster belongs to
    MonsterSummonInfo* m_pMonsterSummonInfo;    // Monster summon info
    list<Effect::EffectClass> m_DefaultEffects; // Effects attached to the monster by default
    bool m_bNormalRegen;                        // Whether this type is picked on a regular regen
    bool m_bHasTreasure;                        // Whether a Treasure.bin file is needed
    bool m_bChief;                              // Whether this is a chief monster

    int m_MonsterClass;     // Monster class
    ItemType_t m_SkullType; // Monster skull type
};


//////////////////////////////////////////////////////////////////////////////
// class MonsterInfoManager
//////////////////////////////////////////////////////////////////////////////

class MonsterInfoManager {
public:
    MonsterInfoManager();
    ~MonsterInfoManager();

public:
    // initialize
    void init();

    // load to database
    void load();
    void reload(MonsterType_t monsterType);

    // add monster info with monster type
    void addMonsterInfo(MonsterType_t monsterType, MonsterInfo* pMonsterInfo);

    // get monster info with monster type
    const MonsterInfo* getMonsterInfo(MonsterType_t monsterType) const;

    // Get the list of monster types that have a given sprite type.
    // (Several monsters may share one sprite type.)
    const vector<MonsterType_t>& getMonsterTypeBySprite(SpriteType_t spriteType) const;
    string getNameBySpriteType(SpriteType_t spriteType) const;

    SpriteType_t getSpriteTypeByName(const string& monsterName) const;
    MonsterType_t getChiefMonsterTypeByName(const string& monsterName) const;
    vector<MonsterType_t>& getMonsterTypesByMonsterClass(int MonsterClass) {
        return m_MonsterClassMap[MonsterClass];
    }

    uint getMaxMonsterType() const {
        return m_MaxMonsterType;
    }
    MonsterType_t getRandomMonsterByClass(int minClass, int maxClass);

    // get debug string
    string toString() const;

protected:
    void clearTreasures();

private:
    uint m_MaxMonsterType;                                      // size of MonsterInfo* array
    MonsterInfo** m_MonsterInfos;                               // array of monster info
    vector<MonsterType_t> m_MonsterSpriteSet[MAX_SPRITE_TYPE];  // array of MonsterType by SpriteType
    unordered_map<string, SpriteType_t> m_MonsterSpriteTypes;   // Find a SpriteType_t by name
    unordered_map<string, MonsterType_t> m_ChiefMonster;        // Chief monster info
    unordered_map<int, vector<SpriteType_t>> m_MonsterClassMap; // Find monster types by monster class

    TreasureLists m_SlayerTreasureLists;  // Treasure lists for slayers
    TreasureLists m_VampireTreasureLists; // Treasure lists for vampires
    TreasureLists m_OustersTreasureLists; // Treasure lists for ousters
};

// global variable declaration
extern MonsterInfoManager* g_pMonsterInfoManager;

#endif
