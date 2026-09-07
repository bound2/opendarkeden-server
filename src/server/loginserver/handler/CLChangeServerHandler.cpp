//////////////////////////////////////////////////////////////////////////////
// Filename    : CLChangeServerHandler.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLChangeServer.h"

#ifdef __LOGIN_SERVER__
#include "Assert1.h"
#include "GameServerGroupInfoManager.h"
#include "GameServerInfoManager.h"
#include "LCPCList.h"
#include "LoginPlayer.h"
#include "OptionInfo.h"
#include "repository/LoginAccountRepository.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// 클라이언트가 PC 의 리스트를 달라고 요청해오면, 로그인 서버는 DB로부터
// PC들의 정보를 로딩해서 LCPCList 패킷에 담아서 전송한다.
//////////////////////////////////////////////////////////////////////////////
void CLChangeServerHandler::execute(CLChangeServer* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);

    ServerGroupID_t CurrentServerGroupID = pPacket->getServerGroupID();
    pLoginPlayer->setServerGroupID(CurrentServerGroupID);

    try {
        LCPCList lcPCList;
        pLoginPlayer->makePCList(lcPCList);
        pLoginPlayer->sendPacket(&lcPCList);
        pLoginPlayer->setPlayerStatus(LPS_PC_MANAGEMENT);

        defaultLoginAccountRepository().setCurrentServerGroup((int)pPacket->getServerGroupID(), pLoginPlayer->getID());
    } catch (const char*) {
        // A SQL failure arrives as END_DB's const char*, already logged to
        // DBError.log (its own message dangles).
        throw DisconnectException("CLChangeServerHandler : SQL error, see DBError.log");
    }

#endif

    __END_DEBUG_EX __END_CATCH
}
