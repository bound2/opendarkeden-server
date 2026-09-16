//////////////////////////////////////////////////////////////////////////////
// Filename    : ConsoleCommandRegistration.cpp
// Description : Where every *command sub-command is named, gated and bound to
//               the body that answers it. The permission level beside a name
//               here is the only one there is: nothing else asks what a caller
//               may run.
//
//               The order of the rows is the order a word is matched against,
//               and a name is compared whole - a longer word reaches nothing.
//////////////////////////////////////////////////////////////////////////////

#include "gm/ConsoleSubcommands.h"

namespace de::gm {

void registerConsoleSubcommands(SubcommandTable& table) {
    table.add("balanceZoneGroup", Permission::God, opBalanceZoneGroup);
    table.add("regenMasterLair", Permission::God, opRegenMasterLair);
    table.add("showMasterLairStatus", Permission::God, opShowMasterLairStatus);
    table.add("invincible", Permission::God, opInvincible);
    table.add("ghost", Permission::God, opGhost);
    table.add("clearInventory", Permission::God, opClearInventory);
    table.add("clearRankBonus", Permission::God, opClearRankBonus);
    table.add("setCastleOwner", Permission::God, opSetCastleOwner);
    table.add("setCastleOwnerGuild", Permission::God, opSetCastleOwnerGuild);
    table.add("showWarList", Permission::God, opShowWarList);
    table.add("startRaceWar", Permission::God, opStartRaceWar);
    table.add("startWar", Permission::God, opStartWar);
    table.add("removeWar", Permission::God, opRemoveWar);
    table.add("removeRaceWar", Permission::God, opRemoveRaceWar);
    table.add("LevelWar", Permission::God, opLevelWar);
    table.add("saveBloodBibleOwner", Permission::God, opSaveBloodBibleOwner);
    table.add("killAllMonster", Permission::God, opKillAllMonster);
    table.add("killAllPC", Permission::God, opKillAllPC);
    table.add("showZonePCNum", Permission::God, opShowZonePCNum);
    table.add("showPKZonePCNum", Permission::God, opShowPKZonePCNum);
    table.add("setPKZonePCNum", Permission::God, opSetPKZonePCNum);
    table.add("suicide", Permission::God, opSuicide);
    table.add("heal", Permission::God, opHeal);
    table.add("setGold", Permission::God, opSetGold);
    table.add("Quest", Permission::God, opQuest);
    table.add("QuestEnding", Permission::God, opQuestEnding);
    table.add("NotifyWin", Permission::God, opNotifyWin);
    table.add("Horn", Permission::God, opHorn);
    table.add("Loud", Permission::God, opLoud);
    table.add("Game", Permission::God, opGame);
    table.add("changeSex", Permission::God, opChangeSex);
    table.add("Firecraker", Permission::God, opFirecraker);
    table.add("Bulletin", Permission::God, opBulletin);
    table.add("SetHP", Permission::God, opSetHP);
    table.add("ResetAttr", Permission::God, opResetAttr);
    table.add("CTF", Permission::God, opCTF);
    table.add("ViewDamage", Permission::God, opViewDamage);
    table.add("GoodsReload", Permission::God, opGoodsReload);
    table.add("PetStash", Permission::God, opPetStash);
    table.add("ZoneEvent", Permission::God, opZoneEvent);
    table.add("EventZonePCLimit", Permission::God, opEventZonePCLimit);
    table.add("KickOutAll", Permission::God, opKickOutAll);
    table.add("StartTrap", Permission::God, opStartTrap);
    table.add("SMSTest", Permission::God, opSMSTest);
    table.add("ForceNick", Permission::God, opForceNick);
    table.add("RemoveNick", Permission::God, opRemoveNick);
    table.add("StartGDRLair", Permission::God, opStartGDRLair);
    table.add("ResetGDRLair", Permission::God, opResetGDRLair);
    table.add("GuildRecall", Permission::God, opGuildRecall);
    table.add("ResetSiege", Permission::God, opResetSiege);
    table.add("InitSiege", Permission::God, opInitSiege);
    table.add("IAmAttacker", Permission::God, opIAmAttacker);
    table.add("IAmDefender", Permission::God, opIAmDefender);
    table.add("IAmReinforce", Permission::God, opIAmReinforce);
    table.add("showpcstat", Permission::God, opShowpcstat);
    table.add("advanceclass", Permission::God, opAdvanceclass);
    table.add("addDynamicZone", Permission::God, opAddDynamicZone);
    table.add("enterDynamicZone", Permission::God, opEnterDynamicZone);
    table.add("clearDynamicZone", Permission::God, opClearDynamicZone);
    table.add("setTimeOutAllZoneEffect", Permission::God, opSetTimeOutAllZoneEffect);
    table.add("printTile", Permission::God, opPrintTile);
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
