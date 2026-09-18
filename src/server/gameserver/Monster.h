//////////////////////////////////////////////////////////////////////////////
// Filename    : Monster.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "Assert.h"
#include "Creature.h"
#include "MonsterInfo.h"
#include "PrecedenceTable.h"
#include "Timeval.h"

//////////////////////////////////////////////////////////////////////////////
// Forward declaration
//////////////////////////////////////////////////////////////////////////////
class Slayer;
class Vampire;
class Ousters;
class MonsterAI;
struct SUMMON_INFO2;

class Monster : public Creature {
    // Constructor / destructor
public:
    Monster(MonsterType_t monsterType);
    virtual ~Monster();

    // Functions inherited from the base class
public:
    virtual CreatureClass getCreatureClass() const {
        return CREATURE_CLASS_MONSTER;
    }
    virtual string getCreatureClassString() const {
        return "CREATURE_CLASS_MONSTER";
    }

    virtual Race_t getRace() const;

    virtual void registerObject();

    virtual bool load() {
        return true;
    }
    virtual void save() const {}

    virtual string toString() const;

    // AI specific methods
public:
    // The main method in which the AI code runs.
    void act(const Timeval& currentTime);
    void actDeadAction(void);

    // enemy specific methods
    void addEnemy(Creature* pCreature);
    void addPotentialEnemy(Creature* pCreature);
    void deleteEnemy(ObjectID_t objectID);
    void deleteAllEnemy();
    Creature* getPrimaryEnemy() const;
    ObjectID_t getEnemy(EnemyPriority enemyPriority = ENEMY_PRIMARY) const;

    // Remove logged-out PCs from the enemy list.
    void verifyEnemies();

    bool isRealEnemy(Creature* pEnemy);

    // Does this monster have at least one enemy?
    bool hasEnemy() const {
        return m_Enemies.size() > 0;
    }
    uint getMaxEnemies() const;

    list<ObjectID_t>& getEnemies(void) {
        return m_Enemies;
    }

    // Enemy check for a pre-emptive attack.
    bool isEnemyToAttack(Creature* pCreature) const;
    bool isEnemyToAttack(Slayer* pSlayer) const;
    bool isEnemyToAttack(Vampire* pVampire) const;
    bool isEnemyToAttack(Monster* pMonster) const;

    bool canMove(ZoneCoord_t nx, ZoneCoord_t ny) const;
    // get monster-class-level attributes
public:
    MonsterType_t getMonsterType() const {
        return m_MonsterType;
    }
    const string& getName() const {
        return m_Name;
    }
    const string& getMonsterName() const {
        return m_Name;
    }
    void setName(const string name) {
        m_Name = name;
    }
    SpriteType_t getSpriteType() const;
    Level_t getLevel() const;
    uint getBodySize() const;
    Color_t getMainColor() const;
    Color_t getSubColor() const;
    MAlignment getAlignment() const;
    // AttackOrder   getAttackOrder() const ;


    // get/set instance-level attributes
public:
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

    HP_t getHP(AttrType attrType = ATTR_CURRENT) const {
        return m_HP[attrType];
    }
    void setHP(HP_t hp, AttrType attrType = ATTR_CURRENT) {
        m_HP[attrType] = hp;
    }
    void setHP(HP_t current, HP_t max) {
        m_HP[ATTR_CURRENT] = current;
        m_HP[ATTR_MAX] = max;
    }

    Defense_t getDefense() const {
        return m_Defense;
    }
    void setDefense(Defense_t ac) {
        m_Defense = ac;
    }

    Protection_t getProtection() const {
        return m_Protection;
    }
    void setProtection(Protection_t ac) {
        m_Protection = ac;
    }

    ToHit_t getToHit() const {
        return m_ToHit;
    }
    void setToHit(ToHit_t tohit) {
        m_ToHit = tohit;
    }

