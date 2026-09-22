
//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectReputoFactum.h
// Written by  :
// Description : Defense reduction effect caused by ReputoFactum
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_REPUTO_FACTUM__
#define __EFFECT_REPUTO_FACTUM__

#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectReputoFactum
//////////////////////////////////////////////////////////////////////////////

class EffectReputoFactum : public Effect {
public:
    EffectReputoFactum(Creature* pCreature);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_REPUTO_FACTUM_LAST;
    }

    void affect() {}
    void affect(Creature* pCreature);

    void unaffect(Creature* pCreature);
    void unaffect();

    string toString() const;
};

#endif // __EFFECT_REPUTO_FACTUM__
