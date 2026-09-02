#include "DB.h"
#include "repository/SMSAddressRepository.h"

namespace {

// MySQL implementation of the SMS address-book seam. The legacy quirks
// are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - The SQL is byte-for-byte the SMSAddressBook.cpp originals.
//  - (eID, OwnerID) is the PRIMARY KEY: insert() raises ER_DUP_ENTRY
//    (1062) for a repeated id — the class checks its in-memory map
//    first, so the row conflict only shows when memory and table drift.
//    Pinned by the integration tier.
//  - The table also has a Time column (datetime, defaulting to a fixed
//    2004 date) that nothing on the server reads or writes.
//  - The load has no ORDER BY; the class rebuilds a hash map from the
//    rows and derives its next id from the maximum, so order is
//    immaterial.
//  - Every text — owner, names, number — is interpolated raw, as before.
class MySQLSMSAddressRepository : public SMSAddressRepository {
public:
    vector<SMSAddressRow> load(const string& ownerName) {
        vector<SMSAddressRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT eID, CharacterName, CustomName, Number FROM SMSAddressBook WHERE OwnerID='%s'",
                ownerName.c_str());

            while (pResult->next()) {
                SMSAddressRow row;
                row.eID = pResult->getInt(1);
                row.characterName = pResult->getString(2);
                row.customName = pResult->getString(3);
                row.number = pResult->getString(4);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void insert(const string& ownerName, DWORD eID, const string& characterName, const string& customName,
                const string& number) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO SMSAddressBook (eID, OwnerID, CharacterName, CustomName, Number) VALUES "
                                "(%u, '%s', '%s', '%s', '%s')",
                                eID, ownerName.c_str(), characterName.c_str(), customName.c_str(), number.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void remove(const string& ownerName, DWORD eID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM SMSAddressBook WHERE eID = %u AND OwnerID = '%s'", eID, ownerName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

SMSAddressRepository& defaultSMSAddressRepository() {
    static MySQLSMSAddressRepository instance;
    return instance;
}
