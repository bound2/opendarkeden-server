// MySQL-backed integration tier for the sharedserver's repositories: the
// real MySQLSharedGuildRepository and MySQLSharedConfigRepository against
// the throwaway MySQL 5.7 loaded with initdb/, on the DARKEDEN connection
// mysql_repository_test.cpp's main() wires for this binary.
//
// Guild rows are seeded with ids from 61700 (GuildID_t is a WORD) and names
// prefixed "it-sg"; the config rows with WorldID / ZoneID 9170 and ids from
// 61700. The gameserver's fixtures in the same binary clean GuildInfo ids
// from 31000 up in their SetUp, so nothing here relies on a row outliving
// its test.

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "DB.h"
#include "repository/SharedConfigRepository.h"
#include "repository/SharedGuildRepository.h"

namespace {

void execSQL(const std::string& sql) {
    Statement* pStmt = NULL;
    BEGIN_DB {
        pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
        pStmt->executeQueryString(sql);
        SAFE_DELETE(pStmt);
    }
    END_DB(pStmt)
}

std::string queryScalar(const std::string& sql) {
    std::string value;
    Statement* pStmt = NULL;
    BEGIN_DB {
        pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
        Result* pResult = pStmt->executeQueryString(sql);
        if (pResult->next())
            value = pResult->getString(1);
        SAFE_DELETE(pStmt);
    }
    END_DB(pStmt)
    return value;
}

std::string q(const std::string& s) {
    return "'" + s + "'";
}

const GuildID_t kGuildA = 61701;
const GuildID_t kGuildB = 61702;

class SharedGuildMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        execSQL("DELETE FROM GuildInfo WHERE GuildID BETWEEN 61700 AND 61799");
        execSQL("DELETE FROM GuildMember WHERE Name LIKE 'it-sg%'");
        execSQL("DELETE FROM GuildUnionMember WHERE OwnerGuildID BETWEEN 61700 AND 61799");
        execSQL("DELETE FROM WarScheduleInfo WHERE WarID BETWEEN 61700 AND 61799");
        execSQL("DELETE FROM Messages WHERE Receiver LIKE 'it-sg%'");
        execSQL("DELETE FROM Slayer WHERE Name LIKE 'it-sg%'");
        execSQL("DELETE FROM Vampire WHERE Name LIKE 'it-sg%'");
        execSQL("DELETE FROM Ousters WHERE Name LIKE 'it-sg%'");
    }

    static void seedGuild(GuildID_t id, const std::string& name, int race, int state, int zoneID,
                          const std::string& master, const std::string& intro = "") {
        execSQL("INSERT INTO GuildInfo (GuildID, GuildName, GuildType, GuildRace, GuildState, ServerGroupID, "
                "GuildZoneID, Master, Date, Intro) VALUES (" +
                std::to_string(id) + ", " + q(name) + ", 1, " + std::to_string(race) + ", " + std::to_string(state) +
                ", 2, " + std::to_string(zoneID) + ", " + q(master) + ", '2026-09-07', " + q(intro) + ")");
    }

    static void seedMember(GuildID_t guildID, const std::string& name, int rank, const std::string& intro = "",
                           int logOn = 0) {
        execSQL("INSERT INTO GuildMember (GuildID, Name, `Rank`, Intro, LogOn) VALUES (" + std::to_string(guildID) +
                ", " + q(name) + ", " + std::to_string(rank) + ", " + q(intro) + ", " + std::to_string(logOn) + ")");
    }

    static std::string memberField(const char* column, const std::string& name) {
        return queryScalar(std::string("SELECT ") + column + " FROM GuildMember WHERE Name = " + q(name));
    }

    static std::string guildField(const char* column, GuildID_t id) {
        return queryScalar(std::string("SELECT ") + column + " FROM GuildInfo WHERE GuildID = " + std::to_string(id));
    }

    static std::string count(const std::string& fromWhere) {
        return queryScalar("SELECT COUNT(*) FROM " + fromWhere);
    }
};

