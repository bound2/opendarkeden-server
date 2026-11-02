// de::escapeSqlLiteral (database/SqlLiteral.h) without a live connection:
// the pass that drops NUL bytes and doubles the backslash and the quote.
// With a connection the client library escapes instead, which
// make integration-test exercises through the UserIPInfo statements.

#include <string>

#include <gtest/gtest.h>

#include "Connection.h"
#include "SqlLiteral.h"

TEST(SqlLiteral, PlainTextIsUnchanged) {
    EXPECT_EQ("Rowan", de::escapeSqlLiteral(NULL, "Rowan"));
    EXPECT_EQ("", de::escapeSqlLiteral(NULL, ""));
    // Bytes above 0x7F and the LIKE wildcards pass as they are.
    EXPECT_EQ("\xed\x95\x9c\xea\xb8\x80 x%_", de::escapeSqlLiteral(NULL, "\xed\x95\x9c\xea\xb8\x80 x%_"));
}

TEST(SqlLiteral, TheQuoteIsDoubledAndTheBackslashEscaped) {
    EXPECT_EQ("O''Brien", de::escapeSqlLiteral(NULL, "O'Brien"));
    EXPECT_EQ("a\\\\b", de::escapeSqlLiteral(NULL, "a\\b"));
    // \' OR 1=1# cannot escape its own doubled quote.
    EXPECT_EQ("\\\\'' OR 1=1#", de::escapeSqlLiteral(NULL, "\\' OR 1=1#"));
}

TEST(SqlLiteral, NulBytesAreDropped) {
    EXPECT_EQ("ab", de::escapeSqlLiteral(NULL, std::string("a\0b", 3)));
    EXPECT_EQ("", de::escapeSqlLiteral(NULL, std::string("\0", 1)));
}

TEST(SqlLiteral, AConnectionThatIsNotUpTakesTheSamePass) {
    Connection connection;
    ASSERT_FALSE(connection.isConnected());
    EXPECT_EQ("O''Brien", de::escapeSqlLiteral(&connection, "O'Brien"));
}
