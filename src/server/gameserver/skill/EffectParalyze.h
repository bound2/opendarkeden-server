//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectParalyze.h
// Written by  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_PARALYZE__
#define __EFFECT_PARALYZE__

#include "Effect.h"
#include "EffectLoader.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectParalyze
//////////////////////////////////////////////////////////////////////////////

class EffectParalyze : public Effect {
public:
    EffectParalyze(Creature* pCreature);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_PARALYZE;
    }

    void affect();
    void affect(Creature* pCreature);

    void unaffect();
    void unaffect(Creature* pCreature);

    string toString() const;

public:
    Level_t getLevel() const {
        return m_Level;
    }
    void setLevel(Level_t level) {
        m_Level = level;
    }


private:
    Level_t m_Level;
};

#endif // __EFFECT_PARALYZE__
