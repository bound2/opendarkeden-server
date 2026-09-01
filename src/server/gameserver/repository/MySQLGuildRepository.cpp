#include "DB.h"
#include "repository/GuildRepository.h"

namespace {

// MySQL implementation of the guild seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte the inline original: the `Rank`
//    backticks (RANK is reserved on MySQL 8), the spaced "GuildMember( ... )
//    VALUES ( ... )" inserts against the unspaced union ones, the quoted
//    numeric keys in the union reads ("UnionID='%u'", "OwnerGuildID='%u'"),
//    the lower-case "and" in the union-member DELETE and the escape-penalty
//    read, "count(*)" against "COUNT(*)", the five-slot attacker OR, and
//    the DATE_FORMAT(Offertime,'%%y%%m%%d') read (the doubled %% survives
//    the format pass as a literal %).
//  - Ids stream as before: GuildID_t (WORD) and the BYTE rank/type/race/
//    state through "%d" in the GuildInfo/GuildMember statements, through
//    "%u" in the union and offer statements; the union id is a uint.
//  - GuildMember.Name and GuildUnionOffer.OwnerGuildID are primary keys, so
//    the "exactly one row" loads and the single-row offer reads see at most
//    one row; the loads return false on none.
//  - Names, dates and intros are interpolated raw (the callers pass intros
//    through Guild::correctString first), as before.
class MySQLGuildRepository : public GuildRepository {
public:
    // --- members ------------------------------------------------------------
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

