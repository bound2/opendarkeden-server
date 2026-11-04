//////////////////////////////////////////////////////////////////////////////
// Filename    : LoginFailureKind.h
// Description : The kind of the exception a catch block is handling, for a
//               log line that must carry no message (a message can hold SQL
//               or account data): a Throwable's name, "DatabaseError",
//               "std::exception" or "unknown".
//////////////////////////////////////////////////////////////////////////////

#ifndef DARKEDEN_LOGIN_FAILURE_KIND_H
#define DARKEDEN_LOGIN_FAILURE_KIND_H

#include <exception>
#include <string>

#include "DatabaseError.h"
#include "Exception.h"

namespace de {

// Called from a catch block only: it rethrows the exception being handled
// to name it.
inline std::string currentFailureKind() noexcept {
    try {
        throw;
    } catch (const Throwable& t) {
        try {
            return t.getName();
        } catch (...) {
            return "Throwable";
        }
    } catch (const DatabaseError&) {
        return "DatabaseError";
    } catch (const std::exception&) {
        return "std::exception";
    } catch (...) {
        return "unknown";
    }
}

} // namespace de

#endif // DARKEDEN_LOGIN_FAILURE_KIND_H
