// What END_DB answers a failed statement with, exercised without a
// database. The macro is a catch clause for the SQLQueryException the
// driver raises, so raising that exception inside the BEGIN_DB block
// reaches it exactly as a refused statement does, and the value it builds
// can be pinned on its own. Nothing here opens a connection; the MySQL
// integration tier covers the driver's half.
//
// The point of the pin is ownership: the error is copied out of the
// handler and read after the catch block is gone. A pointer into the
// handler's own string would be dangling at that point.

#include <unistd.h>

#include <string>

#include <gtest/gtest.h>
#include <type_traits>

#include "DB.h"
#include "Utility.h"

namespace {

// DBError.log is written next to the process, so keep it out of the
// checkout. Each test in this binary runs after this.
class DatabaseErrorTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(0, chdir(::testing::TempDir().c_str()));
    }
};

// The shape every repository method has: a statement pointer the macro
// owns, a body that raises, and END_DB behind it.
void refusedStatement(const std::string& driverText) {
    Statement* pStmt = NULL;
    BEGIN_DB {
        throw SQLQueryException(driverText);
    }
    END_DB(pStmt)
}

void refusedStatementWithContext(const std::string& driverText, const std::string& context) {
    Statement* pStmt = NULL;
    BEGIN_DB_EX {
        throw SQLQueryException(driverText);
    }
    END_DB_EX(pStmt, context)
}

TEST_F(DatabaseErrorTest, TheMessageOutlivesTheHandlerAndNamesTheFunctionAndTheDriverText) {
    DatabaseError caught{""};
    bool refused = false;

    try {
        refusedStatement("Table 'DARKEDEN.NoSuchTable' doesn't exist");
    } catch (const DatabaseError& error) {
        caught = error;
        refused = true;
    }

    ASSERT_TRUE(refused);

    const std::string reported = caught.message();
    EXPECT_NE(std::string::npos, reported.find("refusedStatement")) << "message was: " << reported;
    EXPECT_NE(std::string::npos, reported.find("SQLQueryException")) << "message was: " << reported;
    EXPECT_NE(std::string::npos, reported.find("Table 'DARKEDEN.NoSuchTable' doesn't exist"))
        << "message was: " << reported;
    EXPECT_STREQ(reported.c_str(), caught.what());
}

TEST_F(DatabaseErrorTest, TheContextFormCarriesItsExtraTextToo) {
    DatabaseError caught{""};
    bool refused = false;

    try {
        refusedStatementWithContext("Duplicate entry '1' for key 'PRIMARY'", "saving the stash");
    } catch (const DatabaseError& error) {
        caught = error;
        refused = true;
    }

    ASSERT_TRUE(refused);
    EXPECT_NE(std::string::npos, caught.message().find("Duplicate entry")) << "message was: " << caught.message();
    EXPECT_NE(std::string::npos, caught.message().find("saving the stash")) << "message was: " << caught.message();
}

// The reach of the failure is part of the contract: the catch clauses that
// stand between a statement and the code deciding what the caller is told
// must not match it. __END_CATCH rethrows a Throwable and
// __END_CATCH_NO_RETHROW swallows one, so a DatabaseError that derived
// from Throwable would be eaten before it arrived.
TEST_F(DatabaseErrorTest, ItIsNeitherAThrowableNorAStdException) {
    EXPECT_FALSE((std::is_base_of<Throwable, DatabaseError>::value));
    EXPECT_FALSE((std::is_base_of<std::exception, DatabaseError>::value));

    bool asThrowable = false;
    bool asDatabaseError = false;
    try {
        refusedStatement("Table 'DARKEDEN.NoSuchTable' doesn't exist");
    } catch (Throwable&) {
        asThrowable = true;
    } catch (const DatabaseError&) {
        asDatabaseError = true;
    }
    EXPECT_FALSE(asThrowable);
    EXPECT_TRUE(asDatabaseError);
}

} // namespace