TEST_F(SharedGuildMySQL, MemberRowLifecycle) {
    SharedGuildRepository& repo = defaultSharedGuildRepository();

    EXPECT_FALSE(repo.memberExists("it-sga"));

    repo.insertMember(kGuildA, "it-sga", 1);
    seedMember(kGuildB, "it-sgk", 2, "", 1);

    EXPECT_TRUE(repo.memberExists("it-sga"));
    EXPECT_EQ("", memberField("Intro", "it-sga"));
    EXPECT_EQ("", memberField("ExpireDate", "it-sga"));

    SharedGuildMemberRow row;
    ASSERT_TRUE(repo.loadMember("it-sga", row));
    EXPECT_EQ((int)kGuildA, row.guildID);
    EXPECT_EQ("it-sga", row.name);
    EXPECT_EQ(1, row.rank);
    EXPECT_EQ(0, row.logOn);

    repo.saveMember(kGuildB, 0, "it-sga");
    EXPECT_EQ(std::to_string(kGuildB), memberField("GuildID", "it-sga"));
    EXPECT_EQ("0", memberField("`Rank`", "it-sga"));

    // A missing name answers false and leaves the row alone.
    row.name = "untouched";
    EXPECT_FALSE(repo.loadMember("it-sgnone", row));
    EXPECT_EQ("untouched", row.name);

    repo.deleteMember("it-sga");
    EXPECT_FALSE(repo.memberExists("it-sga"));

    // The other member is whole.
    ASSERT_TRUE(repo.loadMember("it-sgk", row));
    EXPECT_EQ((int)kGuildB, row.guildID);
    EXPECT_EQ(2, row.rank);
    EXPECT_EQ(1, row.logOn);
}

TEST_F(SharedGuildMySQL, WaitingInsertAndRejoinsRewriteTheRowOfThatNameOnly) {
    SharedGuildRepository& repo = defaultSharedGuildRepository();

    repo.insertWaitingMember(kGuildA, "it-sga", 3, "2026-09-07 10:00:00");
    seedMember(kGuildB, "it-sgk", 5);
    execSQL("UPDATE GuildMember SET ExpireDate = '1260807' WHERE Name = 'it-sgk'");

    EXPECT_EQ("2026-09-07 10:00:00", memberField("RequestDateTime", "it-sga"));
    EXPECT_EQ("3", memberField("`Rank`", "it-sga"));

    // A left member (rank 5, ExpireDate set) rejoining another guild: the
    // ExpireDate is cleared with the new guild and rank.
    execSQL("UPDATE GuildMember SET ExpireDate = '1260807', `Rank` = 5 WHERE Name = 'it-sga'");
    repo.rejoinMember(kGuildB, 0, "it-sga");
    EXPECT_EQ(std::to_string(kGuildB), memberField("GuildID", "it-sga"));
    EXPECT_EQ("0", memberField("`Rank`", "it-sga"));
    EXPECT_EQ("", memberField("ExpireDate", "it-sga"));
    EXPECT_EQ("2026-09-07 10:00:00", memberField("RequestDateTime", "it-sga"));

    execSQL("UPDATE GuildMember SET ExpireDate = '1260807' WHERE Name = 'it-sga'");
    repo.rejoinWaitingMember(kGuildA, 3, "2026-09-08 11:00:00", "it-sga");
    EXPECT_EQ(std::to_string(kGuildA), memberField("GuildID", "it-sga"));
    EXPECT_EQ("3", memberField("`Rank`", "it-sga"));
    EXPECT_EQ("", memberField("ExpireDate", "it-sga"));
    EXPECT_EQ("2026-09-08 11:00:00", memberField("RequestDateTime", "it-sga"));

    EXPECT_EQ("1260807", memberField("ExpireDate", "it-sgk"));
    EXPECT_EQ("5", memberField("`Rank`", "it-sgk"));
}

