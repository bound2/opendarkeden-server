//////////////////////////////////////////////////////////////////////////////
// Filename    : MonsterManager.h
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __MONSTER_MANAGER_H__
#define __MONSTER_MANAGER_H__

#include <list>
#include <vector>

#include <unordered_map>

#include "CreatureManager.h"
#include "Item.h"
#include "MonsterCounter.h"
#include "Timeval.h"

//////////////////////////////////////////////////////////////////////////////
// class MonsterManager
//////////////////////////////////////////////////////////////////////////////

class Zone;
class Monster;
class MonsterCorpse;
struct SUMMON_INFO;
struct ITEM_TEMPLATE;

struct EventMonsterInfo {
    MonsterType_t monsterType;
    int regenDelay;
    Timeval regenTime;
    int x, y;
    bool bExist;
};

class MonsterManager : public CreatureManager {
public:
    MonsterManager(Zone* pZone);
    ~MonsterManager();

public:
    // load from database
    void load();

    // add monster
    void addCreature(Creature* pCreature);

    // Add monsters.
    void addMonsters(ZoneCoord_t x, ZoneCoord_t y, MonsterType_t monsterType, int num, const SUMMON_INFO& summonInfo,
                     list<Monster*>* pSummonedMonsters = NULL);

    // delete monster
    void deleteCreature(ObjectID_t objectID); // NoSuchElementException, Error);

    // Process the creatures (NPC, Monster) owned by this manager.
    void processCreatures();

    // Add monsters when their number has dropped.
    void regenerateCreatures();

    // Find a suitable position to add a monster.
    bool findPosition(MonsterType_t monsterType, ZoneCoord_t& x, ZoneCoord_t& y) const;

    // Handle a dead creature.
    void killCreature(Creature* pDeadMonster);

    // Create items from a dead monster.
    void addItem(Monster* pDeadMonster, MonsterCorpse* pMonsterCorpse);

    // For direct access to the hash map
    const unordered_map<MonsterType_t, MonsterCounter*>& getMonsters(void) {
        return m_Monsters;
    }

    // Recognise it as a potential enemy of all, when pCreature has attacked pMonster.
    void addPotentialEnemy(Monster* pMonster, Creature* pCreature);

    // Recognise it as an enemy of all, when pCreature has attacked pMonster.
    void addEnemy(Monster* pMonster, Creature* pCreature);

    // get debug string
    string toString() const;

    // delete AllMonsters
    void deleteAllMonsters(bool bDeleteFromZone = true); // NoSuchElementException, Error);

    // kill AllMonsters
    void
    killAllMonsters(const unordered_map<ObjectID_t, ObjectID_t>& exceptCreatures); // NoSuchElementException, Error);

    int upgradeItemTypeByLuck(int luckLevel, Creature::CreatureClass ownerCreatureClass, ITEM_TEMPLATE& it);
    int upgradeOptionByLuck(int luckLevel, Creature::CreatureClass ownerCreatureClass, ITEM_TEMPLATE& it);

protected:
    void parseMonsterList(const string& text, bool bReload = false);
    void parseEventMonsterList(const string& text, bool bReload = false);

private:
    Zone* m_pZone;                                           // Pointer to the zone this monster manager belongs to
    unordered_map<SpriteType_t, MonsterCounter*> m_Monsters; // Current monster counts in this zone
    Timeval m_RegenTime;                                     // Time of the next monster regeneration

    int m_RICE_CAKE_PROB_RATIO[5];
    int m_SumOfCakeRatio;

    // by sigi. 2002.10.14
    vector<EventMonsterInfo>* m_pEventMonsterInfo;
    int m_nEventMonster;

    ZoneID_t m_CastleZoneID;
};

#endif
