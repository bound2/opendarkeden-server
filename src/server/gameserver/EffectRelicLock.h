//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectRelicLock.h
// Written by  :
// Description : Defense reduction effect caused by Doom
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_RELIC_LOCK__
#define __EFFECT_RELIC_LOCK__

#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectRelicLock
//////////////////////////////////////////////////////////////////////////////
// Effect attached to the relic stand.

class EffectRelicLock : public Effect {
public:
    EffectRelicLock(Item* pItem);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_RELIC_LOCK;
    }

    void affect() {}
    void affect(Item* pItem);

    void unaffect(Item* pItem);
    void unaffect();

    string toString() const;

public:
    // bool   isLockTime() const;
    // void   setLockTime(const Timeval& tv)  { m_LockTime = tv; }

private:
    // Timeval m_LockTime;
};

#endif // __EFFECT_DOOM__
