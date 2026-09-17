
//////////////////////////////////////////////////////////////////////
//
// Filename    : CGAuthKeyHandler.cc
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "CGAuthKey.h"

#ifdef __GAME_SERVER__
#include "EventKick.h"
#include "GCSystemMessage.h"
#include "GamePlayer.h"
#endif

//////////////////////////////////////////////////////////////////////
//
// Method run when the client receives a message from the server.
//
//////////////////////////////////////////////////////////////////////
void CGAuthKeyHandler::execute(CGAuthKey* pPacket, Player* pPlayer)

{
    __BEGIN_TRY

#ifdef __GAME_SERVER__

    Assert(pPacket != NULL);

    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
    Assert(pGamePlayer != NULL);

    DWORD key = pPacket->getKey();
    // 	if ( !pGamePlayer->getCSAuth().CheckAuthDword(key) )
    // 	{
    // 		filelog("CSAuth.log", "[%s] The authentication value is wrong.", pGamePlayer->getID().c_str());

    // 		GCSystemMessage gcSystemMessage;
    // 		gcSystemMessage.setMessage("nProtect GameGuard authentication failed. The executable is wrong or the GameGuard files are damaged.");
    // 		pGamePlayer->sendPacket( &gcSystemMessage );

    // 		EventKick* pKick = new EventKick( pGamePlayer );
    // 		pKick->setDeadline(100);
    // //		pKick->setMessage("The GameGuard authentication code is wrong. The connection closes in 10 seconds.");
    // 		pKick->sendMessage();

    // 		pGamePlayer->addEvent(pKick);
    // 	}

#endif

    __END_CATCH
}