    bool loadMember(const string& name, GuildMemberRow& row) {
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

    vector<GuildMemberListRow> loadActiveMembers() {
        vector<GuildMemberListRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT GuildID, Name, `Rank`, RequestDateTime, LogOn FROM GuildMember WHERE `Rank` IN ( 0, 1, 2, 3 )");

            while (pResult->next()) {
                GuildMemberListRow row;
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

    // --- guilds -------------------------------------------------------------
    void insertGuild(const GuildRecord& record) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO GuildInfo ( GuildID, GuildName, GuildType, GuildRace, GuildState, "
                                "ServerGroupID, GuildZoneID, "
                                "Master, Date, Intro ) VALUES ( %d, '%s', %d, %d, %d, %d, %d, '%s', '%s', '%s' )",
                                record.id, record.name.c_str(), record.type, record.race, record.state,
                                record.serverGroupID, record.zoneID, record.master.c_str(), record.date.c_str(),
                                record.intro.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool loadGuild(GuildID_t id, GuildRow& row) {
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

    void saveGuild(const GuildRecord& record) {
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

    vector<GuildListRow> loadGuildsInStates(int stateA, int stateB) {
        vector<GuildListRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT GuildID, GuildName, GuildType, GuildRace, GuildState, ServerGroupID, "
                                    "GuildZoneID, Master, Date, Intro FROM GuildInfo WHERE GuildState IN ( %d, %d )",
                                    stateA, stateB);

            while (pResult->next()) {
                GuildListRow row;
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

    bool loadGuildNameAndMaster(int guildID, string& name, string& master) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT GuildName, Master FROM GuildInfo WHERE GuildID=%u", guildID);

            if (pResult->getRowCount() != 0) {
                pResult->next();
                name = pResult->getString(1);
                master = pResult->getString(2);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    // --- castles and wars -----------------------------------------------------
    int countCastlesOfGuild(int guildID) {
        return countOf("SELECT count(*) FROM CastleInfo WHERE GuildID = %d", guildID);
    }

    bool loadCastleOfGuild(int guildID, int& serverID, int& zoneID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT ServerID, ZoneID FROM CastleInfo WHERE GuildID = %d", guildID);

            if (pResult->next()) {
                serverID = pResult->getInt(1);
                zoneID = pResult->getInt(2);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    int countWarSchedulesOfAttacker(int guildID) {
        int count = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT count(*) FROM WarScheduleInfo WHERE "
                "(AttackGuildID = %d OR AttackGuildID2 = %d OR AttackGuildID3 = %d OR AttackGuildID4 = "
                "%d OR AttackGuildID5 = %d) AND Status in ('WAIT', 'START')",
                guildID, guildID, guildID, guildID, guildID);

            if (pResult->next())
                count = pResult->getInt(1);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return count;
    }

    int countReinforceRegistrations(int guildID) {
        return countOf(
            "SELECT count(*) FROM ReinforceRegisterInfo, WarScheduleInfo WHERE ReinforceRegisterInfo.WarID = "
            "WarScheduleInfo.WarID AND WarScheduleInfo.Status in ('WAIT', 'START') AND "
            "ReinforceRegisterInfo.ReinforceGuildID = %d AND ReinforceRegisterInfo.Status<>'DENY'",
            guildID);
    }

    int countStartedWarsAtCastle(ServerID_t serverID, ZoneID_t zoneID) {
        int count = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT count(*) FROM WarScheduleInfo WHERE ServerID = %u AND ZoneID = %u AND Status = 'START'",
                serverID, zoneID);

            if (pResult->next())
                count = pResult->getInt(1);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return count;
    }

    int countStartedWarsOfAttacker(int guildID) {
        return countOf("SELECT count(*) FROM WarScheduleInfo WHERE AttackGuildID = %d AND Status = 'START'", guildID);
    }

    // --- unions -----------------------------------------------------------------
    uint insertUnion(GuildID_t masterGuildID) {
        uint unionID = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO GuildUnionInfo (MasterGuildID) VALUES (%u)", masterGuildID);

            unionID = pStmt->getInsertID();

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return unionID;
    }

    void insertUnionMember(uint unionID, GuildID_t guildID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO GuildUnionMember (UnionID, OwnerGuildID) VALUES (%u, %u)", unionID,
                                guildID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool deleteUnionMember(uint unionID, GuildID_t guildID) {
        bool deleted = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM GuildUnionMember WHERE UnionID = %u and OwnerGuildID = %u", unionID,
                                guildID);

            deleted = pStmt->getAffectedRowCount() >= 1;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return deleted;
    }

    void deleteUnion(uint unionID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM GuildUnionInfo WHERE UnionID = %u", unionID);
            pStmt->executeQuery("DELETE FROM GuildUnionMember WHERE UnionID = %u", unionID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<UnionRow> loadUnions() {
        vector<UnionRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT UnionID, MasterGuildID FROM GuildUnionInfo");

            while (pResult->next()) {
                UnionRow row;
                row.unionID = pResult->getInt(1);
                row.masterGuildID = pResult->getInt(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<int> loadUnionMemberGuilds(uint unionID) {
        vector<int> ids;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT OwnerGuildID FROM GuildUnionMember WHERE UnionID = %u", unionID);

            while (pResult->next())
                ids.push_back(pResult->getInt(1));

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return ids;
    }

    bool loadUnionOfGuild(GuildID_t guildID, int& unionID, int& ownerGuildID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT UnionID, OwnerGuildID FROM GuildUnionMember WHERE OwnerGuildID = %u", guildID);

            if (pResult->getRowCount() != 0) {
                pResult->next();
                unionID = pResult->getInt(1);
                ownerGuildID = pResult->getInt(2);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool loadUnionMaster(int unionID, int& masterGuildID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT MasterGuildID FROM GuildUnionInfo WHERE UnionID = %u", unionID);

            if (pResult->getRowCount() != 0) {
                pResult->next();
                masterGuildID = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    int countUnionMembers(uint unionID) {
        return countOf("SELECT COUNT(*) FROM GuildUnionMember WHERE UnionID='%u'", unionID);
    }

    // --- union offers ---------------------------------------------------------------
    int countRecentEscapes(GuildID_t guildID) {
        return countOf("SELECT COUNT(*) FROM GuildUnionOffer WHERE OfferType='ESCAPE' and "
                       "OwnerGuildID='%u' and OfferTime >= now() - interval 10 day",
                       guildID);
    }

    void deleteStaleOffers(GuildID_t guildID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "DELETE FROM GuildUnionOffer WHERE OwnerGuildID='%u' and OfferTime < now() - interval 10 day", guildID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertJoinOffer(uint unionID, GuildID_t guildID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO GuildUnionOffer (UnionID, OfferType, OwnerGuildID, OfferTime) VALUES (%u, "
                                "'JOIN', %u, now())",
                                unionID, guildID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertQuitOffer(uint unionID, GuildID_t guildID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO GuildUnionOffer (UnionID, OfferType, OwnerGuildID, OfferTime) VALUES (%u, "
                                "'QUIT', %u, now())",
                                unionID, guildID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<UnionOfferRow> loadOffers(uint unionID) {
        vector<UnionOfferRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT OfferType+0, OwnerGuildID, DATE_FORMAT(Offertime,'%%y%%m%%d') "
                                    "FROM GuildUnionOffer WHERE UnionID=%u",
                                    unionID);

            while (pResult->next()) {
                UnionOfferRow row;
                row.offerType = pResult->getInt(1);
                row.ownerGuildID = pResult->getInt(2);
                row.date = pResult->getInt(3);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadJoinOfferUnion(GuildID_t guildID, int& unionID) {
        return loadOfferUnion("SELECT UnionID FROM GuildUnionOffer WHERE OfferType='JOIN' AND OwnerGuildID=%u", guildID,
                              unionID);
    }

    bool loadQuitOfferUnion(GuildID_t guildID, int& unionID) {
        return loadOfferUnion("SELECT UnionID FROM GuildUnionOffer WHERE OfferType='QUIT' AND OwnerGuildID=%u", guildID,
                              unionID);
    }

    void deleteOffers(GuildID_t guildID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM GuildUnionOffer WHERE OwnerGuildID=%u", guildID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    int countOffers(GuildID_t guildID) {
        return countOf("SELECT count(*) FROM GuildUnionOffer WHERE OwnerGuildID=%u", guildID);
    }

private:
    // One COUNT(*) with one integer parameter; 0 when the row is missing.
    template <typename T> static int countOf(const char* query, T argument) {
        int count = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(query, argument);

            if (pResult->next())
                count = pResult->getInt(1);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return count;
    }

    static bool loadOfferUnion(const char* query, GuildID_t guildID, int& unionID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(query, guildID);

            if (pResult->next()) {
                unionID = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }
};

} // namespace

GuildRepository& defaultGuildRepository() {
    static MySQLGuildRepository instance;
    return instance;
}
