//////////////////////////////////////////////////////////////////////////////
// Filename    : CLDeletePCHandler.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLDeletePC.h"

#ifdef __LOGIN_SERVER__
#include <cstdio>

#include "Assert.h"
#include "CharacterDeletion.h"
#include "LCDeletePCError.h"
#include "LCDeletePCOK.h"
#include "LoginPlayer.h"
#include "Properties.h"
#include "repository/LoginCharacterPurgeRepository.h"
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CLDeletePCHandler::execute(CLDeletePC* pPacket, Player* pPlayer) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    cout << pPacket->toString() << endl;

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);
    LCDeletePCError lcDeletePCError;
    WorldID_t WorldID = pLoginPlayer->getWorldID();
    LoginCharacterPurgeRepository& repo = defaultLoginCharacterPurgeRepository();

    DeletePCRequest request;
    request.worldID = WorldID;
    request.playerID = pPlayer->getID();
    request.name = pPacket->getName();
    request.slot = pPacket->getSlot();

    try {
        Outcome<void, DeletePCRejection> outcome = decideDeletePC(request, repo);

        if (outcome.isRejected()) {
            switch (outcome.rejection()) {
            case DeletePCRejection::NoSuchCharacter:
                lcDeletePCError.setErrorID(NOT_FOUND_PLAYER);
                cout << "Fail to deletePC : no such slayer exist." << endl;
                break;

            case DeletePCRejection::NotTheOwner:
                // The character list a session is given holds only its own
                // characters, so a request for another account's is logged.
                filelog("DeletePC.log", "Illegal PC Delete : [%s:%s]", request.playerID.c_str(), request.name.c_str());
                cout << "Fail to deletePC : illegal pc delete" << endl;
                break;

            case DeletePCRejection::SlotMismatch:
                lcDeletePCError.setErrorID(NOT_FOUND_ID);
                cout << "Fail to deletePC : no such slayer exist." << endl;
                break;
            }

            pLoginPlayer->sendPacket(&lcDeletePCError);
            return;
        }

#if !defined(__CHINA_SERVER__) && !defined(__THAILAND_SERVER__) && !defined(__NETMARBLE_SERVER__)
        repo.recordDeletion(request.playerID, WorldID, request.name);
#endif

        // The Vampire and Ousters rows, the character's items, couple
        // entry, effects, flags, time-limited items, event and Mofus rows.
        repo.purgeCharacterRows(WorldID, request.name, request.slot);

        LCDeletePCOK lcDeletePCOK;
        pLoginPlayer->sendPacket(&lcDeletePCOK);

        pLoginPlayer->setPlayerStatus(LPS_WAITING_FOR_CL_GET_PC_LIST);
    } catch (const char*) {
        // A SQL failure arrives as END_DB's const char*, already logged to
        // DBError.log (its own message dangles). The client gets the
        // failure packet with the default error id.
        cout << "Fail to deletePC : SQL error, see DBError.log" << endl;

        pLoginPlayer->sendPacket(&lcDeletePCError);
    }

#endif

    __END_DEBUG_EX __END_CATCH
}
