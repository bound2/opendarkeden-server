// MySQL-backed integration tier for the loginserver's account, character
// and config repositories: the real MySQLLogin*Repository impls against the
// throwaway MySQL 5.7 loaded with initdb/, on the connections
// mysql_repository_test.cpp's main() wires for this binary. The character
// repository asks for getConnection(worldID); main() registers no per-world
// connection, so every id reaches the world-default connection, the same
// server and schema as the DARKEDEN connection the seeding below uses.
//
// Seeded names and ids carry the "it-la" prefix; numeric keys use 9170.

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "DB.h"
#include "repository/LoginAccountRepository.h"
#include "repository/LoginCharacterRepository.h"
#include "repository/LoginConfigRepository.h"

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

// The USERINFO database, where LoginPlayerData lives.
void execUserInfoSQL(const std::string& sql) {
    Statement* pStmt = NULL;
    BEGIN_DB {
        pStmt = g_pDatabaseManager->getUserInfoConnection()->createStatement();
        pStmt->executeQueryString(sql);
        SAFE_DELETE(pStmt);
    }
    END_DB(pStmt)
}

std::string queryUserInfoScalar(const std::string& sql) {
    std::string value;
    Statement* pStmt = NULL;
    BEGIN_DB {
        pStmt = g_pDatabaseManager->getUserInfoConnection()->createStatement();
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

const WorldID_t kWorld = 1;

class LoginAccountMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        execSQL("DELETE FROM Player WHERE PlayerID LIKE 'it-la%'");
        execSQL("DELETE FROM TestClientUser WHERE PlayerID LIKE 'it-la%'");
        execSQL("DELETE FROM Event200501Main WHERE PlayerID LIKE 'it-la%'");
        execSQL("DELETE FROM WebLogin WHERE PlayerID LIKE 'it-la%'");
        execSQL("DELETE FROM PCRoomUserInfo WHERE PlayerID LIKE 'it-la%'");
        execSQL("DELETE FROM IPBlockInfo WHERE IP LIKE '91.70%'");
        execUserInfoSQL("DELETE FROM LoginPlayerData WHERE PlayerID LIKE 'it-la%'");
    }

    static void seedAccount(const std::string& id, const std::string& logOn = "LOGOFF",
                            const std::string& access = "ALLOW") {
        execSQL("INSERT INTO Player (PlayerID, Password, Name, SSN, ZipCode, CurrentWorldID, CurrentServerGroupID, "
                "LogOn, Access, LoginIP, LastSlot, CurrentLoginServerID, PayType, PayPlayDate, PayPlayHours, "
                "PayPlayFlag, FamilyPayPlayDate) VALUES (" +
                q(id) + ", 'hash-" + id + "', 'name-" + id + "', '800101-1234567', '123-456', 3, 4, " + q(logOn) +
                ", " + q(access) + ", '10.0.0.9', 2, 7, 5, '2026-01-02 03:04:05', 11, 13, '2026-06-07 08:09:10')");
    }

    static std::string field(const char* column, const std::string& id) {
        return queryScalar(std::string("SELECT ") + column + " FROM Player WHERE PlayerID = " + q(id));
    }
};

TEST_F(LoginAccountMySQL, TheThreeAccountProjections) {
    LoginAccountRepository& repo = defaultLoginAccountRepository();
    seedAccount("it-la-a");

    LoginAccountRow row;
    ASSERT_TRUE(repo.loadAccount("it-la-a", row));
    EXPECT_EQ("it-la-a", row.playerID);
    EXPECT_EQ("800101-1234567", row.ssn);
    EXPECT_EQ(4, row.currentServerGroupID);
    EXPECT_EQ("LOGOFF", row.logOn);
    EXPECT_EQ("ALLOW", row.access);
    EXPECT_EQ("123-456", row.zipCode);
    EXPECT_EQ("10.0.0.9", row.loginIP);
    EXPECT_EQ(5, row.payType);
    EXPECT_EQ("2026-01-02 03:04:05", row.payPlayDate);
    EXPECT_EQ(11, row.payPlayHours);
    EXPECT_EQ(13, row.payPlayFlag);
    EXPECT_EQ("2026-06-07 08:09:10", row.familyPayPlayDate);

    // The web-login projection leaves ZipCode out; the free-pass one SSN
    // too.
    LoginAccountRow web;
    ASSERT_TRUE(repo.loadAccountForWebLogin("it-la-a", web));
    EXPECT_EQ("800101-1234567", web.ssn);
    EXPECT_EQ("", web.zipCode);
    EXPECT_EQ("10.0.0.9", web.loginIP);
    EXPECT_EQ("2026-06-07 08:09:10", web.familyPayPlayDate);

    LoginAccountRow free;
    ASSERT_TRUE(repo.loadAccountForFreePass("it-la-a", free));
    EXPECT_EQ("", free.ssn);
    EXPECT_EQ("", free.zipCode);
    EXPECT_EQ("it-la-a", free.playerID);
    EXPECT_EQ(4, free.currentServerGroupID);
    EXPECT_EQ(13, free.payPlayFlag);

    // A missing account answers false and leaves the row alone.
    row.playerID = "untouched";
    EXPECT_FALSE(repo.loadAccount("it-la-none", row));
    EXPECT_FALSE(repo.loadAccountForWebLogin("it-la-none", row));
    EXPECT_FALSE(repo.loadAccountForFreePass("it-la-none", row));
    EXPECT_EQ("untouched", row.playerID);
}

TEST_F(LoginAccountMySQL, PasswordHashRoundTrip) {
    LoginAccountRepository& repo = defaultLoginAccountRepository();
    seedAccount("it-la-a");
    seedAccount("it-la-k");

    std::string stored = "untouched";
    ASSERT_TRUE(repo.loadPasswordHash("it-la-a", stored));
    EXPECT_EQ("hash-it-la-a", stored);

    repo.updatePassword("$argon2id$new", "it-la-a");
    EXPECT_EQ("$argon2id$new", field("Password", "it-la-a"));
    EXPECT_EQ("hash-it-la-k", field("Password", "it-la-k"));

    stored = "untouched";
    EXPECT_FALSE(repo.loadPasswordHash("it-la-none", stored));
    EXPECT_EQ("untouched", stored);
}

