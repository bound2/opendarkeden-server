// MySQL-backed integration tier for the loginserver's character-purge
// repository: the real MySQLLoginCharacterPurgeRepository against the
// throwaway MySQL 5.7 loaded with initdb/, on the connections
// mysql_repository_test.cpp's main() wires for this binary. The methods
// that ask for getConnection(worldID) reach the world-default connection
// here: main() registers no per-world connection, so DatabaseManager's
// world map is empty and every id falls through to the default — the
// same server and schema as the DARKEDEN connection the seeding below
// uses.
//
// The build-flag variants (the DELETE race rows and the skill-save trio
// under __CHINA_SERVER__ / __THAILAND_SERVER__ / __NETMARBLE_SERVER__,
// the five tables __THAILAND_SERVER__ drops) are not compiled into this
// binary and are not exercised.

#include <string>

#include <gtest/gtest.h>

#include "DB.h"
#include "repository/LoginCharacterPurgeRepository.h"

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

// Any WorldID resolves to the world-default connection in this binary.
const WorldID_t kWorld = 1;

std::string q(const std::string& s) {
    return "'" + s + "'";
}

class LoginCharacterPurgeMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        const char* byName[] = {"Slayer", "Vampire", "Ousters", "DeleteChar"};
        for (size_t i = 0; i < sizeof(byName) / sizeof(byName[0]); i++)
            execSQL(std::string("DELETE FROM ") + byName[i] + " WHERE Name LIKE 'it-lp%'");
        const char* byOwner[] = {"RankBonusData",   "ARObject",        "LuckyBagObject",    "SMSItemObject",
                                 "GQuestSave",      "MittenObject",    "MotorcycleObject",  "MaceObject",
                                 "EventStarObject", "EffectAcidTouch", "EffectMute",        "EnemyErase",
                                 "FlagSet",         "TimeLimitItems",  "EventQuestAdvance", "MofusPowerPoint"};
        for (size_t i = 0; i < sizeof(byOwner) / sizeof(byOwner[0]); i++)
            execSQL(std::string("DELETE FROM ") + byOwner[i] + " WHERE OwnerID LIKE 'it-lp%'");
        execSQL("DELETE FROM CoupleInfo WHERE MalePartnerName LIKE 'it-lp%' OR FemalePartnerName LIKE 'it-lp%'");
    }

    // One ACTIVE row per race table, the way CLCreatePCHandler leaves a
    // character (it never creates both a Vampire and an Ousters row, but
    // the purge issues both statements, so both are seeded).
    static void seedRaceRows(const std::string& name, const std::string& slot, const std::string& active = "ACTIVE") {
        const char* tables[] = {"Slayer", "Vampire", "Ousters"};
        for (size_t i = 0; i < 3; i++)
            execSQL(std::string("INSERT INTO ") + tables[i] + " (Name, PlayerID, Slot, Active) VALUES (" + q(name) +
                    ", 'it-lpacct', " + q(slot) + ", " + q(active) + ")");
    }

    // One row in a table from every stretch of purgeCharacterRows' list:
    // RankBonusData; the first object table, the last before the
    // __THAILAND_SERVER__ block, one inside it, GQuestSave, and the last
    // of the list; CoupleInfo by both columns; the first and the last
    // Effect table; EnemyErase; the four name-keyed tables at the end.
    // 15 rows.
    static void seedPurgeRows(const std::string& name, int id) {
        const std::string n = q(name);
        const std::string i = std::to_string(id);
        execSQL("INSERT INTO RankBonusData (OwnerID, Type) VALUES (" + n + ", 1)");
        execSQL("INSERT INTO ARObject (ItemID, OwnerID) VALUES (" + i + ", " + n + ")");
        execSQL("INSERT INTO LuckyBagObject (ItemID, OwnerID) VALUES (" + i + ", " + n + ")");
        execSQL("INSERT INTO SMSItemObject (ItemID, OwnerID) VALUES (" + i + ", " + n + ")");
        execSQL("INSERT INTO GQuestSave (QuestID, OwnerID) VALUES (" + i + ", " + n + ")");
        execSQL("INSERT INTO MittenObject (ItemID, OwnerID) VALUES (" + i + ", " + n + ")");
        execSQL("INSERT INTO CoupleInfo (MalePartnerName, FemalePartnerName) VALUES (" + n + ", 'x')");
        execSQL("INSERT INTO CoupleInfo (MalePartnerName, FemalePartnerName) VALUES ('y', " + n + ")");
        execSQL("INSERT INTO EffectAcidTouch (OwnerID) VALUES (" + n + ")");
        execSQL("INSERT INTO EffectMute (OwnerID) VALUES (" + n + ")");
        execSQL("INSERT INTO EnemyErase (OwnerID) VALUES (" + n + ")");
        execSQL("INSERT INTO FlagSet (OwnerID) VALUES (" + n + ")");
        execSQL("INSERT INTO TimeLimitItems (OwnerID, ItemID) VALUES (" + n + ", " + i + ")");
        execSQL("INSERT INTO EventQuestAdvance (OwnerID, QuestLevel) VALUES (" + n + ", 1)");
        execSQL("INSERT INTO MofusPowerPoint (OwnerID, Point) VALUES (" + n + ", 5)");
    }

    static std::string purgeRowsOf(const std::string& name) {
        const std::string n = q(name);
        return queryScalar("SELECT (SELECT COUNT(*) FROM RankBonusData WHERE OwnerID = " + n +
                           ") + (SELECT COUNT(*) FROM ARObject WHERE OwnerID = " + n +
                           ") + (SELECT COUNT(*) FROM LuckyBagObject WHERE OwnerID = " + n +
                           ") + (SELECT COUNT(*) FROM SMSItemObject WHERE OwnerID = " + n +
                           ") + (SELECT COUNT(*) FROM GQuestSave WHERE OwnerID = " + n +
                           ") + (SELECT COUNT(*) FROM MittenObject WHERE OwnerID = " + n +
                           ") + (SELECT COUNT(*) FROM CoupleInfo WHERE MalePartnerName = " + n +
                           " OR FemalePartnerName = " + n +
                           ") + (SELECT COUNT(*) FROM EffectAcidTouch WHERE OwnerID = " + n +
                           ") + (SELECT COUNT(*) FROM EffectMute WHERE OwnerID = " + n +
                           ") + (SELECT COUNT(*) FROM EnemyErase WHERE OwnerID = " + n +
                           ") + (SELECT COUNT(*) FROM FlagSet WHERE OwnerID = " + n +
                           ") + (SELECT COUNT(*) FROM TimeLimitItems WHERE OwnerID = " + n +
                           ") + (SELECT COUNT(*) FROM EventQuestAdvance WHERE OwnerID = " + n +
                           ") + (SELECT COUNT(*) FROM MofusPowerPoint WHERE OwnerID = " + n + ")");
    }

    static std::string active(const char* table, const std::string& name) {
        return queryScalar(std::string("SELECT Active FROM ") + table + " WHERE Name = " + q(name));
    }

    // The first, the duplicated and the last table of destroyItems' list,
    // plus one table only purgeCharacterRows names. 4 rows.
    static void seedItemRows(const std::string& name, int id) {
        const std::string n = q(name);
        const std::string i = std::to_string(id);
        execSQL("INSERT INTO MotorcycleObject (ItemID, OwnerID) VALUES (" + i + ", " + n + ")");
        execSQL("INSERT INTO MaceObject (ItemID, OwnerID) VALUES (" + i + ", " + n + ")");
        execSQL("INSERT INTO EventStarObject (ItemID, OwnerID) VALUES (" + i + ", " + n + ")");
        execSQL("INSERT INTO MittenObject (ItemID, OwnerID) VALUES (" + i + ", " + n + ")");
    }

    static std::string itemRowsOf(const std::string& name) {
        const std::string n = q(name);
        return queryScalar("SELECT (SELECT COUNT(*) FROM MotorcycleObject WHERE OwnerID = " + n +
                           ") + (SELECT COUNT(*) FROM MaceObject WHERE OwnerID = " + n +
                           ") + (SELECT COUNT(*) FROM EventStarObject WHERE OwnerID = " + n + ")");
    }
};

