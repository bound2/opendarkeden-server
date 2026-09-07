//////////////////////////////////////////////////////////////////////////////
// Filename    : CLVersionCheckHandler.cpp
// Written By  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLVersionCheck.h"

#ifdef __LOGIN_SERVER__
#include "Assert1.h"
#include "LCVersionCheckError.h"
#include "LCVersionCheckOK.h"
#include "LoginPlayer.h"
#include "repository/LoginConfigRepository.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// �� ��Ŷ�� Ŭ���̾�Ʈ�� ���̵�� �н����带 ��ȣȭ�ؼ�
// �α��� ������ �����Ѵ�. �α��� ������ �� ��Ŷ��
// �޾Ƽ� �÷��̾��� ���̵�� �н����尡 ��Ȯ���� DB�κ���
// �о ���� ��, �α����� ���� ���θ� �����Ѵ�.
//////////////////////////////////////////////////////////////////////////////
void CLVersionCheckHandler::execute(CLVersionCheck* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);
    //----------------------------------------------------------------------
    // *CAUTION*
    // The ClientVersion row must match the version the launcher/patcher distributes.
    //----------------------------------------------------------------------

    // An empty table means a client that never updated; the row is read
    // and otherwise unused.
    int version = 0;
    bool bHasVersion = defaultLoginConfigRepository().loadClientVersion(version);
    Assert(bHasVersion);
    pPacket->getVersion();

    LCVersionCheckOK lcVersionCheckOK;
    pLoginPlayer->sendPacket(&lcVersionCheckOK);

#endif

    __END_DEBUG_EX __END_CATCH
}
