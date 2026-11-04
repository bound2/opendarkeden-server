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
#include "KernelContext.h"
#include "PortCheck.h"
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
    __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

    const string& host = pPacket->getHost();
    DWORD IP = inet_addr(host.c_str());
    uint port = pPacket->getPort();


    try {
        // A name character creation would refuse, or one no statement may
        // carry, is refused before any statement sees it (PortCheck.h); the
        // repository escapes the one it records: INSERT IGNORE and, when
        // that changed no row, the UPDATE.
        de::recordPortCheck(defaultSessionRepository(), pPacket->getPCName(), IP, port,
                            de::kernelContext().config().getPropertyInt("ServerID"));
    } catch (const DatabaseError&) {
        // A SQL failure arrives as END_DB's DatabaseError, already logged
        // to DBError.log; swallowed.
    }
#else
    cout << pPacket->toString() << endl;

#endif

    __END_DEBUG_EX
}
