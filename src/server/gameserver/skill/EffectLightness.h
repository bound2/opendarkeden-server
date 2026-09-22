//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectLightness.h
// Written by  : excel96
// Description :
// Effect created by Lightness that changes the creature sight.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_LIGHTNESS__
#define __EFFECT_LIGHTNESS__

#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectLightness
//////////////////////////////////////////////////////////////////////////////

class EffectLightness : public Effect {
public:
    EffectLightness(Creature* pCreature);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_LIGHTNESS;
    }

    void affect() {}

    void unaffect();
    void unaffect(Creature* pCreature);

    string toString() const;
};

#endif // __EFFECT_LIGHTNESS__