TEST_F(SharedGuildMySQL, RankExpireIntroAndRequestStamp) {
    SharedGuildRepository& repo = defaultSharedGuildRepository();

    seedMember(kGuildA, "it-sga", 0);
    seedMember(kGuildA, "it-sgk", 0);

    repo.setMemberRankAndExpireDate(4, "1260807", "it-sga");
    EXPECT_EQ("4", memberField("`Rank`", "it-sga"));
    EXPECT_EQ("1260807", memberField("ExpireDate", "it-sga"));
    EXPECT_EQ("0", memberField("`Rank`", "it-sgk"));

    repo.saveMemberIntro("hello there", "it-sga");
    std::string intro = "untouched";
    EXPECT_TRUE(repo.loadMemberIntro("it-sga", intro));
    EXPECT_EQ("hello there", intro);
    EXPECT_EQ("", memberField("Intro", "it-sgk"));

    // A missing name answers false and leaves the out-parameter alone.
    intro = "untouched";
    EXPECT_FALSE(repo.loadMemberIntro("it-sgnone", intro));
    EXPECT_EQ("untouched", intro);

    repo.stampMemberRequestDateTime("it-sga");
    EXPECT_EQ("1", queryScalar("SELECT TIMESTAMPDIFF(SECOND, RequestDateTime, NOW()) BETWEEN 0 AND 60 FROM "
                               "GuildMember WHERE Name = 'it-sga'"));
    EXPECT_EQ("0000-00-00 00:00:00", memberField("RequestDateTime", "it-sgk"));
}

TEST_F(SharedGuildMySQL, LoadActiveMembersReturnsRanksZeroToThreeWithEveryColumn) {
    SharedGuildRepository& repo = defaultSharedGuildRepository();

    seedMember(kGuildA, "it-sg0", 0, "", 1);
    seedMember(kGuildA, "it-sg1", 1);
    seedMember(kGuildA, "it-sg2", 2);
    seedMember(kGuildA, "it-sg3", 3);
    seedMember(kGuildA, "it-sg4", 4);
    seedMember(kGuildA, "it-sg5", 5);
    execSQL("UPDATE GuildMember SET RequestDateTime = '2026-09-07 10:00:00' WHERE Name = 'it-sg3'");

    std::vector<SharedGuildMemberListRow> rows = repo.loadActiveMembers();

    int seen = 0;
    bool sawDeniedOrLeft = false;
    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i].name == "it-sg0") {
            seen++;
            EXPECT_EQ((int)kGuildA, rows[i].guildID);
            EXPECT_EQ(0, rows[i].rank);
            EXPECT_EQ(1, rows[i].logOn);
            EXPECT_EQ("0000-00-00 00:00:00", rows[i].requestDateTime);
        } else if (rows[i].name == "it-sg1" || rows[i].name == "it-sg2") {
            seen++;
        } else if (rows[i].name == "it-sg3") {
            seen++;
            EXPECT_EQ(3, rows[i].rank);
            EXPECT_EQ("2026-09-07 10:00:00", rows[i].requestDateTime);
            EXPECT_EQ(0, rows[i].logOn);
        } else if (rows[i].name == "it-sg4" || rows[i].name == "it-sg5") {
            sawDeniedOrLeft = true;
        }
    }
    EXPECT_EQ(4, seen);
    EXPECT_FALSE(sawDeniedOrLeft);
}

