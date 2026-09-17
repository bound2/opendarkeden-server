//----------------------------------------------------------------------
//
// Filename    : CGPortCheckHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include <stdio.h>

#include "CGPortCheck.h"
#include "DatabaseError.h"
#include "Properties.h"

#ifdef __GAME_SERVER__
#include "repository/SessionRepository.h"

#endif

//----------------------------------------------------------------------
//
// CGPortCheckHander::execute()
//
// When the game server gets a CGPortCheck packet from the login server,
// it adds a new ConnectionInfo.
//
//----------------------------------------------------------------------
void CGPortCheckHandler::execute(CGPortCheck* pPacket)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        const string& host = pPacket->getHost();
    DWORD IP = inet_addr(host.c_str());
    uint port = pPacket->getPort();

    // cout << "CGPortCheck: [" << IP << "] " << host.c_str() << ":" << port << endl;

    try {
        // INSERT IGNORE and, when that changed no row, the UPDATE.
        defaultSessionRepository().recordUserIP(pPacket->getPCName(), IP, port, g_pConfig->getPropertyInt("ServerID"));

        // log(LOG_CGCONNECT, pPacket->getPCName(), "", host);

    } catch (const DatabaseError&) {
        // A SQL failure arrives as END_DB's DatabaseError, already logged
        // to DBError.log; swallowed.
        /*
        try {
            // Try once more
            // (an older retry that re-ran the UPDATE alone; the UPDATE is
            // the second half of recordUserIP)
            defaultSessionRepository().recordUserIP(pPacket->getPCName(), IP, port,
                                                   g_pConfig->getPropertyInt("ServerID"));

            //log(LOG_CGCONNECT, pPacket->getPCName(), "", host);

        } catch (const DatabaseError&) {

            // Ignore it.
            //throw ProtocolException("Duplicated IPInfo");
        }
        */
    }
#else
            cout
        << pPacket->toString() << endl;

#endif

    __END_DEBUG_EX __END_CATCH
}
