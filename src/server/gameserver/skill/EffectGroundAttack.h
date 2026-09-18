//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectGroundAttack.h
// Written by  : elca@ewestsoft.com
// Description : Class that handles the Effect of the clerical magic GroundAttack.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_GROUND_ATTACK__
#define __EFFECT_GROUND_ATTACK__

#include "Effect.h"
#include "EffectLoader.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectGroundAttack
//////////////////////////////////////////////////////////////////////////////

class EffectGroundAttack : public Effect {
public:
    EffectGroundAttack(Zone* pZone, ZoneCoord_t zoneX, ZoneCoord_t zoneY);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_GROUND_ATTACK;
    }

    void affect();
    void affect(Creature* pCreature);

    void unaffect();
    void unaffect(Creature* pCreature);

    string toString() const;

public:
    int getDamagePercent(void) const {
        return m_DamagePercent;
    }
    void setDamagePercent(int damagePercent) {
        m_DamagePercent = damagePercent;
    }

    Turn_t getDelay() const {
        return m_Delay;
    }
    void setDelay(Turn_t Delay) {
        m_Delay = Delay;
    }


    ObjectID_t getUserObjectID() const {
        return m_UserObjectID;
    }
    void setUserObjectID(ObjectID_t UserObjectID) {
        m_UserObjectID = UserObjectID;
    }


private:
    int m_DamagePercent;
    Turn_t m_Delay;
    ObjectID_t m_UserObjectID;
};

//////////////////////////////////////////////////////////////////////////////
// class EffectGroundAttackLoader
//////////////////////////////////////////////////////////////////////////////

class EffectGroundAttackLoader : public EffectLoader {
public:
    virtual Effect::EffectClass getEffectClass() const {
        return Effect::EFFECT_CLASS_GROUND_ATTACK;
    }
    virtual string getEffectClassName() const {
        return "EffectGroundAttack";
    }

public:
    virtual void load(Creature* pCreature) {}
};

#endif // __EFFECT_GROUND_ATTACK__
