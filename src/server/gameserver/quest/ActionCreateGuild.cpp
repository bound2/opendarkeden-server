////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionCreateGuild.cpp
// Written By  :
// Description :
// Makes the client open the guild creation window through an NPC.
////////////////////////////////////////////////////////////////////////////////

#include "ActionCreateGuild.h"

#include "Creature.h"
#include "GCNPCResponse.h"
#include "GamePlayer.h"
#include "SystemAvailabilitiesManager.h"

////////////////////////////////////////////////////////////////////////////////
// read from property buffer
////////////////////////////////////////////////////////////////////////////////
void ActionCreateGuild::read(PropertyBuffer& propertyBuffer)

{
    __BEGIN_TRY

    // Opening the guild creation window takes no arguments, so nothing is read here.

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// Execute the action.
////////////////////////////////////////////////////////////////////////////////
void ActionCreateGuild::execute(Creature* pCreature1, Creature* pCreature2)

{
    __BEGIN_TRY

    Assert(pCreature1 != NULL);
    Assert(pCreature2 != NULL);
    Assert(pCreature1->isNPC());
    Assert(pCreature2->isPC());

    SYSTEM_RETURN_IF_NOT(SYSTEM_GUILD);

    Player* pPlayer = pCreature2->getPlayer();
    Assert(pPlayer != NULL);

    GCNPCResponse okpkt;
    okpkt.setCode(NPC_RESPONSE_INTERFACE_CREATE_GUILD);
    pPlayer->sendPacket(&okpkt);

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// get debug string
////////////////////////////////////////////////////////////////////////////////
string ActionCreateGuild::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ActionCreateGuild(" << ")";
    return msg.toString();

    __END_CATCH
}
