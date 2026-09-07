#include "DB.h"
#include "repository/SharedGuildRepository.h"

namespace {

// The race-table statements, indexed by Guild::GuildRace (0 Slayer, 1
// Vampire, 2 Ousters). The GuildID UPDATE is one literal per table; the
// Gold refund is one format string that takes the table name.
struct RaceTableSpec {
    const char* guildIDUpdate;
    const char* table;
};

const int kRaceTableCount = 3;

const RaceTableSpec kRaceTables[kRaceTableCount] = {
    {"UPDATE Slayer SET GuildID = %d WHERE Name = '%s'", "Slayer"},
    {"UPDATE Vampire SET GuildID = %d WHERE Name = '%s'", "Vampire"},
    {"UPDATE Ousters SET GuildID = %d WHERE Name = '%s'", "Ousters"},
};

const char* const kMessageInsert[SHARED_MESSAGE_SQL_SPELLING_MAX] = {
    "INSERT INTO Messages (Receiver, Message ) VALUES ('%s', '%s' )",
    "INSERT INTO Messages ( Receiver, Message ) VALUES ( '%s', '%s' )",
};

// MySQL implementation of SharedGuildRepository. Everything runs on
// getConnection("DARKEDEN"). A race outside the three tables, or a
// spelling outside the enum, runs no statement.
class MySQLSharedGuildRepository : public SharedGuildRepository {
public:
    bool memberExists(const string& name) {
        bool exists = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT GuildID FROM GuildMember WHERE Name = '%s'", name.c_str());

            exists = pResult->getRowCount() != 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return exists;
    }

    void insertMember(GuildID_t guildID, const string& name, GuildMemberRank_t rank) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO GuildMember( GuildID, Name, `Rank` ) VALUES ( %d, '%s', %d )", guildID,
                                name.c_str(), rank);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertWaitingMember(GuildID_t guildID, const string& name, GuildMemberRank_t rank,
                             const string& requestDateTime) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "INSERT INTO GuildMember( GuildID, Name, `Rank`, RequestDateTime ) VALUES ( %d, '%s', %d, '%s' )",
                guildID, name.c_str(), rank, requestDateTime.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void rejoinMember(GuildID_t guildID, GuildMemberRank_t rank, const string& name) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE GuildMember SET GuildID = %d, `Rank` = %d, ExpireDate = '' WHERE Name = '%s'",
                                guildID, rank, name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void rejoinWaitingMember(GuildID_t guildID, GuildMemberRank_t rank, const string& requestDateTime,
                             const string& name) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE GuildMember SET GuildID = %d, `Rank` = %d, ExpireDate = '', "
                                "RequestDateTime = '%s' WHERE Name = '%s'",
                                guildID, rank, requestDateTime.c_str(), name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadMember(const string& name, SharedGuildMemberRow& row) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT GuildID, Name, `Rank`, LogOn FROM GuildMember WHERE Name = '%s'", name.c_str());

            if (pResult->getRowCount() == 1) {
                pResult->next();

                row.guildID = pResult->getInt(1);
                row.name = pResult->getString(2);
                row.rank = pResult->getInt(3);
                row.logOn = pResult->getInt(4);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void saveMember(GuildID_t guildID, GuildMemberRank_t rank, const string& name) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE GuildMember SET GuildID = %d, `Rank` = %d WHERE Name = '%s'", guildID, rank,
                                name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deleteMember(const string& name) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM GuildMember WHERE Name = '%s'", name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void setMemberRankAndExpireDate(int rank, const string& expireDate, const string& name) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE GuildMember SET `Rank` = %d, ExpireDate = '%s' WHERE Name = '%s'", rank,
                                expireDate.c_str(), name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void saveMemberIntro(const string& intro, const string& name) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE GuildMember SET Intro = '%s' WHERE Name = '%s'", intro.c_str(), name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadMemberIntro(const string& name, string& intro) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Intro FROM GuildMember WHERE Name = '%s'", name.c_str());

