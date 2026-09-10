//--------------------------------------------------------------------------------
//
// Filename   : DatabaseError.h
//
//--------------------------------------------------------------------------------

#ifndef __DATABASE_ERROR_H__
#define __DATABASE_ERROR_H__

#include <string>

//--------------------------------------------------------------------------------
//
// class DatabaseError
//
// What END_DB throws when a statement fails: the line it wrote to
// DBError.log, owned by the thrown value, so a handler that catches it can
// still read the function name and the driver's message.
//
// Deliberately unrelated to Throwable and to std::exception. __END_CATCH
// rethrows a Throwable and __END_CATCH_NO_RETHROW swallows one, and both
// stand between a statement and the code that decides what the caller is
// told; a database failure has to travel past them to reach that decision,
// so the type they match may not be one of this one's bases. Nothing else
// in the tree catches it, which is what keeps its reach exactly the reach
// of the catch clauses that name it.
//
//--------------------------------------------------------------------------------

class DatabaseError {
public:
    explicit DatabaseError(std::string message) : m_Message(std::move(message)) {}

    const std::string& message() const {
        return m_Message;
    }

    const char* what() const {
        return m_Message.c_str();
    }

private:
    std::string m_Message;
};

#endif
