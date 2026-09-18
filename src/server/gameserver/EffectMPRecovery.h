//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectMPRecovery.h
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_MP_RECOVERY_H__
#define __EFFECT_MP_RECOVERY_H__

#include "Effect.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectMPRecovery;
//////////////////////////////////////////////////////////////////////////////

class EffectMPRecovery : public Effect {
public:
    EffectMPRecovery();
    EffectMPRecovery(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Creature* pCreature, Turn_t delay);
    virtual ~EffectMPRecovery();

public:
    virtual EffectClass getEffectClass() const {
        return EFFECT_CLASS_MP_RECOVERY;
    }

    // OBJECT_PRIORITY_NONE means this must not go onto a tile.
    virtual ObjectPriority getObjectPriority() const {
        return OBJECT_PRIORITY_NONE;
    }

    virtual void affect();
    virtual void affect(Creature* pCreature);
    virtual void affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pTarget);

    virtual void unaffect();
    virtual void unaffect(Creature* pCreature);
    virtual void unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pTarget);
    void unaffect(Item* pItem) {}

    void create(const string& ownerID) {}
    void save(const string& ownerID) {}
    void destroy(const string& ownerID) {}

    virtual string toString() const;

public:
    void setDelay(Turn_t delay) {
        m_Delay = delay;
    }
    Turn_t getDelay() const {
        return m_Delay;
    }

    void setMPQuantity(MP_t Quantity) {
        m_MPQuantity = Quantity;
    }
    MP_t getMPQuantity() const {
        return m_MPQuantity;
    }

    void setPeriod(Turn_t Period) {
        m_Period = Period;
    }
    Turn_t getPeriod() const {
        return m_Period;
    }

private:
    Turn_t m_Delay;    // How many seconds between ticks
    MP_t m_MPQuantity; // Amount recovered when it completes
    Turn_t m_Period;   // Time the recovery takes to complete
};

#endif
