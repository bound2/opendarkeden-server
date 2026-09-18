//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectBombCrashWalk.h
// Written by  : elca@ewestsoft.com
// Description : Handles the effect of the clerical magic BombCrashWalk.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_BOMB_CRASH_WALK__
#define __EFFECT_BOMB_CRASH_WALK__

#include "Effect.h"
#include "EffectLoader.h"
#include "HitRoll.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectBombCrashWalk
//////////////////////////////////////////////////////////////////////////////

class EffectBombCrashWalk : public Effect {
public:
    EffectBombCrashWalk(Zone* pZone, ZoneCoord_t zoneX, ZoneCoord_t zoneY);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_SKILL_BOMB_CRASH_WALK_TO_ENEMY;
    }

    void affect();
    void affect(Creature* pCreature);

    void unaffect();
    void unaffect(Creature* pCreature);

    string toString() const;

public:
    int getDamage(void) const {
        return m_Damage;
    }
    void setDamage(int damage) {
        m_Damage = damage;
    }

    ObjectID_t getUserObjectID(void) const {
        return m_UserObjectID;
    }
    void setUserObjectID(ObjectID_t UserObjectID) {
        m_UserObjectID = UserObjectID;
    }

    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t skillType) {
        m_SkillType = skillType;
    }

    int getStormTime() const {
        return m_StormTime;
    }
    void setStormTime(int stormTime) {
        m_StormTime = stormTime;
    }

    Turn_t getTick() const {
        return m_Tick;
    }
    void setTick(Turn_t tick) {
        m_Tick = tick;
    }

    bool isLarge() const {
        return m_bLarge;
    }
    void setLarge(bool large) {
        m_bLarge = large;
    }

private:
    int m_Damage;
    ObjectID_t m_UserObjectID;
    SkillType_t m_SkillType;
    int m_StormTime; // How many times damage is dealt
    Turn_t m_Tick;   // Interval in turns between hits
    bool m_bLarge;   // true for 5x5, false for 3x3
};

#endif // __EFFECT_BOMB_CRASH_WALK__