TEST_F(SharedGuildMySQL, GuildRowLifecycle) {
    SharedGuildRepository& repo = defaultSharedGuildRepository();

    SharedGuildRecord record;
    record.id = kGuildA;
    record.name = "it-sg guild";
    record.type = 1;
    record.race = 2;
    record.state = 1;
    record.serverGroupID = 3;
    record.zoneID = 30001;
    record.master = "it-sga";
    record.date = "2026-09-07";
    record.intro = "first intro";
    repo.insertGuild(record);
    seedGuild(kGuildB, "it-sg other", 0, 0, 10001, "it-sgk", "other intro");

    SharedGuildRow row;
    ASSERT_TRUE(repo.loadGuild(kGuildA, row));
    EXPECT_EQ("it-sg guild", row.name);
    EXPECT_EQ(1, row.type);
    EXPECT_EQ(2, row.race);
    EXPECT_EQ(1, row.state);
    EXPECT_EQ(3, row.serverGroupID);
    EXPECT_EQ(30001, row.zoneID);
    EXPECT_EQ("it-sga", row.master);
    EXPECT_EQ("2026-09-07", row.date);
    EXPECT_EQ("first intro", guildField("Intro", kGuildA));

    // A missing id answers false and leaves the row alone.
    row.name = "untouched";
    EXPECT_FALSE(repo.loadGuild(61799, row));
    EXPECT_EQ("untouched", row.name);

    // saveGuild writes every column but the intro.
    record.name = "it-sg renamed";
    record.state = 0;
    record.master = "it-sgb";
    record.intro = "not written";
    repo.saveGuild(record);
    EXPECT_EQ("it-sg renamed", guildField("GuildName", kGuildA));
    EXPECT_EQ("0", guildField("GuildState", kGuildA));
    EXPECT_EQ("it-sgb", guildField("Master", kGuildA));
    EXPECT_EQ("first intro", guildField("Intro", kGuildA));

    repo.saveGuildIntro("second intro", kGuildA);
    EXPECT_EQ("second intro", guildField("Intro", kGuildA));

    repo.updateGuildFields("Master='it-sgc'", kGuildA);
    EXPECT_EQ("it-sgc", guildField("Master", kGuildA));

    execSQL("INSERT INTO GuildUnionMember (UnionID, OwnerGuildID) VALUES (61790, 61701)");
    execSQL("INSERT INTO GuildUnionMember (UnionID, OwnerGuildID) VALUES (61790, 61702)");
    repo.deleteGuild(kGuildA);
    EXPECT_EQ("0", count("GuildInfo WHERE GuildID = 61701"));
    EXPECT_EQ("0", count("GuildUnionMember WHERE OwnerGuildID = 61701"));

    // The other guild is whole.
    EXPECT_EQ("it-sg other", guildField("GuildName", kGuildB));
    EXPECT_EQ("other intro", guildField("Intro", kGuildB));
    EXPECT_EQ("1", count("GuildUnionMember WHERE OwnerGuildID = 61702"));
}

TEST_F(SharedGuildMySQL, LoadGuildsInStatesFiltersByStateAndCarriesTheIntro) {
    SharedGuildRepository& repo = defaultSharedGuildRepository();

    seedGuild(61701, "it-sg active", 0, 0, 10001, "it-sga", "intro a");
    seedGuild(61702, "it-sg wait", 1, 1, 20001, "it-sgb", "intro b");
    seedGuild(61703, "it-sg cancel", 2, 2, 30001, "it-sgc", "intro c");
    seedGuild(61704, "it-sg broken", 0, 3, 10002, "it-sgd", "intro d");

    std::vector<SharedGuildListRow> rows = repo.loadGuildsInStates(1, 0);

    int seen = 0;
    bool sawOther = false;
    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i].id == 61701) {
            seen++;
            EXPECT_EQ("it-sg active", rows[i].name);
            EXPECT_EQ(1, rows[i].type);
            EXPECT_EQ(0, rows[i].race);
            EXPECT_EQ(0, rows[i].state);
            EXPECT_EQ(2, rows[i].serverGroupID);
            EXPECT_EQ(10001, rows[i].zoneID);
            EXPECT_EQ("it-sga", rows[i].master);
            EXPECT_EQ("2026-09-07", rows[i].date);
            EXPECT_EQ("intro a", rows[i].intro);
        } else if (rows[i].id == 61702) {
            seen++;
            EXPECT_EQ(1, rows[i].state);
            EXPECT_EQ("intro b", rows[i].intro);
        } else if (rows[i].id == 61703 || rows[i].id == 61704) {
            sawOther = true;
        }
    }
    EXPECT_EQ(2, seen);
    EXPECT_FALSE(sawOther);
}

