//////////////////////////////////////////////////////////////////////////////
// Filename    : SubcommandTable.cpp
// Description : Name matching, gating and null-player admission for the
//               *command sub-commands.
//////////////////////////////////////////////////////////////////////////////

#include "gm/SubcommandTable.h"

namespace de::gm {

void SubcommandTable::add(std::string_view name, Permission permission, Relay relay, SubcommandHandler handler) {
    m_Subcommands.push_back(Subcommand{std::string(name), permission, relay, handler});
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

        // So is a row whose body reads the player who typed the line when the
        // line has none, which is what a message another game server relayed
        // carries.
        if (pGamePlayer == nullptr && subcommand.relay == Relay::PlayerOnly)
            continue;

        subcommand.handler(pGamePlayer, value1, gcSystemMessage, bSendPacket);
        return true;
    }

    return false;
}

} // namespace de::gm