TEST_F(LoginAccountMySQL, LogOnTransitionsAnswerWhetherARowChanged) {
    LoginAccountRepository& repo = defaultLoginAccountRepository();
    seedAccount("it-la-a", "LOGOFF");
    seedAccount("it-la-k", "LOGOFF");

    // The login: only a LOGOFF row flips.
    EXPECT_TRUE(repo.markLoggedOn("10.1.1.1", 9, "it-la-a"));
    EXPECT_EQ("LOGON", field("LogOn", "it-la-a"));
    EXPECT_EQ("10.1.1.1", field("LoginIP", "it-la-a"));
    EXPECT_EQ("9", field("CurrentLoginServerID", "it-la-a"));
    EXPECT_EQ("1", queryScalar("SELECT LastLoginDate = CURDATE() FROM Player WHERE PlayerID = 'it-la-a'"));
    EXPECT_FALSE(repo.markLoggedOn("10.1.1.2", 9, "it-la-a"));
    EXPECT_EQ("10.1.1.1", field("LoginIP", "it-la-a"));
    EXPECT_EQ("LOGOFF", field("LogOn", "it-la-k"));

    // The logout: only a LOGON row flips.
    repo.markLoggedOff("it-la-a");
    EXPECT_EQ("LOGOFF", field("LogOn", "it-la-a"));

    // setLoggedOn flips whatever the row holds, and answers false only
    // when the row was LOGON already.
    EXPECT_TRUE(repo.setLoggedOn("it-la-a"));
    EXPECT_FALSE(repo.setLoggedOn("it-la-a"));
    EXPECT_EQ("LOGON", field("LogOn", "it-la-a"));

    repo.setLoginIP("10.2.2.2", "it-la-a");
    EXPECT_EQ("10.2.2.2", field("LoginIP", "it-la-a"));
    EXPECT_EQ("10.0.0.9", field("LoginIP", "it-la-k"));

    // The reconnect variant flips a LOGOFF row and sets the server id.
    repo.markLoggedOff("it-la-a");
    EXPECT_TRUE(repo.markLoggedOnForReconnect(12, "it-la-a"));
    EXPECT_EQ("12", field("CurrentLoginServerID", "it-la-a"));
    EXPECT_FALSE(repo.markLoggedOnForReconnect(13, "it-la-a"));
    EXPECT_EQ("12", field("CurrentLoginServerID", "it-la-a"));

    // The registration variant does not care what the row holds.
    repo.markLoggedOnAfterRegister("10.3.3.3", 14, "it-la-a");
    EXPECT_EQ("LOGON", field("LogOn", "it-la-a"));
    EXPECT_EQ("10.3.3.3", field("LoginIP", "it-la-a"));
    EXPECT_EQ("14", field("CurrentLoginServerID", "it-la-a"));
}

TEST_F(LoginAccountMySQL, ComebackEventGrantsOneWeekOnce) {
    LoginAccountRepository& repo = defaultLoginAccountRepository();
    seedAccount("it-la-a");
    seedAccount("it-la-k");
    execSQL("INSERT INTO Event200501Main (PlayerID) VALUES ('it-la-a')");
    execSQL("INSERT INTO Event200501Main (PlayerID, RecvPremiumDate) VALUES ('it-la-k', '2026-01-01')");

    EXPECT_TRUE(repo.hasUnclaimedPremiumEvent("it-la-a"));
    EXPECT_FALSE(repo.hasUnclaimedPremiumEvent("it-la-k"));
    EXPECT_FALSE(repo.hasUnclaimedPremiumEvent("it-la-none"));

    // A pay-play date in the past moves to a week from now; one in the
    // future moves a week further.
    repo.extendPayPlayByWeek("it-la-a");
    EXPECT_EQ("7", queryScalar("SELECT DATEDIFF(PayPlayDate, NOW()) FROM Player WHERE PlayerID = 'it-la-a'"));
    execSQL("UPDATE Player SET PayPlayDate = NOW() + INTERVAL 30 DAY WHERE PlayerID = 'it-la-k'");
    repo.extendPayPlayByWeek("it-la-k");
    EXPECT_EQ("37", queryScalar("SELECT DATEDIFF(PayPlayDate, NOW()) FROM Player WHERE PlayerID = 'it-la-k'"));

    repo.markPremiumEventReceived("it-la-a");
    EXPECT_EQ("1", queryScalar("SELECT RecvPremiumDate = CURDATE() FROM Event200501Main WHERE PlayerID = 'it-la-a'"));
    EXPECT_FALSE(repo.hasUnclaimedPremiumEvent("it-la-a"));
}

TEST_F(LoginAccountMySQL, PrivateAgreementTableIsNotInTheSchema) {
    // PrivateAgreementRemain is not created by initdb/, so the probe is a
    // SQL error crossing as END_DB's DatabaseError. This pins that it throws,
    // not the message.
    EXPECT_ANY_THROW(defaultLoginAccountRepository().hasPrivateAgreementRemaining("it-la-a"));
}

TEST_F(LoginAccountMySQL, NetMarbleAccountInsertIgnoreKeepsTheFirstRow) {
    LoginAccountRepository& repo = defaultLoginAccountRepository();

    repo.insertNetMarbleAccount("it-la-nm", "$argon2id$nm");

    EXPECT_EQ("$argon2id$nm", field("Password", "it-la-nm"));
    EXPECT_EQ("it-la-nm", field("Name", "it-la-nm"));
    EXPECT_EQ("123456-1122339", field("SSN", "it-la-nm"));
    EXPECT_EQ("2", field("SpecialEventCount", "it-la-nm"));
    EXPECT_EQ("0", field("Event", "it-la-nm"));
    EXPECT_EQ("1", queryScalar("SELECT creation_date = CURDATE() FROM Player WHERE PlayerID = 'it-la-nm'"));

    // PlayerID is the primary key, so a second INSERT IGNORE for the same
    // id is dropped and the stored password stays.
    repo.insertNetMarbleAccount("it-la-nm", "$argon2id$nm2");
    EXPECT_EQ("1", queryScalar("SELECT COUNT(*) FROM Player WHERE PlayerID = 'it-la-nm'"));
    EXPECT_EQ("$argon2id$nm", field("Password", "it-la-nm"));
}

