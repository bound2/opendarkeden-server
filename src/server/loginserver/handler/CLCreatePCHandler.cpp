//////////////////////////////////////////////////////////////////////////////
// Filename    : CLCreatePCHandler.cc
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLCreatePC.h"

#ifdef __LOGIN_SERVER__
#include <string.h>

#include <list>
#include <utility>

#include "Assert.h"
#include "CharacterCreation.h"
#include "DatabaseError.h"
#include "GameServerInfoManager.h"
#include "LCCreatePCError.h"
#include "LCCreatePCOK.h"
#include "LoginPlayer.h"
#include "repository/LoginCharacterRepository.h"
#endif


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CLCreatePCHandler::execute(CLCreatePC* pPacket, Player* pPlayer) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);
    LCCreatePCError lcCreatePCError;
    WorldID_t WorldID = pLoginPlayer->getWorldID();
    LoginCharacterRepository& repo = defaultLoginCharacterRepository();

    // The level-1 balance rows never change while the server runs, so one
    // cache serves every creation.
    static CreatePCBalanceCache balance;

    CreatePCRequest request;
    request.worldID = WorldID;
    request.serverGroupID = pPlayer->getServerGroupID();
    request.playerID = pLoginPlayer->getID();
    request.name = pPacket->getName();
    request.slot = pPacket->getSlot();
    request.sex = pPacket->getSex();
    request.hairStyle = pPacket->getHairStyle();
    request.hairColor = pPacket->getHairColor();
    request.skinColor = pPacket->getSkinColor();
    request.str = pPacket->getSTR();
    request.dex = pPacket->getDEX();
    request.inte = pPacket->getINT();
    request.race = pPacket->getRace();

    try {
        Outcome<CreatedCharacter, CreatePCRejection> outcome = decideCreatePC(request, repo, balance);

        if (outcome.isRejected()) {
            switch (outcome.rejection()) {
            case CreatePCRejection::ReservedName:
            case CreatePCRejection::NameTaken:
            case CreatePCRejection::SlotOccupied:
                lcCreatePCError.setErrorID(ALREADY_REGISTER_ID);
                break;

            case CreatePCRejection::DisallowedCharacters:
            case CreatePCRejection::UnknownRace:
                lcCreatePCError.setErrorID(ETC_ERROR);
                break;

            // The three below mean the client is not speaking the protocol,
            // so the connection is dropped rather than answered with an
            // error packet: the creation screen cannot produce any of them.
            case CreatePCRejection::InvalidAttributes:
                throw InvalidProtocolException("CLCreatePCHandler::too large character attribute");

            case CreatePCRejection::InvalidSlot:
                throw InvalidProtocolException("CLCreatePCHandler::slot out of range");

            case CreatePCRejection::InvalidHairStyle:
                throw InvalidProtocolException("CLCreatePCHandler::hair style out of range");
            }

            pLoginPlayer->sendPacket(&lcCreatePCError); // tell the client the creation failed
            return;
        }

        const CreatedCharacter created = std::move(outcome).events();

        // A vampire's Slayer attributes are rolled by the decision; keep the
        // packet in step with the rows that are written.
        pPacket->setSTR(created.str);
        pPacket->setDEX(created.dex);
        pPacket->setINT(created.inte);

        repo.insertSlayer(WorldID, created.slayer);

        if (created.hasOustersRow) {
            repo.insertOusters(WorldID, created.ousters);
        } else {
            repo.insertVampire(WorldID, created.vampire);
        }

        repo.insertFlagSet(WorldID, created.slayer.name, created.flagSet);

        LCCreatePCOK lcCreatePCOK;
        pLoginPlayer->sendPacket(&lcCreatePCOK);
        pLoginPlayer->setPlayerStatus(LPS_WAITING_FOR_CL_GET_PC_LIST);
    } catch (const DatabaseError&) {
        // A SQL failure arrives as END_DB's DatabaseError, already logged to
        // DBError.log; the client gets the failure packet with ETC_ERROR.
        lcCreatePCError.setErrorID(ETC_ERROR);
        pLoginPlayer->sendPacket(&lcCreatePCError); // tell the client the creation failed
    }

#endif

    __END_DEBUG_EX __END_CATCH
}
