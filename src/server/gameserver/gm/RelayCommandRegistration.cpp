//////////////////////////////////////////////////////////////////////////////
// Filename    : RelayCommandRegistration.cpp
// Description : Where every command another game server may relay is named and
//               bound to the body that answers it. A relayed message carries
//               no player and no creature, so a body belongs here only when it
//               is correct without one.
//
//               The order of the rows is the order a relayed message is
//               matched against, and a name is compared as a prefix of the
//               message over as many characters as the name is long.
//////////////////////////////////////////////////////////////////////////////

#include "gm/GMCommands.h"

namespace de::gm {

namespace {

// The shape almost every command has: the player - none, on a relay - the
// message and the offset of the '*'.
template <void (*Command)(GamePlayer*, std::string, int)> void withPlayer(const CommandContext& context) {
    Command(context.pPlayer, context.msg, context.i);
}

// The two guild-union commands take the reach of their own relay as well. One
// that arrived as a relay goes no further than the world it arrived in.
template <void (*Command)(GamePlayer*, std::string, int, bool)> void withinTheWorld(const CommandContext& context) {
    Command(context.pPlayer, context.msg, context.i, true);
}

} // namespace

void registerRelayCommands(CommandRouter& router) {
    router.add("save", Permission::God, withPlayer<opsave>);
    router.add("wall", Permission::God, withPlayer<opwall>);
    router.add("shutdown", Permission::God, withPlayer<opshutdown>);
    router.add("kick", Permission::God, withPlayer<opkick>);
    router.add("mute", Permission::God, withPlayer<opmute>);
    router.add("freezing", Permission::God, withPlayer<opfreezing>);
    router.add("set", Permission::God, withPlayer<opset>);
    router.add("load", Permission::God, withPlayer<opload>);
    router.add("combat", Permission::God, withPlayer<opcombat>);
    router.add("command", Permission::God, withPlayer<opcommand>);
    router.add("modifyunioninfo", Permission::God, withinTheWorld<opmodifyunioninfo>);
    router.add("refreshguildunion", Permission::God, withinTheWorld<oprefreshguildunion>);
}

const CommandRouter& relayCommands() {
    static const CommandRouter router = [] {
        CommandRouter table;
        registerRelayCommands(table);
        return table;
    }();
    return router;
}

} // namespace de::gm
