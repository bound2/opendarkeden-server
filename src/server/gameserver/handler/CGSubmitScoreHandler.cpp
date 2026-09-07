//////////////////////////////////////////////////////////////////////////////
// Filename    : CGSubmitScoreHandler.cpp
// Written By  : crazydog
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGSubmitScore.h"

#ifdef __GAME_SERVER__
#include "CreatureUtil.h"
#include "GCMiniGameScores.h"
#include "GamePlayer.h"
#include "PacketUtil.h"
#include "PlayerCreature.h"
#include "mission/QuestManager.h"
#include "repository/PlayRecordRepository.h"
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGSubmitScoreHandler::execute(CGSubmitScore* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX __BEGIN_DEBUG

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    if (pPacket->getLevel() > 2 || pPacket->getGameType() > 3)
        return;

    try {
        GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
        Assert(pGamePlayer != NULL);

        if (pGamePlayer->getPlayerStatus() == GPS_NORMAL) {
            PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pGamePlayer->getCreature());
            Assert(pPC != NULL);

            defaultPlayRecordRepository().recordMiniGameScore(pPC->getName(), pPacket->getScore(),
                                                              pPacket->getGameType(), pPacket->getLevel());
            //						"INSERT INTO MiniGameScores (Name, Type, Level, Score, Time) VALUES
            //('%s',%u,%u,%u,now())", pPC->getName().c_str(), 						pPacket->getGameType(),
            // pPacket->getLevel(), pPacket->getScore();

            if (pPacket->getLevel() == 2) {
                pPC->getQuestManager()->submitMiniGameScore(pPacket->getGameType(), pPacket->getScore());
                if (pPacket->getGameType() == 0) {
                    addOlympicStat(pPC, 11);
                } else {
                    addOlympicStat(pPC, 12);
                }
            } else
                sendGCMiniGameScores(pPC, pPacket->getGameType(), pPacket->getLevel() + 1);
        }
    } catch (Throwable& t) {
        // cout << t.toString();
    }

#endif

    __END_DEBUG
    __END_DEBUG_EX __END_CATCH
}
