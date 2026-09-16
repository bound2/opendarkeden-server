//////////////////////////////////////////////////////////////////////////////
// Filename    : ConsoleCommandRegistration.cpp
// Description : Where every *command sub-command is named, gated and bound to
//               the body that answers it. The permission level beside a name
//               here is the only one there is: nothing else asks what a caller
//               may run.
//
//               The Relay beside it says whether the body is correct with no
//               player behind the line, which is what a message another game
//               server relays carries. PlayerOnly is a body that reads the
//               player without asking whether there is one; such a row is
//               passed over on a relay rather than run.
//
//               The order of the rows is the order a word is matched against,
//               and a name is compared whole - a longer word reaches nothing.
//////////////////////////////////////////////////////////////////////////////

#include "gm/ConsoleSubcommands.h"

namespace de::gm {

void registerConsoleSubcommands(SubcommandTable& table) {
    table.add("balanceZoneGroup", Permission::God, Relay::Allowed, opBalanceZoneGroup);
    table.add("regenMasterLair", Permission::God, Relay::Allowed, opRegenMasterLair);
    table.add("showMasterLairStatus", Permission::God, Relay::Allowed, opShowMasterLairStatus);
    table.add("invincible", Permission::God, Relay::PlayerOnly, opInvincible);
    table.add("ghost", Permission::God, Relay::PlayerOnly, opGhost);
    table.add("clearInventory", Permission::God, Relay::PlayerOnly, opClearInventory);
    table.add("clearRankBonus", Permission::God, Relay::PlayerOnly, opClearRankBonus);
    table.add("setCastleOwner", Permission::God, Relay::PlayerOnly, opSetCastleOwner);
    table.add("setCastleOwnerGuild", Permission::God, Relay::Allowed, opSetCastleOwnerGuild);
    table.add("showWarList", Permission::God, Relay::PlayerOnly, opShowWarList);
    table.add("startRaceWar", Permission::God, Relay::Allowed, opStartRaceWar);
    table.add("startWar", Permission::God, Relay::PlayerOnly, opStartWar);
    table.add("removeWar", Permission::God, Relay::Allowed, opRemoveWar);
    table.add("removeRaceWar", Permission::God, Relay::Allowed, opRemoveRaceWar);
    table.add("LevelWar", Permission::God, Relay::Allowed, opLevelWar);
    table.add("saveBloodBibleOwner", Permission::God, Relay::Allowed, opSaveBloodBibleOwner);
    table.add("killAllMonster", Permission::God, Relay::Allowed, opKillAllMonster);
    table.add("killAllPC", Permission::God, Relay::Allowed, opKillAllPC);
    table.add("showZonePCNum", Permission::God, Relay::Allowed, opShowZonePCNum);
    table.add("showPKZonePCNum", Permission::God, Relay::Allowed, opShowPKZonePCNum);
    table.add("setPKZonePCNum", Permission::God, Relay::Allowed, opSetPKZonePCNum);
    table.add("suicide", Permission::God, Relay::PlayerOnly, opSuicide);
    table.add("heal", Permission::God, Relay::PlayerOnly, opHeal);
    table.add("setGold", Permission::God, Relay::PlayerOnly, opSetGold);
    table.add("Quest", Permission::God, Relay::PlayerOnly, opQuest);
    table.add("QuestEnding", Permission::God, Relay::PlayerOnly, opQuestEnding);
    table.add("NotifyWin", Permission::God, Relay::Allowed, opNotifyWin);
    table.add("Horn", Permission::God, Relay::PlayerOnly, opHorn);
    table.add("Loud", Permission::God, Relay::PlayerOnly, opLoud);
    table.add("Game", Permission::God, Relay::Allowed, opGame);
    table.add("changeSex", Permission::God, Relay::PlayerOnly, opChangeSex);
    table.add("Firecraker", Permission::God, Relay::PlayerOnly, opFirecraker);
    table.add("Bulletin", Permission::God, Relay::PlayerOnly, opBulletin);
    table.add("SetHP", Permission::God, Relay::PlayerOnly, opSetHP);
    table.add("ResetAttr", Permission::God, Relay::PlayerOnly, opResetAttr);
    table.add("CTF", Permission::God, Relay::Allowed, opCTF);
    table.add("ViewDamage", Permission::God, Relay::PlayerOnly, opViewDamage);
    table.add("GoodsReload", Permission::God, Relay::PlayerOnly, opGoodsReload);
    table.add("PetStash", Permission::God, Relay::PlayerOnly, opPetStash);
    table.add("ZoneEvent", Permission::God, Relay::PlayerOnly, opZoneEvent);
    table.add("EventZonePCLimit", Permission::God, Relay::PlayerOnly, opEventZonePCLimit);
    table.add("KickOutAll", Permission::God, Relay::PlayerOnly, opKickOutAll);
    table.add("StartTrap", Permission::God, Relay::PlayerOnly, opStartTrap);
    table.add("SMSTest", Permission::God, Relay::PlayerOnly, opSMSTest);
    table.add("ForceNick", Permission::God, Relay::PlayerOnly, opForceNick);
    table.add("RemoveNick", Permission::God, Relay::PlayerOnly, opRemoveNick);
    table.add("StartGDRLair", Permission::God, Relay::Allowed, opStartGDRLair);
    table.add("ResetGDRLair", Permission::God, Relay::Allowed, opResetGDRLair);
    table.add("GuildRecall", Permission::God, Relay::Allowed, opGuildRecall);
    table.add("ResetSiege", Permission::God, Relay::PlayerOnly, opResetSiege);
    table.add("InitSiege", Permission::God, Relay::PlayerOnly, opInitSiege);
    table.add("IAmAttacker", Permission::God, Relay::PlayerOnly, opIAmAttacker);
    table.add("IAmDefender", Permission::God, Relay::PlayerOnly, opIAmDefender);
    table.add("IAmReinforce", Permission::God, Relay::PlayerOnly, opIAmReinforce);
    table.add("showpcstat", Permission::God, Relay::PlayerOnly, opShowpcstat);
    table.add("advanceclass", Permission::God, Relay::PlayerOnly, opAdvanceclass);
    table.add("addDynamicZone", Permission::God, Relay::Allowed, opAddDynamicZone);
    table.add("enterDynamicZone", Permission::God, Relay::PlayerOnly, opEnterDynamicZone);
    table.add("clearDynamicZone", Permission::God, Relay::PlayerOnly, opClearDynamicZone);
    table.add("setTimeOutAllZoneEffect", Permission::God, Relay::PlayerOnly, opSetTimeOutAllZoneEffect);
    table.add("printTile", Permission::God, Relay::PlayerOnly, opPrintTile);
}

const SubcommandTable& consoleSubcommands() {
    static const SubcommandTable table = [] {
        SubcommandTable built;
        registerConsoleSubcommands(built);
        return built;
    }();
    return table;
}

} // namespace de::gm
