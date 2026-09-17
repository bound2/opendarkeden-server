//////////////////////////////////////////////////////////////////////////////
// Filename    : CommandRegistration.cpp
// Description : Where every GM chat command is named, gated and bound to the
//               body that answers it. The permission level beside a name here
//               is the only one there is: nothing else asks what a caller may
//               run.
//
//               The order of the rows is the order a message is matched
//               against, and a name is compared as a prefix of the message -
//               so "*savex" runs *save, and where one name starts with
//               another the row registered first answers both.
//////////////////////////////////////////////////////////////////////////////

#include "gm/GMCommands.h"

namespace de::gm {

namespace {

// The shape almost every command has: the player, the message and the offset
// of the '*'.
template <void (*Command)(GamePlayer*, std::string, int)> void withPlayer(const CommandContext& context) {
    Command(context.pPlayer, context.msg, context.i);
}

// The client reports name the creature as well.
template <void (*Command)(Creature*, GamePlayer*, std::string, int)> void withCreature(const CommandContext& context) {
    Command(context.pCreature, context.pPlayer, context.msg, context.i);
}

void runZone(const CommandContext& context) {
    opzone(context.msg, context.i);
}

void runGuild(const CommandContext& context) {
    opguild(context.msg, context.i, context.pCreature);
}

// *world and *allworld carry an operator command of their own: it runs here
// first, for this server, and then the whole message is relayed to the other
// game servers - the same world only, or all of them.
void runWorldRelay(const CommandContext& context, bool bSameWorldOnly) {
    std::string::size_type j = context.msg.find_first_of(' ', context.i + 1);
    std::string command = context.msg.substr(j + 1, context.msg.size() - j - 1).c_str();
    std::string::size_type k = command.find_first_of('*', 0);
    operatorCommands().dispatch(
        CommandContext{context.pCreature, context.pPlayer, command, static_cast<int>(k), context.caller});

    opworld(context.pPlayer, context.msg, context.i, bSameWorldOnly);
}

void runWorld(const CommandContext& context) {
    runWorldRelay(context, true);
}

void runAllWorld(const CommandContext& context) {
    runWorldRelay(context, false);
}

// Three names the chat ladder also compared against, kept here so the table
// still lists every name it tested. None of them can match: each is the
// mojibake left by the move off the legacy code pages, and the byte count of
// the text is larger than the number of characters it is compared over.
constexpr std::string_view kFindAlias = "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd";
constexpr std::string_view kOpenPayMapAlias =
    "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xd5\xb7\xd1\xb5\xef\xbf\xbd\xcd\xbc";
constexpr std::string_view kClosePayMapAlias =
    "\xef\xbf\xbd\xd8\xb1\xef\xbf\xbd\xef\xbf\xbd\xd5\xb7\xd1\xb5\xef\xbf\xbd\xcd\xbc";

} // namespace

void registerOperatorCommands(CommandRouter& router) {
    router.add("save", Permission::DM, withPlayer<opsave>);
    router.add("wall", Permission::Helper, withPlayer<opwall>);
    router.add("shutdown", Permission::God, withPlayer<opshutdown>);
    router.add("pay", Permission::God, withPlayer<oppay>);
    router.add("kick", Permission::Helper, withPlayer<opkick>);
    router.add("mute", Permission::God, withPlayer<opmute>);
    // Before *deny, so that *denychat is not answered as a *deny with a
    // "chat" argument.
    router.add("denychat", Permission::Helper, withPlayer<opdenychat>);
    router.add("freezing", Permission::God, withPlayer<opfreezing>);
    router.add("deny", Permission::Helper, withPlayer<opdeny>);
    router.add("info", Permission::God, withPlayer<opinfo>);
    router.add("trace", Permission::Helper, withPlayer<optrace>);
    router.add("warp", Permission::Helper, withPlayer<opwarp>);
    router.add("create", Permission::God, withPlayer<opcreate>);
    router.add("summon", Permission::God, withPlayer<opsummon>);
    router.add("grant", Permission::God, withPlayer<opgrant>);
    router.add("command", Permission::God, withPlayer<opcommand>);
    router.add("fun", Permission::God, withPlayer<opfun>);
    router.add("recall", Permission::God, withPlayer<oprecall>);
    router.add("user", Permission::God, withPlayer<opuser>);
    router.add("set", Permission::God, withPlayer<opset>);
    router.add("load", Permission::God, withPlayer<opload>);
    router.add("view", Permission::God, withPlayer<opview>);
    router.add("combat", Permission::God, withPlayer<opcombat>);
    router.add("find", Permission::God, withPlayer<opfind>);
    router.add(kFindAlias, 4, Permission::God, withPlayer<opfind>);
    router.add("credit", Permission::God, withPlayer<opcredit>);
    router.add("soulchain", Permission::God, withPlayer<opsoulchain>);
    router.add("log", Permission::God, withPlayer<oplog>);
    router.add("bug_report", Permission::God, withCreature<opbugreport>);
    router.add("CrashReport", Permission::God, withCreature<opcrashreport>);
    router.add("OpenPayMap", Permission::God, withPlayer<opopenpaymap>);
    router.add(kOpenPayMapAlias, 12, Permission::God, withPlayer<opopenpaymap>);
    router.add("ClosePayMap", Permission::God, withPlayer<opclosepaymap>);
    router.add(kClosePayMapAlias, 12, Permission::God, withPlayer<opclosepaymap>);
}

void registerBroadcastCommands(CommandRouter& router) {
    router.add("zone", Permission::God, runZone);
    router.add("guild", Permission::God, runGuild);
    router.add("world", Permission::God, runWorld);
    router.add("allworld", Permission::God, runAllWorld);
}

const CommandRouter& operatorCommands() {
    static const CommandRouter router = [] {
        CommandRouter table;
        registerOperatorCommands(table);
        return table;
    }();
    return router;
}

const CommandRouter& broadcastCommands() {
    static const CommandRouter router = [] {
        CommandRouter table;
        registerBroadcastCommands(table);
        return table;
    }();
    return router;
}

} // namespace de::gm
