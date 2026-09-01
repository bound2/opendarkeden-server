// MySQL-backed integration tier for the task 3.2 repositories
// (docs/RESTRUCTURING.md): runs the REAL MySQL implementations against a
// throwaway MySQL 5.7 loaded with the initdb/ schema and the production
// sql_mode. This is the authority the fakes in tests/support/ are
// corrected against — every quirk a fake pins is pinned HERE first,
// against the actual server (the 2026-09-01 adversarial review of PR #31
// falsified three fake-pinned claims exactly because no tier like this
// existed).
//
// Not part of the default ctest suite: it needs a database, which
// tests/integration/mysql_test.sh provides (a handrolled container
// fixture — start MySQL, wait for the schema import, run this binary on
// the same docker network, tear down). Run `make integration-test` from
// the host.
//
// Characters come from PlayerFixtures.h: ready-made low/mid/high level
// profiles per race. Each test persists exactly the profiles it wants —
// an unpersisted profile doubles as the "character with no rows" case.
//
// Wiring mirrors a zone thread: one world connection and one "dist"
// connection registered for this thread — two sockets, same server, same
// DARKEDEN schema, exactly like production (getDistConnection ignores its
// name argument; see MySQLGoodsRepository.cpp).

#include <cstdlib>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "PlayerFixtures.h"
#include "Thread.h"
#include "repository/BloodBibleSignRepository.h"
#include "repository/CharacterRepository.h"
#include "repository/GoldRepository.h"
#include "repository/GoodsRepository.h"
#include "repository/NicknameRepository.h"
#include "repository/RankBonusRepository.h"
#include "repository/StashRepository.h"

namespace {

std::string env(const char* name, const char* fallback) {
    const char* value = getenv(name);
    return value != NULL && *value != '\0' ? value : fallback;
}

// --- RankBonusData against real MySQL -------------------------------------

class RankBonusMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
        execSQL("DELETE FROM RankBonusData WHERE OwnerID IN ('itvampmid', 'itslaymid')");
    }
};

TEST_F(RankBonusMySQL, LoadTypesComeBackTypeAscendingNotInsertionOrder) {
    // No ORDER BY on the query, but the covering index (OwnerID, Type)
    // fully serves it: InnoDB's index scan returns Type order.
    PlayerFixture vampire = PlayerFixtures::midLevelVampire();
    vampire.persist();

    RankBonusRepository& repository = defaultRankBonusRepository();
    repository.insert(vampire.name, 7);
    repository.insert(vampire.name, 3);

    std::vector<DWORD> types = repository.loadTypes(vampire.name);
    ASSERT_EQ(2u, types.size());
    EXPECT_EQ(3u, types[0]);
    EXPECT_EQ(7u, types[1]);
}

TEST_F(RankBonusMySQL, KeylessTableStoresDuplicatesAndDeleteOneRemovesThemAll) {
    PlayerFixture vampire = PlayerFixtures::midLevelVampire();
    vampire.persist();

    RankBonusRepository& repository = defaultRankBonusRepository();
    repository.insert(vampire.name, 7);
    repository.insert(vampire.name, 7); // no PK, no unique key: second row lands
    repository.insert(vampire.name, 3);
    ASSERT_EQ(3u, repository.loadTypes(vampire.name).size());

    repository.deleteOne(vampire.name, 7);

    std::vector<DWORD> types = repository.loadTypes(vampire.name);
    ASSERT_EQ(1u, types.size());
    EXPECT_EQ(3u, types[0]);
}

TEST_F(RankBonusMySQL, DeleteAllClearsOnlyThatOwner) {
    PlayerFixture vampire = PlayerFixtures::midLevelVampire();
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    vampire.persist();
    slayer.persist();

    RankBonusRepository& repository = defaultRankBonusRepository();
    repository.insert(vampire.name, 7);
    repository.insert(slayer.name, 7);

    repository.deleteAll(vampire.name);

    EXPECT_TRUE(repository.loadTypes(vampire.name).empty());
    EXPECT_EQ(1u, repository.loadTypes(slayer.name).size());
}

