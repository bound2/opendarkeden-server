#include "database/DB.h"
#include "repository/PayPlayRepository.h"

namespace {

// MySQL implementation of PayPlayRepository. Every method creates its
// Statement on getDistConnection("PLAYER_DB") (see the header) and frees
// it on every success path; a SQL failure is logged to DBError.log under
// the method's name and rethrown as END_DB's DatabaseError.
class MySQLPayPlayRepository : public PayPlayRepository {
public:
    bool loadPCRoomByIP(const string& ip, PayPlayPCRoomRow& row) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT r.ID, r.PayType, r.PayStartDate, r.PayPlayDate, r.PayPlayHours, r.PayPlayFlag, r.UserLimit, "
                "r.UserMax FROM PCRoomInfo r, PCRoomIPInfo p WHERE p.IP='%s' AND p.ID=r.ID",
                ip.c_str());

            if (pResult->next()) {
                uint i = 0;
                row.id = pResult->getInt(++i);
                row.payType = pResult->getInt(++i);
                row.payStartDate = pResult->getString(++i);
                row.payPlayDate = pResult->getString(++i);
                row.payPlayHours = pResult->getInt(++i);
                row.payPlayFlag = pResult->getInt(++i);
                row.userLimit = pResult->getInt(++i);
                row.userMax = pResult->getInt(++i);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadPCRoomPeriodByIP(const string& ip, PayPlayPCRoomPeriodRow& row) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT r.ID, r.PayType, r.PayStartDate, r.PayPlayDate, r.PayPlayHours FROM "
                                    "PCRoomInfo r, PCRoomIPInfo p WHERE p.IP='%s' AND p.ID=r.ID",
                                    ip.c_str());

            if (pResult->next()) {
                uint i = 0;
                row.id = pResult->getInt(++i);
                row.payType = pResult->getInt(++i);
                row.payStartDate = pResult->getString(++i);
                row.payPlayDate = pResult->getString(++i);
                row.payPlayHours = pResult->getInt(++i);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool decreasePCRoomPayPlayHours(uint hours, int roomID, int& remaining) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("UPDATE PCRoomInfo SET PayPlayHours=PayPlayHours-%d WHERE ID=%d", hours, roomID);

            Result* pResult = pStmt->executeQuery("SELECT PayPlayHours FROM PCRoomInfo WHERE ID=%d", roomID);

            if (pResult->next()) {
                remaining = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    int loadPCRoomUserCount(int roomID) {
        int users = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT count(*) from PCRoomUserInfo WHERE ID=%d", roomID);

            if (pResult->next()) {
                users = pResult->getInt(1);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return users;
    }

    void insertPCRoomUser(int roomID, const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("INSERT IGNORE INTO PCRoomUserInfo(ID, PlayerID) VALUES(%d, '%s')", roomID,
                                playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deletePCRoomUser(const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("DELETE FROM PCRoomUserInfo WHERE PlayerID='%s'", playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool hasPCRoomPayMonth(int roomID, int year, int month) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT PayPlayMinute FROM PCRoomPayList WHERE PCRoomID=%d AND Year=%d AND Month=%d", roomID, year,
                month);

            found = pResult->next();

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void addPCRoomPayMinutes(uint minutes, int roomID, int year, int month) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery(
                "UPDATE PCRoomPayList SET PayPlayMinute=PayPlayMinute+%d WHERE PCRoomID=%d AND Year=%d AND Month=%d",
                minutes, roomID, year, month);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertPCRoomPayMonth(int roomID, int year, int month, uint minutes) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery(
                "INSERT INTO PCRoomPayList (PCRoomID, Year, Month, PayPlayMinute) VALUES (%d, %d, %d, %d)", roomID,
                year, month, minutes);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadAccountPayPlay(const string& playerID, PayPlayAccountRow& row) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT PayType, PayPlayDate, PayPlayHours, PayPlayFlag, "
                                                  "FamilyPayPlayDate FROM Player WHERE PlayerID='%s'",
                                                  playerID.c_str());

            if (pResult->next()) {
                row.payType = pResult->getInt(1);
                row.payPlayDate = pResult->getString(2);
                row.payPlayHours = pResult->getInt(3);
                row.payPlayFlag = pResult->getInt(4);
                row.familyPayPlayDate = pResult->getString(5);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void clearAccountPayPlay(const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery(
                "UPDATE Player SET PayPlayHours=0, PayPlayDate='2002-11-18 00:00:00' WHERE PlayerID='%s'",
                playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void decreaseAccountPayPlayHours(uint hours, const string& playerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            pStmt->executeQuery("UPDATE Player SET PayPlayHours=PayPlayHours-%d WHERE PlayerID='%s'", hours,
                                playerID.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadAccountPayPlaying(const string& playerID, int& flag) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getDistConnection("PLAYER_DB")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT PayType=0 or PayPlayDate > now() FROM Player WHERE PlayerID='%s'", playerID.c_str());

            if (pResult->next()) {
                flag = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }
};

} // namespace

PayPlayRepository& defaultPayPlayRepository() {
    static MySQLPayPlayRepository instance;
    return instance;
}
