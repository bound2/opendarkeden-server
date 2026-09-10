//////////////////////////////////////////////////////////////////////////////
// Filename    : ResurrectLocationManager.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "ResurrectLocationManager.h"

#include "repository/SharedConfigRepository.h"

//////////////////////////////////////////////////////////////////////////////
// global variable
//////////////////////////////////////////////////////////////////////////////

ResurrectLocationManager* g_pResurrectLocationManager = NULL;

//////////////////////////////////////////////////////////////////////////////
// class ResurrectLocationManager member methods
//////////////////////////////////////////////////////////////////////////////

ResurrectLocationManager::ResurrectLocationManager(){__BEGIN_TRY __END_CATCH}

ResurrectLocationManager::~ResurrectLocationManager() {
    __BEGIN_TRY

    m_SlayerPosition.clear();
    m_VampirePosition.clear();

    __END_CATCH
}

void ResurrectLocationManager::init() {
    __BEGIN_TRY

    load();

    __END_CATCH
}

void ResurrectLocationManager::load() {
    __BEGIN_TRY

    vector<SharedResurrectLocationRow> rows = defaultSharedConfigRepository().loadResurrectLocations();

    if (rows.empty()) {
        cerr << "ResurrectLocationManager::load() : TABLE DOES NOT EXIST!" << endl;
        throw Error("ResurrectLocationManager::load() : TABLE DOES NOT EXIST!");
    }

    for (size_t i = 0; i < rows.size(); i++) {
        ZoneID_t ID = 0;
        ZONE_COORD slayer_coord;
        ZONE_COORD vampire_coord;

        ID = rows[i].zoneID;
        slayer_coord.id = rows[i].slayerZoneID;
        slayer_coord.x = rows[i].slayerX;
        slayer_coord.y = rows[i].slayerY;
        vampire_coord.id = rows[i].vampireZoneID;
        vampire_coord.x = rows[i].vampireX;
        vampire_coord.y = rows[i].vampireY;

        addSlayerPosition(ID, slayer_coord);
        addVampirePosition(ID, vampire_coord);
    }

    __END_CATCH
}

bool ResurrectLocationManager::getSlayerPosition(ZoneID_t id, ZONE_COORD& zoneCoord) const
// NoSuchElementException)
{
    __BEGIN_TRY

    unordered_map<ZoneID_t, ZONE_COORD>::const_iterator itr = m_SlayerPosition.find(id);

    if (itr == m_SlayerPosition.end()) {
        cerr << "ResurrectLocationManager::getPosition() : No Such ZoneID" << endl;
        // throw NoSuchElementException("ResurrectLocationManager::getPosition() : No Such ZoneID");

        // NoSuch제거. by sigi. 2002.5.9
        return false;
    }

    // return itr->second;
    zoneCoord = itr->second;

    return true;

    __END_CATCH
}


void ResurrectLocationManager::addSlayerPosition(ZoneID_t id, const ZONE_COORD& coord) {
    __BEGIN_TRY

    unordered_map<ZoneID_t, ZONE_COORD>::const_iterator itr = m_SlayerPosition.find(id);

    if (itr != m_SlayerPosition.end()) {
        cerr << "ResurrectLocationManager::addPosition() : ZoneID already exist!" << endl;
        throw NoSuchElementException("ResurrectLocationManager::addPosition() : ZoneID already exist!");
    }

    m_SlayerPosition[id] = coord;

    __END_CATCH
}

bool ResurrectLocationManager::getVampirePosition(ZoneID_t id, ZONE_COORD& zoneCoord) const
// NoSuchElementException)
{
    __BEGIN_TRY

    unordered_map<ZoneID_t, ZONE_COORD>::const_iterator itr = m_VampirePosition.find(id);

    if (itr == m_VampirePosition.end()) {
        cerr << "ResurrectLocationManager::getPosition() : No Such ZoneID" << endl;
        // NoSuch제거. by sigi. 2002.5.9
        // throw NoSuchElementException("ResurrectLocationManager::getPosition() : No Such ZoneID");
        return false;
    }

    // return itr->second;

    zoneCoord = itr->second;

    return true;

    __END_CATCH
}


void ResurrectLocationManager::addVampirePosition(ZoneID_t id, const ZONE_COORD& coord) {
    __BEGIN_TRY

    unordered_map<ZoneID_t, ZONE_COORD>::const_iterator itr = m_VampirePosition.find(id);

    if (itr != m_VampirePosition.end()) {
        cerr << "ResurrectLocationManager::addPosition() : ZoneID already exist!" << endl;
        throw NoSuchElementException("ResurrectLocationManager::addPosition() : ZoneID already exist!");
    }

    m_VampirePosition[id] = coord;

    __END_CATCH
}
