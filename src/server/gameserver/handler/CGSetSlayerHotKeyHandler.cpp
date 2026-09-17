//////////////////////////////////////////////////////////////////////////////
// Filename    : CGSetSlayerHotKeyHandler.cpp
// Written By  : reiot@ewestsoft.com , elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGSetSlayerHotKey.h"

#ifdef __GAME_SERVER__
#include "GamePlayer.h"
//	#include "Vampire.h"
//	#include "Slayer.h"
//	#include "Zone.h"
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGSetSlayerHotKeyHandler::execute(CGSetSlayerHotKey* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

    /*	Assert(pPacket != NULL);
        Assert(pPlayer != NULL);

        GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);

    //	if (pGamePlayer->getPlayerStatus() == GPS_NORMAL) {

            // In this state the creature must have loaded correctly, so it must not be NULL.
            // PLAYER_INGAME itself means the creature loading succeeded.
            Creature* pCreature = pGamePlayer->getCreature();
            Assert(pCreature != NULL);
            if (pCreature->isSlayer()) {
                Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
                for(int i = 0; i < 4; i++) {
                    pSlayer->setHotKey(i, pPacket->getHotKey(i));
                }
            }
    //	}*/

#endif

        __END_DEBUG_EX __END_CATCH
}
