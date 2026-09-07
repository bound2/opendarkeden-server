//////////////////////////////////////////////////////////////////////////////
// Filename    : CLSelectServerHandler.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLSelectServer.h"

#ifdef __LOGIN_SERVER__
#include "Assert1.h"
#include "GameServerGroupInfoManager.h"
#include "GameServerInfoManager.h"
#include "GameWorldInfoManager.h"
#include "LCPCList.h"
#include "LoginPlayer.h"
#include "OptionInfo.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// 클라이언트가 PC 의 리스트를 달라고 요청해오면, 로그인 서버는 DB로부터
// PC들의 정보를 로딩해서 LCPCList 패킷에 담아서 전송한다.
//////////////////////////////////////////////////////////////////////////////
void CLSelectServerHandler::execute(CLSelectServer* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);

    ServerGroupID_t CurrentServerGroupID = pPacket->getServerGroupID();

    WorldID_t WorldID = pLoginPlayer->getWorldID();

    // Assert (WorldID <= g_pGameWorldInfoManager->getSize());
    int MaxWorldID = g_pGameWorldInfoManager->getSize();
    if (WorldID > MaxWorldID) {
        WorldID = MaxWorldID;
    }

    // Assert (CurrentServerGroupID <= g_pGameServerGroupInfoManager->getSize(WorldID ));
    int MaxServerGroupID = g_pGameServerGroupInfoManager->getSize(WorldID);
    if (CurrentServerGroupID > MaxServerGroupID) {
        CurrentServerGroupID = MaxServerGroupID;
    }


    // by sigi. 2003.1.7
    GameServerGroupInfo* pGameServerGroupInfo =
        g_pGameServerGroupInfoManager->getGameServerGroupInfo(CurrentServerGroupID, WorldID);

    Assert(pGameServerGroupInfo != NULL);
    if (pGameServerGroupInfo->getStat() == SERVER_DOWN) {
        filelog("errorLogin.txt", "Server Closed: %d", CurrentServerGroupID);
        throw DisconnectException("ServerClosed");
    }


    pLoginPlayer->setServerGroupID(CurrentServerGroupID);

    //----------------------------------------------------------------------
    // 이제 LCPCList 패킷을 만들어 보내자
    //----------------------------------------------------------------------
    LCPCList lcPCList;
    pLoginPlayer->makePCList(lcPCList);

#ifdef __NETMARBLE_SERVER__
    // 넷마블 사용자 약관 동의 여부 확인
    lcPCList.setAgree(pLoginPlayer->isAgree());
#endif

    pLoginPlayer->sendPacket(&lcPCList);
    pLoginPlayer->setPlayerStatus(LPS_PC_MANAGEMENT);

    // The selected group is not written back here; CLChangeServerHandler
    // does that through LoginAccountRepository::setCurrentServerGroup.

#endif

    __END_DEBUG_EX __END_CATCH
}
