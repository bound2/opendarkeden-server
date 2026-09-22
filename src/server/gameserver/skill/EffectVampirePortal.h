//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectVampirePortal.h
// Written by  : excel96
// Description :
// A Vampire opening a portal creates this tile effect both at the spot
// where the portal was opened and at the destination.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_VAMPIRE_PORTAL__
#define __EFFECT_VAMPIRE_PORTAL__

#include "Effect.h"
#include "EffectLoader.h"
#include "Mutex.h"
#include "Tile.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectVampirePortal
//////////////////////////////////////////////////////////////////////////////

class VampirePortalItem;

class EffectVampirePortal : public Effect {
public:
    EffectVampirePortal(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_VAMPIRE_PORTAL;
    }

    void affect() {}
    void affect(Creature* pCreature);
    void affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject);

    void unaffect();
    void unaffect(Creature* pCreature) {};
    void unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject);
    void unaffect(Item* pItem) {}

    string toString() const;

public:
    string getOwnerID(void) const {
        return m_OwnerID;
    }
    void setOwnerID(string ownerID) {
        m_OwnerID = ownerID;
    }

    ZONE_COORD getZoneCoord(void) const {
        return m_ZoneCoord;
    }
    void setZoneCoord(ZONE_COORD& rCoord) {
        m_ZoneCoord = rCoord;
    }
    void setZoneCoord(ZoneID_t id, ZoneCoord_t x, ZoneCoord_t y) {
        m_ZoneCoord.id = id;
        m_ZoneCoord.x = x;
        m_ZoneCoord.y = y;
    }

    Duration_t getDuration() const {
        return m_Duration;
    }
    void setDuration(Duration_t d) {
        m_Duration = d;
    }

    int getCount(void) const {
        return m_Count;
    }
    void setCount(int count) {
        m_Count = count;
    }

private:
    string m_OwnerID;       // Owner of this portal
    ZONE_COORD m_ZoneCoord; // Destination zone id and coordinates
    Duration_t m_Duration;  // Duration of the magic
    int m_Count;            // Maximum number of uses for this portal
};

#endif // __EFFECT_VAMPIRE_PORTAL__
