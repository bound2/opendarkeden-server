//////////////////////////////////////////////////////////////////////////////
// Filename    : CGRequestIPHandler.cc
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGRequestIP.h"

#ifdef __GAME_SERVER__
#include "Creature.h"
#include "GCRequestFailed.h"
#include "GCRequestedIP.h"
#include "GamePlayer.h"
#include "PCFinder.h"
#include "repository/SessionRepository.h"
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGRequestIPHandler::execute(CGRequestIP* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    try {
        // UserIPInfo 테이블에서 사용자 IP를 쿼리 한다.
        {
            DWORD ip = 0;
            DWORD port = 0;

            if (!defaultSessionRepository().loadUserIP(pPacket->getName(), ip, port)) {
                throw NoSuchElementException("요청한 ID의 IP정보가 없음다.");
            } else {
                IP_t IP = ip;
                uint Port = port;
                // cout << "Requested IP : " << IP	<< endl;

                GCRequestedIP gcRequestedIP;
                gcRequestedIP.setIP(IP);
                gcRequestedIP.setPort(Port);
                gcRequestedIP.setName(pPacket->getName().c_str());
                pPlayer->sendPacket(&gcRequestedIP);
            }
        }
    }
    // catch (NoSuchElementException & nsee)
    catch (Throwable& t) {
        // no such인 경우..
        GCRequestFailed gcRequestFailed;
        gcRequestFailed.setCode(REQUEST_FAILED_IP);
        gcRequestFailed.setName(pPacket->getName());
        pPlayer->sendPacket(&gcRequestFailed);
    } catch (...) {
        GCRequestFailed gcRequestFailed;
        gcRequestFailed.setCode(REQUEST_FAILED_IP);
        gcRequestFailed.setName(pPacket->getName());
        pPlayer->sendPacket(&gcRequestFailed);
    }

#endif

    __END_DEBUG_EX __END_CATCH
}
