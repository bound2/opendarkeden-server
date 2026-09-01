#include "DB.h"
#include "repository/RegenZoneRepository.h"

namespace {

// MySQL implementation of the regen-zone seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - The SELECT is byte-for-byte the RegenZoneManager.cpp original
//    (load and reload issued the identical statement).
//  - No ORDER BY; ID is the primary key, so a clustered scan returns
//    ID order today — the optimizer's choice, not a contract (see
//    MySQLSkillSaveRepository.cpp). The callers key everything by ID.
//  - Owner is tinyint unsigned defaulting to 3 (unowned); the callers
//    Assert(Owner < 4) on what they read.
//  - Neither original caller freed its Statement (a leak per boot and
//    per reload); the seam does — fixed knowingly, as in earlier
//    rounds.
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
