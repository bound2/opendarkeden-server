#include "DB.h"
#include "repository/BloodBibleSignRepository.h"

namespace {

// MySQL implementation of the BloodBibleSignObject persistence seam. The
// legacy schema quirks are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - OwnerID is the character *name* (varchar(10)) — denormalized; a
//    character rename orphans these rows.
//  - ItemType is a tinyint widened into ItemType_t (WORD) on read.
//  - Duplicate ItemType rows are possible (the key is the auto-increment
//    ItemID) and are surfaced as-is, exactly as the inline loop did.
//  - Owner names are interpolated raw (no escaping), as the call site
//    always did.
class MySQLBloodBibleSignRepository : public BloodBibleSignRepository {
public:
    vector<ItemType_t> loadItemTypes(const string& ownerName) {
        vector<ItemType_t> itemTypes;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT ItemType FROM BloodBibleSignObject WHERE OwnerID='%s' ORDER BY ItemType", ownerName.c_str());

            while (pResult->next()) {
                itemTypes.push_back(pResult->getInt(1));
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return itemTypes;
    }
};

} // namespace

BloodBibleSignRepository& defaultBloodBibleSignRepository() {
    static MySQLBloodBibleSignRepository instance;
    return instance;
}