TEST_F(SharedGuildMySQL, PurgeGuildRemovesItsRowsAndCancelsOnlyTheWarsItLeads) {
    SharedGuildRepository& repo = defaultSharedGuildRepository();

    seedGuild(61701, "it-sg a", 0, 0, 10001, "it-sga");
    seedGuild(61702, "it-sg b", 0, 0, 10002, "it-sgb");
    seedMember(61701, "it-sga", 1);
    seedMember(61701, "it-sga2", 0);
    seedMember(61702, "it-sgb", 1);
    execSQL("INSERT INTO GuildUnionMember (UnionID, OwnerGuildID) VALUES (61790, 61701)");
    execSQL("INSERT INTO GuildUnionMember (UnionID, OwnerGuildID) VALUES (61790, 61702)");
    execSQL("INSERT INTO WarScheduleInfo (WarID, AttackGuildID, Status) VALUES (61701, 61701, 'WAIT')");
    execSQL("INSERT INTO WarScheduleInfo (WarID, AttackGuildID, AttackGuildID2, Status) "
            "VALUES (61702, 61702, 61701, 'WAIT')");
    execSQL("INSERT INTO WarScheduleInfo (WarID, AttackGuildID, Status) VALUES (61703, 61701, 'END')");

    repo.purgeGuild(61701);

    EXPECT_EQ("0", count("GuildInfo WHERE GuildID = 61701"));
    EXPECT_EQ("0", count("GuildMember WHERE GuildID = 61701"));
    EXPECT_EQ("0", count("GuildUnionMember WHERE OwnerGuildID = 61701"));
    // Every war the guild leads is cancelled, whatever its status; a war it
    // joins as AttackGuildID2 is not.
    EXPECT_EQ("CANCEL", queryScalar("SELECT Status FROM WarScheduleInfo WHERE WarID = 61701"));
    EXPECT_EQ("CANCEL", queryScalar("SELECT Status FROM WarScheduleInfo WHERE WarID = 61703"));
    EXPECT_EQ("WAIT", queryScalar("SELECT Status FROM WarScheduleInfo WHERE WarID = 61702"));

    // The other guild is whole.
    EXPECT_EQ("1", count("GuildInfo WHERE GuildID = 61702"));
    EXPECT_EQ("1", count("GuildMember WHERE GuildID = 61702"));
    EXPECT_EQ("1", count("GuildUnionMember WHERE OwnerGuildID = 61702"));
}

TEST_F(SharedGuildMySQL, CountAndMaxProbes) {
    SharedGuildRepository& repo = defaultSharedGuildRepository();

    int guildsBefore = repo.countGuilds();
    int slayerBefore = repo.countGuildsOfRace(0);
    int vampireBefore = repo.countGuildsOfRace(1);

    seedGuild(61701, "it-sg a", 0, 0, 10005, "it-sga");
    seedGuild(61702, "it-sg b", 0, 0, 10003, "it-sgb");
    seedGuild(61703, "it-sg c", 1, 0, 20007, "it-sgc");

    EXPECT_EQ(guildsBefore + 3, repo.countGuilds());
    EXPECT_EQ(slayerBefore + 2, repo.countGuildsOfRace(0));
    EXPECT_EQ(vampireBefore + 1, repo.countGuildsOfRace(1));
    // The seeded ids and zone ids are above anything initdb/ or the other
    // fixtures leave behind.
    EXPECT_EQ(61703, repo.loadMaxGuildID());
    EXPECT_EQ(10005, repo.loadMaxGuildZoneIDOfRace(0));
    EXPECT_EQ(20007, repo.loadMaxGuildZoneIDOfRace(1));
}

