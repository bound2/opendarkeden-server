//////////////////////////////////////////////////////////////////////////////
// Filename    : SubcommandTable.h
// Description : The table a *command sub-command is looked up in: its name,
//               the permission level it demands and the body that answers it.
//               A name is matched whole, unlike the chat commands, which match
//               as a prefix of the message.
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

struct Subcommand {
    // The word that follows *command, compared whole.
    std::string name;
    Permission permission;
    SubcommandHandler handler;
};

// Sub-commands are answered in registration order: the first whose name is the
// word given and whose gate the caller clears runs, and nothing else does.
class SubcommandTable {
public:
    void add(std::string_view name, Permission permission, SubcommandHandler handler);

    // Runs the sub-command the word names, if the caller may run it. Answers
    // false when none ran - an unknown word, or one every matching row's gate
    // refused.
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
