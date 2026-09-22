//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectFierceFlame.h
// Written by  : elca@ewestsoft.com
// Description : Class that handles the Effect of the clerical magic FierceFlame.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_FIRECE_FLAME__
#define __EFFECT_FIRECE_FLAME__

#include "Creature.h"
#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectFierceFlame
//////////////////////////////////////////////////////////////////////////////

class EffectFierceFlame : public Effect {
public:
    EffectFierceFlame(Creature* pCreature);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_FIERCE_FLAME;
    }

    void affect();
    void affect(Creature* pCreature);

    void unaffect();
    void unaffect(Creature* pCreature);

    string toString() const;

public:
    ObjectID_t getUserOID(void) const {
        return m_UserOID;
    }
    void setUserOID(ObjectID_t UserOID) {
        m_UserOID = UserOID;
    }

    Damage_t getDamage(void) const {
        return m_Damage;
    }
    void setDamage(Damage_t Damage) {
        m_Damage = Damage;
    }

    Turn_t getDuration(void) const {
        return m_Duration;
    }
    void setDuration(Turn_t Duration) {
        m_Duration = Duration;
    }

private:
    ObjectID_t m_UserOID;
    Damage_t m_Damage;
    Turn_t m_Duration;
};

#endif // __EFFECT_FIRECE_FLAME__
