#include "DB.h"
#include "repository/RegenZoneRepository.h"

namespace {

// MySQL implementation of RegenZoneRepository. Quirks:
//  - No ORDER BY; ID is the primary key, so a clustered scan returns
//    ID order today — the optimizer's choice, not a contract (see
//    MySQLSkillSaveRepository.cpp). The callers key everything by ID.
//  - Owner is tinyint unsigned defaulting to 3 (unowned); the callers
//    Assert(Owner < 4) on what they read.
class MySQLRegenZoneRepository : public RegenZoneRepository {
public:
    vector<RegenZoneRow> loadPositions() {
        vector<RegenZoneRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT ID, ZoneID, ZoneX, ZoneY, Owner FROM RegenZonePosition");

            while (pResult->next()) {
                RegenZoneRow row;
                row.id = pResult->getInt(1);
                row.zoneID = pResult->getInt(2);
                row.zoneX = pResult->getInt(3);
                row.zoneY = pResult->getInt(4);
                row.owner = pResult->getInt(5);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }
};

} // namespace

RegenZoneRepository& defaultRegenZoneRepository() {
    static MySQLRegenZoneRepository instance;
    return instance;
}
