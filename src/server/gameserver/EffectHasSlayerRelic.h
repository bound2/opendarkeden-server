//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectHasSlayerRelic.h
// Written by  :
// Description : Announces at intervals where the slayer relic is.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_HAS_SLAYER_RELIC__
#define __EFFECT_HAS_SLAYER_RELIC__

#include "EffectHasRelic.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectHasSlayerRelic
//////////////////////////////////////////////////////////////////////////////

class EffectHasSlayerRelic : public EffectHasRelic {
public:
    EffectHasSlayerRelic(Creature* pCreature);
    EffectHasSlayerRelic(Item* pItem);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_HAS_SLAYER_RELIC;
    }

    void affect(Creature* pCreature);
    void affect(Item* pItem);

    string toString() const;
};

#endif // __EFFECT_DOOM__