            if (pResult->next()) {
                intro = pResult->getString(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void stampMemberRequestDateTime(const string& name) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE GuildMember SET RequestDateTime=now() WHERE Name='%s'", name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<SharedGuildMemberListRow> loadActiveMembers() {
        vector<SharedGuildMemberListRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT GuildID, Name, `Rank`, RequestDateTime, LogOn FROM GuildMember WHERE `Rank` IN ( 0, 1, 2, 3 )");

            while (pResult->next()) {
                SharedGuildMemberListRow row;
                row.guildID = pResult->getInt(1);
                row.name = pResult->getString(2);
                row.rank = pResult->getInt(3);
                row.requestDateTime = pResult->getString(4);
                row.logOn = pResult->getInt(5);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void insertGuild(const SharedGuildRecord& record) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "INSERT INTO GuildInfo ( GuildID, GuildName, GuildType, GuildRace, GuildState, ServerGroupID, "
                "GuildZoneID, "
                "Master, Date, Intro ) VALUES ( %d, '%s', %d, %d, %d, %d, %d, '%s', '%s', '%s' )",
                record.id, record.name.c_str(), record.type, record.race, record.state, record.serverGroupID,
                record.zoneID, record.master.c_str(), record.date.c_str(), record.intro.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadGuild(GuildID_t id, SharedGuildRow& row) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT GuildName, GuildType, GuildRace, GuildState, ServerGroupID, GuildZoneID, "
                                    "Master, Date FROM GuildInfo WHERE GuildID = %d",
                                    id);

            if (pResult->getRowCount() == 1) {
                pResult->next();

                row.name = pResult->getString(1);
                row.type = pResult->getInt(2);
                row.race = pResult->getInt(3);
                row.state = pResult->getInt(4);
                row.serverGroupID = pResult->getInt(5);
                row.zoneID = pResult->getInt(6);
                row.master = pResult->getString(7);
                row.date = pResult->getString(8);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void saveGuild(const SharedGuildRecord& record) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "UPDATE GuildInfo SET GuildName = '%s', GuildType = %d, GuildRace = %d, GuildState = %d, "
                "ServerGroupID = %d, GuildZoneID = %d, Master = '%s', Date = '%s' WHERE GuildID = %d",
                record.name.c_str(), record.type, record.race, record.state, record.serverGroupID, record.zoneID,
                record.master.c_str(), record.date.c_str(), record.id);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void deleteGuild(GuildID_t id) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            pStmt->executeQuery("DELETE FROM GuildInfo WHERE GuildID = %d", id);
            pStmt->executeQuery("DELETE FROM GuildUnionMember WHERE OwnerGuildID = %d", id);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void saveGuildIntro(const string& intro, GuildID_t id) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE GuildInfo SET Intro = '%s' WHERE GuildID = %u", intro.c_str(), id);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateGuildFields(const string& assignments, GuildID_t id) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE GuildInfo SET %s WHERE GuildID = %u", assignments.c_str(), id);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<SharedGuildListRow> loadGuildsInStates(int stateA, int stateB) {
        vector<SharedGuildListRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT GuildID, GuildName, GuildType, GuildRace, GuildState, ServerGroupID, "
                                    "GuildZoneID, Master, Date, Intro FROM GuildInfo WHERE GuildState IN ( %d, %d )",
                                    stateA, stateB);

            while (pResult->next()) {
                SharedGuildListRow row;
                row.id = pResult->getInt(1);
                row.name = pResult->getString(2);
                row.type = pResult->getInt(3);
                row.race = pResult->getInt(4);
                row.state = pResult->getInt(5);
                row.serverGroupID = pResult->getInt(6);
                row.zoneID = pResult->getInt(7);
                row.master = pResult->getString(8);
                row.date = pResult->getString(9);
                row.intro = pResult->getString(10);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void purgeGuild(GuildID_t id) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            pStmt->executeQuery("DELETE FROM GuildInfo WHERE GuildID=%d", id);
            pStmt->executeQuery("DELETE FROM GuildMember WHERE GuildID=%d", id);
            pStmt->executeQuery("DELETE FROM GuildUnionMember WHERE OwnerGuildID=%d", id);
            pStmt->executeQuery("UPDATE WarScheduleInfo SET Status='CANCEL' WHERE AttackGuildID=%d", id);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    int countGuilds() {
        int count = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT COUNT(*) FROM GuildInfo");

            pResult->next();
            count = pResult->getInt(1);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return count;
    }

    int loadMaxGuildID() {
        int maxID = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT MAX(GuildID) FROM GuildInfo");

            pResult->next();
            maxID = pResult->getInt(1);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return maxID;
    }

    int countGuildsOfRace(int race) {
        int count = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT COUNT(*) FROM GuildInfo WHERE GuildRace = %d", race);

            pResult->next();
            count = pResult->getInt(1);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return count;
    }

    int loadMaxGuildZoneIDOfRace(int race) {
        int maxZoneID = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT MAX(GuildZoneID) FROM GuildInfo WHERE GuildRace = %d", race);

            pResult->next();
            maxZoneID = pResult->getInt(1);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return maxZoneID;
    }

    void setCharacterGuildID(GuildRace_t race, int guildID, const string& name) {
        if (race >= kRaceTableCount)
            return;

        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(kRaceTables[race].guildIDUpdate, guildID, name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void addCharacterGold(GuildRace_t race, int gold, const string& name) {
        if (race >= kRaceTableCount)
            return;

        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE %s SET Gold = Gold + %d WHERE Name = '%s'", kRaceTables[race].table, gold,
                                name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertMessage(SharedMessageSpelling spelling, const string& receiver, const string& message) {
        if (spelling >= SHARED_MESSAGE_SQL_SPELLING_MAX)
            return;

        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(kMessageInsert[spelling], receiver.c_str(), message.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

SharedGuildRepository& defaultSharedGuildRepository() {
    static MySQLSharedGuildRepository instance;
    return instance;
}
