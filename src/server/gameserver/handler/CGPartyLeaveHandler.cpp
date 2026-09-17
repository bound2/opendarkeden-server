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
        // filelog("PARTY_EXCEPTION.log", "CGPartyLeaveHandler::execute() : the party ID is 0. [%s]",
        // pCreature->toString().c_str());
        throw ProtocolException();
    }

    GCPartyError gcPartyError;

    // An empty target name means one wants to leave the party oneself.
    if (TargetName == "") {
        // Delete from the global party.
        // If one of two members expelled the other,
        // the global party is deleted inside this.
        // The remaining party members' IDs become 0.
        // cout << "===== Global party manager state before leaving the party" << endl;
        // cout << g_pGlobalPartyManager->toString() << endl;
        // cout << "================================================" << endl;

        g_pGlobalPartyManager->deletePartyMember(PartyID, pCreature);

        // cout << "===== Global party manager state after leaving the party" << endl;
        // cout << g_pGlobalPartyManager->toString() << endl;
        // cout << "================================================" << endl;

        // Delete from the local party.
        LocalPartyManager* pLocalPartyManager = pZone->getLocalPartyManager();
        Assert(pLocalPartyManager != NULL);
        pLocalPartyManager->deletePartyMember(PartyID, pCreature);
    }
    // A different name means one wants to expel another member of the party.
    else {
        // cout << "===== Global party manager state before the expulsion" << endl;
        // cout << g_pGlobalPartyManager->toString() << endl;
        // cout << "=======================================" << endl;

        // Delete from the global party.
        // If one of two members expelled the other,
        // the global party is deleted inside this.
        // The remaining party members' IDs become 0.
        g_pGlobalPartyManager->expelPartyMember(PartyID, pCreature, TargetName);

        // cout << "===== Global party manager state after the expulsion" << endl;
        // cout << g_pGlobalPartyManager->toString() << endl;
        // cout << "=======================================" << endl;

        // Delete the expelled one from the local party.
        __ENTER_CRITICAL_SECTION((*g_pPCFinder))

        Creature* pTargetCreature = g_pPCFinder->getCreature_LOCKED(TargetName);
        // Assert(pTargetCreature != NULL);

        // NoSuch removed.
        if (pTargetCreature == NULL) {
            return;
        }

        Zone* pTargetZone = pTargetCreature->getZone();
        Assert(pTargetZone != NULL);
        LocalPartyManager* pLocalPartyManager = pTargetZone->getLocalPartyManager();
        Assert(pLocalPartyManager != NULL);

        // cout << "===== Local party manager state before the expulsion" << endl;
        // cout << pLocalPartyManager->toString() << endl;
        // cout << "=======================================" << endl;

        pLocalPartyManager->deletePartyMember(PartyID, pTargetCreature);
        // cout << "The expelled player was deleted from the local party." << endl;

        __LEAVE_CRITICAL_SECTION((*g_pPCFinder))
    }

#endif

    __END_DEBUG_EX __END_CATCH
}
