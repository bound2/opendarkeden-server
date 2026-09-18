//////////////////////////////////////////////////////////////////////////////
// Filename    : CGPartyLeaveHandler.cpp
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGPartyLeave.h"

#ifdef __GAME_SERVER__
#include "Creature.h"
#include "GCPartyError.h"
#include "GCPartyLeave.h"
#include "GamePlayer.h"
#include "PCFinder.h"
#include "Party.h"
#include "SystemAvailabilitiesManager.h"
#include "Zone.h"
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGPartyLeaveHandler::execute(CGPartyLeave* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    SYSTEM_ASSERT(SYSTEM_PARTY);

    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
    string TargetName = pPacket->getTargetName();

    Creature* pCreature = pGamePlayer->getCreature();
    Assert(pCreature != NULL);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    int PartyID = pCreature->getPartyID();
    if (PartyID == 0) {
        throw ProtocolException();
    }

    GCPartyError gcPartyError;

    // An empty target name means one wants to leave the party oneself.
    if (TargetName == "") {
        // Delete from the global party.
        // If one of two members expelled the other,
        // the global party is deleted inside this.
        // The remaining party members' IDs become 0.

        g_pGlobalPartyManager->deletePartyMember(PartyID, pCreature);


        // Delete from the local party.
        LocalPartyManager* pLocalPartyManager = pZone->getLocalPartyManager();
        Assert(pLocalPartyManager != NULL);
        pLocalPartyManager->deletePartyMember(PartyID, pCreature);
    }
    // A different name means one wants to expel another member of the party.
    else {
        // Delete from the global party.
        // If one of two members expelled the other,
        // the global party is deleted inside this.
        // The remaining party members' IDs become 0.
        g_pGlobalPartyManager->expelPartyMember(PartyID, pCreature, TargetName);


        // Delete the expelled one from the local party.
        __ENTER_CRITICAL_SECTION((*g_pPCFinder))

        Creature* pTargetCreature = g_pPCFinder->getCreature_LOCKED(TargetName);

        // NoSuch removed.
        if (pTargetCreature == NULL) {
            return;
        }

        Zone* pTargetZone = pTargetCreature->getZone();
        Assert(pTargetZone != NULL);
        LocalPartyManager* pLocalPartyManager = pTargetZone->getLocalPartyManager();
        Assert(pLocalPartyManager != NULL);


        pLocalPartyManager->deletePartyMember(PartyID, pTargetCreature);

        __LEAVE_CRITICAL_SECTION((*g_pPCFinder))
    }

#endif

    __END_DEBUG_EX __END_CATCH
}
