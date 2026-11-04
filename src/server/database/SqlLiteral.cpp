#include "SqlLiteral.h"

#include <vector>

#include <mysql/mysql.h>

#include "Connection.h"

namespace de {

std::string escapeSqlLiteral(Connection* pConnection, const std::string& input) {
    // Strip NUL bytes up front so the result can never truncate a "%s" query.
    std::string value;
    value.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] != '\0')
            value += input[i];
    }

    if (pConnection != NULL && pConnection->isConnected()) {
        // mysql_real_escape_string() needs room for 2*length + 1 bytes. It
        // reports (unsigned long)-1 only under NO_BACKSLASH_ESCAPES, when it
        // cannot know the quoting character; the pass below then still never
        // under-escapes.
        std::vector<char> buffer(value.size() * 2 + 1);
        const unsigned long escapedLength =
            mysql_real_escape_string(pConnection->getMYSQL(), &buffer[0], value.c_str(), (unsigned long)value.size());
        if (escapedLength != (unsigned long)-1)
            return std::string(&buffer[0], escapedLength);
    }

    // One pass, each byte mapped on its own, so an escape it emits is never
    // fed back through the loop: the quote is DOUBLED, never written as \',
    // which a backslash pass would turn into \\' with the quote left live.
    std::string result;
    result.reserve(value.size());
    for (size_t i = 0; i < value.size(); ++i) {
        const char c = value[i];
        if (c == '\\')
            result += "\\\\";
        else if (c == '\'')
            result += "''";
        else
            result += c;
    }
    return result;
}

} // namespace de
