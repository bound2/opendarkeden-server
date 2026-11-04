//////////////////////////////////////////////////////////////////////////////
// Filename    : SqlLiteral.h
// Description : A value escaped for a single-quoted SQL string literal,
//               for a statement that interpolates a text the wire or a
//               client supplied. NUL bytes are dropped first (a "%s" stops
//               at the first, which would truncate the statement), then
//               mysql_real_escape_string escapes against the connection
//               while it is up, aware of its character set and of
//               NO_BACKSLASH_ESCAPES; without a connection one pass
//               doubles the backslash and the quote, which never
//               under-escapes (under NO_BACKSLASH_ESCAPES it would store a
//               doubled backslash, a mode this project's sql_mode
//               excludes: docs/database/overview.md). The result is safe
//               only inside '...': never in an identifier, a double-quoted
//               literal, an unquoted numeric slot or a LIKE pattern, where
//               % and _ stay wildcards. Nothing about length: the caller
//               keeps the column's width and Statement's buffer.
//////////////////////////////////////////////////////////////////////////////

#ifndef DARKEDEN_SQL_LITERAL_H
#define DARKEDEN_SQL_LITERAL_H

#include <string>

class Connection;

namespace de {

std::string escapeSqlLiteral(Connection* pConnection, const std::string& value);

} // namespace de

#endif // DARKEDEN_SQL_LITERAL_H
