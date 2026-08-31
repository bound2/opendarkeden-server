// Task 3.2 pilot: pins the NicknameRepository contract and the pure
// nickname-book rules extracted from NicknameBook::load(). The fake stands
// in for the MySQL implementation in domain tests; a MySQL-backed
// integration tier is a later task.

#include <gtest/gtest.h>

#include "FakeNicknameRepository.h"
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

} // namespace
