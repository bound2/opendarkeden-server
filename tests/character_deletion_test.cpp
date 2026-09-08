// The loginserver's character-deletion decision
// (src/server/loginserver/CharacterDeletion.cpp): every rejection with the
// input that triggers it, the precedence between them, and what an
// accepted deletion has already done to the Slayer row that indexes the
// character. The repository is a fake, so no database is involved; the
// MySQL-backed integration tier
// (tests/integration/mysql_loginserver_repository_test.cpp) is the
// authority on what the real repository answers.

#include <string>

#include <gtest/gtest.h>

#include "CharacterDeletion.h"
#include "FakeLoginCharacterPurgeRepository.h"

namespace {

// A request that is accepted as-is once the repository holds the matching
// character.
DeletePCRequest rowanRequest() {
    DeletePCRequest request;
    request.worldID = 1;
    request.playerID = "account";
    request.name = "Rowan";
    request.slot = SLOT2;
    return request;
}

// The character the request above names: on "account", in SLOT2.
void addRowan(FakeLoginCharacterPurgeRepository& repository) {
    repository.addActiveSlayer("Rowan", "account", SLOT2);
}

// --- rejections -----------------------------------------------------------

TEST(DecideDeletePC, ANameWithNoActiveSlayerRowIsRefused) {
    FakeLoginCharacterPurgeRepository repository;

    Outcome<void, DeletePCRejection> outcome = decideDeletePC(rowanRequest(), repository);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(DeletePCRejection::NoSuchCharacter, outcome.rejection());
    EXPECT_EQ(0, repository.retireSlayerCalls);
    EXPECT_TRUE(repository.purges.empty());
}

TEST(DecideDeletePC, ACharacterOnAnotherAccountIsRefusedBeforeTheRetirement) {
    FakeLoginCharacterPurgeRepository repository;
    repository.addActiveSlayer("Rowan", "someone-else", SLOT2);

    Outcome<void, DeletePCRejection> outcome = decideDeletePC(rowanRequest(), repository);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(DeletePCRejection::NotTheOwner, outcome.rejection());
    // The row still stands: nothing was retired on another account's
    // character.
    EXPECT_EQ(0, repository.retireSlayerCalls);
    EXPECT_EQ(1u, repository.activeSlayers.size());
}

TEST(DecideDeletePC, TheOwnerComparisonIsExact) {
    FakeLoginCharacterPurgeRepository repository;
    // A prefix of the owning account is not the owning account.
    repository.addActiveSlayer("Rowan", "accountant", SLOT2);

    Outcome<void, DeletePCRejection> outcome = decideDeletePC(rowanRequest(), repository);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(DeletePCRejection::NotTheOwner, outcome.rejection());
}

TEST(DecideDeletePC, ACharacterInAnotherSlotIsRefusedByTheRetirement) {
    FakeLoginCharacterPurgeRepository repository;
    repository.addActiveSlayer("Rowan", "account", SLOT3);

    Outcome<void, DeletePCRejection> outcome = decideDeletePC(rowanRequest(), repository);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(DeletePCRejection::SlotMismatch, outcome.rejection());
    // The retirement was attempted, and changed nothing.
    EXPECT_EQ(1, repository.retireSlayerCalls);
    EXPECT_EQ(1u, repository.activeSlayers.size());
}

TEST(DecideDeletePC, ARetirementThatChangesNoRowLeavesNothingPurged) {
    FakeLoginCharacterPurgeRepository repository;
    repository.addActiveSlayer("Rowan", "account", SLOT3);

    Outcome<void, DeletePCRejection> outcome = decideDeletePC(rowanRequest(), repository);

    ASSERT_TRUE(outcome.isRejected());
    // The purge belongs to the caller and is reached only on Ok, so a
    // refused deletion cannot have started one.
    EXPECT_TRUE(repository.purges.empty());
    EXPECT_TRUE(repository.recordedDeletions.empty());
}

// --- an accepted deletion --------------------------------------------------

TEST(DecideDeletePC, TheOwnersCharacterInTheNamedSlotIsAccepted) {
    FakeLoginCharacterPurgeRepository repository;
    addRowan(repository);

    Outcome<void, DeletePCRejection> outcome = decideDeletePC(rowanRequest(), repository);

    EXPECT_TRUE(outcome.isOk());
}

TEST(DecideDeletePC, AnAcceptedDeletionHasAlreadyRetiredTheSlayerRow) {
    FakeLoginCharacterPurgeRepository repository;
    addRowan(repository);

    Outcome<void, DeletePCRejection> outcome = decideDeletePC(rowanRequest(), repository);

    ASSERT_TRUE(outcome.isOk());
    ASSERT_EQ(1u, repository.retirements.size());
    EXPECT_EQ(1, (int)repository.retirements[0].worldID);
    EXPECT_EQ("Rowan", repository.retirements[0].name);
    EXPECT_EQ(SLOT2, repository.retirements[0].slot);
    EXPECT_TRUE(repository.activeSlayers.empty());
}

TEST(DecideDeletePC, TheDecisionItselfPurgesNothing) {
    FakeLoginCharacterPurgeRepository repository;
    addRowan(repository);

    Outcome<void, DeletePCRejection> outcome = decideDeletePC(rowanRequest(), repository);

    ASSERT_TRUE(outcome.isOk());
    // The DeleteChar record and the 112-statement purge are the handler's,
    // so an accepted decision has made neither.
    EXPECT_TRUE(repository.recordedDeletions.empty());
    EXPECT_TRUE(repository.purges.empty());
    EXPECT_TRUE(repository.destroyedItemOwners.empty());
}

TEST(DecideDeletePC, DeletingTheSameCharacterTwiceIsRefusedTheSecondTime) {
    FakeLoginCharacterPurgeRepository repository;
    addRowan(repository);

    Outcome<void, DeletePCRejection> first = decideDeletePC(rowanRequest(), repository);
    ASSERT_TRUE(first.isOk());

    Outcome<void, DeletePCRejection> second = decideDeletePC(rowanRequest(), repository);

    ASSERT_TRUE(second.isRejected());
    // The row is no longer ACTIVE, so the ownership read is what refuses.
    EXPECT_EQ(DeletePCRejection::NoSuchCharacter, second.rejection());
}

TEST(DecideDeletePC, EverySlotCanBeDeleted) {
    const Slot slots[] = {SLOT1, SLOT2, SLOT3};

    for (size_t i = 0; i < sizeof(slots) / sizeof(slots[0]); i++) {
        FakeLoginCharacterPurgeRepository repository;
        repository.addActiveSlayer("Rowan", "account", slots[i]);

        DeletePCRequest request = rowanRequest();
        request.slot = slots[i];

        Outcome<void, DeletePCRejection> outcome = decideDeletePC(request, repository);

        EXPECT_TRUE(outcome.isOk()) << "slot " << (int)slots[i];
    }
}

TEST(DecideDeletePC, TheWorldIdIsPassedThroughToBothStatements) {
    FakeLoginCharacterPurgeRepository repository;
    addRowan(repository);

    DeletePCRequest request = rowanRequest();
    request.worldID = 9;

    Outcome<void, DeletePCRejection> outcome = decideDeletePC(request, repository);

    ASSERT_TRUE(outcome.isOk());
    ASSERT_EQ(1u, repository.ownerLookupWorldIDs.size());
    EXPECT_EQ(9, (int)repository.ownerLookupWorldIDs[0]);
    ASSERT_EQ(1u, repository.retirements.size());
    EXPECT_EQ(9, (int)repository.retirements[0].worldID);
}

} // namespace
