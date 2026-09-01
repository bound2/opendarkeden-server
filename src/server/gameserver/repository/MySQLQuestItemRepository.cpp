#include "DB.h"
#include "repository/QuestItemRepository.h"

namespace {

// MySQL implementation of the quest-item seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - The SQL is byte-for-byte what GQuestInventory.cpp and the two
//    GQuestGive*Element.cpp emitted (the three inserts were already the
//    same literal).
//  - removeOne's "LIMIT 1" is load-bearing: the table stores one row
//    per item INSTANCE (auto-increment ItemID, never read), so taking
//    one item of a type the owner holds twice must leave the other.
//    Pinned by the integration tier.
//  - ItemType is tinyint unsigned; the %u conversion prints the WORD
//    ItemType_t promoted to int, so a type above 255 would be clamped
//    by the column under the non-strict sql_mode. Not reached by the
//    quest item ids in use.
//  - The load has no ORDER BY and the class keeps a list in whatever
//    order arrives; not a contract.
//  - Two of the three original callers (GQuestInventory::load and
//    ::removeOne) leaked their Statement on the success path; the seam
//    frees it — a per-call leak fixed knowingly, as in the earlier
//    rounds.
class MySQLQuestItemRepository : public QuestItemRepository {
public:
    vector<int> loadItemTypes(const string& ownerName) {
        vector<int> itemTypes;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT ItemType FROM GQuestItemObject WHERE OwnerID='%s'", ownerName.c_str());

            while (pResult->next())
                itemTypes.push_back(pResult->getInt(1));

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return itemTypes;
    }

    void insert(const string& ownerName, ItemType_t itemType) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO GQuestItemObject(ItemType, OwnerID) VALUES (%u, '%s')", itemType,
                                ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void removeOne(const string& ownerName, ItemType_t itemType) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM GQuestItemObject WHERE OwnerID='%s' AND ItemType=%u LIMIT 1",
                                ownerName.c_str(), itemType);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

QuestItemRepository& defaultQuestItemRepository() {
    static MySQLQuestItemRepository instance;
    return instance;
}
