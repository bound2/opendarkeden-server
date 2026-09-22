////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionDownSkill.cpp
// Written By  :
// Description :
// Makes the client open the skill-lowering window through an NPC.
////////////////////////////////////////////////////////////////////////////////

#include "ActionDownSkill.h"

#include "Creature.h"
#include "GCNPCResponse.h"
#include "GamePlayer.h"

////////////////////////////////////////////////////////////////////////////////
// read from property buffer
////////////////////////////////////////////////////////////////////////////////
void ActionDownSkill::read(PropertyBuffer& propertyBuffer)

{
    __BEGIN_TRY

    // Opening the skill-lowering window takes no arguments, so nothing is read here.

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// Execute the action.
////////////////////////////////////////////////////////////////////////////////
void ActionDownSkill::execute(Creature* pCreature1, Creature* pCreature2)

{
    __BEGIN_TRY

    Assert(pCreature1 != NULL);
    Assert(pCreature2 != NULL);
    Assert(pCreature1->isNPC());
    Assert(pCreature2->isPC());

    Player* pPlayer = pCreature2->getPlayer();
    Assert(pPlayer != NULL);

    GCNPCResponse okpkt;
    okpkt.setCode(NPC_RESPONSE_DOWN_SKILL);
    pPlayer->sendPacket(&okpkt);

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// get debug string
////////////////////////////////////////////////////////////////////////////////
string ActionDownSkill::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ActionDownSkill(" << ")";
    return msg.toString();

    __END_CATCH
}
