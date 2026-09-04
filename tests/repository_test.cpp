// Task 3.2: pins the repository contracts and the pure nickname-book rules
// extracted from NicknameBook::load(). The fakes stand in for the MySQL
// implementations in domain tests — these tests pin the INTENDED contract
// only and are not evidence about MySQL. The MySQL-backed integration tier
// (tests/integration/, `make integration-test`) runs the real
// implementations against a throwaway MySQL 5.7 with the initdb/ schema
// and is the authority the fakes are corrected against.

#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "FakeBloodBibleSignRepository.h"
#include "FakeGoldRepository.h"
#include "FakeGoodsRepository.h"
#include "FakeNicknameRepository.h"
#include "FakeRankBonusRepository.h"
#include "FakeStashRepository.h"
#include "repository/NicknameRecord.h"

namespace {

NicknameRecord record(WORD id, BYTE type, const std::string& nickname = "", WORD index = 0) {
    NicknameRecord r;
    r.id = id;
    r.type = type;
    r.nickname = nickname;
    r.index = index;
    return r;
}

// --- pure rules -----------------------------------------------------------

TEST(NextNicknameID, StartsAt10000ForAnEmptyBook) {
    std::vector<NicknameRecord> records;
    EXPECT_EQ(10000, nextNicknameIDAfter(records));
}

TEST(NextNicknameID, BuiltInSlotsBelow10000DoNotAdvanceIt) {
    std::vector<NicknameRecord> records;
    records.push_back(record(0, NicknameInfo::NICK_CUSTOM, " "));
    records.push_back(record(11, NicknameInfo::NICK_BUILT_IN));
    EXPECT_EQ(10000, nextNicknameIDAfter(records));
}

TEST(NextNicknameID, ContinuesAfterTheHighestStoredCustomID) {
    std::vector<NicknameRecord> records;
    records.push_back(record(10000, NicknameInfo::NICK_CUSTOM, "first"));
    records.push_back(record(10002, NicknameInfo::NICK_CUSTOM, "third"));
    EXPECT_EQ(10003, nextNicknameIDAfter(records));
}

TEST(NextNicknameID, NickNoneRowsNeverClaimAnID) {
    std::vector<NicknameRecord> records;
    records.push_back(record(12000, NicknameInfo::NICK_NONE));
    EXPECT_EQ(10000, nextNicknameIDAfter(records));
}

TEST(HasCustomSlot, FalseForAnEmptyBook) {
    std::vector<NicknameRecord> records;
    EXPECT_FALSE(hasCustomSlot(records));
}

TEST(HasCustomSlot, TrueOnceTheIDZeroRowExists) {
    std::vector<NicknameRecord> records;
    records.push_back(record(0, NicknameInfo::NICK_CUSTOM, " "));
    EXPECT_TRUE(hasCustomSlot(records));
}

TEST(HasCustomSlot, ANickNoneRowAtZeroDoesNotCount) {
    std::vector<NicknameRecord> records;
    records.push_back(record(0, NicknameInfo::NICK_NONE));
    EXPECT_FALSE(hasCustomSlot(records));
}

// --- repository contract, pinned via the fake -----------------------------

TEST(NicknameRepositoryContract, LoadOfAnUnknownOwnerIsEmpty) {
    FakeNicknameRepository repository;
    EXPECT_TRUE(repository.load("nobody").empty());
}

TEST(NicknameRepositoryContract, InsertedRowsComeBackOnLoad) {
    FakeNicknameRepository repository;
    repository.insert("Hyanggi", 10000, NicknameInfo::NICK_CUSTOM, "hunter");

    std::vector<NicknameRecord> records = repository.load("Hyanggi");
    ASSERT_EQ(1u, records.size());
    EXPECT_EQ(10000, records[0].id);
    EXPECT_EQ(NicknameInfo::NICK_CUSTOM, records[0].type);
    EXPECT_EQ("hunter", records[0].nickname);
    EXPECT_EQ(0, records[0].index); // NickIndex takes the column default
}

TEST(NicknameRepositoryContract, DefaultCustomSlotIsASingleSpaceAtIDZero) {
    FakeNicknameRepository repository;
    repository.insertDefaultCustomSlot("Hyanggi");

    std::vector<NicknameRecord> records = repository.load("Hyanggi");
    ASSERT_EQ(1u, records.size());
    EXPECT_EQ(0, records[0].id);
    EXPECT_EQ(NicknameInfo::NICK_CUSTOM, records[0].type);
    EXPECT_EQ(" ", records[0].nickname);
    EXPECT_TRUE(hasCustomSlot(records));
}

TEST(NicknameRepositoryContract, DefaultCustomSlotInsertIsIdempotent) {
    FakeNicknameRepository repository;
    repository.insertDefaultCustomSlot("Hyanggi");
    repository.updateNickname("Hyanggi", 0, "renamed");
    repository.insertDefaultCustomSlot("Hyanggi"); // INSERT IGNORE: no-op

    std::vector<NicknameRecord> records = repository.load("Hyanggi");
    ASSERT_EQ(1u, records.size());
    EXPECT_EQ("renamed", records[0].nickname);
}

TEST(NicknameRepositoryContract, UpdateRenamesOneRowInPlace) {
    FakeNicknameRepository repository;
    repository.insert("Hyanggi", 10000, NicknameInfo::NICK_CUSTOM, "before");
    repository.insert("Hyanggi", 10001, NicknameInfo::NICK_CUSTOM, "kept");

    repository.updateNickname("Hyanggi", 10000, "after");

    std::vector<NicknameRecord> records = repository.load("Hyanggi");
    ASSERT_EQ(2u, records.size());
    EXPECT_EQ("after", records[0].nickname);
    EXPECT_EQ("kept", records[1].nickname);
}

TEST(NicknameRepositoryContract, OwnersAreIsolated) {
    FakeNicknameRepository repository;
    repository.insert("Hyanggi", 10000, NicknameInfo::NICK_CUSTOM, "mine");

    EXPECT_TRUE(repository.load("Someone").empty());
    ASSERT_EQ(1u, repository.load("Hyanggi").size());
}

TEST(NicknameRepositoryContract, DuplicateInsertThrows) {
    // The real table has PRIMARY KEY (nID, OwnerID) and the plain INSERT is
    // not INSERT IGNORE: a duplicate raises through the DB layer. Reachable
    // in production — nextNicknameIDAfter() skips NICK_NONE rows, so a book
    // holding one can re-issue a taken id.
    FakeNicknameRepository repository;
    repository.insert("Hyanggi", 12000, NicknameInfo::NICK_CUSTOM, "taken");

    EXPECT_THROW(repository.insert("Hyanggi", 12000, NicknameInfo::NICK_CUSTOM, "again"), std::runtime_error);
    // a different owner may reuse the id (the key is compound)
    repository.insert("Someone", 12000, NicknameInfo::NICK_CUSTOM, "theirs");
}

TEST(NicknameRepositoryContract, LoadReturnsNIDAscendingNotInsertionOrder) {
    // The real SELECT has no ORDER BY, but the secondary index IDX_OwnerID
    // carries the primary key (nID, OwnerID) as its suffix, so the ref
    // scan returns nID ascending — pinned against real MySQL by the
    // integration tier. (This test originally asserted insertion order;
    // the 2026-09-01 review round falsified that.)
    FakeNicknameRepository repository;
    repository.insert("Hyanggi", 10001, NicknameInfo::NICK_CUSTOM, "second-id-first");
    repository.insert("Hyanggi", 10000, NicknameInfo::NICK_CUSTOM, "first-id-second");

    std::vector<NicknameRecord> records = repository.load("Hyanggi");
    ASSERT_EQ(2u, records.size());
    EXPECT_EQ(10000, records[0].id);
    EXPECT_EQ(10001, records[1].id);
}

TEST(NicknameRepositoryContract, NicknameTruncatesToColumnWidth) {
    // Nickname is varchar(22) latin1 and the server runs with
    // STRICT_TRANS_TABLES off: over-long values silently truncate.
    FakeNicknameRepository repository;
    repository.insert("Hyanggi", 10000, NicknameInfo::NICK_CUSTOM, "abcdefghijklmnopqrstuvwxyz");
    EXPECT_EQ("abcdefghijklmnopqrstuv", repository.load("Hyanggi")[0].nickname);

    repository.updateNickname("Hyanggi", 10000, std::string(30, 'x'));
    EXPECT_EQ(std::string(22, 'x'), repository.load("Hyanggi")[0].nickname);
}

// --- RankBonusData contract, pinned via the fake --------------------------

TEST(RankBonusRepositoryContract, LoadOfAnUnknownOwnerIsEmpty) {
    FakeRankBonusRepository repository;
    EXPECT_TRUE(repository.loadTypes("nobody").empty());
}

TEST(RankBonusRepositoryContract, LoadTypesComeBackTypeAscending) {
    // The query has no ORDER BY, but the covering index (OwnerID, Type)
    // fully serves it, so InnoDB returns Type order — NOT insertion order.
    FakeRankBonusRepository repository;
    repository.insert("Hyanggi", 7);
    repository.insert("Hyanggi", 3);

    std::vector<DWORD> types = repository.loadTypes("Hyanggi");
    ASSERT_EQ(2u, types.size());
    EXPECT_EQ(3u, types[0]);
    EXPECT_EQ(7u, types[1]);
}

TEST(RankBonusRepositoryContract, DuplicateInsertStoresTwoRows) {
    // The table has no primary or unique key: a re-learned bonus that was
    // never cleaned up really does store a second identical row, and the
    // in-memory book is what dedups on load.
    FakeRankBonusRepository repository;
    repository.insert("Hyanggi", 7);
    repository.insert("Hyanggi", 7);

    EXPECT_EQ(2u, repository.loadTypes("Hyanggi").size());
}

TEST(RankBonusRepositoryContract, DeleteOneRemovesEveryRowOfThatType) {
    FakeRankBonusRepository repository;
    repository.insert("Hyanggi", 7);
    repository.insert("Hyanggi", 7); // duplicate row, keyless table
    repository.insert("Hyanggi", 3);

    repository.deleteOne("Hyanggi", 7);

    std::vector<DWORD> types = repository.loadTypes("Hyanggi");
    ASSERT_EQ(1u, types.size());
    EXPECT_EQ(3u, types[0]);
}

TEST(RankBonusRepositoryContract, DeleteAllClearsOnlyThatOwner) {
    FakeRankBonusRepository repository;
    repository.insert("Hyanggi", 7);
    repository.insert("Someone", 7);

    repository.deleteAll("Hyanggi");

    EXPECT_TRUE(repository.loadTypes("Hyanggi").empty());
    EXPECT_EQ(1u, repository.loadTypes("Someone").size());
}

// --- stash-column contract, pinned via the fake ---------------------------

TEST(StashRepositoryContract, NonOustersSaveWritesSlayerAndVampireTables) {
    // The legacy quirk: the Slayer table is written UNCONDITIONALLY, then
    // Vampire for a non-Ousters character — slayers and vampires share one
    // name across those two rows.
    FakeStashRepository repository;
    repository.saveStashNum("Hyanggi", false, 3);

    ASSERT_EQ(2u, repository.writes().size());
    EXPECT_EQ("Slayer", repository.writes()[0].table);
    EXPECT_EQ("Vampire", repository.writes()[1].table);
    EXPECT_EQ("StashNum", repository.writes()[0].column);
    EXPECT_EQ(3, repository.writes()[0].value);
    EXPECT_EQ("Hyanggi", repository.writes()[0].ownerName);
}

TEST(StashRepositoryContract, OustersSaveWritesSlayerAndOustersTables) {
    FakeStashRepository repository;
    repository.saveStashGold("Yerin", true, 5000);

    ASSERT_EQ(2u, repository.writes().size());
    EXPECT_EQ("Slayer", repository.writes()[0].table);
    EXPECT_EQ("Ousters", repository.writes()[1].table);
    EXPECT_EQ("StashGold", repository.writes()[0].column);
    EXPECT_EQ(5000, repository.writes()[0].value);
}

TEST(StashRepositoryContract, GoldAboveIntMaxMarshalsNegativeAndStoresZero) {
    // Gold_t is a DWORD but the SQL interpolation goes through (int): a
    // balance above 2^31-1 emits a NEGATIVE literal — which the UNSIGNED
    // StashGold column then clamps to 0 under the non-strict sql_mode.
    // The balance is destroyed, not stored negative. Unreachable with the
    // MAX_MONEY cap, preserved anyway.
    FakeStashRepository repository;
    repository.addRow(CHARACTER_RACE_SLAYER, "Hyanggi");
    repository.saveStashGold("Hyanggi", false, 4000000000u);

    ASSERT_EQ(2u, repository.writes().size());
    EXPECT_EQ(-294967296, repository.writes()[0].value); // the marshalled literal

    int gold = -1;
    ASSERT_TRUE(repository.loadStashGold("Hyanggi", CHARACTER_RACE_SLAYER, gold));
    EXPECT_EQ(0, gold); // the stored outcome
}

TEST(StashRepositoryContract, SaveAgainstAMissingRowIsASilentNoOp) {
    // An UPDATE against a table with no row for the name matches zero
    // rows: no error, no warning, nothing stored.
    FakeStashRepository repository;
    repository.saveStashGold("Nobody", false, 500);

    int gold = -1;
    EXPECT_FALSE(repository.loadStashGold("Nobody", CHARACTER_RACE_SLAYER, gold));
    EXPECT_EQ(2u, repository.writes().size()); // the attempts still happened
}

TEST(StashRepositoryContract, LoadStashGoldReadsTheCharactersOwnTable) {
    // The integrity check reads ONE table (the character's own race),
    // while the writes fan out to Slayer + the race's own table.
    FakeStashRepository repository;
    repository.addRow(CHARACTER_RACE_SLAYER, "Yerin");
    repository.addRow(CHARACTER_RACE_OUSTERS, "Yerin");
    repository.saveStashGold("Yerin", true, 700);

    int gold = -1;
    ASSERT_TRUE(repository.loadStashGold("Yerin", CHARACTER_RACE_OUSTERS, gold));
    EXPECT_EQ(700, gold);
    ASSERT_TRUE(repository.loadStashGold("Yerin", CHARACTER_RACE_SLAYER, gold));
    EXPECT_EQ(700, gold); // the unconditional Slayer write landed too
    EXPECT_FALSE(repository.loadStashGold("Yerin", CHARACTER_RACE_VAMPIRE, gold));
}

// --- BloodBibleSignObject contract, pinned via the fake -------------------

TEST(BloodBibleSignRepositoryContract, LoadOfAnUnknownOwnerIsEmpty) {
    FakeBloodBibleSignRepository repository;
    EXPECT_TRUE(repository.loadItemTypes("nobody").empty());
}

TEST(BloodBibleSignRepositoryContract, LoadReturnsItemTypeOrderNotInsertionOrder) {
    // The query carries ORDER BY ItemType — the sign list the client gets
    // is sorted regardless of grant order.
    FakeBloodBibleSignRepository repository;
    repository.addRow("Hyanggi", 5);
    repository.addRow("Hyanggi", 2);
    repository.addRow("Hyanggi", 9);

    std::vector<ItemType_t> itemTypes = repository.loadItemTypes("Hyanggi");
    ASSERT_EQ(3u, itemTypes.size());
    EXPECT_EQ(2, itemTypes[0]);
    EXPECT_EQ(5, itemTypes[1]);
    EXPECT_EQ(9, itemTypes[2]);
}

TEST(BloodBibleSignRepositoryContract, DuplicateItemTypesSurvive) {
    // The table's key is an unrelated auto-increment: duplicate signs are
    // storable and come back as-is.
    FakeBloodBibleSignRepository repository;
    repository.addRow("Hyanggi", 5);
    repository.addRow("Hyanggi", 5);

    EXPECT_EQ(2u, repository.loadItemTypes("Hyanggi").size());
}

TEST(BloodBibleSignRepositoryContract, OwnersAreIsolated) {
    FakeBloodBibleSignRepository repository;
    repository.addRow("Hyanggi", 5);

    EXPECT_TRUE(repository.loadItemTypes("Someone").empty());
}

// --- GoodsListObject contract, pinned via the fake ------------------------

TEST(GoodsRepositoryContract, PendingIsFilteredByWorldPlayerAndName) {
    FakeGoodsRepository repository;
    repository.addPurchase("101", 1, "account", "Hyanggi", 5000, 2);
    repository.addPurchase("102", 2, "account", "Hyanggi", 5000, 2); // other world
    repository.addPurchase("103", 1, "other", "Hyanggi", 5000, 2);   // other account
    repository.addPurchase("104", 1, "account", "Yerin", 5000, 2);   // other character

    std::vector<GoodsRecord> records = repository.loadPending(1, "account", "Hyanggi");
    ASSERT_EQ(1u, records.size());
    EXPECT_EQ("101", records[0].id);
    EXPECT_EQ(5000u, records[0].goodsID);
    EXPECT_EQ(2, records[0].num);
}

TEST(GoodsRepositoryContract, TakeOneDecrementsAndFlipsStatusOnTheLastUnit) {
    // The single UPDATE's IF() reads the ALREADY-DECREMENTED Num (MySQL
    // left-to-right SET evaluation): the row leaves 'NOT' in the same
    // statement that takes its last unit.
    FakeGoodsRepository repository;
    repository.addPurchase("101", 1, "account", "Hyanggi", 5000, 2);

    EXPECT_TRUE(repository.takeOne("101"));
    ASSERT_EQ(1u, repository.loadPending(1, "account", "Hyanggi").size());
    EXPECT_EQ(1, repository.loadPending(1, "account", "Hyanggi")[0].num);

    EXPECT_TRUE(repository.takeOne("101"));
    EXPECT_TRUE(repository.loadPending(1, "account", "Hyanggi").empty());
}

TEST(GoodsRepositoryContract, TakeOneOfAnUnknownIdIsFalse) {
    FakeGoodsRepository repository;
    EXPECT_FALSE(repository.takeOne("999"));
}

TEST(GoodsRepositoryContract, TakingAZeroCountRowThrowsAndLeavesItStuck) {
    // A pending row can sit at Num=0 (the loader still delivers one item:
    // its loop runs max(1, min(50, num)) times), but taking it FAILS:
    // Num - 1 on the UNSIGNED column raises ER_DATA_OUT_OF_RANGE
    // regardless of strict mode, and the row is left untouched — so the
    // purchase stays pending and is re-delivered on the next load (the
    // pre-existing stuck-item bug documented on the MySQL impl).
    FakeGoodsRepository repository;
    repository.addPurchase("101", 1, "account", "Hyanggi", 5000, 0);

    EXPECT_THROW(repository.takeOne("101"), std::runtime_error);

    std::vector<GoodsRecord> records = repository.loadPending(1, "account", "Hyanggi");
    ASSERT_EQ(1u, records.size());
    EXPECT_EQ(0, records[0].num);
}

// --- carried-gold contract, pinned via the fake ---------------------------

TEST(GoldRepositoryContract, IncreaseIsRelativeOnTheRowBalance) {
    FakeGoldRepository repository;
    repository.addRow(CHARACTER_RACE_SLAYER, "Hyanggi", 100);

    repository.increaseGold("Hyanggi", CHARACTER_RACE_SLAYER, 50);

    int gold = -1;
    ASSERT_TRUE(repository.loadGold("Hyanggi", CHARACTER_RACE_SLAYER, gold));
    EXPECT_EQ(150, gold);
}

TEST(GoldRepositoryContract, OperationsTargetOnlyTheOwnRaceTable) {
    // Unlike the stash writes, gold has NO Slayer fan-out: a slayer's
    // twin Vampire row keeps its own balance.
    FakeGoldRepository repository;
    repository.addRow(CHARACTER_RACE_SLAYER, "Hyanggi", 2000);
    repository.addRow(CHARACTER_RACE_VAMPIRE, "Hyanggi", 0);

    repository.increaseGold("Hyanggi", CHARACTER_RACE_SLAYER, 500);

    int gold = -1;
    ASSERT_TRUE(repository.loadGold("Hyanggi", CHARACTER_RACE_SLAYER, gold));
    EXPECT_EQ(2500, gold);
    ASSERT_TRUE(repository.loadGold("Hyanggi", CHARACTER_RACE_VAMPIRE, gold));
    EXPECT_EQ(0, gold);
}

TEST(GoldRepositoryContract, IncreaseAgainstAMissingRowIsASilentNoOp) {
    FakeGoldRepository repository;
    repository.increaseGold("Nobody", CHARACTER_RACE_SLAYER, 500);

    int gold = -1;
    EXPECT_FALSE(repository.loadGold("Nobody", CHARACTER_RACE_SLAYER, gold));
}

TEST(GoldRepositoryContract, TheClampedGuildFeeEmptiesARowThatCannotPayInsteadOfThrowing) {
    FakeGoldRepository repository;
    repository.addRow(CHARACTER_RACE_VAMPIRE, "payer", 1000);

    repository.decreaseGoldClamped("payer", CHARACTER_RACE_VAMPIRE, 400);
    int gold = -1;
    ASSERT_TRUE(repository.loadGold("payer", CHARACTER_RACE_VAMPIRE, gold));
    EXPECT_EQ(600, gold);

    // Where decreaseGold would raise, the clamped write empties the row.
    repository.decreaseGoldClamped("payer", CHARACTER_RACE_VAMPIRE, 5000);
    ASSERT_TRUE(repository.loadGold("payer", CHARACTER_RACE_VAMPIRE, gold));
    EXPECT_EQ(0, gold);

    // Still a silent no-op against a row that is not there.
    repository.decreaseGoldClamped("ghost", CHARACTER_RACE_SLAYER, 100);
    EXPECT_FALSE(repository.loadGold("ghost", CHARACTER_RACE_SLAYER, gold));
}

TEST(GoldRepositoryContract, DecreaseBelowTheRowBalanceThrowsAndLeavesTheRowUntouched) {
    // The caller clamps the delta against its IN-MEMORY balance; when the
    // row holds less (integrity drift), the unsigned subtraction raises
    // ER_DATA_OUT_OF_RANGE and the row is untouched — the same failure
    // shape as taking a Num=0 goods row.
    FakeGoldRepository repository;
    repository.addRow(CHARACTER_RACE_VAMPIRE, "Hyanggi", 10);

    EXPECT_THROW(repository.decreaseGold("Hyanggi", CHARACTER_RACE_VAMPIRE, 50), std::runtime_error);

    int gold = -1;
    ASSERT_TRUE(repository.loadGold("Hyanggi", CHARACTER_RACE_VAMPIRE, gold));
    EXPECT_EQ(10, gold);
}

} // namespace
