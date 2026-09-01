#include "DB.h"
#include "repository/ZoneInfoRepository.h"

namespace {

// MySQL implementation of the zone-configuration seam. The legacy
// quirks are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte the inline original, including
//    the two spellings of the same read: the group list with and
//    without ORDER BY, the zone list with and without ORDER BY. Which
//    one a caller gets is its own choice (ZoneGroupManager::load wants
//    deterministic bootstrap order; the load-balancer's
//    makeDefaultLoadInfo and ThreadManager::init never cared).
//  - The unordered variants have no ORDER BY, so their row order is the
//    optimizer's choice (see MySQLSkillSaveRepository.cpp) — both
//    tables have a primary key on the selected column, so a clustered
//    scan returns key order today; not a contract.
//  - ZoneInfoManager's SELECT spells SMPFilename/SSIFilename where the
//    schema has SmpFileName/SsiFileName; MySQL resolves column names
//    case-insensitively. Kept verbatim.
//  - Zone ids reach the WHERE clauses through the same conversions as
//    before: %d for ZoneTriggers (the caller's ZoneID_t promotes to
//    int), %u for EffectPKZoneRegen and WayPointInfo; the way-point
//    race is the RACE_OUSTERS enum through %d.
//  - ZoneTriggers' coordinate columns are int unsigned but the loaders
//    read them through getInt, exactly as before.
class MySQLZoneInfoRepository : public ZoneInfoRepository {
public:
    vector<int> loadZoneGroupIDs(bool orderedByID) {
        vector<int> ids;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = orderedByID
                                  ? pStmt->executeQuery("SELECT ZoneGroupID FROM ZoneGroupInfo ORDER BY ZoneGroupID")
                                  : pStmt->executeQuery("SELECT ZoneGroupID FROM ZoneGroupInfo");

            while (pResult->next())
                ids.push_back(pResult->getInt(1));

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return ids;
    }

    vector<int> loadZoneIDsOfGroup(int zoneGroupID, bool orderedByID) {
        vector<int> ids;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                orderedByID ? pStmt->executeQuery("SELECT ZoneID FROM ZoneInfo WHERE ZoneGroupID = %d ORDER BY ZoneID",
                                                  zoneGroupID)
                            : pStmt->executeQuery("SELECT ZoneID FROM ZoneInfo WHERE ZoneGroupID = %d", zoneGroupID);

            while (pResult->next())
                ids.push_back(pResult->getInt(1));

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return ids;
    }

    vector<ZoneInfoRow> loadZoneInfos() {
        vector<ZoneInfoRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT ZoneID, ZoneGroupID, Type, Level, AccessMode, OwnerID, "
                                    "PayPlayZone, PremiumZone, PKZone, NoPortalZone, HolyLand, Available, "
                                    "OpenLevel, SMPFilename, SSIFilename, FullName, ShortName FROM ZoneInfo");

