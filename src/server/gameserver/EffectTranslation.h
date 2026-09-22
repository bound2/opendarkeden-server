//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectTranslation.h
// Written by  :
// Description : Translation effect attached to a creature.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_TRANSLATION__
#define __EFFECT_TRANSLATION__

#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectTranslation
//////////////////////////////////////////////////////////////////////////////

class EffectTranslation : public Effect {
public:
    EffectTranslation(Creature* pCreature);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_TRANSLATION;
    }

    void affect() {}
    void affect(Creature* pCreature);

    void unaffect(Creature* pCreature);
    void unaffect();

    string toString() const;
};

#endif // __EFFECT_DOOM__
