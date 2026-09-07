// Account password hashing (src/server/loginserver/PasswordHash.cpp) over
// the vendored argon2 reference implementation (third_party/argon2).
//
// The known-answer strings are upstream's own (src/test.c in the 20190702
// release), so they pin the vendored library as well as the wrapper. The
// seed-account case reads initdb/DARKEDEN.sql and checks that every shipped
// Player row carries a current argon2id hash of its documented password.

#include <fstream>
#include <map>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "PasswordHash.h"

using de::password::Verify;

namespace {

// Upstream vectors: password "password", salt "somesalt", version 0x13.
constexpr const char* kUpstreamId_m65536_t2_p1 =
    "$argon2id$v=19$m=65536,t=2,p=1$c29tZXNhbHQ$CTFhFdXPJO1aFaMaO6Mm5c8y7cJHAph8ArZWb2GRPPc";
constexpr const char* kUpstreamId_m256_t2_p2 =
    "$argon2id$v=19$m=256,t=2,p=2$c29tZXNhbHQ$bQk8UB/VmZZF4Oo79iDXuL5/0ttZwg2f/5U52iv1cDc";
constexpr const char* kUpstreamI_m65536_t2_p1 =
    "$argon2i$v=19$m=65536,t=2,p=1$c29tZXNhbHQ$wWKIMhR9lyDFvRz9YTZweHKfbftvj+qf+YFY4NeBbtA";
// password "differentpassword", same salt.
constexpr const char* kUpstreamId_differentpassword =
    "$argon2id$v=19$m=65536,t=2,p=1$c29tZXNhbHQ$C4TWUs9rDEvq7w3+J4umqA32aWKB1+DSiRuBfYxFj94";

const std::string kCurrentPrefix = "$argon2id$v=19$m=65536,t=3,p=1$";

} // namespace

TEST(PasswordHash, HashRoundTrip) {
    const std::string hashed = de::password::hash("correct horse battery staple");

    EXPECT_TRUE(de::password::isHashed(hashed));
    EXPECT_EQ(hashed.substr(0, kCurrentPrefix.size()), kCurrentPrefix);
    EXPECT_EQ(hashed.size(), de::password::encodedLength());

    EXPECT_EQ(de::password::verify(hashed, "correct horse battery staple"), Verify::Accepted);
    EXPECT_EQ(de::password::verify(hashed, "correct horse battery stapl"), Verify::Rejected);
    EXPECT_EQ(de::password::verify(hashed, "correct horse battery staple "), Verify::Rejected);
    EXPECT_EQ(de::password::verify(hashed, "Correct horse battery staple"), Verify::Rejected);
    EXPECT_EQ(de::password::verify(hashed, ""), Verify::Rejected);
}

TEST(PasswordHash, FreshSaltPerHash) {
    EXPECT_NE(de::password::hash("same"), de::password::hash("same"));
}

TEST(PasswordHash, EncodedLengthFitsTheColumn) {
    // Player.Password is varchar(255) since the argon2 change.
    EXPECT_LE(de::password::encodedLength(), 255u);
    // Prefix + 22 base64 chars of a 16-byte salt + '$' + 43 of a 32-byte hash.
    EXPECT_EQ(de::password::encodedLength(), kCurrentPrefix.size() + 22 + 1 + 43);
}

TEST(PasswordHash, HashesAreSqlLiteralSafe) {
    const std::string hashed = de::password::hash("p'a\\s\"s;word");
    EXPECT_EQ(hashed.find_first_of("'\\\";"), std::string::npos);
    EXPECT_EQ(de::password::verify(hashed, "p'a\\s\"s;word"), Verify::Accepted);
}

TEST(PasswordHash, UpstreamKnownAnswers) {
    // Correct password, but t=2 rather than the current t=3: accepted with a
    // rehash request, never a plain Accepted.
    EXPECT_EQ(de::password::verify(kUpstreamId_m65536_t2_p1, "password"), Verify::AcceptedRehash);
    EXPECT_EQ(de::password::verify(kUpstreamId_m256_t2_p2, "password"), Verify::AcceptedRehash);
    EXPECT_EQ(de::password::verify(kUpstreamId_differentpassword, "differentpassword"), Verify::AcceptedRehash);

    // A hash made by the plain `argon2` CLI (argon2i by default) is honoured
    // and upgraded to argon2id on its next login.
    EXPECT_EQ(de::password::verify(kUpstreamI_m65536_t2_p1, "password"), Verify::AcceptedRehash);

    EXPECT_EQ(de::password::verify(kUpstreamId_m65536_t2_p1, "differentpassword"), Verify::Rejected);
    EXPECT_EQ(de::password::verify(kUpstreamId_differentpassword, "password"), Verify::Rejected);
    EXPECT_EQ(de::password::verify(kUpstreamI_m65536_t2_p1, "Password"), Verify::Rejected);
}

