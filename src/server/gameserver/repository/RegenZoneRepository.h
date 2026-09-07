#ifndef __REGEN_ZONE_REPOSITORY_H__
#define __REGEN_ZONE_REPOSITORY_H__

#include <vector>

// Read-only access to the RegenZonePosition table: the race-war regen
// towers — where each stands and which
// race currently owns it. The gameserver reads the table at boot
// (RegenZoneManager::load builds the towers) and on a race-war reload
// (RegenZoneManager::reload re-applies the owners); nothing in this
// tree writes it.
//
// What loadPositions() returns — every field as the driver's getInt
// returned it.
struct RegenZoneRow {
    int id;
    int zoneID;
    int zoneX;
    int zoneY;
    int owner;
};

class RegenZoneRepository {
public:
    virtual ~RegenZoneRepository() {}

    virtual std::vector<RegenZoneRow> loadPositions() = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLRegenZoneRepository.cpp.
RegenZoneRepository& defaultRegenZoneRepository();

#endif
