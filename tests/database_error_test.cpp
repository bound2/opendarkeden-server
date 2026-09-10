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

#include <new>
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


//--------------------------------------------------------------------------------
// The statement is closed whatever leaves the block
//
// A statement site opens a statement, runs a query and deletes it at the
// end of the block. Only the SQL clause used to stand behind that, so an
// exception raised between the query and the delete -- a bad_alloc, an
// out-of-bounds Throwable, an Error from a helper -- carried the site's
// last reference away with it and the statement and the result set it owns
// were never freed. The catch-all clause closes it and rethrows.
//
// The macro deletes the pointer with the type the site declares. Every real
// site declares Statement*, so what runs is Statement's own destructor, the
// one that deletes the result set; this subclass records the delete and
// runs that destructor as part of its own.
//--------------------------------------------------------------------------------

int g_statementsDestroyed = 0;

class TrackedStatement : public Statement {
public:
    ~TrackedStatement() {
        ++g_statementsDestroyed;
    }
};

enum class Raise { Nothing, NotSQL, StdException, SQL };

void raiseInside(Raise what) {
    switch (what) {
    case Raise::Nothing:
        return;
    case Raise::NotSQL:
        throw OutOfBoundException("row index past the result");
    case Raise::StdException:
        throw std::bad_alloc();
    case Raise::SQL:
        throw SQLQueryException("Table 'DARKEDEN.NoSuchTable' doesn't exist");
    }
}

// The shape a repository method has when the failure is not the statement's:
// the statement is open, a helper raises, and the site's own SAFE_DELETE is
// never reached.
void openStatementThen(Raise what) {
    TrackedStatement* pStmt = NULL;
    BEGIN_DB {
        pStmt = new TrackedStatement();
        raiseInside(what);
        SAFE_DELETE(pStmt);
    }
    END_DB(pStmt)
}

void openStatementWithContextThen(Raise what) {
    TrackedStatement* pStmt = NULL;
    BEGIN_DB_EX {
        pStmt = new TrackedStatement();
        raiseInside(what);
        SAFE_DELETE(pStmt);
    }
    END_DB_EX(pStmt, "saving the stash")
}

// A site that finished with its statement early: SAFE_DELETE cleared the
// pointer, so the clause finds nothing left to close.
void closeStatementThen(Raise what) {
    TrackedStatement* pStmt = NULL;
    BEGIN_DB {
        pStmt = new TrackedStatement();
        SAFE_DELETE(pStmt);
        raiseInside(what);
    }
    END_DB(pStmt)
}

TEST_F(DatabaseErrorTest, AStatementStillOpenIsDestroyedWhenAThrowableLeavesTheBlock) {
    g_statementsDestroyed = 0;
    bool asOutOfBound = false;

    try {
        openStatementThen(Raise::NotSQL);
    } catch (OutOfBoundException& e) {
        asOutOfBound = true;
        EXPECT_NE(std::string::npos, e.toString().find("row index past the result"));
    }

    EXPECT_TRUE(asOutOfBound) << "the clause must rethrow what it caught, unchanged";
    EXPECT_EQ(1, g_statementsDestroyed);
}

TEST_F(DatabaseErrorTest, AStatementStillOpenIsDestroyedWhenAStdExceptionLeavesTheBlock) {
    g_statementsDestroyed = 0;
    bool asBadAlloc = false;

    try {
        openStatementThen(Raise::StdException);
    } catch (std::bad_alloc&) {
        asBadAlloc = true;
    }

    EXPECT_TRUE(asBadAlloc);
    EXPECT_EQ(1, g_statementsDestroyed);
}

TEST_F(DatabaseErrorTest, TheContextFormClosesItTheSameWay) {
    g_statementsDestroyed = 0;
    bool asOutOfBound = false;

    try {
        openStatementWithContextThen(Raise::NotSQL);
    } catch (OutOfBoundException&) {
        asOutOfBound = true;
    }

    EXPECT_TRUE(asOutOfBound);
    EXPECT_EQ(1, g_statementsDestroyed);
}

TEST_F(DatabaseErrorTest, TheSQLClauseStillClosesTheStatementItAnswersFor) {
    g_statementsDestroyed = 0;
    bool refused = false;

    try {
        openStatementThen(Raise::SQL);
    } catch (const DatabaseError&) {
        refused = true;
    }

    EXPECT_TRUE(refused);
    EXPECT_EQ(1, g_statementsDestroyed);
}

// SAFE_DELETE clears the pointer, so the two clauses cannot delete a
// statement the site already deleted.
TEST_F(DatabaseErrorTest, ASiteThatClosedItsStatementEarlyDoesNotCloseItTwice) {
    g_statementsDestroyed = 0;

    try {
        closeStatementThen(Raise::NotSQL);
    } catch (OutOfBoundException&) {
    }
    EXPECT_EQ(1, g_statementsDestroyed);

    g_statementsDestroyed = 0;
    try {
        closeStatementThen(Raise::SQL);
    } catch (const DatabaseError&) {
    }
    EXPECT_EQ(1, g_statementsDestroyed);
}

TEST_F(DatabaseErrorTest, ABlockThatFinishesNormallyLeavesTheClausesUnrun) {
    g_statementsDestroyed = 0;
    openStatementThen(Raise::Nothing);
    EXPECT_EQ(1, g_statementsDestroyed);
}

} // namespace
