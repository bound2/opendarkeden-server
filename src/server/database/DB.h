//--------------------------------------------------------------------------------
//
// Filename   : DB.h
// Written By : Reiot
//
//--------------------------------------------------------------------------------

#ifndef __DB_H__
#define __DB_H__

#include <source_location>

#include "Connection.h"
#include "DatabaseError.h"
#include "DatabaseManager.h"
#include "Result.h"
#include "Statement.h"

#define BEGIN_DB try
#define BEGIN_DB_EX try

// These two stay macros (they are catch clauses, not calls); the enclosing
// function name comes from std::source_location::current(), evaluated inside
// that function. Under Clang it is the text __PRETTY_FUNCTION__ produces, so
// DBError.log keeps its historical format. The same line is what the thrown
// DatabaseError carries, so a handler can report the failure the log records.
#define END_DB(STMT)                                                    \
    catch (SQLQueryException & sqe) {                                   \
        delete STMT;                                                    \
        string msg;                                                     \
        msg += string(std::source_location::current().function_name()); \
        msg += " : ";                                                   \
        msg += string(sqe.toString());                                  \
        filelog("DBError.log", "%s", msg.c_str());                      \
        throw DatabaseError{msg};                                       \
    }
#define END_DB_EX(STMT, MSG)                                            \
    catch (SQLQueryException & sqe) {                                   \
        delete STMT;                                                    \
        string msg;                                                     \
        msg += string(std::source_location::current().function_name()); \
        msg += string(" : ");                                           \
        msg += string(sqe.toString());                                  \
        msg += string(" : ");                                           \
        msg += string(MSG);                                             \
        filelog("DBError.log", "%s", msg.c_str());                      \
        throw DatabaseError{msg};                                       \
    }

#define NEW_STMT g_pDatabaseManager->getConnection("DARKEDEN")->createStatement()

#endif
