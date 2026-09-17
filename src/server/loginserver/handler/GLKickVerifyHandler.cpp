//----------------------------------------------------------------------
//
// Filename    : GLKickVerifyHandler.cpp
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

// include files
#include "GLKickVerify.h"

#ifdef __LOGIN_SERVER__

#include "LCLoginOK.h"
#include "LoginPlayer.h"
#include "LoginPlayerManager.h"

#endif


//----------------------------------------------------------------------
//
// GLKickVerifyHander::execute()
//
// When the game server receives a GLKickVerify packet from the login server,
// a new ReconnectLoginInfo is added.
//
//----------------------------------------------------------------------
void GLKickVerifyHandler::execute(GLKickVerify* pPacket)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX __BEGIN_DEBUG
#ifdef __LOGIN_SERVER__

        // cout << pPacket->toString().c_str() << endl;

        try {

        g_pLoginPlayerManager->lock();

        // LoginPlayer* pLoginPlayer = g_pLoginPlayerManager->getPlayer_NOLOCKED(pPacket->getPCName());
        Player* pPlayer = ((PlayerManager*)g_pLoginPlayerManager)->getPlayer(pPacket->getID());
        LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);

        if (pLoginPlayer != NULL) // not strictly needed since NoSuch is used..
        {
            // The character names must match.
            const string& name1 = pLoginPlayer->getLastCharacterName();
            const string& name2 = pPacket->getPCName();

            if (name1.size() != 0 && name2.size() != 0 && name1 == name2) {
                //				if (pLoginPlayer->isMultiLogin() )
                //				{
                //					pLoginPlayer->setMultiLogin(false);
                pLoginPlayer->sendLCLoginOK();
                //				}
                //				cout << "-_-" << endl;
            } else {
                // A different person. Nothing to worry about.
            }
        }

        g_pLoginPlayerManager->unlock();

    } catch (Throwable&) { // (NoSuchException&) { // would be pointless.
        g_pLoginPlayerManager->unlock();
    }

#endif

    __END_DEBUG
    __END_DEBUG_EX __END_CATCH
}
