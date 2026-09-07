//////////////////////////////////////////////////////////////////////////////
// Filename    : CLGetServerListHandler.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLGetServerList.h"

#ifdef __LOGIN_SERVER__
#include "Assert1.h"
#include "GameServerGroupInfoManager.h"
#include "GameServerInfoManager.h"
#include "LCServerList.h"
#include "LoginPlayer.h"
#include "ServerGroupInfo.h"
#include "UserInfoManager.h"
#include "repository/LoginAccountRepository.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// 클라이언트가 서버의 리스트를 달라고 요청해오면, 로그인 서버는 DB로부터
// 서버들의 정보를 로딩해서 LCServerList 패킷에 담아서 전송한다.
//////////////////////////////////////////////////////////////////////////////
void CLGetServerListHandler::execute(CLGetServerList* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);
    // cout << "Start execute" << endl;

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);

    WorldID_t WorldID = pLoginPlayer->getWorldID();

    try {
        int GroupNum = g_pGameServerGroupInfoManager->getSize(WorldID);

        // cout << "ServerNum : " << GroupNum << endl;

        ServerGroupInfo* aServerGroupInfo[GroupNum];

        for (int i = 0; i < GroupNum; i++) {
            ServerGroupInfo* pServerGroupInfo = new ServerGroupInfo();
            GameServerGroupInfo* pGameServerGroupInfo =
                g_pGameServerGroupInfoManager->getGameServerGroupInfo(i, WorldID);
            pServerGroupInfo->setGroupID(pGameServerGroupInfo->getGroupID());
            pServerGroupInfo->setGroupName(pGameServerGroupInfo->getGroupName());
            pServerGroupInfo->setStat(SERVER_FREE);

            UserInfo* pUserInfo = g_pUserInfoManager->getUserInfo(pGameServerGroupInfo->getGroupID(), WorldID);


            WORD UserModify = 800;
            WORD UserMax = 1500;

            if (pUserInfo->getUserNum() < 100 + UserModify) {
                pServerGroupInfo->setStat(SERVER_FREE);
            } else if (pUserInfo->getUserNum() < 250 + UserModify) {
                pServerGroupInfo->setStat(SERVER_NORMAL);
            } else if (pUserInfo->getUserNum() < 400 + UserModify) {
                pServerGroupInfo->setStat(SERVER_BUSY);
            } else if (pUserInfo->getUserNum() < 500 + UserModify) {
                pServerGroupInfo->setStat(SERVER_VERY_BUSY);
            } else // if (pUserInfo->getUserNum() >= 500 + UserModify )
            {
                pServerGroupInfo->setStat(SERVER_FULL);
            }
            // else
            {
                // pServerGroupInfo->setStat(SERVER_DOWN);
            }

            if (pUserInfo->getUserNum() >= UserMax) {
                pServerGroupInfo->setStat(SERVER_FULL);
            }

            if (pGameServerGroupInfo->getStat() == SERVER_DOWN) {
                pServerGroupInfo->setStat(SERVER_DOWN);
            }

            aServerGroupInfo[i] = pServerGroupInfo;

            // cout << "AddServer : " << pServerGroupInfo->getGroupName() << endl;
        }

        LCServerList lcServerList;

        // The account's current world and group. Both values land in the
        // server group field, the second overwriting the first.
        int currentWorldID = 0;
        int currentServerGroupID = 0;
        if (defaultLoginAccountRepository().loadCurrentLocation(LOGIN_LOCATION_SQL_LOWER, pLoginPlayer->getID(),
                                                                currentWorldID, currentServerGroupID)) {
            lcServerList.setCurrentServerGroupID(currentWorldID);
            lcServerList.setCurrentServerGroupID(currentServerGroupID);
        }

        for (int k = 0; k < GroupNum; k++) {
            lcServerList.addListElement(aServerGroupInfo[k]);
        }

        pLoginPlayer->sendPacket(&lcServerList);

        pLoginPlayer->setPlayerStatus(LPS_PC_MANAGEMENT);
    } catch (Throwable& t) {
        // cout << t.toString() << endl;
    }
    // cout << "End execute" << endl;

#endif

    __END_DEBUG_EX __END_CATCH
}
