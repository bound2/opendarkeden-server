#include "DB.h"
#include "repository/NicknameRepository.h"

// Legacy ad-hoc SQL escaper, moved here from CGModifyNicknameHandler.cpp
// (PetItem.cpp still declares it extern). The original accumulated into a
// char[100] and, when input reached the boundary, wrote the escaped byte
// and the terminator up to TWO bytes past the buffer — with client-
// controlled input reaching it through CGModifyNickname (docs/FIXES.md).
// This version emits byte-identical output for every input the old code
// handled without overflowing and truncates cleanly at the same ~100-byte
// horizon for the rest (the Nickname column is varchar(22) regardless).
// Belongs in the database layer once more repositories need it.
string getDBString(const string& str) {
    string ret;
    ret.reserve(str.size() + 8);
    for (string::size_type i = 0; i < str.size() && ret.size() < 100; ++i) {
        char c = str[i];
        if (c == '\\' || c == '\'') {
            ret += '\\';
        }
        ret += c;
    }
    return ret;
}

namespace {

// MySQL implementation of the NicknameBook persistence seam. The legacy
// schema quirks are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - OwnerID is the character *name*, not a numeric id — denormalized; a
//    character rename orphans these rows.
//  - The id-0 custom slot stores a single space, never an empty string
//    (the client renders '' as no slot), and is created with INSERT IGNORE
//    so re-login of a character that already has one is a no-op.
//  - Plain inserts omit NickIndex and take the column default; only the
//    id-0 slot insert writes it (as 0) explicitly.
//  - Time is write-only bookkeeping (now() on insert); nothing reads it.
//  - Nickname strings get exactly the getDBString escaping above; owner
//    names are interpolated raw, as the call sites always did.
class MySQLNicknameRepository : public NicknameRepository {
public:
    vector<NicknameRecord> load(const string& ownerName) {
        vector<NicknameRecord> records;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT nID, NickType, Nickname, NickIndex FROM NicknameBook WHERE OwnerID='%s'", ownerName.c_str());

            while (pResult->next()) {
                NicknameRecord record;
                record.id = pResult->getInt(1);
                record.type = pResult->getInt(2);
                record.nickname = pResult->getString(3);
                record.index = pResult->getInt(4);
                records.push_back(record);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return records;
    }

    void insertDefaultCustomSlot(const string& ownerName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "INSERT IGNORE INTO NicknameBook (nID, OwnerID, NickType, Nickname, NickIndex, Time) VALUES "
                "(0, '%s', %u, ' ', 0, now())",
                ownerName.c_str(), NicknameInfo::NICK_CUSTOM);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insert(const string& ownerName, WORD id, BYTE type, const string& nickname) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO NicknameBook (nID, OwnerID, NickType, Nickname, Time) "
                                "VALUES (%u, '%s', %u, '%s', now())",
                                id, ownerName.c_str(), type, getDBString(nickname).c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateNickname(const string& ownerName, WORD id, const string& nickname) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE NicknameBook SET Nickname='%s' WHERE OwnerID='%s' AND nID=%u",
                                getDBString(nickname).c_str(), ownerName.c_str(), id);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

NicknameRepository& defaultNicknameRepository() {
    static MySQLNicknameRepository instance;
    return instance;
}
