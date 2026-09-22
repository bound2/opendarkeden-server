//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectSlayerRelic.h
// Written by  :
// Description : Slayer relic carried by a creature or a monster corpse.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_SLAYER_RELIC__
#define __EFFECT_SLAYER_RELIC__

#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectSlayerRelic
//////////////////////////////////////////////////////////////////////////////
// Effect attached to the relic stand.

class EffectSlayerRelic : public Effect {
public:
    EffectSlayerRelic(Creature* pCreature);
    EffectSlayerRelic(Item* pItem);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_SLAYER_RELIC;
    }

    void affect() {}
    void affect(Creature* pCreature);
    void affect(Item* pItem);

    void unaffect(Creature* pCreature);
    void unaffect(Item* pItem);
    void unaffect();

    string toString() const;

public:
    int getHPRecovery(void) const {
        return m_HPRecovery;
    }
    void setHPRecovery(int hp) {
        m_HPRecovery = hp;
    }

    int getTick(void) const {
        return m_Tick;
    }
    void setTick(Turn_t Tick) {
        m_Tick = Tick;
    }

private:
    int m_HPRecovery;
    Turn_t m_Tick;
};

#endif // __EFFECT_DOOM__
