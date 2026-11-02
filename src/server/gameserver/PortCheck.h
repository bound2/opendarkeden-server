//////////////////////////////////////////////////////////////////////////////
// Filename    : PortCheck.h
// Description : What the gameserver does with a port check, the one
//               datagram its socket admits from anywhere, since a native
//               client sends it on reconnect (docs/protocol/datagram-sources.md):
//               the name it carries is recorded in UserIPInfo with the
//               sender's address and port, read back by CGRequestIP and
//               the GM commands. A name character creation would refuse,
//               or one no statement may carry, is refused before any
//               statement sees it, and the repository escapes the name it
//               records (SqlLiteral.h). Nothing ties the name to the
//               sender: a port check naming a recorded character overwrites
//               that character's row, which the fix log records.
//////////////////////////////////////////////////////////////////////////////

#ifndef DARKEDEN_PORT_CHECK_H
#define DARKEDEN_PORT_CHECK_H

#include <optional>
#include <string>

#include "Types.h"

class SessionRepository;

namespace de {

// Why a port check's name is refused.
enum class PortCheckRejection {
    EmptyName,
    NameTooLong,  // past the 20 bytes CGPortCheck::read also refuses
    ControlByte,  // a byte below 0x20 or 0x7F, NUL included
    NotUtf8,      // no well-formed UTF-8, which the utf8mb4 column cannot keep
    ReservedName, // a token character creation refuses (CharacterName.h)
};

std::optional<PortCheckRejection> portCheckNameRejection(const std::string& name);

// Records the port check through the repository; a refused name throws
// InvalidProtocolException naming the rejection and records nothing.
void recordPortCheck(SessionRepository& repository, const std::string& name, DWORD ip, uint port, int serverID);

} // namespace de

#endif // DARKEDEN_PORT_CHECK_H