TEST_F(LoginAccountMySQL, IPBlocksMatchByPrefixAndClass) {
    LoginAccountRepository& repo = defaultLoginAccountRepository();
    execSQL("INSERT INTO IPBlockInfo (IP, first, last, class) VALUES ('91.70', '10', '20', 2)");
    execSQL("INSERT INTO IPBlockInfo (IP, first, last, class) VALUES ('91.70.1', '0', '255', 0)");
    execSQL("INSERT INTO IPBlockInfo (IP, first, last, class) VALUES ('91.70', '30', '40', 1)");

    // classA '91', classB '91.70', classC '91.70.1': the class-2 row on
    // the class-B prefix, the class-0 row on the class-C prefix; the
    // class-1 row on '91.70' matches through the bare classC clause only
    // when classC is '91.70', so not here.
    std::vector<LoginIPBlockRow> rows = repo.loadIPBlocks("91", "91.70", "91.70.1");
    ASSERT_EQ(2u, rows.size());
    int classes = 0;
    for (size_t i = 0; i < rows.size(); i++) {
        classes += 1 << rows[i].ipClass;
        if (rows[i].ipClass == 2) {
            EXPECT_EQ(10, rows[i].first);
            EXPECT_EQ(20, rows[i].last);
        } else {
            EXPECT_EQ(0, rows[i].first);
            EXPECT_EQ(255, rows[i].last);
        }
    }
    EXPECT_EQ((1 << 2) | (1 << 0), classes);

    EXPECT_EQ(0u, repo.loadIPBlocks("91", "91.71", "91.71.1").size());
}

TEST_F(LoginAccountMySQL, WebLoginKeyLoadAndDelete) {
    LoginAccountRepository& repo = defaultLoginAccountRepository();
    execSQL("INSERT INTO WebLogin (PlayerID, LoginKey, CreateTime) VALUES ('it-la-a', 'key-a', '2026-09-07 10:00:00')");
    execSQL("INSERT INTO WebLogin (PlayerID, LoginKey, CreateTime) VALUES ('it-la-k', 'key-k', '2026-09-07 11:00:00')");

    std::string key = "untouched";
    std::string created = "untouched";
    std::string now = "untouched";
    ASSERT_TRUE(repo.loadWebLoginKey("it-la-a", key, created, now));
    EXPECT_EQ("key-a", key);
    EXPECT_EQ("2026-09-07 10:00:00", created);
    // now() is the database clock as text.
    EXPECT_EQ(19u, now.size());

    key = "untouched";
    EXPECT_FALSE(repo.loadWebLoginKey("it-la-none", key, created, now));
    EXPECT_EQ("untouched", key);

    repo.deleteWebLoginKey("it-la-a");
    EXPECT_EQ("0", queryScalar("SELECT COUNT(*) FROM WebLogin WHERE PlayerID = 'it-la-a'"));
    EXPECT_EQ("1", queryScalar("SELECT COUNT(*) FROM WebLogin WHERE PlayerID = 'it-la-k'"));
}

TEST_F(LoginAccountMySQL, LoginRecordGoesToUserInfo) {
    defaultLoginAccountRepository().insertLoginRecord("it-la-a", "10.4.4.4", "2026-09-07", "12:34:56");
    defaultLoginAccountRepository().insertTestClientUser("it-la-t", "10.5.5.5");

    EXPECT_EQ("10.4.4.4", queryUserInfoScalar("SELECT IP FROM LoginPlayerData WHERE PlayerID = 'it-la-a'"));
    EXPECT_EQ("2026-09-07", queryUserInfoScalar("SELECT Date FROM LoginPlayerData WHERE PlayerID = 'it-la-a'"));
    EXPECT_EQ("12:34:56", queryUserInfoScalar("SELECT Time FROM LoginPlayerData WHERE PlayerID = 'it-la-a'"));

    EXPECT_EQ("10.5.5.5", queryScalar("SELECT IP FROM TestClientUser WHERE PlayerID = 'it-la-t'"));
    EXPECT_EQ("1", queryScalar("SELECT TIMESTAMPDIFF(SECOND, LoginDate, NOW()) BETWEEN 0 AND 60 FROM TestClientUser "
                               "WHERE PlayerID = 'it-la-t'"));
}

TEST_F(LoginAccountMySQL, CurrentWorldGroupAndSlot) {
    LoginAccountRepository& repo = defaultLoginAccountRepository();
    seedAccount("it-la-a");
    seedAccount("it-la-k");

    int worldID = -1;
    int groupID = -1;
    int slot = -1;
    ASSERT_TRUE(repo.loadLastLocation("it-la-a", worldID, groupID, slot));
    EXPECT_EQ(3, worldID);
    EXPECT_EQ(4, groupID);
    EXPECT_EQ(2, slot);

    repo.setCurrentServerGroup(6, "it-la-a");
    EXPECT_EQ("6", field("CurrentServerGroupID", "it-la-a"));
    EXPECT_EQ("4", field("CurrentServerGroupID", "it-la-k"));

    repo.setCurrentLocation(8, 9, 1, "it-la-a");
    EXPECT_EQ("8", field("CurrentWorldID", "it-la-a"));
    EXPECT_EQ("9", field("CurrentServerGroupID", "it-la-a"));
    EXPECT_EQ("1", field("LastSlot", "it-la-a"));

    worldID = -1;
    groupID = -1;
    ASSERT_TRUE(repo.loadCurrentLocation(LOGIN_LOCATION_SQL_LOWER, "it-la-a", worldID, groupID));
    EXPECT_EQ(8, worldID);
    EXPECT_EQ(9, groupID);
    worldID = -1;
    groupID = -1;
    ASSERT_TRUE(repo.loadCurrentLocation(LOGIN_LOCATION_SQL_UPPER, "it-la-k", worldID, groupID));
    EXPECT_EQ(3, worldID);
    EXPECT_EQ(4, groupID);
    EXPECT_FALSE(repo.loadCurrentLocation(LOGIN_LOCATION_SQL_SPELLING_MAX, "it-la-a", worldID, groupID));

    worldID = -1;
    ASSERT_TRUE(repo.loadCurrentWorld("it-la-a", worldID));
    EXPECT_EQ(8, worldID);
    groupID = -1;
    ASSERT_TRUE(repo.loadCurrentServerGroup("it-la-a", groupID));
    EXPECT_EQ(9, groupID);

    EXPECT_FALSE(repo.loadLastLocation("it-la-none", worldID, groupID, slot));
    EXPECT_FALSE(repo.loadCurrentWorld("it-la-none", worldID));
    EXPECT_FALSE(repo.loadCurrentServerGroup("it-la-none", groupID));
}