// --- stash columns against real MySQL -------------------------------------

class StashMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
    }
};

TEST_F(StashMySQL, NonOustersSaveWritesSlayerAndVampireTables) {
    // Character creation gives every non-Ousters character a Slayer AND a
    // Vampire row; the save hits both — the Slayer-unconditional quirk.
    PlayerFixture slayer = PlayerFixtures::lowLevelSlayer();
    slayer.persist();

    defaultStashRepository().saveStashGold(slayer.name, false, 1234);

    int gold = -1;
    ASSERT_TRUE(defaultStashRepository().loadStashGold(slayer.name, CHARACTER_RACE_SLAYER, gold));
    EXPECT_EQ(1234, gold);
    ASSERT_TRUE(defaultStashRepository().loadStashGold(slayer.name, CHARACTER_RACE_VAMPIRE, gold));
    EXPECT_EQ(1234, gold);
}

TEST_F(StashMySQL, OustersSaveWritesSlayerAndOustersTables) {
    PlayerFixture ousters = PlayerFixtures::midLevelOusters();
    ousters.persist();

    defaultStashRepository().saveStashNum(ousters.name, true, 3);

    EXPECT_EQ("3", queryScalar("SELECT StashNum FROM Slayer WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("3", queryScalar("SELECT StashNum FROM Ousters WHERE Name='" + ousters.name + "'"));
}

TEST_F(StashMySQL, SaveAgainstMissingRowsIsASilentNoOp) {
    // Deliberately NOT persisted: the profile stands in for a character
    // with no rows. The UPDATEs match zero rows — no error, no warning.
    PlayerFixture ghost = PlayerFixtures::highLevelVampire();

    defaultStashRepository().saveStashGold(ghost.name, false, 500);

    int gold = -1;
    EXPECT_FALSE(defaultStashRepository().loadStashGold(ghost.name, CHARACTER_RACE_VAMPIRE, gold));
}

TEST_F(StashMySQL, GoldAboveIntMaxClampsToZeroDestroyingTheBalance) {
    // The (int) cast emits a NEGATIVE literal; the UNSIGNED StashGold
    // column clamps it to 0 (warning 1264) under the non-strict sql_mode.
    // NOT stored negative — the balance is destroyed. Unreachable with
    // the MAX_MONEY cap, pinned anyway.
    PlayerFixture vampire = PlayerFixtures::highLevelVampire();
    vampire.persist();
    defaultStashRepository().saveStashGold(vampire.name, false, 5000); // a real balance first

    defaultStashRepository().saveStashGold(vampire.name, false, 4000000000u);

    int gold = -1;
    ASSERT_TRUE(defaultStashRepository().loadStashGold(vampire.name, CHARACTER_RACE_VAMPIRE, gold));
    EXPECT_EQ(0, gold);
}

// --- carried gold against real MySQL --------------------------------------

class GoldMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
    }
};

TEST_F(GoldMySQL, IncreaseAndDecreaseAreRelativeOnTheRowBalance) {
    PlayerFixture vampire = PlayerFixtures::midLevelVampire();
    vampire.persist(); // Vampire.Gold column default is 0

    defaultGoldRepository().increaseGold(vampire.name, CHARACTER_RACE_VAMPIRE, 500);
    defaultGoldRepository().decreaseGold(vampire.name, CHARACTER_RACE_VAMPIRE, 200);

    int gold = -1;
    ASSERT_TRUE(defaultGoldRepository().loadGold(vampire.name, CHARACTER_RACE_VAMPIRE, gold));
    EXPECT_EQ(300, gold);
}

TEST_F(GoldMySQL, OperationsTargetOnlyTheOwnRaceTableAndSlayerRowsStartAt2000) {
    // Unlike the stash writes, gold has NO Slayer fan-out — and the
    // Slayer table's Gold column DEFAULTS to 2000 while Vampire/Ousters
    // default to 0, so a fresh slayer's twin Vampire row sits at 0.
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    slayer.persist();

    defaultGoldRepository().increaseGold(slayer.name, CHARACTER_RACE_SLAYER, 500);

    int gold = -1;
    ASSERT_TRUE(defaultGoldRepository().loadGold(slayer.name, CHARACTER_RACE_SLAYER, gold));
    EXPECT_EQ(2500, gold);
    ASSERT_TRUE(defaultGoldRepository().loadGold(slayer.name, CHARACTER_RACE_VAMPIRE, gold));
    EXPECT_EQ(0, gold);
}

