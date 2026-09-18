//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectDarknessForbidden.h
// Written by  : elca@ewestsoft.com
// Description : Class that handles the Effect of the clerical magic DarknessForbidden.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_DARKNESS_FORBIDDEN__
#define __EFFECT_DARKNESS_FORBIDDEN__

#include "Effect.h"
#include "EffectLoader.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectDarknessForbidden
//////////////////////////////////////////////////////////////////////////////

class EffectDarknessForbidden : public Effect {
public:
    EffectDarknessForbidden(Zone* pZone, ZoneCoord_t zoneX, ZoneCoord_t zoneY);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_DARKNESS_FORBIDDEN;
    }

    void unaffect();

    string toString() const;
};

#endif // __EFFECT_DARKNESS_FORBIDDEN__
