//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectEnergyDropToCreature.h
// Date        : 2002. 3. 28
//
// Description : EffectEnergyDropToCreature deals three consecutive hits
//               to the target.
//               The Effect series uses Tick and deadline to implement
//               arbitrary behavior, so another skill's Effect can be
//               copied verbatim and reused.
//               EffectEnergyDropToCreature itself has the same shape as
//               EffectPoison.
//
//               EffectEnergyDropToCreature requires
//               Effect.h to define
//                EFFECT_CLASS_ENERGY_DROP_TO_CREATURE
//               before it can be used.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_ENERGY_DROP_TO_CREATURE__
#define __EFFECT_ENERGY_DROP_TO_CREATURE__

#include "Effect.h"
#include "EffectLoader.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectEnergyDropToCreature
//////////////////////////////////////////////////////////////////////////////

class EffectEnergyDropToCreature : public Effect {
public:
    EffectEnergyDropToCreature(Creature* pCreature);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_ENERGY_DROP_TO_CREATURE;
    }

    void affect();
    void affect(Creature* pCreature);
    void affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject);

    void unaffect();
    void unaffect(Creature* pCreature);
    void unaffect(Item* pItem) {}
    void unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject);

    string toString() const;

public:
    ObjectID_t getUserObjectID() const {
        return m_UserObjectID;
    }
    void setUserObjectID(ObjectID_t UserObjectID) {
        m_UserObjectID = UserObjectID;
    }

    Level_t getLevel() const {
        return m_Level;
    }
    void setLevel(Level_t Level) {
        m_Level = Level;
    }

    HP_t getPoint() const {
        return m_Point;
    }
    void setPoint(HP_t Point) {
        m_Point = Point;
    }

    void setTick(Turn_t Tick) {
        m_Tick = Tick;
    }
    Turn_t getTick() const {
        return m_Tick;
    }

private:
    ObjectID_t m_UserObjectID;
    Level_t m_Level;
    HP_t m_Point;
    Turn_t m_Tick;
};

#endif // __EFFECT_STORM__
