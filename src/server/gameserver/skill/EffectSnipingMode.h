//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectSnipingMode.h
// Written by  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_SNIPING_MODE__
#define __EFFECT_SNIPING_MODE__

#include "Effect.h"
#include "EffectLoader.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectSnipingMode
//////////////////////////////////////////////////////////////////////////////

class EffectSnipingMode : public Effect {
public:
    EffectSnipingMode(Creature* pCreature);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_SNIPING_MODE;
    }

    void affect() {}
    void affect(Creature* pCreature);
    void affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject);

    void unaffect();
    void unaffect(Creature* pCreature);
    void unaffect(Item* pItem) {}
    void unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject);

    string toString() const;

public:
    int getRevealRatio(void) const {
        return m_RevealRatio;
    }
    void setRevealRatio(int ratio) {
        m_RevealRatio = ratio;
    }


private:
    int m_RevealRatio;
};

#endif // __EFFECT_SNIPING_MODE__