TEST_F(LoginAccountMySQL, ReconnectProjection) {
    LoginAccountRepository& repo = defaultLoginAccountRepository();
    seedAccount("it-la-a", "LOGOFF", "DENY");

    LoginReconnectRow row;
    ASSERT_TRUE(repo.loadAccountForReconnect("it-la-a", row));
    EXPECT_EQ(3, row.currentWorldID);
    EXPECT_EQ(4, row.currentServerGroupID);
    EXPECT_EQ("LOGOFF", row.logOn);
    EXPECT_EQ("DENY", row.access);
    EXPECT_EQ(5, row.payType);
    EXPECT_EQ("2026-01-02 03:04:05", row.payPlayDate);
    EXPECT_EQ(11, row.payPlayHours);
    EXPECT_EQ(13, row.payPlayFlag);

    row.logOn = "untouched";
    EXPECT_FALSE(repo.loadAccountForReconnect("it-la-none", row));
    EXPECT_EQ("untouched", row.logOn);
}

TEST_F(LoginAccountMySQL, StartupCleanupLogsOffThisServersAccountsOnly) {
    LoginAccountRepository& repo = defaultLoginAccountRepository();
    seedAccount("it-la-a", "LOGON");
    seedAccount("it-la-b", "LOGON");
    seedAccount("it-la-c", "LOGOFF");
    execSQL("UPDATE Player SET CurrentLoginServerID = 91 WHERE PlayerID IN ('it-la-a', 'it-la-c')");
    execSQL("UPDATE Player SET CurrentLoginServerID = 92 WHERE PlayerID = 'it-la-b'");
    execSQL("INSERT INTO PCRoomUserInfo (ID, PlayerID) VALUES (9170, 'it-la-a')");
    execSQL("INSERT INTO PCRoomUserInfo (ID, PlayerID) VALUES (9170, 'it-la-b')");

    std::vector<std::string> ids = repo.loadLoggedOnAccounts(91);
    ASSERT_EQ(1u, ids.size());
    EXPECT_EQ("it-la-a", ids[0]);

    repo.deletePCRoomUser("it-la-a");
    EXPECT_EQ("0", queryScalar("SELECT COUNT(*) FROM PCRoomUserInfo WHERE PlayerID = 'it-la-a'"));
    EXPECT_EQ("1", queryScalar("SELECT COUNT(*) FROM PCRoomUserInfo WHERE PlayerID = 'it-la-b'"));

    repo.logOffAllOnServer(91);
    EXPECT_EQ("LOGOFF", field("LogOn", "it-la-a"));
    EXPECT_EQ("LOGON", field("LogOn", "it-la-b"));
    EXPECT_EQ("LOGOFF", field("LogOn", "it-la-c"));
}

TEST_F(LoginAccountMySQL, RegistrationInsertsEveryColumn) {
    LoginAccountRepository& repo = defaultLoginAccountRepository();

    EXPECT_FALSE(repo.accountExists("it-la-r"));
    EXPECT_FALSE(repo.accountNameExists("it-la-r"));

    LoginNewAccount a;
    a.playerID = "it-la-r";
    a.password = "$argon2id$r";
    a.name = "Registered";
    a.sex = "FEMALE";
    a.ssn = "900101-2345678";
    a.telephone = "02-123-4567";
    a.cellular = "010-1234-5678";
    a.zipCode = "135-090";
    a.address = "Somewhere 1";
    a.nation = 82;
    a.email = "r@example.com";
    a.homepage = "example.com";
    a.profile = "hello";
    a.pub = "PUBLIC";
    repo.insertAccount(a);

    EXPECT_TRUE(repo.accountExists("it-la-r"));
    EXPECT_TRUE(repo.accountNameExists("it-la-r"));
    EXPECT_EQ("$argon2id$r", field("Password", "it-la-r"));
    EXPECT_EQ("Registered", field("Name", "it-la-r"));
    EXPECT_EQ("FEMALE", field("Sex", "it-la-r"));
    EXPECT_EQ("900101-2345678", field("SSN", "it-la-r"));
    EXPECT_EQ("02-123-4567", field("Telephone", "it-la-r"));
    EXPECT_EQ("010-1234-5678", field("Cellular", "it-la-r"));
    EXPECT_EQ("135-090", field("ZipCode", "it-la-r"));
    EXPECT_EQ("Somewhere 1", field("Address", "it-la-r"));
    EXPECT_EQ("82", field("Nation", "it-la-r"));
    EXPECT_EQ("r@example.com", field("Email", "it-la-r"));
    EXPECT_EQ("example.com", field("Homepage", "it-la-r"));
    EXPECT_EQ("hello", field("Profile", "it-la-r"));
    EXPECT_EQ("PUBLIC", field("Pub", "it-la-r"));
    // The columns the statement does not name keep their defaults.
    EXPECT_EQ("1", field("CurrentWorldID", "it-la-r"));
    EXPECT_EQ("LOGOFF", field("LogOn", "it-la-r"));
}

class LoginCharacterMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        execSQL("DELETE FROM Slayer WHERE Name LIKE 'it-la%'");
        execSQL("DELETE FROM Vampire WHERE Name LIKE 'it-la%'");
        execSQL("DELETE FROM Ousters WHERE Name LIKE 'it-la%'");
        execSQL("DELETE FROM FlagSet WHERE OwnerID LIKE 'it-la%'");
        execSQL("DELETE FROM STRBalanceInfo WHERE Level = 9170");
        execSQL("DELETE FROM DEXBalanceInfo WHERE Level = 9170");
        execSQL("DELETE FROM INTBalanceInfo WHERE Level = 9170");
    }

    static void seedSlayer(const std::string& name, const std::string& playerID, const std::string& slot,
                           const std::string& active = "ACTIVE", const std::string& race = "SLAYER") {
        execSQL("INSERT INTO Slayer (Race, Name, PlayerID, Slot, Active, Sex) VALUES (" + q(race) + ", " + q(name) +
                ", " + q(playerID) + ", " + q(slot) + ", " + q(active) + ", 'MALE')");
    }

    static std::string slayerField(const char* column, const std::string& name) {
        return queryScalar(std::string("SELECT ") + column + " FROM Slayer WHERE Name = " + q(name));
    }
    static std::string vampireField(const char* column, const std::string& name) {
        return queryScalar(std::string("SELECT ") + column + " FROM Vampire WHERE Name = " + q(name));
    }
    static std::string oustersField(const char* column, const std::string& name) {
        return queryScalar(std::string("SELECT ") + column + " FROM Ousters WHERE Name = " + q(name));
    }
};

TEST_F(LoginCharacterMySQL, NameAndSlotProbes) {
    LoginCharacterRepository& repo = defaultLoginCharacterRepository();
    seedSlayer("it-la-s1", "it-laacct", "SLOT1");
    seedSlayer("it-la-s2", "it-laacct", "SLOT2", "INACTIVE");

    // The name probe sees INACTIVE rows; the slot probe does not.
    EXPECT_TRUE(repo.slayerNameExists(kWorld, "it-la-s1"));
    EXPECT_TRUE(repo.slayerNameExists(kWorld, "it-la-s2"));
    EXPECT_FALSE(repo.slayerNameExists(kWorld, "it-la-none"));

    EXPECT_TRUE(repo.slotOccupied(kWorld, "it-laacct", "SLOT1"));
    EXPECT_FALSE(repo.slotOccupied(kWorld, "it-laacct", "SLOT2"));
    EXPECT_FALSE(repo.slotOccupied(kWorld, "it-laacct", "SLOT3"));
    EXPECT_FALSE(repo.slotOccupied(kWorld, "it-laother", "SLOT1"));

    // The slot name read sees INACTIVE rows.
    std::string name = "untouched";
    ASSERT_TRUE(repo.loadSlayerNameInSlot(kWorld, "it-laacct", 2, name));
    EXPECT_EQ("it-la-s2", name);
    name = "untouched";
    EXPECT_FALSE(repo.loadSlayerNameInSlot(kWorld, "it-laacct", 3, name));
    EXPECT_EQ("untouched", name);
}

TEST_F(LoginCharacterMySQL, BalanceProbesReadTheSeededTablesAndAnswerFalseForMissingRows) {
    LoginCharacterRepository& repo = defaultLoginCharacterRepository();

    // initdb/ ships the level-1 rows; the answers match a direct read.
    int value = -1;
    for (int rankType = 0; rankType < 3; rankType++) {
        value = -1;
        ASSERT_TRUE(repo.loadRankGoalExp(kWorld, rankType, value));
        EXPECT_EQ(queryScalar("SELECT GoalExp FROM RankEXPInfo WHERE Level=1 AND RankType=" + std::to_string(rankType)),
                  std::to_string(value));
    }
    value = -1;
    ASSERT_TRUE(repo.loadVampireGoalExp(kWorld, value));
    EXPECT_EQ(queryScalar("SELECT GoalExp FROM VampEXPBalanceInfo WHERE Level=1"), std::to_string(value));
    value = -1;
    ASSERT_TRUE(repo.loadOustersGoalExp(kWorld, value));
    EXPECT_EQ(queryScalar("SELECT GoalExp FROM OustersEXPBalanceInfo WHERE Level=1"), std::to_string(value));
    value = -1;
    EXPECT_FALSE(repo.loadRankGoalExp(kWorld, 9, value));
    EXPECT_EQ(-1, value);

    execSQL("INSERT INTO STRBalanceInfo (Level, GoalExp, AccumExp) VALUES (9170, 11, 12)");
    execSQL("INSERT INTO DEXBalanceInfo (Level, GoalExp, AccumExp) VALUES (9170, 21, 22)");
    execSQL("INSERT INTO INTBalanceInfo (Level, GoalExp, AccumExp) VALUES (9170, 31, 32)");

    ASSERT_TRUE(repo.loadAttrGoalExp(kWorld, LOGIN_ATTR_TABLE_STR, 9170, value));
    EXPECT_EQ(11, value);
    ASSERT_TRUE(repo.loadAttrAccumExp(kWorld, LOGIN_ATTR_TABLE_STR, 9170, value));
    EXPECT_EQ(12, value);
    ASSERT_TRUE(repo.loadAttrGoalExp(kWorld, LOGIN_ATTR_TABLE_DEX, 9170, value));
    EXPECT_EQ(21, value);
    ASSERT_TRUE(repo.loadAttrAccumExp(kWorld, LOGIN_ATTR_TABLE_DEX, 9170, value));
    EXPECT_EQ(22, value);
    ASSERT_TRUE(repo.loadAttrGoalExp(kWorld, LOGIN_ATTR_TABLE_INT, 9170, value));
    EXPECT_EQ(31, value);
    ASSERT_TRUE(repo.loadAttrAccumExp(kWorld, LOGIN_ATTR_TABLE_INT, 9170, value));
    EXPECT_EQ(32, value);

    value = -1;
    EXPECT_FALSE(repo.loadAttrGoalExp(kWorld, LOGIN_ATTR_TABLE_STR, 9171, value));
    EXPECT_FALSE(repo.loadAttrAccumExp(kWorld, LOGIN_ATTR_TABLE_MAX, 9170, value));
    EXPECT_EQ(-1, value);
}

