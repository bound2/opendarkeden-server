////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionWarpToResurrectPosition.cpp
// Written By  :
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "ActionWarpToResurrectPosition.h"

#include "Creature.h"
#include "GCNPCResponse.h"
#include "GameContext.h"
#include "GamePlayer.h"
#include "NPC.h"
#include "PlayerCreature.h"
#include "ResurrectLocationManager.h"
#include "ZoneUtil.h"
#include "couple/PartnerWaitingManager.h"

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void ActionWarpToResurrectPosition::read(PropertyBuffer& propertyBuffer)

{
    __BEGIN_TRY

    try {
    } catch (NoSuchElementException& nsee) {
        throw Error(nsee.toString());
    }

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// Execute the action.
////////////////////////////////////////////////////////////////////////////////
void ActionWarpToResurrectPosition::execute(Creature* pCreature1, Creature* pCreature2)

{
    __BEGIN_TRY

    Assert(pCreature2 != NULL);
    Assert(pCreature2->isPC());

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature2);
    if (pPC == NULL)
        return;

    ZONE_COORD zoneCoord;
    context().resurrectLocations().getPosition(pPC, zoneCoord);

    transportCreature(pCreature2, zoneCoord.id, zoneCoord.x, zoneCoord.y);

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// get debug string
////////////////////////////////////////////////////////////////////////////////
string ActionWarpToResurrectPosition::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ActionWarpToResurrectPosition(" << ")";

    return msg.toString();

    __END_CATCH
}
