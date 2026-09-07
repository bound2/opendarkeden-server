//////////////////////////////////////////////////////////////////////////////
// Filename    : CLQueryPlayerIDHandler.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLQueryPlayerID.h"

#ifdef __LOGIN_SERVER__
#include "Assert1.h"
#include "LCQueryResultPlayerID.h"
#include "LoginPlayer.h"
#include "repository/LoginAccountRepository.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// DB 로부터 특정 플레이어 아이디를 찾아서 그 여부를 클라이언트로 리턴해준다.
//////////////////////////////////////////////////////////////////////////////
void CLQueryPlayerIDHandler::execute(CLQueryPlayerID* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);

    // A SQL failure leaves as END_DB's const char*, the way the
    // SQLQueryException did.
    bool bExists = defaultLoginAccountRepository().accountNameExists(pPacket->getPlayerID());

    LCQueryResultPlayerID lcQueryResultPlayerID;

    lcQueryResultPlayerID.setPlayerID(pPacket->getPlayerID());

    lcQueryResultPlayerID.setExist(bExists);

    pLoginPlayer->sendPacket(&lcQueryResultPlayerID);

    // The client may query several ids; the status stays where the
    // registration is expected next.
    pLoginPlayer->setPlayerStatus(LPS_WAITING_FOR_CL_REGISTER_PLAYER);

#endif

    __END_DEBUG_EX __END_CATCH
}