TEST_F(SharedGuildMySQL, CharacterGuildIDAndGoldGoToTheRaceTableOfThatNameOnly) {
    SharedGuildRepository& repo = defaultSharedGuildRepository();

    execSQL("INSERT INTO Slayer (Name, PlayerID, GuildID, Gold) VALUES ('it-sgs', 'it-sgacct', 99, 100)");
    execSQL("INSERT INTO Slayer (Name, PlayerID, GuildID, Gold) VALUES ('it-sgs2', 'it-sgacct', 99, 100)");
    execSQL("INSERT INTO Vampire (Name, PlayerID, GuildID, Gold) VALUES ('it-sgv', 'it-sgacct', 0, 100)");
    execSQL("INSERT INTO Ousters (Name, PlayerID, GuildID, Gold) VALUES ('it-sgo', 'it-sgacct', 66, 100)");

    repo.setCharacterGuildID(0, 61701, "it-sgs");
    repo.setCharacterGuildID(1, 61702, "it-sgv");
    repo.setCharacterGuildID(2, 61703, "it-sgo");
    // Race 3 names no table: nothing happens, nothing throws.
    EXPECT_NO_THROW(repo.setCharacterGuildID(3, 61704, "it-sgs"));
    EXPECT_NO_THROW(repo.addCharacterGold(3, 50, "it-sgs"));

    EXPECT_EQ("61701", queryScalar("SELECT GuildID FROM Slayer WHERE Name = 'it-sgs'"));
    EXPECT_EQ("99", queryScalar("SELECT GuildID FROM Slayer WHERE Name = 'it-sgs2'"));
    EXPECT_EQ("61702", queryScalar("SELECT GuildID FROM Vampire WHERE Name = 'it-sgv'"));
    EXPECT_EQ("61703", queryScalar("SELECT GuildID FROM Ousters WHERE Name = 'it-sgo'"));

    repo.addCharacterGold(0, 50, "it-sgs");
    repo.addCharacterGold(1, 60, "it-sgv");
    repo.addCharacterGold(2, 70, "it-sgo");

    EXPECT_EQ("150", queryScalar("SELECT Gold FROM Slayer WHERE Name = 'it-sgs'"));
    EXPECT_EQ("100", queryScalar("SELECT Gold FROM Slayer WHERE Name = 'it-sgs2'"));
    EXPECT_EQ("160", queryScalar("SELECT Gold FROM Vampire WHERE Name = 'it-sgv'"));
    EXPECT_EQ("170", queryScalar("SELECT Gold FROM Ousters WHERE Name = 'it-sgo'"));
}

TEST_F(SharedGuildMySQL, InsertMessageWritesReceiverAndTextInEitherSpelling) {
    SharedGuildRepository& repo = defaultSharedGuildRepository();

    repo.insertMessage(SHARED_MESSAGE_SQL_COMPACT, "it-sga", "compact text");
    repo.insertMessage(SHARED_MESSAGE_SQL_SPACED, "it-sgb", "spaced text");
    EXPECT_NO_THROW(repo.insertMessage(SHARED_MESSAGE_SQL_SPELLING_MAX, "it-sgc", "never written"));

    EXPECT_EQ("compact text", queryScalar("SELECT Message FROM Messages WHERE Receiver = 'it-sga'"));
    EXPECT_EQ("spaced text", queryScalar("SELECT Message FROM Messages WHERE Receiver = 'it-sgb'"));
    EXPECT_EQ("0", count("Messages WHERE Receiver = 'it-sgc'"));
    // Sender is left at the column default.
    EXPECT_EQ("", queryScalar("SELECT Sender FROM Messages WHERE Receiver = 'it-sga'"));
}

class SharedConfigMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        execSQL("DELETE FROM GameServerGroupInfo WHERE WorldID BETWEEN 9170 AND 9179");
        execSQL("DELETE FROM GameServerInfo WHERE WorldID BETWEEN 9170 AND 9179");
        execSQL("DELETE FROM ZoneInfo WHERE ZoneID BETWEEN 9170 AND 9179");
        execSQL("DELETE FROM SSStringPool WHERE ID BETWEEN 61700 AND 61799");
    }
};

TEST_F(SharedConfigMySQL, GameServerGroupsAndTheirMaxWorldID) {
    SharedConfigRepository& repo = defaultSharedConfigRepository();

    size_t before = repo.loadGameServerGroups().size();

    execSQL("INSERT INTO GameServerGroupInfo (GroupID, WorldID, GroupName, Stat) VALUES (7, 9170, 'it-sg group', 1)");
    execSQL("INSERT INTO GameServerGroupInfo (GroupID, WorldID, GroupName, Stat) VALUES (8, 9171, 'it-sg other', 0)");

    std::vector<SharedGameServerGroupRow> rows = repo.loadGameServerGroups();
    EXPECT_EQ(before + 2, rows.size());
    int seen = 0;
    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i].worldID == 9170) {
            seen++;
            EXPECT_EQ(7, rows[i].groupID);
            EXPECT_EQ("it-sg group", rows[i].groupName);
        }
    }
    EXPECT_EQ(1, seen);

    // initdb/ seeds WorldID 0 only, so the seeded 9171 is the maximum.
    int maxWorldID = 0;
    EXPECT_TRUE(repo.loadMaxGameServerGroupWorldID(maxWorldID));
    EXPECT_EQ(9171, maxWorldID);
}

