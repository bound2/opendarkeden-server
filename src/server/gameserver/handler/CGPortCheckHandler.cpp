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
        // INSERT IGNORE and, when that changed no row, the UPDATE.
        defaultSessionRepository().recordUserIP(pPacket->getPCName(), IP, port, g_pConfig->getPropertyInt("ServerID"));

        // log(LOG_CGCONNECT, pPacket->getPCName(), "", host);

    } catch (const char*) {
        // A SQL failure arrives as END_DB's const char*, already logged
        // to DBError.log; swallowed.
        /*
        try {
            // 다시 한번 시도
            // (an older retry that re-ran the UPDATE alone; the UPDATE is
            // the second half of recordUserIP)
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
