// The port check's name rule and the record behind it (PortCheck.h): a
// name character creation would refuse, or one no statement may carry, is
// refused with InvalidProtocolException before the repository is asked,
// and an accepted one is recorded as sent.
// The repository's own escaping of the name is make integration-test's.

#include <string>

#include <gtest/gtest.h>

#include "Exception.h"
#include "FakeSessionRepository.h"
#include "PortCheck.h"

using de::portCheckNameRejection;
using de::PortCheckRejection;
using de::recordPortCheck;

TEST(PortCheckName, ACharactersNameIsAccepted) {
    EXPECT_FALSE(portCheckNameRejection("Rowan").has_value());
    EXPECT_FALSE(portCheckNameRejection("a").has_value());
    EXPECT_FALSE(portCheckNameRejection(std::string(20, 'x')).has_value());
    // Korean as the client sends it: UTF-8 bytes above 0x7F; a four-byte
    // character too.
    EXPECT_FALSE(portCheckNameRejection("\xed\x95\x9c\xea\xb8\x80").has_value());
    EXPECT_FALSE(portCheckNameRejection("\xf0\x9f\x98\x80").has_value());
    // A quote, a backslash and a space are the repository's to escape, not
    // the rule's to refuse: character creation accepts them.
    EXPECT_FALSE(portCheckNameRejection("O'Brien \\ x").has_value());
}

TEST(PortCheckName, AnEmptyOrOverlongNameIsRefused) {
    EXPECT_EQ(PortCheckRejection::EmptyName, portCheckNameRejection(""));
    EXPECT_EQ(PortCheckRejection::NameTooLong, portCheckNameRejection(std::string(21, 'x')));
}

TEST(PortCheckName, AControlByteIsRefused) {
    EXPECT_EQ(PortCheckRejection::ControlByte, portCheckNameRejection(std::string("Ro\0wan", 6)));
    EXPECT_EQ(PortCheckRejection::ControlByte, portCheckNameRejection("Ro\x01wan"));
    EXPECT_EQ(PortCheckRejection::ControlByte, portCheckNameRejection("Rowan\n"));
    EXPECT_EQ(PortCheckRejection::ControlByte, portCheckNameRejection("Rowan\x7f"));
}

TEST(PortCheckName, BytesThatAreNoUtf8AreRefused) {
    // No lead byte, a continuation byte alone, a sequence cut short, an
    // overlong form, a surrogate, a value past U+10FFFF.
    EXPECT_EQ(PortCheckRejection::NotUtf8, portCheckNameRejection("Ro\xffwan"));
    EXPECT_EQ(PortCheckRejection::NotUtf8, portCheckNameRejection("Ro\x80wan"));
    EXPECT_EQ(PortCheckRejection::NotUtf8, portCheckNameRejection("Rowan\xed\x95"));
    EXPECT_EQ(PortCheckRejection::NotUtf8, portCheckNameRejection("\xc0\xaf"));
    EXPECT_EQ(PortCheckRejection::NotUtf8, portCheckNameRejection("\xed\xa0\x80"));
    EXPECT_EQ(PortCheckRejection::NotUtf8, portCheckNameRejection("\xf4\x90\x80\x80"));
}

TEST(PortCheckName, AReservedTokenIsRefusedAsCreationRefusesIt) {
    EXPECT_EQ(PortCheckRejection::ReservedName, portCheckNameRejection("aGMb"));
    EXPECT_EQ(PortCheckRejection::ReservedName, portCheckNameRejection("NONE"));
    EXPECT_EQ(PortCheckRejection::ReservedName, portCheckNameRejection("\xec\xa7\x81\xec\x9b\x90"
                                                                       "1"));
}

TEST(PortCheckName, AControlByteIsRefusedBeforeTheTokensAreLookedFor) {
    // A NUL would end the C string the token search reads.
    EXPECT_EQ(PortCheckRejection::ControlByte, portCheckNameRejection(std::string("x\0GM", 4)));
}

TEST(RecordPortCheck, AnAcceptedNameIsRecordedAsSent) {
    FakeSessionRepository repository;

    recordPortCheck(repository, "O'Brien", 0x0100007fu, 9858, 31000);

    ASSERT_EQ(1u, repository.userIPs.size());
    EXPECT_EQ("O'Brien", repository.userIPs[0].name);
    EXPECT_EQ(0x0100007fu, repository.userIPs[0].ip);
    EXPECT_EQ(9858u, repository.userIPs[0].port);
    EXPECT_EQ(31000, repository.userIPs[0].serverID);
}

TEST(RecordPortCheck, ARefusedNameThrowsBeforeTheRepositoryIsAsked) {
    FakeSessionRepository repository;

    EXPECT_THROW(recordPortCheck(repository, "Rowan\n", 1, 2, 3), InvalidProtocolException);
    EXPECT_THROW(recordPortCheck(repository, "aGMb", 1, 2, 3), InvalidProtocolException);
    EXPECT_THROW(recordPortCheck(repository, "Ro\xffwan", 1, 2, 3), InvalidProtocolException);
    EXPECT_THROW(recordPortCheck(repository, std::string("x\0y", 3), 1, 2, 3), InvalidProtocolException);
    EXPECT_THROW(recordPortCheck(repository, "", 1, 2, 3), InvalidProtocolException);

    EXPECT_TRUE(repository.userIPs.empty());
}
