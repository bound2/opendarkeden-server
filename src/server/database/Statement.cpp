//////////////////////////////////////////////////////////////////////////////
// File Name 	: Statement.cpp
// Written by	: Gday29@ewestsoft.com
// Description	: SQL statement wrapper
//////////////////////////////////////////////////////////////////////////////

#include "Statement.h"

#include <stdio.h>

#include <mysql/mysql.h>
#include <sys/time.h>

#include "Assert.h"
#include "Mutex.h"
#include "Profile.h"
#include "Result.h"

#define __FULL_PROFILE__

#ifndef __FULL_PROFILE__
#undef beginProfileEx
#define beginProfileEx(name) ((void)0)
#undef endProfileEx
#define endProfileEx(name) ((void)0)
#endif

//////////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////////
Statement::Statement() {
    __BEGIN_TRY

    m_pConnection = NULL;
    m_pResult = NULL;
    m_nAffectedRows = 0;

    __END_CATCH
}

Statement::Statement(const char* fmt, ...)

{
    __BEGIN_TRY

    // variable argument list
    va_list valist;

    va_start(valist, fmt);

    char buffer[2048];

    int nchars = vsnprintf(buffer, 2048, fmt, valist);

    // If the buffer is not big enough, fail instead of truncating.
    if (nchars == -1 || nchars > 2048)
        throw Error("more buffer size needed for SQL statement buffer...");

    va_end(valist);

    // m_Statement is a string, so assigning the buffer copies it.
    // If it were a char*, pointing at a local variable would be dangerous.
    m_Statement = buffer;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// destructor
//
// Frees the result it created internally.
//
//////////////////////////////////////////////////////////////////////

Statement::~Statement() {
    if (m_pResult != NULL) {
        delete m_pResult;
        m_pResult = NULL;
    }
}


//////////////////////////////////////////////////////////////////////
//
// executeQuery()
//
// Takes the Connection and the SQL statement string and returns the
// resulting Result *.
//
//////////////////////////////////////////////////////////////////////

Result* Statement::executeQuery()

{
    __BEGIN_TRY

    Assert(m_pConnection != NULL);

    if (m_pResult != NULL) {
        // The application did not take the previous Result; drop it here.
        delete m_pResult;
        m_pResult = NULL;
    }

    beginProfileEx("ZPM_QUERY");

    if (mysql_real_query(m_pConnection->getMYSQL(), m_Statement.c_str(), m_Statement.size()) != 0) {
        cout << "Stmt::EQ real Query Error" << endl;
        cout << "Stmt [" << m_Statement << "]" << endl;
        cout << getError() << endl;

        throw SQLQueryException(getError());
    }

    MYSQL_RES* pResult = mysql_store_result(m_pConnection->getMYSQL());

    // A NULL result means the statement was an update, or an error.
    if (pResult != NULL) {
        m_pResult = new Result(pResult, m_Statement);
    } else {
        if (mysql_field_count(m_pConnection->getMYSQL()) != 0) {
            cerr << "Stmt::EQ Unknown Error > " << getError() << endl;

            throw SQLQueryException(getError());
        } else {
            m_nAffectedRows = mysql_affected_rows(m_pConnection->getMYSQL());
        }
    }

    endProfileEx("ZPM_QUERY");

    return m_pResult;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// executeQuery ()
//
// Takes the Connection and the SQL statement string and returns the
// resulting Result *.
//
//////////////////////////////////////////////////////////////////////

Result* Statement::executeQueryString(const string& sqlStatement) {
    __BEGIN_TRY

    m_Statement = sqlStatement;

    return executeQuery();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// executeQuery ()
//
// Takes the Connection and the SQL statement string and returns the
// resulting Result *.
//
//////////////////////////////////////////////////////////////////////

Result* Statement::executeQuery(const char* fmt, ...) {
    __BEGIN_TRY

    // Build the SQL statement.
    va_list valist;

    va_start(valist, fmt);

    char buffer[2048 + 1];

    int nchars = vsnprintf(buffer, 2048, fmt, valist);

    // If the buffer is not big enough, throw instead of truncating.
    if (nchars == -1 || nchars > 2048)
        throw Error("more buffer size needed for SQL statement buffer...");

    va_end(valist);

    m_Statement = buffer;

    return executeQuery();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
//	setStatement()
//
//	Rebuild the statement.
//
//////////////////////////////////////////////////////////////////////

void Statement::setStatement(const char* fmt, ...)

{
    __BEGIN_TRY

    // variable argument list
    va_list valist;

    va_start(valist, fmt);

    // buffer size = 1kb..Too big??
    char buffer[2048 + 1];

    int nchars = vsnprintf(buffer, 2048, fmt, valist);

    // If the buffer is not big enough, throw instead of truncating.
    if (nchars == -1 || nchars > 2048)
        throw Error("more buffer size needed for SQL statement buffer...");

    va_end(valist);

    m_Statement = buffer;

    __END_CATCH
}

uint Statement::getInsertID() const {
    __BEGIN_TRY
    return mysql_insert_id(m_pConnection->getMYSQL());
    __END_CATCH
}
