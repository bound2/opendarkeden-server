//////////////////////////////////////////////////////////////////////////////
// Filename    : CGUntransformHandler.cpp
// Written By  : reiot@ewestsoft.com , elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGUntransform.h"

#ifdef __GAME_SERVER__
#include "Creature.h"
#include "GCRemoveEffect.h"
#include "GCUntransformFail.h"
#include "GamePlayer.h"
#include "Ousters.h"
#include "Zone.h"
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGUntransformHandler::execute(CGUntransform* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);

    if (pGamePlayer->getPlayerStatus() == GPS_NORMAL) {
        /*
        Creature* pCreature = pGamePlayer->getCreature();
        Assert(pCreature != NULL);

        if (pCreature->isOusters() )
        {
            Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);
            if (pOusters->isFlag(Effect::EFFECT_CLASS_SUMMON_SYLPH) )
            {

                // Take the Flag off the player, and
                pOusters->removeFlag(Effect::EFFECT_CLASS_SUMMON_SYLPH);
                Zone* pZone = pCreature->getZone();

                // tell the zone the effect was removed, and
                GCRemoveEffect removeEffect;
                removeEffect.setObjectID(pOusters->getObjectID());
                removeEffect.addEffectList(Effect::EFFECT_CLASS_SUMMON_SYLPH);
                pZone->broadcastPacket(pOusters->getX(), pOusters->getY(), &removeEffect);
                pPlayer->sendPacket(&removeEffect);

                GCModifyInformation gcMI;

                // recompute defense and protection and send them
                OUSTERS_RECORD prev;
                pOusters->getOustersRecord(prev);
                pOusters->initAllStat();
                pOusters->addModifyInfo(prev, gcMI);

                pPlayer->sendPacket(&gcMI);
            }
        }*/
        /*
        if (pCreature->isDead())
            return;

        Zone* pZone = NULL;
        if ((pZone = pCreature->getZone()) == NULL)
            return;

        pZone->addUntransformCreature(pCreature , true);



        if (pSkillHandler != NULL &&
                (pVampire->isFlag(Effeft::EFFECT_CLASS_TRANSFORM_TO_BAT) ||
        pVampire->isFlag(Effect::EFFECT_CLASS_TRANSFORM_TO_WOLF))) { Assert(pCreature->isVampire()); pSkillHandler =
        g_pSkillHandlerManager->getSkillHandler(SKILL_ATTACK_MELEE); pSkillHandler->execute(pVampire,
        pCGAttack->getObjectID());
        }
        */
    }

#endif

    __END_DEBUG_EX __END_CATCH
}

/*
        int packetX = pPacket->getX(), packetY = pPacket->getY();

        int x = pCreature->getX(), y = pCreature->getY();
        int minX = max(0, x-1), maxX = min(x+1, pZone->getWidth()-1);
        int minY = max(0, y-1), maxY = min(y+1, pZone->getHeight()-1);

        if (!(pCreature->isFlag(Effect::EFFECT_CLASS_TRANSFORM_TO_WOLF)
                || pCreature->isFlag(Effect::EFFECT_CLASS_TRANSFORM_TO_BAT))
            && (minX <= packetX && maxX >= packetX)
            && (minY <= packetY && maxY >= packetY)
            )
        {
            GCUntransformFail gcUntransformFail;
            pGamePlayer->sendPacket(&gcUntransformFail);
            return;
        }
*/
