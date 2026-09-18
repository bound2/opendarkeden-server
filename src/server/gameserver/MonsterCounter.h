//////////////////////////////////////////////////////////////////////////////
// Filename    : MonsterCounter.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __MONSTER_COUNTER_H__
#define __MONSTER_COUNTER_H__

#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class MonsterCounter;
//////////////////////////////////////////////////////////////////////////////

class MonsterCounter {
public:
    MonsterCounter(SpriteType_t spriteType = 0, WORD maxMonsters = 0, WORD currentMonsters = 0);

public:
    SpriteType_t getSpriteType() const {
        return m_SpriteType;
    }
    void setSpriteType(SpriteType_t monsterType) {
        m_SpriteType = monsterType;
    }

    WORD getMaxMonsters() const {
        return m_MaxMonsters;
    }
    void setMaxMonsters(WORD n) {
        m_MaxMonsters = n;
    }

    WORD getCurrentMonsters() const {
        return m_CurrentMonsters;
    }
    void setCurrentMonsters(WORD n) {
        m_CurrentMonsters = n;
    }

    void addMonster();
    void addMonster(WORD Count);
    void deleteMonster();

    string toString() const;

private:
    SpriteType_t m_SpriteType; // Sprite type
    WORD m_MaxMonsters;        // Maximum number of monsters
    WORD m_CurrentMonsters;    // Current number of monsters

    static DWORD s_MonsterCounter;
};

#endif
