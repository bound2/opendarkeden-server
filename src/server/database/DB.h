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
#include "DatabaseManager.h"
#include "Result.h"
#include "Statement.h"

#define BEGIN_DB try
#define BEGIN_DB_EX try

// These two stay macros (they are catch clauses, not calls), but the enclosing
// function name now comes from std::source_location::current() instead of
// __PRETTY_FUNCTION__. Both are evaluated inside the enclosing function, and
// under Clang they produce identical text, so DBError.log is unchanged.
#define END_DB(STMT)                                                    \
    catch (SQLQueryException & sqe) {                                   \
        delete STMT;                                                    \
        string msg;                                                     \
        msg += string(std::source_location::current().function_name()); \
        msg += " : ";                                                   \
        msg += string(sqe.toString());                                  \
        filelog("DBError.log", "%s", msg.c_str());                      \
        throw msg.c_str();                                              \
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
        throw msg.c_str();                                              \
    }

#define NEW_STMT g_pDatabaseManager->getConnection("DARKEDEN")->createStatement()

#endif