TEST_F(LoginCharacterPurgeMySQL, LoadActiveSlayerOwnerAnswersOnlyForAnActiveRow) {
    seedRaceRows("it-lpa", "SLOT1");
    seedRaceRows("it-lpb", "SLOT2", "INACTIVE");

    std::string playerID = "untouched";
    EXPECT_TRUE(defaultLoginCharacterPurgeRepository().loadActiveSlayerOwner(kWorld, "it-lpa", playerID));
    EXPECT_EQ("it-lpacct", playerID);

    // An INACTIVE row and a missing row both answer false and leave the
    // out-parameter alone.
    playerID = "untouched";
    EXPECT_FALSE(defaultLoginCharacterPurgeRepository().loadActiveSlayerOwner(kWorld, "it-lpb", playerID));
    EXPECT_EQ("untouched", playerID);
    EXPECT_FALSE(defaultLoginCharacterPurgeRepository().loadActiveSlayerOwner(kWorld, "it-lpnone", playerID));
    EXPECT_EQ("untouched", playerID);
}

TEST_F(LoginCharacterPurgeMySQL, RetireSlayerFlipsTheRowOfThatNameAndSlotOnly) {
    seedRaceRows("it-lpa", "SLOT1");
    seedRaceRows("it-lpk", "SLOT1");

    // The wrong slot matches nothing and reports it.
    EXPECT_FALSE(defaultLoginCharacterPurgeRepository().retireSlayer(kWorld, "it-lpa", SLOT2));
    EXPECT_EQ("ACTIVE", active("Slayer", "it-lpa"));

    EXPECT_TRUE(defaultLoginCharacterPurgeRepository().retireSlayer(kWorld, "it-lpa", SLOT1));
    EXPECT_EQ("INACTIVE", active("Slayer", "it-lpa"));

    // A row already INACTIVE changes nothing, so the affected count is 0
    // and the method answers false.
    EXPECT_FALSE(defaultLoginCharacterPurgeRepository().retireSlayer(kWorld, "it-lpa", SLOT1));

    // Only the Slayer row of that name: its Vampire and Ousters rows and
    // the other character are untouched.
    EXPECT_EQ("ACTIVE", active("Vampire", "it-lpa"));
    EXPECT_EQ("ACTIVE", active("Ousters", "it-lpa"));
    EXPECT_EQ("ACTIVE", active("Slayer", "it-lpk"));
}