    Damage_t getDamage(AttrType attrType = ATTR_CURRENT) const {
        return m_Damage[attrType];
    }
    void setDamage(Damage_t damage, AttrType attrType = ATTR_CURRENT) {
        m_Damage[attrType] = damage;
    }
    void setDamage(Damage_t current, Damage_t max) {
        m_Damage[ATTR_CURRENT] = current;
        m_Damage[ATTR_MAX] = max;
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

    Moral_t getMoral(void) const {
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

    Timeval getAccuDelay() const {
        return m_AccuDelay;
    }
    void setAccuDelay(Timeval delay) {
        m_AccuDelay = delay;
    }
    void addAccuDelay(Timeval delay) {
        m_AccuDelay = m_AccuDelay + delay;
    }
    void clearAccuDelay() {
        m_AccuDelay.tv_sec = 0;
        m_AccuDelay.tv_usec = 0;
    }

    void clearEnemyLimitTime();

    Timeval getNextTurn() const {
        return m_NextTurn;
    }
    void setNextTurn(Timeval NextTurn) {
        m_NextTurn = NextTurn;
    }

    void setDamaged(bool value = true);

    void initAllStat(void);

    Silver_t getSilverDamage(void) const {
        return m_SilverDamage;
    }
    void setSilverDamage(Silver_t damage) {
        m_SilverDamage = damage;
    }

    MonsterAI* getBrain(void) const {
        return m_pBrain;
    }
    void setBrain(MonsterAI* pBrain);
    void removeBrain(void);

    virtual ClanType_t getClanType(void) const {
        return m_ClanType;
    }
    virtual void setClanType(ClanType_t clan) {
        m_ClanType = clan;
    }

    PrecedenceTable* getPrecedenceTable(void) {
        return &m_PrecedenceTable;
    }
    //	void addPrecedence(Creature* pCreature, int damage);
    void addPrecedence(const string& Name, int PartyID, int damage);
    int getOustersExp(Ousters* pOusters);

    string getHostName(void) const {
        return m_HostName;
    }
    void setHostName(const string& name) {
        m_HostName = name;
    }

    int getHostPartyID(void) const {
        return m_HostPartyID;
    }
    void setHostPartyID(int ID) {
        m_HostPartyID = ID;
    }

    CreatureClass getLastHitCreatureClass(void) const {
        return m_LastHitCreatureClass;
    }
    void setLastHitCreatureClass(CreatureClass CClass) {
        m_LastHitCreatureClass = CClass;
    }

    // Whether this monster is an event monster.
    bool getEventMonsterFlag(void) const {
        return m_isEventMonster;
    }
    void setEventMonsterFlag(bool flag) {
        m_isEventMonster = flag;
    }

    bool isChief(void) const {
        return m_bChief;
    }
    void setChief(bool flag = true) {
        m_bChief = flag;
    }

    // ..
public:
    bool isAlive() const {
        return m_HP[ATTR_CURRENT] > 0;
    }
    bool isDead() const {
        return m_HP[ATTR_CURRENT] == 0;
    }

public:
    bool hasRelic() const {
        return m_RelicIndex;
    }
    void setRelic(int relicIndex) {
        m_RelicIndex = relicIndex;
    }

public:
    bool isScanEnemy() const {
        return m_bScanEnemy;
    }
    void setScanEnemy(bool bScan = true) {
        m_bScanEnemy = bScan;
    }

    bool isEnemyLimit() const {
        return m_bEnemyLimit;
    }

    bool isMaster() const {
        return m_bMaster;
    }
    void setMaster(bool bMaster = true) {
        m_bMaster = bMaster;
    }

    // Does this monster drop items?
    bool hasTreasure() const {
        return m_bTreasure;
    }
    void setTreasure(bool bTreasure = true) {
        m_bTreasure = m_bTreasure && bTreasure;
    } // Only monsters that already had treasure may keep it.

    // The kinds of monster this monster summons.
    bool getMonsterSummonInfo(SUMMON_INFO2& summonInfo);
    bool hasNextMonsterSummonInfo();
    int getMonsterSummonStep() const {
        return m_MonsterSummonStep;
    }
    void setMonsterSummonStep(int ss) {
        m_MonsterSummonStep = ss;
    }

    // attackOrder - by sigi. 2002.9.4
    AttackOrder getAttackOrder() const {
        return m_AttackOrder;
    }
    void setAttackOrder(AttackOrder attackOrder) {
        m_AttackOrder = attackOrder;
    }

    void setOwnerObjectID(ObjectID_t id) {
        m_OwnerObjectID = id;
    }
    ObjectID_t getOwnerObjectID() const {
        return m_OwnerObjectID;
    }

    void setEventMonsterIndex(int index) {
        m_EventMonsterIndex = index;
    }
    int getEventMonsterIndex() const {
        return m_EventMonsterIndex;
    }
    bool isEventMonster() const {
        return m_EventMonsterIndex < 0xFFFF;
    } // -_-;

    void setQuestItem(Item* pItem) {
        m_pQuestItem = pItem;
    }
    Item* getQuestItem() const {
        return m_pQuestItem;
    }

    void setLastKiller(ObjectID_t id) {
        m_LastKiller = id;
    }
    ObjectID_t getLastKiller() const {
        return m_LastKiller;
    }

    // data members
private:
    MonsterType_t m_MonsterType;
    string m_Name;

    Attr_t m_STR;
    Attr_t m_DEX;
    Attr_t m_INT;
    HP_t m_HP[2];
    Defense_t m_Defense;
    Protection_t m_Protection;
    ToHit_t m_ToHit;
    Damage_t m_Damage[2];
    int m_MeleeRange;
    int m_MissileRange;
    bool m_isEventMonster;
    bool m_bChief;

    // Exp : the experience granted for killing this monster.
    Exp_t m_Exp;

    // Morale -- how committed the monster is to the fight.
    Moral_t m_Moral;

    // Processing delay -- it has to vary with the monster's state.
    // A monster under some strange spell gets a longer delay and so
    // reacts more slowly. This is per instance.
    Turn_t m_Delay;
    Turn_t m_AttackDelay;
    Timeval m_AccuDelay;

    // enemy list
    // *CAUTION*
    // A list of Creature* would be the obvious implementation, but then,
    // when a PC designated as an enemy logs out, every monster that named
    // it as an enemy would have to be found and the entry deleted from its
    // list. So the OID is stored instead, and that value is used to look
    // the PC up.
    list<ObjectID_t> m_Enemies;

    // Time of the next action.
    Timeval m_NextTurn;

    // MonsterAI class
    MonsterAI* m_pBrain;

    // Silver damage.
    Silver_t m_SilverDamage;

    // Clan type.
    ClanType_t m_ClanType;

    // Precedence table that prevents loot stealing.
    PrecedenceTable m_PrecedenceTable;

    // Name and party ID of whoever has precedence on this monster's drops.
    string m_HostName;
    int m_HostPartyID;

    // Creature class of the race that hit this monster last.
    // It decides which items are created when the monster dies.
    CreatureClass m_LastHitCreatureClass;

    // Does this monster carry a Relic?
    int m_RelicIndex;

    // #ifdef __XMAS_EVENT_CODE__
    Timeval m_LastSpeechTime;
    // #endif

    bool m_bScanEnemy;
    Timeval m_NextScanTurn;

    // The monster's enemy recognition time.
    bool m_bEnemyLimit;
    Timeval m_EnemyLimitTime;
    Creature* m_pOldEnemy;

    // 2002.9.2
    bool m_bMaster;
    bool m_bTreasure;
    int m_MonsterSummonStep; // summon step

    // AttackOrder
    AttackOrder m_AttackOrder;

    // Owner information, for summoned monsters and others.
    ObjectID_t m_OwnerObjectID;

    // 2002.10.14
    int m_EventMonsterIndex;

    Item* m_pQuestItem;

    ObjectID_t m_LastKiller;

    HP_t m_RegenAmount;
    Timeval m_NextRegenTime;
};

#endif // __MONSTER_H__
