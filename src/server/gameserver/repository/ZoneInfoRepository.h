#ifndef __ZONE_INFO_REPOSITORY_H__
#define __ZONE_INFO_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Read-only seam for the zone CONFIGURATION tables (task 3.2, the Zone
// milestone): ZoneGroupInfo, ZoneInfo, ZoneTriggers, EffectPKZoneRegen
// and WayPointInfo — the data the gameserver reads while it bootstraps
// its zone groups, zones, threads and per-zone effects, and never
// writes. The config round added ZoneEffectInfo (one zone, one effect
// class), ZoneInfo's MonsterList/EventMonsterList texts, and the
// PKZoneInfo / EventZoneInfo / LevelWarZoneInfo tables.
// Every row field is typed to the driver getter the inline code
// called on that column (getInt → int, getString → std::string), so
// each narrowing a caller performed when it stored the value still
// happens there, on the same value.
//
// The loginserver and sharedserver read ZoneGroupInfo/ZoneInfo with
// their own inline SELECTs (their own extraction), and the
// MAX(ZoneGroupID) probes in ConnectionInfoManager and CGSayHandler are
// not enclosed here either (EffectShutDown's two went through
// loadMaxZoneGroupID in the info round).

// ZoneInfoManager::load — the 17 columns of a ZoneInfo row, in SELECT
// order. The SELECT spells three columns differently from the schema
// (OwnerID, SMPFilename, SSIFilename vs OwnerId, SmpFileName,
// SsiFileName): MySQL column names are case-insensitive, so it
// resolves; kept verbatim.
struct ZoneInfoRow {
    int zoneID;
    int zoneGroupID;
    std::string type;
    int level;
    std::string accessMode;
    std::string ownerID;
    int payPlayZone;
    int premiumZone;
    int pkZone;
    int noPortalZone;
    int holyLand;
    int available;
    int openLevel;
    std::string smpFilename;
    std::string ssiFilename;
    std::string fullName;
    std::string shortName;
};

// ResurrectLocationManager::load — a zone and its three per-race
// resurrect positions.
struct ResurrectLocationRow {
    int zoneID;
    int slayerZoneID;
    int slayerX;
    int slayerY;
    int vampireZoneID;
    int vampireX;
    int vampireY;
    int oustersZoneID;
    int oustersX;
    int oustersY;
};

// A rectangle: ZoneTriggers' X1/Y1/X2/Y2 and EffectPKZoneRegen's
// LeftX/TopY/RightX/BottomY.
struct ZoneRectRow {
    int left;
    int top;
    int right;
    int bottom;
};

// WayPointInfo's X/Y (the per-zone, per-race query).
struct ZonePointRow {
    int x;
    int y;
};

// WayPointInfo's full row (WayPointManager::load).
struct WayPointRow {
    int zoneID;
    int x;
    int y;
    int race;
};

// ZoneEffectInfo's row: a rectangle plus the three effect values (the
// bridge loader reads the rectangle only; other effects read the values).
struct ZoneEffectRow {
    int left;
    int top;
    int right;
    int bottom;
    int value1;
    int value2;
    int value3;
};

// The same table read without its values: EffectDarkness's loader names four
// columns and reads them by explicit index.
struct ZoneEffectBoundsRow {
    int left;
    int top;
    int right;
    int bottom;
};

// PKZoneInfo's row (PKZoneInfoManager).
struct PKZoneRow {
    int zoneID;
    int race;
    int enterX;
    int enterY;
    int resurrectX;
    int resurrectY;
    int pcLimit;
};

// EventZoneInfo's row (EventZoneInfoManager).
struct EventZoneRow {
    int eventID;
    int zoneID;
    int enterX;
    int enterY;
    int resurrectX;
    int resurrectY;
    int pcLimit;
};

// LevelWarZoneInfo's row (LevelWarZoneInfoManager).
struct LevelWarZoneRow {
    int id;
    int zoneID;
    int sweeperTypeMin;
    int sweeperTypeMax;
    int slayerMin;
    int slayerMax;
    int vampireMin;
    int vampireMax;
    int oustersMin;
    int oustersMax;
    std::string zoneIDList;
};

class ZoneInfoRepository {
public:
    virtual ~ZoneInfoRepository() {}

    // ZoneEffectInfo rectangles of one effect class in a zone: the seven-column
    // literal EffectOnBridgeLoader and six skill loaders share (AcidSwamp,
    // ContinualBloodyWall, GreenPoison, IceField, Prominence, YellowPoison — all
    // callers now). BloodyWall and GrayDarkness only mention the table inside
    // commented-out loaders, so they are not callers.
    virtual std::vector<ZoneEffectRow> loadZoneEffectRects(ZoneID_t zoneID, int effectID) = 0;
    // EffectDarknessLoader's own statement: the same table, four columns, and a
    // "%u" for the zone id where the seven-column one has "%d".
    virtual std::vector<ZoneEffectBoundsRow> loadZoneEffectBounds(ZoneID_t zoneID, int effectID) = 0;

    // ZoneInfo's MonsterList / EventMonsterList texts for a zone
    // (MonsterManager::load). Returns false when the zone has no row.
    virtual bool loadMonsterLists(ZoneID_t zoneID, std::string& monsterList, std::string& eventMonsterList) = 0;

    // The whole PKZoneInfo / EventZoneInfo / LevelWarZoneInfo tables.
    virtual std::vector<PKZoneRow> loadPKZones() = 0;
    virtual std::vector<EventZoneRow> loadEventZones() = 0;
    virtual std::vector<LevelWarZoneRow> loadLevelWarZones() = 0;

    // ZoneGroupInfo.ZoneGroupID for every group. ZoneGroupManager::load
    // asks for them ORDER BY ZoneGroupID; makeDefaultLoadInfo and
    // ThreadManager::init do not — two distinct statements, kept.
    virtual std::vector<int> loadZoneGroupIDs(bool orderedByID) = 0;

    // ZoneInfo.ZoneID of every zone in a group; ORDER BY ZoneID for
    // ZoneGroupManager::load, unordered for makeDefaultLoadInfo.
    virtual std::vector<int> loadZoneIDsOfGroup(int zoneGroupID, bool orderedByID) = 0;

    // Every ZoneInfo row (ZoneInfoManager::load).
    virtual std::vector<ZoneInfoRow> loadZoneInfos() = 0;

    // Every zone's resurrect positions (ResurrectLocationManager::load).
    virtual std::vector<ResurrectLocationRow> loadResurrectLocations() = 0;

    // ZoneTriggers rectangles of a zone (Zone::loadTriggeredPortal).
    virtual std::vector<ZoneRectRow> loadTriggerRects(ZoneID_t zoneID) = 0;

    // EffectPKZoneRegen rectangles of a zone (Zone::loadEffect).
    virtual std::vector<ZoneRectRow> loadPKZoneRegenRects(ZoneID_t zoneID) = 0;

    // WayPointInfo points of a zone for one race (Zone::loadEffect).
    virtual std::vector<ZonePointRow> loadWayPoints(ZoneID_t zoneID, int race) = 0;

    // Every WayPointInfo row (WayPointManager::load).
    virtual std::vector<WayPointRow> loadAllWayPoints() = 0;

    // MAX(ZoneGroupID) — the group count EffectShutDown iterates; false on
    // an empty table (one NULL row, which the inline code atoi(NULL)'d).
    virtual bool loadMaxZoneGroupID(int& maxZoneGroupID) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLZoneInfoRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
ZoneInfoRepository& defaultZoneInfoRepository();

#endif
