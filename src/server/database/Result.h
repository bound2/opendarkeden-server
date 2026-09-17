//////////////////////////////////////////////////////////////////////////////
// File Name	: Result.h
// Written By	: Gday29@ewestsoft.com
// Description	: Definition of Result Class
//////////////////////////////////////////////////////////////////////////////

#ifndef __RESULT_H__
#define __RESULT_H__

#include <mysql/mysql.h>
#include <sys/time.h>

#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// forward declaration
//////////////////////////////////////////////////////////////////////////////
class Connection;
class Statement;

#define T_RESULT MYSQL_RES

//////////////////////////////////////////////////////////////////////////////
// class Result;
// A Result need not be deleted. The caller only deletes the Statement.
//////////////////////////////////////////////////////////////////////////////

class Result {
public:
    Result(T_RESULT*, const string& statement);
    ~Result();

public:
    // Move to the next row.
    bool next();

    // Get the value of a specific field (column).
    char* getField(uint index);
    char getChar(uint index) {
        return (getField(index))[0];
    }
    int getInt(uint index) {
        return atoi(getField(index));
    }
    uint getUInt(uint index) {
        return (uint)atoi(getField(index));
    }
    BYTE getBYTE(uint index) {
        return (BYTE)atoi(getField(index));
    }
    WORD getWORD(uint index) {
        return (WORD)atoi(getField(index));
    }
    DWORD getDWORD(uint index) {
        return strtoul(getField(index), (char**)NULL, 10);
    }
    const char* getString(uint index);

    // Returns the number of rows/columns the query result holds.
    uint getRowCount() const {
        return m_RowCount;
    }
    uint getFieldCount() const {
        return m_FieldCount;
    }

    string getStatement(void) const {
        return m_Statement;
    }

private:
    T_RESULT* m_pResult; // MYSQL structure holding the result
    MYSQL_ROW m_pRow;    // the row currently being processed
    uint m_RowCount;     // number of rows the query returned
    uint m_FieldCount;
    string m_Statement; // which query produced this result
};

#endif // __RESULT_H__
