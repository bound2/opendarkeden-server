//////////////////////////////////////////////////////////////////////////////
// Filename    : ConsoleSubcommands.h
// Description : The sub-commands *command carries - one function per name, in
//               ConsoleCommands.cpp - and the table they are registered on. A
//               sub-command's permission level is written down where it is
//               registered, in ConsoleCommandRegistration.cpp, and nowhere else.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GM_CONSOLE_SUBCOMMANDS_H__
#define __GM_CONSOLE_SUBCOMMANDS_H__

#include <string>

#include "gm/SubcommandTable.h"

class GCSystemMessage;
class GamePlayer;

namespace de::gm {

// The sub-commands, in the order a word is matched against them.
void registerConsoleSubcommands(SubcommandTable& table);

// The table, built on first use.
const SubcommandTable& consoleSubcommands();

//////////////////////////////////////////////////////////////////////////////
// The bodies, named after the sub-command each one answers.
//////////////////////////////////////////////////////////////////////////////

void opBalanceZoneGroup(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opRegenMasterLair(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opShowMasterLairStatus(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opInvincible(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opGhost(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opClearInventory(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opClearRankBonus(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opSetCastleOwner(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opSetCastleOwnerGuild(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opShowWarList(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opStartRaceWar(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opStartWar(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opRemoveWar(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opRemoveRaceWar(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opLevelWar(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opSaveBloodBibleOwner(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opKillAllMonster(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opKillAllPC(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opShowZonePCNum(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opShowPKZonePCNum(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opSetPKZonePCNum(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opSuicide(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opHeal(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opSetGold(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opQuest(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opQuestEnding(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opNotifyWin(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opHorn(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opLoud(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opGame(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opChangeSex(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opFirecraker(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opBulletin(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opSetHP(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opResetAttr(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opCTF(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opViewDamage(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opGoodsReload(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opPetStash(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opZoneEvent(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opEventZonePCLimit(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opKickOutAll(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opStartTrap(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opSMSTest(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opForceNick(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opRemoveNick(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opStartGDRLair(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opResetGDRLair(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opGuildRecall(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opResetSiege(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opInitSiege(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opIAmAttacker(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opIAmDefender(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opIAmReinforce(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opShowpcstat(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opAdvanceclass(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opAddDynamicZone(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opEnterDynamicZone(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opClearDynamicZone(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opSetTimeOutAllZoneEffect(GamePlayer*, const std::string&, GCSystemMessage&, bool&);
void opPrintTile(GamePlayer*, const std::string&, GCSystemMessage&, bool&);

} // namespace de::gm

#endif // __GM_CONSOLE_SUBCOMMANDS_H__
