#include "DB.h"
#include "repository/CoupleRepository.h"

namespace {

// Which CoupleInfo column holds a character of a given sex. This is
// CoupleManager::getFieldName / getCounterFieldName, moved: Sex is
// FEMALE = 0, MALE = 1 (src/Core/types/CreatureTypes.h) and the array
// is ordered to match, so fieldName(sex) is the column holding a
// character of that sex and counterFieldName(sex) is the partner's.
//
// The indexing is unchanged, INCLUDING its hazard: neither lookup
// bounds-checks, so a Sex outside {FEMALE, MALE} reads past the array
// ([2]) or before it ([1 - 2] = [-1]). Task 3.2 moves statements
// without changing what they do, so no clamp is added here; the hazard
// is inherited from CoupleManager and belongs to whichever round
// decides to bound it.
const char* const SEX_FIELD_NAME[] = {
    "FemalePartnerName",
    "MalePartnerName",
};

const char* fieldName(Sex sex) {
    return SEX_FIELD_NAME[(int)sex];
}

const char* counterFieldName(Sex sex) {
    return SEX_FIELD_NAME[1 - (int)sex];
}

// MySQL implementation of the couple seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte the inline original: the
//    lower-case "where" and "and" in the probes, the upper-case WHERE in
//    removeCouple's DELETE against the lower-case one in
//    removeCoupleForce's, the space before the closing paren of the
//    INSERT's column list, and the unspaced "'%s','%s',%u, now()".
//  - The column NAMES are interpolated through %s, so they are part of
//    the varargs like any other string. They come from the table above
//    and never from a caller, so no caller can put SQL in an identifier
//    position.
//  - Race streams as the (uint) cast the call sites applied, through
//    "%u"; names are interpolated raw (no escaping), as before.
//  - The count(*) probes always return a row, so next() cannot fail on
//    a live connection; each returns 0 if it ever did, which the callers
//    read as "not a couple" — the same answer the inline code's
//    unmodified bRet gave.
class MySQLCoupleRepository : public CoupleRepository {
public:
    int countPairingWithPartner(Sex sex, const string& name, const string& partnerName) {
        return countOf(fieldName(sex), name, counterFieldName(sex), partnerName);
    }

    int countPairing(Sex sex1, const string& name1, Sex sex2, const string& name2) {
        return countOf(fieldName(sex1), name1, fieldName(sex2), name2);
    }

    int countPairingsOf(Sex sex, const string& name) {
        int count = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT count(*) FROM CoupleInfo where %s='%s'", fieldName(sex), name.c_str());

            if (pResult->next())
                count = pResult->getInt(1);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return count;
    }

    bool loadPartnerName(Sex sex, const string& name, string& partnerName) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT %s FROM CoupleInfo where %s='%s'", counterFieldName(sex),
                                                  fieldName(sex), name.c_str());

            if (pResult->next()) {
                partnerName = pResult->getString(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void insertCouple(Sex sex1, const string& name1, Sex sex2, const string& name2, uint race) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO CoupleInfo (%s, %s, Race, CoupleDate ) VALUES ('%s','%s',%u, now())",
                                fieldName(sex1), fieldName(sex2), name1.c_str(), name2.c_str(), race);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deletePairing(Sex sex1, const string& name1, Sex sex2, const string& name2, uint race) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM CoupleInfo WHERE %s='%s' AND %s='%s' AND Race=%u", fieldName(sex1),
                                name1.c_str(), fieldName(sex2), name2.c_str(), race);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deletePairingWithPartner(Sex sex, const string& name, const string& partnerName, uint race) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            // Lower-case "where"; deletePairing's is upper-case.
            pStmt->executeQuery("DELETE FROM CoupleInfo where %s='%s' AND %s='%s' AND Race=%u", fieldName(sex),
                                name.c_str(), counterFieldName(sex), partnerName.c_str(), race);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deletePairingsOf(Sex sex, const string& name, uint race) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM CoupleInfo where %s='%s' AND Race=%u", fieldName(sex), name.c_str(), race);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

private:
    // The two-column probe both isCouple overloads run, spelled once.
    static int countOf(const char* ownColumn, const string& ownName, const char* partnerColumn,
                       const string& partnerName) {
        int count = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT count(*) FROM CoupleInfo where %s='%s' and %s='%s'",
                                                  ownColumn, ownName.c_str(), partnerColumn, partnerName.c_str());

            if (pResult->next())
                count = pResult->getInt(1);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return count;
    }
};

} // namespace

CoupleRepository& defaultCoupleRepository() {
    static MySQLCoupleRepository instance;
    return instance;
}
