////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionRegisterSiege.cpp
// Written By  :
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "ActionRegisterSiege.h"

#include "CastleInfoManager.h"
#include "Creature.h"
#include "GCModifyInformation.h"
#include "GCNPCResponse.h"
#include "GamePlayer.h"
#include "Guild.h"
#include "GuildManager.h"
#include "GuildWar.h"
#include "NPC.h"
#include "PlayerCreature.h"
#include "Properties.h"
#include "SiegeWar.h"
#include "SystemAvailabilitiesManager.h"
#include "VariableManager.h"
#include "WarSchedule.h"
#include "WarScheduler.h"
#include "Zone.h"
#include "ZoneUtil.h"

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void ActionRegisterSiege::read(PropertyBuffer& propertyBuffer)

{
    __BEGIN_TRY

    try {
        // read script id
        m_ZoneID = propertyBuffer.getPropertyInt("ZoneID");
    } catch (NoSuchElementException& nsee) {
        throw Error(nsee.toString());
    }

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// Execute the action.
////////////////////////////////////////////////////////////////////////////////
void ActionRegisterSiege::execute(Creature* pCreature1, Creature* pCreature2)

{
    __BEGIN_TRY

    Assert(pCreature1 != NULL);
    Assert(pCreature2 != NULL);
    Assert(pCreature1->isNPC());
    Assert(pCreature2->isPC());

    SYSTEM_RETURN_IF_NOT(SYSTEM_GUILD_WAR);

    GCNPCResponse gcNPCResponse;

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature2);
    GuildID_t guildID = pPC->getGuildID();

    if (!g_pVariableManager->isWarActive() || !g_pVariableManager->isActiveGuildWar()) {
        gcNPCResponse.setCode(NPC_RESPONSE_WAR_UNAVAILABLE);
        pPC->getPlayer()->sendPacket(&gcNPCResponse);
        return;
    }

    if (!g_pGuildManager->isGuildMaster(guildID, pPC)) {
        gcNPCResponse.setCode(NPC_RESPONSE_NOT_GUILD_MASTER);
        pPC->getPlayer()->sendPacket(&gcNPCResponse);
        return;
    }

    Gold_t warRegistrationFee = g_pVariableManager->getVariable(WAR_REGISTRATION_FEE);
    if (pPC->getGold() < warRegistrationFee) {
        gcNPCResponse.setCode(NPC_RESPONSE_NOT_ENOUGH_MONEY);
        pPC->getPlayer()->sendPacket(&gcNPCResponse);
        return;
    }

    Zone* pZone = getZoneByZoneID(m_ZoneID);
    Assert(pZone != NULL);
    Assert(pZone->isCastle());

    WarScheduler* pWarScheduler = pZone->getWarScheduler();
    Assert(pWarScheduler != NULL);


    CastleInfo* pCastleInfo = g_pCastleInfoManager->getCastleInfo(m_ZoneID);
    GuildID_t ownerGuildID = pCastleInfo->getGuildID();

    if (guildID == ownerGuildID) {
        gcNPCResponse.setCode(NPC_RESPONSE_ALREADY_HAS_CASTLE);
        pPC->getPlayer()->sendPacket(&gcNPCResponse);
        return;
    }

    // Has a war already been registered?
    if (g_pGuildManager->hasWarSchedule(guildID)) {
        gcNPCResponse.setCode(NPC_RESPONSE_WAR_ALREADY_REGISTERED);
        pPC->getPlayer()->sendPacket(&gcNPCResponse);
        return;
    }

    // Is the war schedule full?

    Schedule* pNextSchedule = pWarScheduler->getRecentSchedule();

    Work* pNextWork = NULL;
    if (pNextSchedule != NULL)
        pNextWork = pNextSchedule->getWork();

    SiegeWar* pNextWar = dynamic_cast<SiegeWar*>(pNextWork);

    if (pNextWar == NULL) {
        pNextWar = new SiegeWar(m_ZoneID, War::WAR_STATE_WAIT);
        pNextWar->addRegistrationFee(warRegistrationFee);
        pNextWar->addChallengerGuild(guildID);

        if (!pWarScheduler->addWar(pNextWar)) {
            gcNPCResponse.setCode(NPC_RESPONSE_WAR_SCHEDULE_FULL);
            pPC->getPlayer()->sendPacket(&gcNPCResponse);

            SAFE_DELETE(pNextWar);
            return;
        }
    } else if (pNextWar->getChallengerGuildCount() < 5) {
        WarSchedule* pNextWarSchedule = dynamic_cast<WarSchedule*>(pNextSchedule);
        Assert(pNextWarSchedule != NULL);

        pNextWar->addRegistrationFee(warRegistrationFee);
        pNextWar->addChallengerGuild(guildID);
        pNextWarSchedule->save();
    } else {
        gcNPCResponse.setCode(NPC_RESPONSE_WAR_SCHEDULE_FULL);
        pPC->getPlayer()->sendPacket(&gcNPCResponse);

        return;
    }

    pPC->decreaseGoldEx(warRegistrationFee);

    GCModifyInformation gcMI;
    gcMI.addLongData(MODIFY_GOLD, pPC->getGold());
    pPC->getPlayer()->sendPacket(&gcMI);

    gcNPCResponse.setCode(NPC_RESPONSE_WAR_REGISTRATION_OK);
    pPC->getPlayer()->sendPacket(&gcNPCResponse);
    return;

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
// get debug string
////////////////////////////////////////////////////////////////////////////////
string ActionRegisterSiege::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ActionRegisterSiege(" << ",ZoneID:" << (int)m_ZoneID << ")";

    return msg.toString();

    __END_CATCH
}
