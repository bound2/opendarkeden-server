//////////////////////////////////////////////////////////////////////////////
// Filename    : ResurrectLocationManager.h
// Written by  : excel96
// Description :
// Map that stores, per zone, the resurrection location where a player is
// reborn after dying.
//////////////////////////////////////////////////////////////////////////////

#ifndef __RESURRECTMANAGER_H__
#define __RESURRECTMANAGER_H__

#include <unordered_map>

#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ResurrectLocationManager
//
// Map that stores, per zone, the resurrection location where a player is
// reborn after dying.
//
// There should be functions to set the default resurrection location for
// Slayers and Vampires; for now they are hard-coded in Resurrect.cpp.
//////////////////////////////////////////////////////////////////////////////

class PlayerCreature;

class ResurrectLocationManager {
public:
    ResurrectLocationManager();
    ~ResurrectLocationManager();

public:
    void init();
    void load();

public:
    bool getSlayerPosition(ZoneID_t id, ZONE_COORD& zoneCoord) const;  // NoSuchElementException);
    bool getVampirePosition(ZoneID_t id, ZONE_COORD& zoneCoord) const; // NoSuchElementException);
    bool getOustersPosition(ZoneID_t id, ZONE_COORD& zoneCoord) const; // NoSuchElementException);
    bool getRaceDefaultPosition(Race_t, ZONE_COORD& zoneCoord) const;

    bool getPosition(PlayerCreature* pPC, ZONE_COORD& zondCoord) const;
    bool getBasicPosition(PlayerCreature* pPC, ZONE_COORD& zondCoord) const;

    void addSlayerPosition(ZoneID_t id, const ZONE_COORD& coord);
    void addVampirePosition(ZoneID_t id, const ZONE_COORD& coord);
    void addOustersPosition(ZoneID_t id, const ZONE_COORD& coord);


protected:
    unordered_map<ZoneID_t, ZONE_COORD> m_SlayerPosition;
    unordered_map<ZoneID_t, ZONE_COORD> m_VampirePosition;
    unordered_map<ZoneID_t, ZONE_COORD> m_OustersPosition;
};


//////////////////////////////////////////////////////////////////////////////
// global variable
//////////////////////////////////////////////////////////////////////////////
extern ResurrectLocationManager* g_pResurrectLocationManager;

#endif
