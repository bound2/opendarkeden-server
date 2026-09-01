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
    ASSERT_TRUE(defaultStashRepository().loadStashGold(slayer.name, STASH_RACE_SLAYER, gold));
    EXPECT_EQ(1234, gold);
    ASSERT_TRUE(defaultStashRepository().loadStashGold(slayer.name, STASH_RACE_VAMPIRE, gold));
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
    EXPECT_FALSE(defaultStashRepository().loadStashGold(ghost.name, STASH_RACE_VAMPIRE, gold));
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
    ASSERT_TRUE(defaultStashRepository().loadStashGold(vampire.name, STASH_RACE_VAMPIRE, gold));
    EXPECT_EQ(0, gold);
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
