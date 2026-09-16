//////////////////////////////////////////////////////////////////////////////
// Filename    : SubcommandTable.cpp
// Description : Name matching and gating for the *command sub-commands.
//////////////////////////////////////////////////////////////////////////////

#include "gm/SubcommandTable.h"

namespace de::gm {

void SubcommandTable::add(std::string_view name, Permission permission, SubcommandHandler handler) {
    m_Subcommands.push_back(Subcommand{std::string(name), permission, handler});
}

bool SubcommandTable::dispatch(const std::string& name, Permission caller, GamePlayer* pGamePlayer,
                               const std::string& value1, GCSystemMessage& gcSystemMessage, bool& bSendPacket) const {
    for (const Subcommand& subcommand : m_Subcommands) {
        if (subcommand.name != name)
            continue;

        // A row the caller may not run is passed over rather than refused out
        // loud, the way the chat commands are.
        if (!admits(caller, subcommand.permission))
            continue;

        subcommand.handler(pGamePlayer, value1, gcSystemMessage, bSendPacket);
        return true;
    }

    return false;
}

} // namespace de::gm
