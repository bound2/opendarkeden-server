//////////////////////////////////////////////////////////////////////////////
// Filename    : SimpleSkill.h
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SIMPLESKILL__
#define __SIMPLESKILL__

#include <list>

#include "SkillHandler.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class TILE_MASK {
public:
    TILE_MASK() {
        x = 0;
        y = 0;
        penalty = 100;
    }
    TILE_MASK(int _x, int _y, int _penalty = 100) {
        x = _x;
        y = _y;
        penalty = _penalty;
    }

public:
    int x;       // x offset from the center coordinate
    int y;       // y offset from the center coordinate
    int penalty; // Penalty applied when computing damage
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class SIMPLE_SKILL_INPUT {
public:
    SIMPLE_SKILL_INPUT() {
        SkillType = SKILL_MAX;
        SkillDamage = 0;
        Delay = 0;
        ItemClass = Item::ITEM_CLASS_MAX;
        STRMultiplier = 0;
        DEXMultiplier = 0;
        INTMultiplier = 0;
        Level = 0;
        bMagicHitRoll = false;
        bMagicDamage = false;
        bAdd = false;
        bExpForTotalDamage = false;
        Grade = 0;
    }

    ~SIMPLE_SKILL_INPUT() {
        MaskList.clear();
    }

public:
    void addMask(const TILE_MASK& mask) {
        MaskList.push_back(mask);
    }
    void addMask(int x, int y, int penalty) {
        MaskList.push_back(TILE_MASK(x, y, penalty));
    }

public:
    SkillType_t SkillType;     // Skill type
    Damage_t SkillDamage;      // Skill effect value
    Turn_t Delay;              // Skill delay
    Item::ItemClass ItemClass; // Item required to use the skill
    int STRMultiplier;         // Slayer stat experience multiplier
    int DEXMultiplier;         // Slayer stat experience multiplier
    int INTMultiplier;         // Slayer stat experience multiplier
    int Level;                 // Level used for various purposes
    bool bMagicHitRoll;        // Magic hit roll or normal attack roll
    bool bMagicDamage;         // Whether the skill damage is magic damage
    bool bAdd;                 // Whether SkillDamage is added or used as is
    list<TILE_MASK> MaskList;  // Mask list for a tile skill
    bool bExpForTotalDamage;   // Raises experience for the total damage
    BYTE Grade;                // Skill grade
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class SIMPLE_SKILL_OUTPUT {
public:
    SIMPLE_SKILL_OUTPUT() {
        bSuccess = false;
        pTargetCreature = NULL;
    }

public:
    bool bSuccess;
    Creature* pTargetCreature;
    list<Creature*> targetCreatures;
};

#endif
