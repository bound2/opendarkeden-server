//////////////////////////////////////////////////////////////////////////////
// Filename    : GMCommands.h
// Description : The GM chat commands - one function per command - and the two
//               routers they are registered on. A command's permission level
//               is written down where it is registered, in
//               CommandRegistration.cpp, and nowhere else.
//
//               Every command reads its arguments out of the chat message
//               itself: msg is the whole line and i the offset of the '*'
//               that introduces the command.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GM_COMMANDS_H__
#define __GM_COMMANDS_H__

#include <string>

#include "gm/CommandRouter.h"

class Creature;
class GamePlayer;

namespace de::gm {

//////////////////////////////////////////////////////////////////////////////
// The tables
//////////////////////////////////////////////////////////////////////////////

// The operator commands, answered for a message that names one of them.
void registerOperatorCommands(CommandRouter& router);

// The four prefixes answered before the operator commands: the two guild and
// zone test commands, and the two that relay a command to the other game
// servers. A message one of these answers reaches no operator command.
void registerBroadcastCommands(CommandRouter& router);

// The commands another game server may relay. A relayed message carries no
// player and no creature, so only bodies that are correct without one are
// named here.
void registerRelayCommands(CommandRouter& router);

// The three tables, built on first use.
const CommandRouter& operatorCommands();
const CommandRouter& broadcastCommands();
const CommandRouter& relayCommands();

//////////////////////////////////////////////////////////////////////////////
// ServerCommands.cpp - the server as a whole
//////////////////////////////////////////////////////////////////////////////

void opcombat(GamePlayer* pPlayer, std::string msg, int i);
void opset(GamePlayer* pPlayer, std::string msg, int i);
void opview(GamePlayer* pPlayer, std::string msg, int i);
void opload(GamePlayer* pPlayer, std::string msg, int i);
void opsave(GamePlayer* pPlayer, std::string msg, int i);
void opwall(GamePlayer* pPlayer, std::string msg, int i);
void opshutdown(GamePlayer* pPlayer, std::string msg, int i);
void oplog(GamePlayer* pPlayer, std::string msg, int i);
// Relays the message to the other game servers: the same world only, or all
// of them.
void opworld(GamePlayer* pPlayer, std::string msg, int i, bool bSameWorldOnly);
// The two reports the client sends as chat: both name the creature as well,
// because the row they write carries the character name beside the account.
void opbugreport(Creature* pCreature, GamePlayer* pPlayer, std::string msg, int i);
void opcrashreport(Creature* pCreature, GamePlayer* pPlayer, std::string msg, int i);

//////////////////////////////////////////////////////////////////////////////
// PlayerCommands.cpp - one character or account
//////////////////////////////////////////////////////////////////////////////

void opkick(GamePlayer* pPlayer, std::string msg, int i);
void opmute(GamePlayer* pPlayer, std::string msg, int i);
void opdenychat(GamePlayer* pPlayer, std::string msg, int i);
void opfreezing(GamePlayer* pPlayer, std::string msg, int i);
void opdeny(GamePlayer* pPlayer, std::string msg, int i);
void opinfo(GamePlayer* pPlayer, std::string msg, int i);
void opfind(GamePlayer* pPlayer, std::string msg, int i);
void opcredit(GamePlayer* pPlayer, std::string msg, int i);
void opuser(GamePlayer* pPlayer, std::string msg, int i);
void optrace(GamePlayer* pPlayer, std::string msg, int i);
void oppay(GamePlayer* pPlayer, std::string msg, int i);
void opfun(GamePlayer* pPlayer, std::string msg, int i);
void opgrant(GamePlayer* pPlayer, std::string msg, int i);
void opsoulchain(GamePlayer* pPlayer, std::string msg, int i);

//////////////////////////////////////////////////////////////////////////////
// ZoneCommands.cpp - a zone and what stands in it
//////////////////////////////////////////////////////////////////////////////

void opzone(std::string msg, int i);
void opwarp(GamePlayer* pPlayer, std::string msg, int i);
void oprecall(GamePlayer* pPlayer, std::string msg, int i);
void opsummon(GamePlayer* pPlayer, std::string msg, int i);
void opopenpaymap(GamePlayer* pPlayer, std::string msg, int i);
void opclosepaymap(GamePlayer* pPlayer, std::string msg, int i);

//////////////////////////////////////////////////////////////////////////////
// ItemCommands.cpp
//////////////////////////////////////////////////////////////////////////////

void opcreate(GamePlayer* pPlayer, std::string msg, int i);

//////////////////////////////////////////////////////////////////////////////
// GuildCommands.cpp
//////////////////////////////////////////////////////////////////////////////

void opguild(std::string msg, int i, Creature* pCreature);
void opmodifyunioninfo(GamePlayer* pPlayer, std::string msg, int i, bool bSameWorldOnly);
void oprefreshguildunion(GamePlayer* pPlayer, std::string msg, int i, bool bSameWorldOnly);

//////////////////////////////////////////////////////////////////////////////
// ConsoleCommands.cpp
//////////////////////////////////////////////////////////////////////////////

// Carries a sub-command of its own, looked up in the table in
// ConsoleSubcommands.h.
void opcommand(GamePlayer* pPlayer, std::string msg, int i);

} // namespace de::gm

#endif // __GM_COMMANDS_H__
