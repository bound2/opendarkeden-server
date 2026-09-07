//////////////////////////////////////////////////////////////////////////////
// Filename    : CLGetWorldListHandler.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLGetWorldList.h"

#ifdef __LOGIN_SERVER__
#include "Assert1.h"
#include "GameWorldInfoManager.h"
#include "LCWorldList.h"
#include "LoginPlayer.h"
#include "WorldInfo.h"
#include "repository/LoginAccountRepository.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// 클라이언트가 서버의 리스트를 달라고 요청해오면, 로그인 서버는 DB로부터
// 서버들의 정보를 로딩해서 LCWorldList 패킷에 담아서 전송한다.
//////////////////////////////////////////////////////////////////////////////
void CLGetWorldListHandler::execute(CLGetWorldList* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);
    // cout << "Start execute" << endl;

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);

    try {
        int Num = g_pGameWorldInfoManager->getSize();

        // cout << "WorldNum : " << Num << endl;

        WorldInfo* aWorldInfo[Num];

        for (int i = 1; i < Num + 1; i++) {
            WorldInfo* pWorldInfo = new WorldInfo();
            GameWorldInfo* pGameWorldInfo = g_pGameWorldInfoManager->getGameWorldInfo(i);
            pWorldInfo->setID(pGameWorldInfo->getID());
            pWorldInfo->setName(pGameWorldInfo->getName());

            // by bezz. 2002.12.20
            pWorldInfo->setStat(pGameWorldInfo->getStatus());

            aWorldInfo[i] = pWorldInfo;

            // 트랜실(2) 빼기
            // if (i==2) pWorldInfo->setStat(WORLD_CLOSE);

            // cout << "AddWorld : " << pWorldInfo->getName() << endl;
        }

        LCWorldList lcWorldList;

        int currentWorldID = 0;
        if (defaultLoginAccountRepository().loadCurrentWorld(pLoginPlayer->getID(), currentWorldID)) {
            lcWorldList.setCurrentWorldID(currentWorldID);
        }

        for (int k = 1; k < Num + 1; k++) {
            lcWorldList.addListElement(aWorldInfo[k]);
        }

        pLoginPlayer->sendPacket(&lcWorldList);

        //		pLoginPlayer->setPlayerStatus(LPS_PC_MANAGEMENT);
    } catch (Throwable& t) {
        // cout << t.toString() << endl;
    }
    // cout << "End execute" << endl;

#endif

    __END_DEBUG_EX __END_CATCH
}
