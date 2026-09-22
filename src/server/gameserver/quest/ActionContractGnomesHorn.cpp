////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionContractGnomesHorn.cpp
// Written By  :
// Description :
// Action where the NPC sells items to the player. In practice it
// starts the trade; all it does is send the player the NPC's
// current shop version as a packet.
////////////////////////////////////////////////////////////////////////////////

#include "ActionContractGnomesHorn.h"

#include "Creature.h"
#include "FlagSet.h"
#include "GCNPCResponse.h"
#include "GamePlayer.h"
#include "NPC.h"
#include "PlayerCreature.h"
#include "Script.h"
#include "Trigger.h"

////////////////////////////////////////////////////////////////////////////////
// The ActionContractGnomesHorn action only sends the ShopVersion to the client, so
// there are no parameters that need reading.
////////////////////////////////////////////////////////////////////////////////
void ActionContractGnomesHorn::read(PropertyBuffer& propertyBuffer)

{
    __BEGIN_TRY
    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// Execute the action.
////////////////////////////////////////////////////////////////////////////////
void ActionContractGnomesHorn::execute(Creature* pCreature1, Creature* pCreature2)

{
    __BEGIN_TRY

    Assert(pCreature1 != NULL);
    Assert(pCreature2 != NULL);
    Assert(pCreature1->isNPC());
    Assert(pCreature2->isPC());

    NPC* pNPC = dynamic_cast<NPC*>(pCreature1);
    Player* pPlayer = pCreature2->getPlayer();
    Assert(pPlayer != NULL);

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature2);
    Assert(pPC != NULL);

    FlagSet* pFlagSet = pPC->getFlagSet();
    Assert(pFlagSet != NULL);

    pFlagSet->turnOn(FLAGSET_GNOMES_HORN);
    pFlagSet->save(pPC->getName());

    GCNPCResponse gcNPCResponse;
    gcNPCResponse.setCode(NPC_RESPONSE_GNOME_CONTRACT_OK);

    pPC->getPlayer()->sendPacket(&gcNPCResponse);

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// get debug string
////////////////////////////////////////////////////////////////////////////////
string ActionContractGnomesHorn::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ActionContractGnomesHorn(" << ")";
    return msg.toString();

    __END_CATCH
}