TEST_F(LoginCharacterMySQL, CreationInsertsAndTheCharacterListReadsThemBack) {
    LoginCharacterRepository& repo = defaultLoginCharacterRepository();

    LoginNewSlayer s;
    s.race = "SLAYER";
    s.name = "it-la-s";
    s.playerID = "it-laacct";
    s.slot = "SLOT1";
    s.serverGroupID = 2;
    s.sex = "FEMALE";
    s.hairStyle = "HAIR_STYLE2";
    s.hairColor = 3;
    s.skinColor = 4;
    s.str = 10;
    s.strExp = 11;
    s.strGoalExp = 12;
    s.dex = 13;
    s.dexExp = 14;
    s.dexGoalExp = 15;
    s.inte = 7;
    s.intExp = 17;
    s.intGoalExp = 18;
    s.rank = 1;
    s.rankExp = 0;
    s.rankGoalExp = 19;
    s.hp = 20;
    s.currentHP = 20;
    s.mp = 14;
    s.currentMP = 14;
    s.shape = 3;
    s.helmetColor = 21;
    s.jacketColor = 22;
    s.pantsColor = 23;
    s.weaponColor = 24;
    s.shieldColor = 25;
    repo.insertSlayer(kWorld, s);

    // A Vampire character: a Slayer index row plus the Vampire row.
    LoginNewSlayer vs = s;
    vs.race = "VAMPIRE";
    vs.name = "it-la-v";
    vs.slot = "SLOT2";
    repo.insertSlayer(kWorld, vs);

    LoginNewVampire v;
    v.name = "it-la-v";
    v.playerID = "it-laacct";
    v.slot = "SLOT2";
    v.serverGroupID = 2;
    v.sex = "MALE";
    v.skinColor = 5;
    v.goalExp = 250;
    v.rankGoalExp = 26;
    v.shape = 1;
    repo.insertVampire(kWorld, v);

    LoginNewSlayer os = s;
    os.race = "OUSTERS";
    os.name = "it-la-o";
    os.slot = "SLOT3";
    repo.insertSlayer(kWorld, os);

    LoginNewOusters o;
    o.name = "it-la-o";
    o.playerID = "it-laacct";
    o.slot = "SLOT3";
    o.serverGroupID = 2;
    o.str = 15;
    o.dex = 15;
    o.inte = 15;
    o.goalExp = 300;
    o.rankGoalExp = 27;
    o.hairColor = 6;
    repo.insertOusters(kWorld, o);

    // The fixed columns of each statement.
    EXPECT_EQ("2101", slayerField("ZoneID", "it-la-s"));
    EXPECT_EQ("0", slayerField("Gold", "it-la-s"));
    EXPECT_EQ("1", queryScalar("SELECT creation_date = CURDATE() FROM Slayer WHERE Name = 'it-la-s'"));
    EXPECT_EQ("HAIR_STYLE2", slayerField("HairStyle", "it-la-s"));
    EXPECT_EQ("1003", vampireField("ZoneID", "it-la-v"));
    EXPECT_EQ("377", vampireField("CoatColor", "it-la-v"));
    EXPECT_EQ("1311", oustersField("ZoneID", "it-la-o"));
    EXPECT_EQ("FEMALE", oustersField("Sex", "it-la-o"));

    // The character list: three ACTIVE Slayer rows, the race telling the
    // caller where the rest lives.
    std::vector<LoginSlayerListRow> list = repo.loadSlayerList(kWorld, "it-laacct");
    ASSERT_EQ(3u, list.size());
    int seen = 0;
    for (size_t i = 0; i < list.size(); i++) {
        if (list[i].name == "it-la-s") {
            seen++;
            EXPECT_EQ("SLAYER", list[i].race);
            EXPECT_EQ("SLOT1", list[i].slot);
            EXPECT_EQ("FEMALE", list[i].sex);
            EXPECT_EQ(3, list[i].hairColor);
            EXPECT_EQ(4, list[i].skinColor);
            EXPECT_EQ(0, list[i].advancementClass);
            EXPECT_EQ(10, list[i].str);
            EXPECT_EQ(11, list[i].strExp);
            EXPECT_EQ(13, list[i].dex);
            EXPECT_EQ(14, list[i].dexExp);
            EXPECT_EQ(7, list[i].inte);
            EXPECT_EQ(17, list[i].intExp);
            EXPECT_EQ(20, list[i].hp);
            EXPECT_EQ(20, list[i].currentHP);
            EXPECT_EQ(14, list[i].mp);
            EXPECT_EQ(14, list[i].currentMP);
            EXPECT_EQ(0, list[i].fame);
            for (int j = 0; j < 6; j++)
                EXPECT_EQ(0, list[i].domainLevel[j]);
            EXPECT_EQ(7500, list[i].alignment);
            EXPECT_EQ(3u, list[i].shape);
            EXPECT_EQ(21, list[i].helmetColor);
            EXPECT_EQ(22, list[i].jacketColor);
            EXPECT_EQ(23, list[i].pantsColor);
            EXPECT_EQ(24, list[i].weaponColor);
            EXPECT_EQ(25, list[i].shieldColor);
            EXPECT_EQ(1, list[i].rank);
        } else if (list[i].name == "it-la-v") {
            seen++;
            EXPECT_EQ("VAMPIRE", list[i].race);
        } else if (list[i].name == "it-la-o") {
            seen++;
            EXPECT_EQ("OUSTERS", list[i].race);
        }
    }
    EXPECT_EQ(3, seen);

    LoginVampireListRow vr;
    ASSERT_TRUE(repo.loadVampireListRow(kWorld, "it-laacct", "it-la-v", vr));
    EXPECT_EQ("it-la-v", vr.name);
    EXPECT_EQ("SLOT2", vr.slot);
    EXPECT_EQ("MALE", vr.sex);
    EXPECT_EQ(0, vr.batColor);
    EXPECT_EQ(5, vr.skinColor);
    EXPECT_EQ(0, vr.advancementClass);
    EXPECT_EQ(20, vr.str);
    EXPECT_EQ(20, vr.dex);
    EXPECT_EQ(20, vr.inte);
    EXPECT_EQ(50, vr.hp);
    EXPECT_EQ(50, vr.currentHP);
    EXPECT_EQ(1, vr.rank);
    EXPECT_EQ(250, vr.goalExp);
    EXPECT_EQ(1, vr.level);
    EXPECT_EQ(0, vr.bonus);
    EXPECT_EQ(0, vr.fame);
    EXPECT_EQ(7500, vr.alignment);
    EXPECT_EQ(1u, vr.shape);
    EXPECT_EQ(377, vr.coatColor);

    LoginOustersListRow orow;
    ASSERT_TRUE(repo.loadOustersListRow(kWorld, "it-laacct", "it-la-o", orow));
    EXPECT_EQ("it-la-o", orow.name);
    EXPECT_EQ("SLOT3", orow.slot);
    EXPECT_EQ("FEMALE", orow.sex);
    EXPECT_EQ(0, orow.advancementClass);
    EXPECT_EQ(15, orow.str);
    EXPECT_EQ(15, orow.dex);
    EXPECT_EQ(15, orow.inte);
    EXPECT_EQ(50, orow.hp);
    EXPECT_EQ(50, orow.currentHP);
    EXPECT_EQ(1, orow.rank);
    EXPECT_EQ(0, orow.exp);
    EXPECT_EQ(1, orow.level);
    EXPECT_EQ(0, orow.bonus);
    EXPECT_EQ(0, orow.skillBonus);
    EXPECT_EQ(0, orow.fame);
    EXPECT_EQ(7500, orow.alignment);
    EXPECT_EQ(0, orow.coatType);
    EXPECT_EQ(0, orow.armType);
    EXPECT_EQ(377, orow.coatColor);
    EXPECT_EQ(6, orow.hairColor);
    EXPECT_EQ(377, orow.armColor);
    EXPECT_EQ(377, orow.bootsColor);

    // Another account's name, or a wrong name, answers false.
    EXPECT_FALSE(repo.loadVampireListRow(kWorld, "it-laother", "it-la-v", vr));
    EXPECT_FALSE(repo.loadOustersListRow(kWorld, "it-laacct", "it-la-none", orow));

    // An INACTIVE Slayer row leaves the list.
    execSQL("UPDATE Slayer SET Active = 'INACTIVE' WHERE Name = 'it-la-o'");
    EXPECT_EQ(2u, repo.loadSlayerList(kWorld, "it-laacct").size());
}

