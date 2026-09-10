//////////////////////////////////////////////////////////////////////////////
// Filename    : CLQueryCharacterNameHandler.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLQueryCharacterName.h"

#ifdef __LOGIN_SERVER__
#include "Assert.h"
#include "GameWorldInfoManager.h"
#include "LCQueryResultCharacterName.h"
#include "LoginPlayer.h"
#include "repository/LoginCharacterRepository.h"
#endif

bool isAvailableID(const char* pID);

//////////////////////////////////////////////////////////////////////////////
// DB 로부터 특정 플레이어 아이디를 찾아서 그 여부를 클라이언트로 리턴해준다.
//////////////////////////////////////////////////////////////////////////////
void CLQueryCharacterNameHandler::execute(CLQueryCharacterName* pPacket, Player* pPlayer) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);

    WorldID_t WorldID = pLoginPlayer->getWorldID();

    Assert(WorldID <= g_pGameWorldInfoManager->getSize());

    // A SQL failure leaves as END_DB's DatabaseError, the way the
    // SQLQueryException did.
    bool bExists = defaultLoginCharacterRepository().slayerNameExists(WorldID, pPacket->getCharacterName());

    LCQueryResultCharacterName lcQueryResultCharacterName;

    lcQueryResultCharacterName.setCharacterName(pPacket->getCharacterName());

    lcQueryResultCharacterName.setExist(bExists);

    // A name reserved for staff is reported as taken.
    if (!isAvailableID(pPacket->getCharacterName().c_str())) {
        lcQueryResultCharacterName.setExist(true);
    }

    pLoginPlayer->sendPacket(&lcQueryResultCharacterName);

    // The client may query several names; the status stays where the
    // character list is expected next.
    pLoginPlayer->setPlayerStatus(LPS_WAITING_FOR_CL_GET_PC_LIST);

#endif

    __END_DEBUG_EX __END_CATCH
}
