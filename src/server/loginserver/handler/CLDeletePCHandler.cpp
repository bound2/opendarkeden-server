//////////////////////////////////////////////////////////////////////////////
// Filename    : CLDeletePCHandler.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLDeletePC.h"

#ifdef __LOGIN_SERVER__
#include <cstdio>

#include "Assert.h"
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

    try {
        // The character must exist as an ACTIVE Slayer row and belong to
        // this account.
        string id;
        if (!repo.loadActiveSlayerOwner(WorldID, pPacket->getName(), id)) {
            lcDeletePCError.setErrorID(NOT_FOUND_PLAYER);
            throw InvalidProtocolException("no such slayer exist.");
        }

        if (id != pPlayer->getID()) {
            filelog("DeletePC.log", "Illegal PC Delete : [%s:%s]", pPlayer->getID().c_str(),
                    pPacket->getName().c_str());
            throw InvalidProtocolException("illegal pc delete");
        }

        // Retire the Slayer row of that name and slot.
        if (!repo.retireSlayer(WorldID, pPacket->getName(), pPacket->getSlot())) {
            lcDeletePCError.setErrorID(NOT_FOUND_ID);
            throw InvalidProtocolException("no such slayer exist.");
        }

#if !defined(__CHINA_SERVER__) && !defined(__THAILAND_SERVER__) && !defined(__NETMARBLE_SERVER__)
        repo.recordDeletion(pLoginPlayer->getID(), WorldID, pPacket->getName());
#endif

        // The Vampire and Ousters rows, the character's items, couple
        // entry, effects, flags, time-limited items, event and Mofus rows.
        repo.purgeCharacterRows(WorldID, pPacket->getName(), pPacket->getSlot());

        LCDeletePCOK lcDeletePCOK;
        pLoginPlayer->sendPacket(&lcDeletePCOK);

        pLoginPlayer->setPlayerStatus(LPS_WAITING_FOR_CL_GET_PC_LIST);
    } catch (InvalidProtocolException& ipe) {
        cout << "Fail to deletePC : " << ipe.toString() << endl;

        pLoginPlayer->sendPacket(&lcDeletePCError);
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