TEST_F(GoldMySQL, DecreaseBelowTheRowBalanceRaisesOutOfRangeAndLeavesTheRowUntouched) {
    // The caller clamps the delta against its IN-MEMORY balance; when the
    // row holds less (integrity drift), Gold - delta on the UNSIGNED
    // column raises ER_DATA_OUT_OF_RANGE (1690) — same failure shape as
    // taking a Num=0 goods row — and the row keeps its balance.
    PlayerFixture ousters = PlayerFixtures::lowLevelOusters();
    ousters.persist();
    defaultGoldRepository().increaseGold(ousters.name, CHARACTER_RACE_OUSTERS, 10);

    EXPECT_ANY_THROW(defaultGoldRepository().decreaseGold(ousters.name, CHARACTER_RACE_OUSTERS, 50));

    int gold = -1;
    ASSERT_TRUE(defaultGoldRepository().loadGold(ousters.name, CHARACTER_RACE_OUSTERS, gold));
    EXPECT_EQ(10, gold);
}

TEST_F(GoldMySQL, OperationsAgainstMissingRowsAreSilentNoOps) {
    PlayerFixture ghost = PlayerFixtures::highLevelSlayer(); // never persisted

    defaultGoldRepository().increaseGold(ghost.name, CHARACTER_RACE_SLAYER, 500);
    defaultGoldRepository().decreaseGold(ghost.name, CHARACTER_RACE_SLAYER, 500);

    int gold = -1;
    EXPECT_FALSE(defaultGoldRepository().loadGold(ghost.name, CHARACTER_RACE_SLAYER, gold));
}

// --- character-row saves against real MySQL -------------------------------
// CharacterRepository is a write-only seam, so it has NO fake tier: these
// tests are its whole safety net (maintainer's call — integration over
// fakes for a project this thinly tested).

class CharacterMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
    }
};

