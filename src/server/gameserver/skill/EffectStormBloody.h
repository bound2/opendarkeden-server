//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectStormBloody.h
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_STORM_BLOODY__
#define __EFFECT_STORM_BLOODY__

#include "Effect.h"
#include "EffectLoader.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectStormBloody
//////////////////////////////////////////////////////////////////////////////

class EffectStormBloody : public Effect {
public:
    EffectStormBloody(Creature* pCreature);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_STORM_BLOODY;
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

    void setUserObjectID(ObjectID_t oid) {
        m_UserObjectID = oid;
    }
    ObjectID_t getUserObjectID() const {
        return m_UserObjectID;
    }


private:
    Level_t m_Level;
    HP_t m_Point;
    Turn_t m_Tick;
    ObjectID_t m_UserObjectID;
};

#endif // __EFFECT_STORM__
