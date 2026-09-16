//////////////////////////////////////////////////////////////////////////////
// Filename    : SubcommandTable.h
// Description : The table a *command sub-command is looked up in: its name,
//               the permission level it demands, whether its body runs with no
//               player behind the line, and the body that answers it. A name
//               is matched whole, unlike the chat commands, which match as a
//               prefix of the message.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GM_SUBCOMMAND_TABLE_H__
#define __GM_SUBCOMMAND_TABLE_H__

#include <string>
#include <vector>

#include <string_view>

#include "gm/CommandRouter.h"

class GCSystemMessage;
class GamePlayer;

namespace de::gm {

// What a sub-command body is handed: the player who typed the line - NULL when
// another game server relayed it - the rest of the line after the sub-command's
// name, and the reply the console sends while bSendPacket stands. A body that
// answers nothing clears bSendPacket.
using SubcommandHandler = void (*)(GamePlayer* pGamePlayer, const std::string& value1, GCSystemMessage& gcSystemMessage,
                                   bool& bSendPacket);

// Whether a body may run with no player behind the line.
//
//   Allowed    - the body is correct end to end without one: it either never
//                reads the player or asks whether there is one first.
//   PlayerOnly - the body reads the player straight away, so it runs only for
//                a line someone typed.
enum class Relay : unsigned char { PlayerOnly = 0, Allowed = 1 };

struct Subcommand {
    // The word that follows *command, compared whole.
    std::string name;
    Permission permission;
    Relay relay;
    SubcommandHandler handler;
};

// Sub-commands are answered in registration order: the first whose name is the
// word given, whose gate the caller clears and whose body the line has a player
// for runs, and nothing else does.
class SubcommandTable {
public:
    void add(std::string_view name, Permission permission, Relay relay, SubcommandHandler handler);

    // Runs the sub-command the word names, if the caller may run it and the
    // line carries what its body needs. Answers false when none ran - an
    // unknown word, one every matching row's gate refused, or one every
    // matching row needs a player for when there is none.
    bool dispatch(const std::string& name, Permission caller, GamePlayer* pGamePlayer, const std::string& value1,
                  GCSystemMessage& gcSystemMessage, bool& bSendPacket) const;

    const std::vector<Subcommand>& commands() const {
        return m_Subcommands;
    }

private:
    std::vector<Subcommand> m_Subcommands;
};

} // namespace de::gm

#endif // __GM_SUBCOMMAND_TABLE_H__
