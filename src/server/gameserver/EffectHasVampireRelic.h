//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectHasVampireRelic.h
// Written by  :
// Description : Defense reduction effect caused by Doom
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_HAS_VAMPIRE_RELIC__
#define __EFFECT_HAS_VAMPIRE_RELIC__

#include "EffectHasRelic.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectHasVampireRelic
//////////////////////////////////////////////////////////////////////////////

class EffectHasVampireRelic : public EffectHasRelic {
public:
    EffectHasVampireRelic(Creature* pCreature);
    EffectHasVampireRelic(Item* pItem);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_HAS_VAMPIRE_RELIC;
    }

    void affect(Creature* pCreature);
    void affect(Item* pItem);

    string toString() const;
};

#endif // __EFFECT_DOOM__
