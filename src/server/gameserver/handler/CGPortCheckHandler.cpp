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
#include "Properties.h"

#ifdef __GAME_SERVER__
#include "repository/SessionRepository.h"

#endif

//----------------------------------------------------------------------
//
// CGPortCheckHander::execute()
//
// 게임 서버가 로그인 서버로부터 CGPortCheck 패킷을 받게 되면,
// ConnectionInfo를 새로 추가하게 된다.
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
        // The INSERT IGNORE and, when it changed no row, the UPDATE — one
        // seam call, same two statements on one Statement as before.
        defaultSessionRepository().recordUserIP(pPacket->getPCName(), IP, port, g_pConfig->getPropertyInt("ServerID"));

        // log(LOG_CGCONNECT, pPacket->getPCName(), "", host);

    } catch (const char*) {
        // A SQL failure crosses the seam as END_DB's const char* (the
        // SQLQueryException this caught before is converted inside the
        // seam, which also writes the DBError.log line the handler never
        // wrote). Swallowed, as before.
        /*
        try {
            // 다시 한번 시도
            // (an older retry of the UPDATE that recordUserIP now runs when the
            // INSERT IGNORE changed no row; this copy fed the DWORD to %ld and
            // the uint to %d where the live one feeds %lu and %u)
            defaultSessionRepository().recordUserIP(pPacket->getPCName(), IP, port,
                                                   g_pConfig->getPropertyInt("ServerID"));

            //log(LOG_CGCONNECT, pPacket->getPCName(), "", host);

        } catch (const char*) {

            // 무시한다.
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
