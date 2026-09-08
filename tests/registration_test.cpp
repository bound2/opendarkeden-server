// The loginserver's account-registration decision
// (src/server/loginserver/Registration.cpp): every refusal with the input
// that triggers it, the precedence between them, and the Player row an
// accepted registration hands to the caller. The repository is a fake, so
// no database is involved; the MySQL-backed integration tier
// (tests/integration/mysql_loginserver_repository_test.cpp) is the
// authority on what the real repository answers.
//
// The password hashing is the real argon2id, which costs 64 MiB per call,
// so the cases that reach it are kept few. Neither a password nor a hash is
// ever passed to a gtest matcher that would print it on failure.

#include <cstddef>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "FakeLoginAccountRepository.h"
#include "PasswordHash.h"
#include "Registration.h"

namespace {

const char* const kPassword = "correct horse";

// A request every field of which is acceptable.
RegisterPlayerRequest goodRequest() {
    RegisterPlayerRequest request;
    request.playerID = "newcomer";
    request.password = kPassword;
    request.name = "Ada Lovelace";
    request.sex = FEMALE;
    request.ssn = "800101-1234567";
    request.telephone = "02-000-0000";
    request.cellular = "010-0000-0000";
    request.zipCode = "123-456";
    request.address = "1 Analytical Engine Way";
    request.nation = 1;
    request.email = "ada@example.com";
    request.homepage = "http://example.com";
    request.profile = "counts things";
    request.publicProfile = true;
    return request;
}

// Refuses without asking the repository anything, and — for everything
// decided before the hashing — without paying for an argon2 call.
void expectRefusedBeforeAnyRead(const RegisterPlayerRequest& request, RegisterPlayerRejection reason) {
    FakeLoginAccountRepository repository;

    Outcome<LoginNewAccount, RegisterPlayerRefusal> outcome = decideRegisterPlayer(request, repository);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(reason, outcome.rejection().reason);
    EXPECT_EQ(0, repository.accountExistsCalls);
    EXPECT_TRUE(repository.insertedAccounts.empty());
}

// --- the validation refusals ----------------------------------------------

TEST(DecideRegisterPlayer, AnEmptyIDIsRefused) {
    RegisterPlayerRequest request = goodRequest();
    request.playerID = "";
    expectRefusedBeforeAnyRead(request, RegisterPlayerRejection::EmptyID);
}

TEST(DecideRegisterPlayer, AnIDShorterThanFourCharactersIsRefused) {
    RegisterPlayerRequest request = goodRequest();
    request.playerID = "abc";
    expectRefusedBeforeAnyRead(request, RegisterPlayerRejection::ShortID);
}

TEST(DecideRegisterPlayer, TheShortestAcceptedIDIsFourCharacters) {
    RegisterPlayerRequest request = goodRequest();
    request.playerID = "abcd";
    // Not refused for its length: the next thing it meets is the hashing,
    // then the id probe, and an unknown id is accepted.
    FakeLoginAccountRepository repository;

    Outcome<LoginNewAccount, RegisterPlayerRefusal> outcome = decideRegisterPlayer(request, repository);

    EXPECT_TRUE(outcome.isOk());
}

TEST(DecideRegisterPlayer, EverySqlMetaCharacterInTheIDIsRefused) {
    const char* const ids[] = {"new'comer", "new\\comer", "new\"comer", "new;comer"};

    for (size_t i = 0; i < sizeof(ids) / sizeof(ids[0]); i++) {
        RegisterPlayerRequest request = goodRequest();
        request.playerID = ids[i];
        expectRefusedBeforeAnyRead(request, RegisterPlayerRejection::InvalidID);
    }
}

TEST(DecideRegisterPlayer, AnEmptyPasswordIsRefused) {
    RegisterPlayerRequest request = goodRequest();
    request.password = "";
    expectRefusedBeforeAnyRead(request, RegisterPlayerRejection::EmptyPassword);
}

TEST(DecideRegisterPlayer, APasswordShorterThanSixCharactersIsRefused) {
    RegisterPlayerRequest request = goodRequest();
    request.password = "abcde";
    expectRefusedBeforeAnyRead(request, RegisterPlayerRejection::ShortPassword);
}

TEST(DecideRegisterPlayer, AnEmptyNameIsRefused) {
    RegisterPlayerRequest request = goodRequest();
    request.name = "";
    expectRefusedBeforeAnyRead(request, RegisterPlayerRejection::EmptyName);
}

TEST(DecideRegisterPlayer, AnEmptyRegistrationNumberIsRefused) {
    RegisterPlayerRequest request = goodRequest();
    request.ssn = "";
    expectRefusedBeforeAnyRead(request, RegisterPlayerRejection::EmptySSN);
}

TEST(DecideRegisterPlayer, EveryProfileFieldIsCheckedForSqlMetaCharacters) {
    RegisterPlayerRequest fields[9];
    for (size_t i = 0; i < 9; i++)
        fields[i] = goodRequest();

    fields[0].name += "'";
    fields[1].ssn += "'";
    fields[2].telephone += "'";
    fields[3].cellular += "'";
    fields[4].zipCode += "'";
    fields[5].address += "'";
    fields[6].email += "'";
    fields[7].homepage += "'";
    fields[8].profile += "'";

    for (size_t i = 0; i < 9; i++)
        expectRefusedBeforeAnyRead(fields[i], RegisterPlayerRejection::InvalidProfileField);
}

TEST(DecideRegisterPlayer, TheIDIsCheckedBeforeThePassword) {
    RegisterPlayerRequest request = goodRequest();
    request.playerID = "";
    request.password = "";
    expectRefusedBeforeAnyRead(request, RegisterPlayerRejection::EmptyID);
}

TEST(DecideRegisterPlayer, ThePasswordIsCheckedBeforeTheName) {
    RegisterPlayerRequest request = goodRequest();
    request.password = "";
    request.name = "";
    expectRefusedBeforeAnyRead(request, RegisterPlayerRejection::EmptyPassword);
}

TEST(DecideRegisterPlayer, AnEmptyNameIsRefusedForBeingEmptyNotForItsCharacters) {
    RegisterPlayerRequest request = goodRequest();
    request.name = "";
    request.profile += "'";
    expectRefusedBeforeAnyRead(request, RegisterPlayerRejection::EmptyName);
}

// --- the id probe -----------------------------------------------------------

TEST(DecideRegisterPlayer, AnIDThatIsTakenIsRefused) {
    FakeLoginAccountRepository repository;
    repository.registeredIDs.insert("newcomer");

    Outcome<LoginNewAccount, RegisterPlayerRefusal> outcome = decideRegisterPlayer(goodRequest(), repository);

    ASSERT_TRUE(outcome.isRejected());
    EXPECT_EQ(RegisterPlayerRejection::AlreadyRegistered, outcome.rejection().reason);
    // The decision writes nothing: the INSERT is the caller's.
    EXPECT_TRUE(repository.insertedAccounts.empty());
}

TEST(DecideRegisterPlayer, TheIDProbeRunsOnceAndOnlyAfterTheValidation) {
    FakeLoginAccountRepository repository;

    Outcome<LoginNewAccount, RegisterPlayerRefusal> outcome = decideRegisterPlayer(goodRequest(), repository);

    ASSERT_TRUE(outcome.isOk());
    EXPECT_EQ(1, repository.accountExistsCalls);
}

// --- an accepted registration ----------------------------------------------

TEST(DecideRegisterPlayer, TheAcceptedRowCarriesThePacketsFields) {
    FakeLoginAccountRepository repository;
    const RegisterPlayerRequest request = goodRequest();

    Outcome<LoginNewAccount, RegisterPlayerRefusal> outcome = decideRegisterPlayer(request, repository);

    ASSERT_TRUE(outcome.isOk());
    const LoginNewAccount account = std::move(outcome).events();

    EXPECT_EQ("newcomer", account.playerID);
    EXPECT_EQ("Ada Lovelace", account.name);
    EXPECT_EQ("FEMALE", account.sex);
    EXPECT_EQ("800101-1234567", account.ssn);
    EXPECT_EQ("02-000-0000", account.telephone);
    EXPECT_EQ("010-0000-0000", account.cellular);
    EXPECT_EQ("123-456", account.zipCode);
    EXPECT_EQ("1 Analytical Engine Way", account.address);
    EXPECT_EQ(1, account.nation);
    EXPECT_EQ("ada@example.com", account.email);
    EXPECT_EQ("http://example.com", account.homepage);
    EXPECT_EQ("counts things", account.profile);
    EXPECT_EQ("PUBLIC", account.pub);
}

TEST(DecideRegisterPlayer, TheStoredPasswordIsAnArgon2idHashOfWhatWasSent) {
    FakeLoginAccountRepository repository;
    const RegisterPlayerRequest request = goodRequest();

    Outcome<LoginNewAccount, RegisterPlayerRefusal> outcome = decideRegisterPlayer(request, repository);

    ASSERT_TRUE(outcome.isOk());
    const LoginNewAccount account = std::move(outcome).events();

    // Neither value is handed to a matcher that would print it.
    EXPECT_FALSE(account.password == request.password);
    EXPECT_TRUE(de::password::isHashed(account.password));
    EXPECT_TRUE(de::password::verify(account.password, request.password) == de::password::Verify::Accepted);
    EXPECT_TRUE(de::password::verify(account.password, "wrong password") == de::password::Verify::Rejected);
}

TEST(DecideRegisterPlayer, APrivateProfileIsSpelledPRIVATE) {
    FakeLoginAccountRepository repository;
    RegisterPlayerRequest request = goodRequest();
    request.publicProfile = false;
    request.sex = MALE;

    Outcome<LoginNewAccount, RegisterPlayerRefusal> outcome = decideRegisterPlayer(request, repository);

    ASSERT_TRUE(outcome.isOk());
    const LoginNewAccount account = std::move(outcome).events();
    EXPECT_EQ("PRIVATE", account.pub);
    EXPECT_EQ("MALE", account.sex);
}

} // namespace