TEST_F(CharacterMySQL, SlayerVitalsLandInTheSlayerRow) {
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    slayer.persist();

    SlayerVitalsRecord record;
    record.currentHP = 111;
    record.maxHP = 222;
    record.currentMP = 33;
    record.maxMP = 44;
    record.zoneID = 21;
    record.x = 100;
    record.y = 200;
    defaultCharacterRepository().saveSlayerVitals(slayer.name, record);

    EXPECT_EQ("111", queryScalar("SELECT CurrentHP FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("44", queryScalar("SELECT MP FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("21", queryScalar("SELECT ZoneID FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("200", queryScalar("SELECT YCoord FROM Slayer WHERE Name='" + slayer.name + "'"));
}

TEST_F(CharacterMySQL, VampireVitalsCarrySilverDamageInsteadOfMP) {
    PlayerFixture vampire = PlayerFixtures::midLevelVampire();
    vampire.persist();

    VampireVitalsRecord record;
    record.currentHP = 150;
    record.maxHP = 300;
    record.silverDamage = 12;
    record.zoneID = 23;
    record.x = 50;
    record.y = 60;
    defaultCharacterRepository().saveVampireVitals(vampire.name, record);

    EXPECT_EQ("150", queryScalar("SELECT CurrentHP FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("12", queryScalar("SELECT SilverDamage FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("23", queryScalar("SELECT ZoneID FROM Vampire WHERE Name='" + vampire.name + "'"));
}

TEST_F(CharacterMySQL, OustersVitalsLandInTheOustersRow) {
    PlayerFixture ousters = PlayerFixtures::lowLevelOusters();
    ousters.persist();

    OustersVitalsRecord record;
    record.currentHP = 90;
    record.maxHP = 120;
    record.currentMP = 70;
    record.maxMP = 80;
    record.zoneID = 51;
    record.x = 10;
    record.y = 20;
    defaultCharacterRepository().saveOustersVitals(ousters.name, record);

    EXPECT_EQ("90", queryScalar("SELECT CurrentHP FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("80", queryScalar("SELECT MP FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("51", queryScalar("SELECT ZoneID FROM Ousters WHERE Name='" + ousters.name + "'"));
}

TEST_F(CharacterMySQL, SlayerExpsTailLandsInFull) {
    PlayerFixture slayer = PlayerFixtures::highLevelSlayer();
    slayer.persist();

    SlayerExpsRecord record;
    record.strGoalExp = 1001;
    record.dexGoalExp = 1002;
    record.intGoalExp = 1003;
    record.bladeGoalExp = 2001;
    record.swordGoalExp = 2002;
    record.gunGoalExp = 2003;
    record.enchantGoalExp = 2004;
    record.healGoalExp = 2005;
    record.etcGoalExp = 2006;
    record.alignment = -50;
    record.fame = 777;
    record.rank = 9;
    record.rankGoalExp = 3001;
    record.advancementClass = 4;
    record.advancementGoalExp = 4001;
    record.advancedSTR = 11;
    record.advancedDEX = 12;
    record.advancedINT = 13;
    record.advancedAttrBonus = 14;
    defaultCharacterRepository().saveSlayerExps(slayer.name, record);

    EXPECT_EQ("1001", queryScalar("SELECT STRGoalExp FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("2006", queryScalar("SELECT ETCGoalExp FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("777", queryScalar("SELECT Fame FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("9", queryScalar("SELECT `Rank` FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("14", queryScalar("SELECT Bonus FROM Slayer WHERE Name='" + slayer.name + "'"));
}

TEST_F(CharacterMySQL, VampireExpsSkipSilverDamageWhenZero) {
    // The original composed an optional ",SilverDamage = %d" fragment: a
    // zero value leaves the column UNTOUCHED — this save cannot reset a
    // vampire's silver damage to zero.
    PlayerFixture vampire = PlayerFixtures::highLevelVampire();
    vampire.persist();
    execSQL("UPDATE Vampire SET SilverDamage = 7 WHERE Name='" + vampire.name + "'");

    VampireExpsRecord record;
    record.alignment = 10;
    record.fame = 55;
    record.goalExp = 900;
    record.silverDamage = 0;
    record.rank = 3;
    record.rankGoalExp = 800;
    record.advancementClass = 1;
    record.advancementGoalExp = 700;
    defaultCharacterRepository().saveVampireExps(vampire.name, record);
    EXPECT_EQ("7", queryScalar("SELECT SilverDamage FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("55", queryScalar("SELECT Fame FROM Vampire WHERE Name='" + vampire.name + "'"));

    record.silverDamage = 5;
    defaultCharacterRepository().saveVampireExps(vampire.name, record);
    EXPECT_EQ("5", queryScalar("SELECT SilverDamage FROM Vampire WHERE Name='" + vampire.name + "'"));
}

TEST_F(CharacterMySQL, OustersExpsAlwaysWriteSilverDamage) {
    // Unlike the vampire's conditional fragment, the ousters save writes
    // SilverDamage unconditionally — zero included.
    PlayerFixture ousters = PlayerFixtures::midLevelOusters();
    ousters.persist();
    execSQL("UPDATE Ousters SET SilverDamage = 7 WHERE Name='" + ousters.name + "'");

    OustersExpsRecord record;
    record.alignment = 10;
    record.fame = 55;
    record.goalExp = 900;
    record.silverDamage = 0;
    record.rank = 3;
    record.rankGoalExp = 800;
    record.advancementClass = 1;
    record.advancementGoalExp = 700;
    defaultCharacterRepository().saveOustersExps(ousters.name, record);

    EXPECT_EQ("0", queryScalar("SELECT SilverDamage FROM Ousters WHERE Name='" + ousters.name + "'"));
}

TEST_F(CharacterMySQL, TinysaveAppliesTheFragmentToTheOwnTableOnly) {
    PlayerFixture slayer = PlayerFixtures::lowLevelSlayer();
    slayer.persist(); // Slayer + twin Vampire row

    defaultCharacterRepository().tinysave(slayer.name, CHARACTER_RACE_SLAYER, "StashNum=9");

    EXPECT_EQ("9", queryScalar("SELECT StashNum FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("0", queryScalar("SELECT StashNum FROM Vampire WHERE Name='" + slayer.name + "'"));
}

TEST_F(CharacterMySQL, ResetSlayerRewardZeroesTheColumn) {
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    slayer.persist();
    execSQL("UPDATE Slayer SET Reward = 5 WHERE Name='" + slayer.name + "'");

    defaultCharacterRepository().resetSlayerReward(slayer.name);

    EXPECT_EQ("0", queryScalar("SELECT Reward FROM Slayer WHERE Name='" + slayer.name + "'"));
}

// --- BloodBibleSignObject against real MySQL ------------------------------

class BloodBibleSignMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
        execSQL("DELETE FROM BloodBibleSignObject WHERE OwnerID = 'itvamphi'");
    }
};

TEST_F(BloodBibleSignMySQL, LoadReturnsItemTypeAscending) {
    PlayerFixture vampire = PlayerFixtures::highLevelVampire();
    vampire.persist();
    // Rows arrive from OUTSIDE the server process (the interface is
    // read-only on purpose) — seeded here the way the web side would.
    execSQL("INSERT INTO BloodBibleSignObject (ItemType, OwnerID) VALUES (5, '" + vampire.name + "'), (2, '" +
            vampire.name + "'), (9, '" + vampire.name + "')");

    std::vector<ItemType_t> itemTypes = defaultBloodBibleSignRepository().loadItemTypes(vampire.name);
    ASSERT_EQ(3u, itemTypes.size());
    EXPECT_EQ(2, itemTypes[0]);
    EXPECT_EQ(5, itemTypes[1]);
    EXPECT_EQ(9, itemTypes[2]);
}

TEST_F(BloodBibleSignMySQL, DuplicateItemTypesSurvive) {
    PlayerFixture vampire = PlayerFixtures::highLevelVampire();
    vampire.persist();
    execSQL("INSERT INTO BloodBibleSignObject (ItemType, OwnerID) VALUES (5, '" + vampire.name + "'), (5, '" +
            vampire.name + "')");

    EXPECT_EQ(2u, defaultBloodBibleSignRepository().loadItemTypes(vampire.name).size());
}

// --- GoodsListObject against real MySQL -----------------------------------

class GoodsMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
        execSQL("DELETE FROM GoodsListObject WHERE PlayerID = 'itaccount'");
    }

    static void seed(const std::string& id, int world, const std::string& name, int num, const std::string& status) {
        char sql[240];
        sprintf(sql,
                "INSERT INTO GoodsListObject (BuyID, ID, World, PlayerID, Name, GoodsID, Num, Status) "
                "VALUES ('itbuy', %s, %d, 'itaccount', '%s', 5000, %d, '%s')",
                id.c_str(), world, name.c_str(), num, status.c_str());
        execSQL(sql);
    }
};

TEST_F(GoodsMySQL, PendingIsFilteredByWorldPlayerNameAndStatus) {
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    PlayerFixture ousters = PlayerFixtures::lowLevelOusters();
    slayer.persist();
    ousters.persist();

    seed("9101", 1, slayer.name, 2, "NOT");
    seed("9102", 2, slayer.name, 2, "NOT");  // other world
    seed("9103", 1, ousters.name, 2, "NOT"); // other character
    seed("9104", 1, slayer.name, 2, "GET");  // already taken

    std::vector<GoodsRecord> records = defaultGoodsRepository().loadPending(1, "itaccount", slayer.name);
    ASSERT_EQ(1u, records.size());
    EXPECT_EQ("9101", records[0].id);
    EXPECT_EQ(5000u, records[0].goodsID);
    EXPECT_EQ(2, records[0].num);
}

TEST_F(GoodsMySQL, TakeOneDecrementsAndFlipsStatusOnTheLastUnit) {
    // One UPDATE, left-to-right SET evaluation: the IF() reads the
    // already-decremented Num, so the last unit flips Status to 'GET' in
    // the statement that takes it.
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    slayer.persist();
    seed("9101", 1, slayer.name, 2, "NOT");

    EXPECT_TRUE(defaultGoodsRepository().takeOne("9101"));
    std::vector<GoodsRecord> records = defaultGoodsRepository().loadPending(1, "itaccount", slayer.name);
    ASSERT_EQ(1u, records.size());
    EXPECT_EQ(1, records[0].num);

    EXPECT_TRUE(defaultGoodsRepository().takeOne("9101"));
    EXPECT_TRUE(defaultGoodsRepository().loadPending(1, "itaccount", slayer.name).empty());
    EXPECT_EQ("GET", queryScalar("SELECT Status FROM GoodsListObject WHERE ID=9101"));
    EXPECT_EQ("0", queryScalar("SELECT Num FROM GoodsListObject WHERE ID=9101"));
}

TEST_F(GoodsMySQL, TakingAZeroCountRowFailsAndLeavesItUntouched) {
    // Num - 1 on the UNSIGNED column raises ER_DATA_OUT_OF_RANGE (1690)
    // regardless of strict mode; the row is untouched and the error
    // escapes as an exception (through END_DB, as a raw const char*) —
    // the stuck-item path documented on MySQLGoodsRepository.
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    slayer.persist();
    seed("9101", 1, slayer.name, 0, "NOT");

    EXPECT_ANY_THROW(defaultGoodsRepository().takeOne("9101"));

    EXPECT_EQ("0", queryScalar("SELECT Num FROM GoodsListObject WHERE ID=9101"));
    EXPECT_EQ("NOT", queryScalar("SELECT Status FROM GoodsListObject WHERE ID=9101"));
}

TEST_F(GoodsMySQL, TakeOneOfAnUnknownIdIsFalse) {
    EXPECT_FALSE(defaultGoodsRepository().takeOne("999999"));
}

// --- NicknameBook against real MySQL (the 3.2 pilot) ----------------------

class NicknameMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
        execSQL("DELETE FROM NicknameBook WHERE OwnerID = 'itoustmid'");
    }
};

TEST_F(NicknameMySQL, LoadReturnsNIDAscendingNotInsertionOrder) {
    // The pilot's fake documented "insertion order" for this ORDER-BY-less
    // SELECT; flagged in the PR #31 review as unverified. Reality: the
    // secondary index IDX_OwnerID carries the primary key (nID, OwnerID)
    // as its suffix, so the ref scan returns nID ascending.
    PlayerFixture ousters = PlayerFixtures::midLevelOusters();
    ousters.persist();

    NicknameRepository& repository = defaultNicknameRepository();
    repository.insert(ousters.name, 10001, NicknameInfo::NICK_CUSTOM, "second-id-first");
    repository.insert(ousters.name, 10000, NicknameInfo::NICK_CUSTOM, "first-id-second");

    std::vector<NicknameRecord> records = repository.load(ousters.name);
    ASSERT_EQ(2u, records.size());
    EXPECT_EQ(10000, records[0].id);
    EXPECT_EQ(10001, records[1].id);
}

} // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    std::string host = env("IT_DB_HOST", "127.0.0.1");
    std::string db = env("IT_DB_DB", "DARKEDEN");
    std::string user = env("IT_DB_USER", "elcastle");
    std::string password = env("IT_DB_PASSWORD", "elca110");
    uint port = (uint)atoi(env("IT_DB_PORT", "3306").c_str());

    g_pDatabaseManager = new DatabaseManager();
    g_pDatabaseManager->addConnection((int)(long)Thread::self(), new Connection(host, db, user, password, port));
    g_pDatabaseManager->addDistConnection((int)(long)Thread::self(), new Connection(host, db, user, password, port));

    return RUN_ALL_TESTS();
}
