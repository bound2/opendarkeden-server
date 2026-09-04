//////////////////////////////////////////////////////////////////////
//
// File Name 	: Statement.h
// Written by	: Gday29@ewestsoft.com
// Description	: SQL ¹®À» ¸¸µç´Ù..
//
//////////////////////////////////////////////////////////////////////

#ifndef __STATEMENT_H__
#define __STATEMENT_H__

// include files
#include "Connection.h"
#include "Exception.h"
#include "Types.h"

// forward declaration
class Result;

//////////////////////////////////////////////////////////////////////
//
// class Statement;
//
// SQL¹®À» ¸¸µé¾î µðºñ¿¡ Äõ¸®ÇÑ´Ù.
//
//////////////////////////////////////////////////////////////////////

class Statement {
public:
    // constructor
    Statement();

    // constructor
    Statement(char* fmt, ...);

    // destructor
    ~Statement();

public:
    // »çÀü¿¡ ÁöÁ¤µÈ SQL ¹®À» °¡Áö°í Äõ¸®ÇÑ´Ù.
    Result* executeQuery();

    // SQL ¹®À» ¹Þ¾Æ¼­ Äõ¸®ÇÑ´Ù.
    Result* executeQuery(char*, ...);
    Result* executeQuery(const string& sqlStatement);

    // get SQL statement
    string getStatement() const {
        return m_Statement;
    }

    // SQL ¹®À» ÁöÁ¤ÇÑ´Ù.
    void setStatement(char* fmt, ...);

    // get connection object
    Connection* getConnection() const {
        return m_pConnection;
    }

    // set connection object
    void setConnection(Connection* pConnection) {
        m_pConnection = pConnection;
    }

    // get warning/error string
    string getError() const {
        return (m_pConnection == NULL) ? ("Not Associated with Connection Object") : (m_pConnection->getError());
    }

    // get affected rows
    uint getAffectedRowCount() const {
        return m_nAffectedRows;
    }


private:
    // Connection
    Connection* m_pConnection;

    // SQL Statement¹
    string m_Statement;

    // Query Result
    Result* m_pResult;

    // insert, update, delete ÇßÀ» ¶§ ¿µÇâÀ» ¹ÞÀº row ÀÇ °³¼ö
    uint m_nAffectedRows;
};

#endif // __STATEMENT_H__
