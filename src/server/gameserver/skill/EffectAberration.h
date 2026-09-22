//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectAberration.h
// Written by  :
// Description : Defense reduction effect caused by Aberration
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_ABERRATION__
#define __EFFECT_ABERRATION__

#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectAberration
//////////////////////////////////////////////////////////////////////////////

class EffectAberration : public Effect {
public:
    EffectAberration(Creature* pCreature);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_ABERRATION;
    }

    void affect() {}
    void affect(Creature* pCreature);

    void unaffect(Creature* pCreature);
    void unaffect();

    string toString() const;

public:
    int getRatio() const {
        return m_Ratio;
    }
    void setRatio(int ratio) {
        m_Ratio = ratio;
    }

private:
    int m_Ratio;
};

#endif // __EFFECT_ABERRATION__