TEST(PasswordHash, LegacyPlaintextRows) {
    EXPECT_FALSE(de::password::isHashed("111111"));
    EXPECT_EQ(de::password::verify("111111", "111111"), Verify::AcceptedRehash);
    EXPECT_EQ(de::password::verify("111111", "111112"), Verify::Rejected);
    EXPECT_EQ(de::password::verify("111111", "11111"), Verify::Rejected);
    EXPECT_EQ(de::password::verify("111111", "1111111"), Verify::Rejected);
    // The old WHERE clause compared case-insensitively (latin1 collation);
    // the C++ comparison is exact.
    EXPECT_EQ(de::password::verify("Secret", "secret"), Verify::Rejected);
}

TEST(PasswordHash, EmptyStoredValueNeverMatches) {
    EXPECT_EQ(de::password::verify("", ""), Verify::Rejected);
    EXPECT_EQ(de::password::verify("", "x"), Verify::Rejected);
}

TEST(PasswordHash, MalformedEncodingsAreRejected) {
    EXPECT_EQ(de::password::verify("$argon2id$", "password"), Verify::Rejected);
    EXPECT_EQ(de::password::verify("$argon2id$v=19$m=65536,t=2,p=1$c29tZXNhbHQ", "password"), Verify::Rejected);
    EXPECT_EQ(de::password::verify(
                  "$argon2id$v=19$m=65536,t=2,p=1$c29tZXNhbHQ$CTFhFdXPJO1aFaMaO6Mm5c8y7cJHAph8ArZWb2GRPP", "password"),
              Verify::Rejected);
    EXPECT_EQ(de::password::verify(
                  "$argon2x$v=19$m=65536,t=2,p=1$c29tZXNhbHQ$CTFhFdXPJO1aFaMaO6Mm5c8y7cJHAph8ArZWb2GRPPc", "password"),
              Verify::Rejected);
    // A plaintext row that merely resembles the prefix is not a hash.
    EXPECT_EQ(de::password::verify("$argon2", "$argon2"), Verify::Rejected);
}

namespace {

// Splits one `INSERT INTO ... VALUES (...),(...);` line into its tuples,
// each tuple into its top-level fields, honouring quotes and backslash
// escapes. Only the first two fields are needed, but the whole tuple has to
// be walked to find where it ends.
std::vector<std::vector<std::string>> parseInsertTuples(const std::string& line) {
    std::vector<std::vector<std::string>> tuples;
    const std::string::size_type values = line.find(" VALUES ");
    if (values == std::string::npos)
        return tuples;

    std::vector<std::string> fields;
    std::string field;
    bool inTuple = false, inQuote = false;
    for (std::string::size_type i = values + 8; i < line.size(); ++i) {
        const char c = line[i];
        if (inQuote) {
            if (c == '\\' && i + 1 < line.size()) {
                field += line[++i];
            } else if (c == '\'') {
                inQuote = false;
            } else {
                field += c;
            }
        } else if (c == '\'') {
            inQuote = true;
        } else if (c == '(' && !inTuple) {
            inTuple = true;
            fields.clear();
            field.clear();
        } else if (c == ',' && inTuple) {
            fields.push_back(field);
            field.clear();
        } else if (c == ')' && inTuple) {
            fields.push_back(field);
            tuples.push_back(fields);
            inTuple = false;
        } else if (inTuple) {
            field += c;
        }
    }
    return tuples;
}

} // namespace

TEST(PasswordHash, SeedAccountsShipCurrentHashes) {
    // Every account in the shipped dump and the password it is documented
    // with (README.md, "Accounts"). A new seed row must be added here too.
    const std::map<std::string, std::string> documented = {{"111111", "111111"}, {"222222", "222222"}};

    std::ifstream dump(PASSWORD_TEST_INITDB_DIR "/DARKEDEN.sql");
    ASSERT_TRUE(dump.is_open()) << PASSWORD_TEST_INITDB_DIR "/DARKEDEN.sql";

    std::vector<std::vector<std::string>> rows;
    std::string line;
    while (std::getline(dump, line)) {
        if (line.rfind("INSERT INTO `Player` VALUES ", 0) == 0) {
            const std::vector<std::vector<std::string>> tuples = parseInsertTuples(line);
            rows.insert(rows.end(), tuples.begin(), tuples.end());
        }
    }

    ASSERT_EQ(rows.size(), documented.size()) << "Player seed rows changed; update the documented map";
    for (const std::vector<std::string>& row : rows) {
        ASSERT_GE(row.size(), 2u);
        const std::string& playerID = row[0];
        const std::string& stored = row[1];

        const auto it = documented.find(playerID);
        ASSERT_NE(it, documented.end()) << "undocumented seed account " << playerID;

        EXPECT_TRUE(de::password::isHashed(stored)) << playerID << " ships a plaintext password";
        EXPECT_EQ(stored.substr(0, kCurrentPrefix.size()), kCurrentPrefix) << playerID;
        EXPECT_EQ(de::password::verify(stored, it->second), Verify::Accepted) << playerID;
        EXPECT_EQ(de::password::verify(stored, it->second + "x"), Verify::Rejected) << playerID;
    }
}