TEST_F(LoginCharacterMySQL, FlagSetPresetsAndInsertIgnore) {
    LoginCharacterRepository& repo = defaultLoginCharacterRepository();

    repo.insertFlagSet(kWorld, "it-la-s", LOGIN_FLAGSET_SLAYER);
    repo.insertFlagSet(kWorld, "it-la-v", LOGIN_FLAGSET_OTHER);
    EXPECT_NO_THROW(repo.insertFlagSet(kWorld, "it-la-x", LOGIN_FLAGSET_MAX));

    EXPECT_EQ("11110010001", queryScalar("SELECT FlagData FROM FlagSet WHERE OwnerID = 'it-la-s'"));
    EXPECT_EQ("00000000001", queryScalar("SELECT FlagData FROM FlagSet WHERE OwnerID = 'it-la-v'"));
    EXPECT_EQ("0", queryScalar("SELECT COUNT(*) FROM FlagSet WHERE OwnerID = 'it-la-x'"));

    // OwnerID is the primary key: a second insert is ignored.
    repo.insertFlagSet(kWorld, "it-la-s", LOGIN_FLAGSET_OTHER);
    EXPECT_EQ("11110010001", queryScalar("SELECT FlagData FROM FlagSet WHERE OwnerID = 'it-la-s'"));
}