TEST_F(SharedConfigMySQL, GameServersWithEveryColumnAndAPerWorldMaxGroupID) {
    SharedConfigRepository& repo = defaultSharedConfigRepository();

    size_t before = repo.loadGameServers().size();

    execSQL("INSERT INTO GameServerInfo (ServerID, Nickname, IP, TCPPort, UDPPort, WorldID, GroupID, Stat) "
            "VALUES (61701, 'it-sg game1', '10.0.0.1', 33064, 9997, 9170, 5, 1)");
    execSQL("INSERT INTO GameServerInfo (ServerID, Nickname, IP, TCPPort, UDPPort, WorldID, GroupID, Stat) "
            "VALUES (61702, 'it-sg game2', '10.0.0.2', 33065, 9998, 9170, 9, 0)");

    std::vector<SharedGameServerRow> rows = repo.loadGameServers();
    EXPECT_EQ(before + 2, rows.size());
    int seen = 0;
    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i].serverID == 61701) {
            seen++;
            EXPECT_EQ("it-sg game1", rows[i].nickname);
            EXPECT_EQ("10.0.0.1", rows[i].ip);
            EXPECT_EQ(33064, rows[i].tcpPort);
            EXPECT_EQ(9997, rows[i].udpPort);
            EXPECT_EQ(5, rows[i].groupID);
            EXPECT_EQ(1, rows[i].stat);
            EXPECT_EQ(9170, rows[i].worldID);
        }
    }
    EXPECT_EQ(1, seen);

    int maxGroupID = 0;
    EXPECT_TRUE(repo.loadMaxGameServerGroupID(9170, maxGroupID));
    EXPECT_EQ(9, maxGroupID);

    // A world with no servers: MAX() is one NULL row, reported as false
    // with the out-parameter untouched.
    maxGroupID = -1;
    EXPECT_FALSE(repo.loadMaxGameServerGroupID(9171, maxGroupID));
    EXPECT_EQ(-1, maxGroupID);
}

TEST_F(SharedConfigMySQL, ResurrectLocationsReadTheSlayerAndVampireColumns) {
    SharedConfigRepository& repo = defaultSharedConfigRepository();

    size_t before = repo.loadResurrectLocations().size();

    execSQL("INSERT INTO ZoneInfo (ZoneID, SResurrectZoneID, SResurrectX, SResurrectY, VResurrectZoneID, "
            "VResurrectX, VResurrectY, OResurrectZoneID, OResurrectX, OResurrectY) "
            "VALUES (9170, 1001, 11, 12, 2002, 21, 22, 3003, 31, 32)");

    std::vector<SharedResurrectLocationRow> rows = repo.loadResurrectLocations();
    EXPECT_EQ(before + 1, rows.size());
    int seen = 0;
    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i].zoneID == 9170) {
            seen++;
            EXPECT_EQ(1001, rows[i].slayerZoneID);
            EXPECT_EQ(11, rows[i].slayerX);
            EXPECT_EQ(12, rows[i].slayerY);
            EXPECT_EQ(2002, rows[i].vampireZoneID);
            EXPECT_EQ(21, rows[i].vampireX);
            EXPECT_EQ(22, rows[i].vampireY);
        }
    }
    EXPECT_EQ(1, seen);
}

TEST_F(SharedConfigMySQL, StringsRoundTrip) {
    SharedConfigRepository& repo = defaultSharedConfigRepository();

    size_t before = repo.loadStrings().size();

    execSQL("INSERT INTO SSStringPool (ID, String) VALUES (61701, 'it-sg string one')");
    execSQL("INSERT INTO SSStringPool (ID, String) VALUES (61702, 'it-sg string two')");

    std::vector<SharedStringRow> rows = repo.loadStrings();
    EXPECT_EQ(before + 2, rows.size());
    int seen = 0;
    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i].id == 61701) {
            seen++;
            EXPECT_EQ("it-sg string one", rows[i].text);
        }
    }
    EXPECT_EQ(1, seen);
}

} // namespace
