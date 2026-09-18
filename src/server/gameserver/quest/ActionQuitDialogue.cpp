////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionQuitDialogue.cpp
// Written By  :
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "ActionQuitDialogue.h"

#include "Creature.h"
#include "GCNPCResponse.h"
#include "GamePlayer.h"

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void ActionQuitDialogue::read(PropertyBuffer& propertyBuffer)

{
    __BEGIN_TRY

    // This action ends the dialogue between the NPC and the player, so it has
    // no parameters to read.

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
// Execute the action.
////////////////////////////////////////////////////////////////////////////////
void ActionQuitDialogue::execute(Creature* pCreature1, Creature* pCreature2)

{
    __BEGIN_TRY

    Assert(pCreature1 != NULL);
    Assert(pCreature2 != NULL);
    Assert(pCreature1->isNPC());
    Assert(pCreature2->isPC());

    Player* pPlayer = pCreature2->getPlayer();
    Assert(pPlayer != NULL);

    GCNPCResponse response;
    response.setCode(NPC_RESPONSE_QUIT_DIALOGUE);
    pPlayer->sendPacket(&response);

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
// get debug string
////////////////////////////////////////////////////////////////////////////////
string ActionQuitDialogue::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ActionQuitDialogue(" << ")";
    return msg.toString();

    __END_CATCH
}
