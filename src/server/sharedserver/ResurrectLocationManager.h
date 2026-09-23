//////////////////////////////////////////////////////////////////////////////
// Filename    : ResurrectLocationManager.h
// Written by  : excel96
// Description :
// Map storing, per zone, the resurrection location where a player is reborn
// after dying.
//////////////////////////////////////////////////////////////////////////////

#ifndef __SHARED_SERVER_RESURRECT_LOCATION_MANAGER_H__
#define __SHARED_SERVER_RESURRECT_LOCATION_MANAGER_H__

#include <unordered_map>

#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ResurrectLocationManager
//
// Map storing, per zone, the resurrection location where a player is reborn
// after dying.
//
// There should be a function that can set the default resurrection location
// for slayers and vampires separately. At the moment it sits in Resurrect.cpp at source level.
//////////////////////////////////////////////////////////////////////////////

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

    void addSlayerPosition(ZoneID_t id, const ZONE_COORD& coord);
    void addVampirePosition(ZoneID_t id, const ZONE_COORD& coord);


protected:
    unordered_map<ZoneID_t, ZONE_COORD> m_SlayerPosition;
    unordered_map<ZoneID_t, ZONE_COORD> m_VampirePosition;
};


#endif