TEST_F(LoginCharacterMySQL, SelectionReadsOneRaceTableAndWritesAllThree) {
    LoginCharacterRepository& repo = defaultLoginCharacterRepository();
    seedSlayer("it-la-s", "it-laacct", "SLOT1");
    execSQL("UPDATE Slayer SET ZoneID = 42, SwordLevel = 3, BladeLevel = 9, GunLevel = 4, EnchantLevel = 1, "
            "HealLevel = 2, Competence = 2 WHERE Name = 'it-la-s'");
    execSQL("INSERT INTO Vampire (Name, PlayerID, Slot, Active, ZoneID, Level, Competence) "
            "VALUES ('it-la-v', 'it-laacct', 'SLOT2', 'ACTIVE', 43, 17, 1)");
    execSQL("INSERT INTO Ousters (Name, PlayerID, Slot, Active, ZoneID, Level, Competence) "
            "VALUES ('it-la-o', 'it-laacct', 'SLOT3', 'ACTIVE', 44, 18, 3)");
    execSQL("INSERT INTO Ousters (Name, PlayerID, Slot, Active, ZoneID, Level, Competence) "
            "VALUES ('it-la-i', 'it-laacct', 'SLOT1', 'INACTIVE', 45, 19, 3)");

    LoginSelectRow pc;
    ASSERT_TRUE(repo.loadCharacterForSelect(kWorld, LOGIN_RACE_TABLE_SLAYER, "it-la-s", "it-laacct", pc));
    EXPECT_EQ(42, pc.zoneID);
    EXPECT_EQ("SLOT1", pc.slot);
    // GREATEST of the five skill domains.
    EXPECT_EQ(9, pc.level);
    EXPECT_EQ(2, pc.competence);

    ASSERT_TRUE(repo.loadCharacterForSelect(kWorld, LOGIN_RACE_TABLE_VAMPIRE, "it-la-v", "it-laacct", pc));
    EXPECT_EQ(43, pc.zoneID);
    EXPECT_EQ("SLOT2", pc.slot);
    EXPECT_EQ(17, pc.level);
    EXPECT_EQ(1, pc.competence);

    ASSERT_TRUE(repo.loadCharacterForSelect(kWorld, LOGIN_RACE_TABLE_OUSTERS, "it-la-o", "it-laacct", pc));
    EXPECT_EQ(44, pc.zoneID);
    EXPECT_EQ(18, pc.level);

    // Another account, an INACTIVE row, the wrong table, or a table
    // outside the enum answer false and leave the row alone.
    pc.slot = "untouched";
    EXPECT_FALSE(repo.loadCharacterForSelect(kWorld, LOGIN_RACE_TABLE_SLAYER, "it-la-s", "it-laother", pc));
    EXPECT_FALSE(repo.loadCharacterForSelect(kWorld, LOGIN_RACE_TABLE_OUSTERS, "it-la-i", "it-laacct", pc));
    EXPECT_FALSE(repo.loadCharacterForSelect(kWorld, LOGIN_RACE_TABLE_VAMPIRE, "it-la-s", "it-laacct", pc));
    EXPECT_FALSE(repo.loadCharacterForSelect(kWorld, LOGIN_RACE_TABLE_MAX, "it-la-s", "it-laacct", pc));
    EXPECT_EQ("untouched", pc.slot);

    // The group write hits every race table of that name, and only that
    // name.
    seedSlayer("it-la-k", "it-laacct", "SLOT2");
    execSQL("INSERT INTO Vampire (Name, PlayerID, Slot, Active) VALUES ('it-la-s', 'it-laacct', 'SLOT1', 'ACTIVE')");
    execSQL("INSERT INTO Ousters (Name, PlayerID, Slot, Active) VALUES ('it-la-s', 'it-laacct', 'SLOT1', 'ACTIVE')");
    repo.setCharacterServerGroup(kWorld, 7, "it-la-s");
    EXPECT_EQ("7", slayerField("ServerGroupID", "it-la-s"));
    EXPECT_EQ("7", vampireField("ServerGroupID", "it-la-s"));
    EXPECT_EQ("7", oustersField("ServerGroupID", "it-la-s"));
    EXPECT_EQ("0", slayerField("ServerGroupID", "it-la-k"));
}

class LoginConfigMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        execSQL("DELETE FROM GameServerGroupInfo WHERE WorldID BETWEEN 9170 AND 9179");
        execSQL("DELETE FROM ZoneGroupInfo WHERE ZoneGroupID BETWEEN 9170 AND 9179");
        execSQL("DELETE FROM ZoneInfo WHERE ZoneID BETWEEN 9170 AND 9179");
    }
};

TEST_F(LoginConfigMySQL, GameServerGroupsInBothProjections) {
    LoginConfigRepository& repo = defaultLoginConfigRepository();

    size_t before = repo.loadGameServerGroups().size();
    EXPECT_EQ(before, repo.loadGameServerGroupIDs().size());

    execSQL("INSERT INTO GameServerGroupInfo (GroupID, WorldID, GroupName, Stat) VALUES (7, 9170, 'it-la group', 1)");
    execSQL("INSERT INTO GameServerGroupInfo (GroupID, WorldID, GroupName, Stat) VALUES (8, 9171, 'it-la other', 0)");

    std::vector<LoginGameServerGroupRow> rows = repo.loadGameServerGroups();
    EXPECT_EQ(before + 2, rows.size());
    int seen = 0;
    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i].worldID == 9170) {
            seen++;
            EXPECT_EQ(7, rows[i].groupID);
            EXPECT_EQ("it-la group", rows[i].groupName);
            EXPECT_EQ(1, rows[i].stat);
        }
    }
    EXPECT_EQ(1, seen);

    std::vector<LoginGameServerGroupIDRow> ids = repo.loadGameServerGroupIDs();
    EXPECT_EQ(before + 2, ids.size());
    seen = 0;
    for (size_t i = 0; i < ids.size(); i++) {
        if (ids[i].worldID == 9171) {
            seen++;
            EXPECT_EQ(8, ids[i].groupID);
        }
    }
    EXPECT_EQ(1, seen);

    // initdb/ seeds WorldID 0 only, so the seeded 9171 is the maximum.
    int maxWorldID = 0;
    EXPECT_TRUE(repo.loadMaxGameServerGroupWorldID(maxWorldID));
    EXPECT_EQ(9171, maxWorldID);
}

TEST_F(LoginConfigMySQL, ZoneGroupsAndZones) {
    LoginConfigRepository& repo = defaultLoginConfigRepository();

    size_t groupsBefore = repo.loadZoneGroups().size();
    size_t zonesBefore = repo.loadZones().size();

    execSQL("INSERT INTO ZoneGroupInfo (ZoneGroupID, ServerID) VALUES (9170, 3)");
    execSQL("INSERT INTO ZoneInfo (ZoneID, ZoneGroupID) VALUES (9170, 9170)");

    std::vector<LoginZoneGroupRow> groups = repo.loadZoneGroups();
    EXPECT_EQ(groupsBefore + 1, groups.size());
    int seen = 0;
    for (size_t i = 0; i < groups.size(); i++) {
        if (groups[i].zoneGroupID == 9170) {
            seen++;
            EXPECT_EQ(3, groups[i].serverID);
        }
    }
    EXPECT_EQ(1, seen);

    std::vector<LoginZoneRow> zones = repo.loadZones();
    EXPECT_EQ(zonesBefore + 1, zones.size());
    seen = 0;
    for (size_t i = 0; i < zones.size(); i++) {
        if (zones[i].zoneID == 9170) {
            seen++;
            EXPECT_EQ(9170, zones[i].zoneGroupID);
        }
    }
    EXPECT_EQ(1, seen);
}

TEST_F(LoginConfigMySQL, ClientVersionIsTheSeededRow) {
    int version = -1;
    ASSERT_TRUE(defaultLoginConfigRepository().loadClientVersion(version));
    EXPECT_EQ(queryScalar("SELECT Version FROM ClientVersion"), std::to_string(version));
}

} // namespace