TEST_F(LoginCharacterPurgeMySQL, RecordDeletionInsertsOneRowStampedNow) {
    defaultLoginCharacterPurgeRepository().recordDeletion("it-lpacct", 3, "it-lpa");

    EXPECT_EQ("1", queryScalar("SELECT COUNT(*) FROM DeleteChar WHERE Name = 'it-lpa'"));
    EXPECT_EQ("it-lpacct", queryScalar("SELECT PlayerID FROM DeleteChar WHERE Name = 'it-lpa'"));
    EXPECT_EQ("3", queryScalar("SELECT WorldID FROM DeleteChar WHERE Name = 'it-lpa'"));
    // delDate is the database's now(): within a minute of it, not the
    // column's zero default.
    EXPECT_EQ("1", queryScalar("SELECT TIMESTAMPDIFF(SECOND, delDate, NOW()) BETWEEN 0 AND 60 FROM DeleteChar "
                               "WHERE Name = 'it-lpa'"));
    EXPECT_EQ("0", queryScalar("SELECT COUNT(*) FROM DeleteChar WHERE Name = 'it-lpk'"));
}

TEST_F(LoginCharacterPurgeMySQL, PurgeCharacterRowsRetiresTheRaceRowsOfThatSlotAndDeletesEveryOtherRowOfThatNameOnly) {
    seedRaceRows("it-lpa", "SLOT1");
    seedRaceRows("it-lpk", "SLOT1");
    seedPurgeRows("it-lpa", 32000);
    seedPurgeRows("it-lpk", 32001);
    ASSERT_EQ("15", purgeRowsOf("it-lpa"));
    ASSERT_EQ("15", purgeRowsOf("it-lpk"));

    defaultLoginCharacterPurgeRepository().purgeCharacterRows(kWorld, "it-lpa", SLOT1);

    // Vampire and Ousters flip to INACTIVE; Slayer is not this method's.
    EXPECT_EQ("INACTIVE", active("Vampire", "it-lpa"));
    EXPECT_EQ("INACTIVE", active("Ousters", "it-lpa"));
    EXPECT_EQ("ACTIVE", active("Slayer", "it-lpa"));
    EXPECT_EQ("0", purgeRowsOf("it-lpa"));

    // The other character is whole.
    EXPECT_EQ("ACTIVE", active("Slayer", "it-lpk"));
    EXPECT_EQ("ACTIVE", active("Vampire", "it-lpk"));
    EXPECT_EQ("ACTIVE", active("Ousters", "it-lpk"));
    EXPECT_EQ("15", purgeRowsOf("it-lpk"));

    // A name with no rows anywhere purges without complaint.
    EXPECT_NO_THROW(defaultLoginCharacterPurgeRepository().purgeCharacterRows(kWorld, "it-lpnone", SLOT1));
}

TEST_F(LoginCharacterPurgeMySQL, PurgeCharacterRowsKeysOnlyTheRaceRowsBySlot) {
    seedRaceRows("it-lpa", "SLOT1");
    seedPurgeRows("it-lpa", 32002);

    defaultLoginCharacterPurgeRepository().purgeCharacterRows(kWorld, "it-lpa", SLOT3);

    // The race rows sit in another slot and stay ACTIVE; every name-keyed
    // row is gone regardless.
    EXPECT_EQ("ACTIVE", active("Vampire", "it-lpa"));
    EXPECT_EQ("ACTIVE", active("Ousters", "it-lpa"));
    EXPECT_EQ("0", purgeRowsOf("it-lpa"));
}

TEST_F(LoginCharacterPurgeMySQL, DestroyItemsDeletesItsOwnTablesForThatOwnerOnly) {
    seedItemRows("it-lpa", 32003);
    seedItemRows("it-lpk", 32004);
    ASSERT_EQ("3", itemRowsOf("it-lpa"));

    defaultLoginCharacterPurgeRepository().destroyItems("it-lpa");

    EXPECT_EQ("0", itemRowsOf("it-lpa"));
    // MittenObject is on purgeCharacterRows' list, not on this one.
    EXPECT_EQ("1", queryScalar("SELECT COUNT(*) FROM MittenObject WHERE OwnerID = 'it-lpa'"));
    EXPECT_EQ("3", itemRowsOf("it-lpk"));
    EXPECT_EQ("1", queryScalar("SELECT COUNT(*) FROM MittenObject WHERE OwnerID = 'it-lpk'"));
}

} // namespace
