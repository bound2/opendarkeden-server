//////////////////////////////////////////////////////////////////////////////
// Filename    : CGReadyHandler.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGReady.h"

#ifdef __GAME_SERVER__
#include "GamePlayer.h"
#include "IncomingPlayerManager.h"
#include "Zone.h"
#include "ZoneGroup.h"
#include "ZonePlayerManager.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// When the client finishes loading its data it sends the game server a CGReady packet.
// The one that gets this packet puts the PC into the Zone's queue and, finally,
// moves the player from the IPM to the ZPM.
//////////////////////////////////////////////////////////////////////////////
void CGReadyHandler::execute(CGReady* pPacket, Player* pPlayer)

{
    __BEGIN_TRY
    __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

    Assert(pPacket != NULL);
    Assert(pPlayer != NULL);


    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);

    Creature* pCreature = pGamePlayer->getCreature();
    Assert(pCreature != NULL);


    //--------------------------------------------------------------------------------
    // Delete the player from the IPM and move it to the ZPM.
    //--------------------------------------------------------------------------------
    try {
        g_pIncomingPlayerManager->deletePlayer(pGamePlayer->getSocket()->getSOCKET());

        // With the Core structure changed, the heartbeat sends them all at once to keep the threads from interfering.
        g_pIncomingPlayerManager->pushOutPlayer(pGamePlayer);
    } catch (NoSuchElementException& nsee) {
        StringStream msg;
        msg << "Critical Error : IPM에 플레이어가 없네용. 무슨 일이지..  - -;\n" << nsee.toString();
        throw Error(msg.toString());
    }

    // For a short while it cannot be attacked by an enemy.
    pGamePlayer->setPlayerStatus(GPS_NORMAL);


#endif

    __END_DEBUG_EX
    __END_CATCH
}
