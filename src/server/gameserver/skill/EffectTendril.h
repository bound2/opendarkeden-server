//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectTendril.h
// Written by  :
// Description : Defense reduction effect caused by Tendril
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_TENDRIL__
#define __EFFECT_TENDRIL__

#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectTendril
//////////////////////////////////////////////////////////////////////////////

class EffectTendril : public Effect {
public:
    EffectTendril(Creature* pCreature);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_TENDRIL;
    }

    void affect() {}
    void affect(Creature* pCreature);

    void unaffect(Creature* pCreature);
    void unaffect();

    string toString() const;

public:
    int getLevel() const {
        return m_Level;
    }
    void setLevel(int level) {
        m_Level = level;
    }

    int getDefensePenalty(void) const {
        return m_DefensePenalty;
    }
    void setDefensePenalty(int penalty) {
        m_DefensePenalty = penalty;
    }

private:
    int m_Level;
    int m_DefensePenalty;
};

#endif // __EFFECT_TENDRIL__