            while (pResult->next()) {
                uint i = 0;
                ZoneInfoRow row;
                row.zoneID = pResult->getInt(++i);
                row.zoneGroupID = pResult->getInt(++i);
                row.type = pResult->getString(++i);
                row.level = pResult->getInt(++i);
                row.accessMode = pResult->getString(++i);
                row.ownerID = pResult->getString(++i);
                row.payPlayZone = pResult->getInt(++i);
                row.premiumZone = pResult->getInt(++i);
                row.pkZone = pResult->getInt(++i);
                row.noPortalZone = pResult->getInt(++i);
                row.holyLand = pResult->getInt(++i);
                row.available = pResult->getInt(++i);
                row.openLevel = pResult->getInt(++i);
                row.smpFilename = pResult->getString(++i);
                row.ssiFilename = pResult->getString(++i);
                row.fullName = pResult->getString(++i);
                row.shortName = pResult->getString(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ResurrectLocationRow> loadResurrectLocations() {
        vector<ResurrectLocationRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT ZoneID, SResurrectZoneID, SResurrectX, SResurrectY, VResurrectZoneID, "
                "VResurrectX, VResurrectY, OResurrectZoneID, OResurrectX, OResurrectY FROM ZoneInfo");

            while (pResult->next()) {
                ResurrectLocationRow row;
                row.zoneID = pResult->getInt(1);
                row.slayerZoneID = pResult->getInt(2);
                row.slayerX = pResult->getInt(3);
                row.slayerY = pResult->getInt(4);
                row.vampireZoneID = pResult->getInt(5);
                row.vampireX = pResult->getInt(6);
                row.vampireY = pResult->getInt(7);
                row.oustersZoneID = pResult->getInt(8);
                row.oustersX = pResult->getInt(9);
                row.oustersY = pResult->getInt(10);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ZoneRectRow> loadTriggerRects(ZoneID_t zoneID) {
        vector<ZoneRectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT X1, Y1, X2, Y2 FROM ZoneTriggers WHERE ZoneID=%d", zoneID);

            while (pResult->next()) {
                ZoneRectRow row;
                row.left = pResult->getInt(1);
                row.top = pResult->getInt(2);
                row.right = pResult->getInt(3);
                row.bottom = pResult->getInt(4);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ZoneRectRow> loadPKZoneRegenRects(ZoneID_t zoneID) {
        vector<ZoneRectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT LeftX, TopY, RightX, BottomY FROM EffectPKZoneRegen WHERE ZoneID=%u", zoneID);

            while (pResult->next()) {
                int count = 0;
                ZoneRectRow row;
                row.left = pResult->getInt(++count);
                row.top = pResult->getInt(++count);
                row.right = pResult->getInt(++count);
                row.bottom = pResult->getInt(++count);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ZonePointRow> loadWayPoints(ZoneID_t zoneID, int race) {
        vector<ZonePointRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT X, Y FROM WayPointInfo WHERE ZoneID = %u AND Race = %d", zoneID, race);

            while (pResult->next()) {
                ZonePointRow row;
                row.x = pResult->getInt(1);
                row.y = pResult->getInt(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ZoneEffectRow> loadZoneEffectRects(ZoneID_t zoneID, int effectID) {
        vector<ZoneEffectRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT LeftX, TopY, RightX, BottomY, Value1, Value2, Value3 FROM ZoneEffectInfo "
                                    "WHERE ZoneID = %d AND EffectID = %d",
                                    zoneID, effectID);

            while (pResult->next()) {
                int count = 0;
                ZoneEffectRow row;
                row.left = pResult->getInt(++count);
                row.top = pResult->getInt(++count);
                row.right = pResult->getInt(++count);
                row.bottom = pResult->getInt(++count);
                row.value1 = pResult->getInt(++count);
                row.value2 = pResult->getInt(++count);
                row.value3 = pResult->getInt(++count);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadMonsterLists(ZoneID_t zoneID, string& monsterList, string& eventMonsterList) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT MonsterList, EventMonsterList from ZoneInfo WHERE ZoneID=%d", zoneID);

            if (pResult->next()) {
                monsterList = pResult->getString(1);
                eventMonsterList = pResult->getString(2);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    vector<PKZoneRow> loadPKZones() {
        vector<PKZoneRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT ZoneID, Race, EnterX, EnterY, ResurrectX, ResurrectY, PCLimit FROM PKZoneInfo");

            while (pResult->next()) {
                int count = 0;
                PKZoneRow row;
                row.zoneID = pResult->getInt(++count);
                row.race = pResult->getInt(++count);
                row.enterX = pResult->getInt(++count);
                row.enterY = pResult->getInt(++count);
                row.resurrectX = pResult->getInt(++count);
                row.resurrectY = pResult->getInt(++count);
                row.pcLimit = pResult->getInt(++count);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<EventZoneRow> loadEventZones() {
        vector<EventZoneRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT EventID, ZoneID, EnterX, EnterY, ResurrectX, ResurrectY, PCLimit FROM EventZoneInfo");

            while (pResult->next()) {
                EventZoneRow row;
                row.eventID = pResult->getInt(1);
                row.zoneID = pResult->getInt(2);
                row.enterX = pResult->getInt(3);
                row.enterY = pResult->getInt(4);
                row.resurrectX = pResult->getInt(5);
                row.resurrectY = pResult->getInt(6);
                row.pcLimit = pResult->getInt(7);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<LevelWarZoneRow> loadLevelWarZones() {
        vector<LevelWarZoneRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT ID, ZoneID, SweeperTypeMin, SweeperTypeMax, SlayerMin, SlayerMax, VampireMin, "
                "VampireMax, OustersMin, OustersMax, ZoneIDList FROM LevelWarZoneInfo");

            while (pResult->next()) {
                uint i = 0;
                LevelWarZoneRow row;
                row.id = pResult->getInt(++i);
                row.zoneID = pResult->getInt(++i);
                row.sweeperTypeMin = pResult->getInt(++i);
                row.sweeperTypeMax = pResult->getInt(++i);
                row.slayerMin = pResult->getInt(++i);
                row.slayerMax = pResult->getInt(++i);
                row.vampireMin = pResult->getInt(++i);
                row.vampireMax = pResult->getInt(++i);
                row.oustersMin = pResult->getInt(++i);
                row.oustersMax = pResult->getInt(++i);
                row.zoneIDList = pResult->getString(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<WayPointRow> loadAllWayPoints() {
        vector<WayPointRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT ZoneID, X, Y, Race FROM WayPointInfo");

            while (pResult->next()) {
                WayPointRow row;
                row.zoneID = pResult->getInt(1);
                row.x = pResult->getInt(2);
                row.y = pResult->getInt(3);
                row.race = pResult->getInt(4);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }
};

} // namespace

ZoneInfoRepository& defaultZoneInfoRepository() {
    static MySQLZoneInfoRepository instance;
    return instance;
}
