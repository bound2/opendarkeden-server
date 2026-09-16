//////////////////////////////////////////////////////////////////////////////
// Filename    : CommandRouter.cpp
// Description : Name matching and gating for the GM chat commands.
//////////////////////////////////////////////////////////////////////////////

#include "gm/CommandRouter.h"

namespace de::gm {

void CommandRouter::add(std::string_view name, Permission permission, CommandHandler handler) {
    add(name, name.size(), permission, handler);
}

void CommandRouter::add(std::string_view name, std::string::size_type matchLength, Permission permission,
                        CommandHandler handler) {
    m_Commands.push_back(Command{std::string(name), matchLength, permission, handler});
}

bool CommandRouter::dispatch(const CommandContext& context) const {
    for (const Command& command : m_Commands) {
        if (context.msg.substr(context.i + 1, command.matchLength) != command.name)
            continue;

        // A command the caller may not run is passed over rather than
        // refused out loud: a later command whose name also matches still
        // gets its turn, and a caller who may run none of them is answered
        // with silence.
        if (!admits(context.caller, command.permission))
            continue;

        command.handler(context);
        return true;
    }

    return false;
}

} // namespace de::gm
