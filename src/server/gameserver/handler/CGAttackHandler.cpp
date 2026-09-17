//////////////////////////////////////////////////////////////////////////////
// Filename    : CGAttackHandler.cc
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGAttack.h"

#ifdef __GAME_SERVER__
#include "CreatureUtil.h"
#include "Effect.h"
#include "GCAttack.h"
#include "GCGetDamage.h"
#include "GCSkillFailed1.h"
#include "GamePlayer.h"
#include "ItemUtil.h"
#include "Monster.h"
#include "Ousters.h"
#include "Skill.h"
#include "SkillHandlerManager.h"
#include "Vampire.h"
#include "ZoneUtil.h"
#include "skill/Sniping.h"

// #define __PROFILE_SKILLS__

#ifdef __PROFILE_SKILLS__
#include "Profile.h"
#endif
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGAttackHandler::execute(CGAttack* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX __BEGIN_DEBUG

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    try {
        GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
        Assert(pGamePlayer != NULL); // by sigi

        // Return if the player is not in a normal state.
        if (pGamePlayer->getPlayerStatus() != GPS_NORMAL) {
            GCSkillFailed1 _GCSkillFailed1;
            _GCSkillFailed1.setSkillType(SKILL_ATTACK_MELEE);
            pPlayer->sendPacket(&_GCSkillFailed1);
            return;
        }

        Creature* pCreature = pGamePlayer->getCreature();
        Assert(pCreature != NULL); // by sigi

        Zone* pZone = pCreature->getZone();
        Assert(pZone != NULL);

        // A complete safe zone forbids skill use.
        ZoneLevel_t ZoneLevel = pZone->getZoneLevel(pCreature->getX(), pCreature->getY());
        if (ZoneLevel & COMPLETE_SAFE_ZONE) {
            GCSkillFailed1 _GCSkillFailed1;
            _GCSkillFailed1.setSkillType(SKILL_ATTACK_MELEE);
            pPlayer->sendPacket(&_GCSkillFailed1);
            return;
        }

        Creature* pTarget = pZone->getCreature(pPacket->getObjectID());
        if (pTarget == NULL) {
            GCSkillFailed1 _GCSkillFailed1;
            _GCSkillFailed1.setSkillType(SKILL_ATTACK_MELEE);
            pPlayer->sendPacket(&_GCSkillFailed1);
            return;
        }

        pCreature->setLastTarget(pTarget->getObjectID());

        // Do not attack a relic table of one's own race.

        /*
        if ( pTarget->isMonster() )
        {
            Monster* pMonster = dynamic_cast<Monster*>(pTarget);
            Assert( pMonster != NULL );

            MonsterType_t type = pMonster->getMonsterType();

            if ( ( type == 371 || type == 372 || type == 373 ) && pCreature->isSlayer() )
            {
                GCSkillFailed1 _GCSkillFailed1;
                _GCSkillFailed1.setSkillType(SKILL_ATTACK_MELEE);
                pPlayer->sendPacket( &_GCSkillFailed1 );
                return;
            }

            if ( ( type == 374 || type == 375 || type == 376) && pCreature->isVampire() )
            {
                GCSkillFailed1 _GCSkillFailed1;
                _GCSkillFailed1.setSkillType(SKILL_ATTACK_MELEE);
                pPlayer->sendPacket( &_GCSkillFailed1 );
                return;
            }

            if ( type >= 371 && type <= 376)
            {
                string RelicName = "";

                switch(type)
                {
                    case 371:
                        RelicName = "Rommel's Medal";
                        break;
                    case 372:
                        RelicName = "Holy Robe";
                        break;
                    case 374:
                        RelicName = "Virgin's Blood";
                        break;
                    case 375:
                        RelicName = "Inverted Cross";
                        break;
                    default:
                        RelicName = pMonster->getName();
                }

                // Make the effect
                EffectCombatMessage1* pEffect = new EffectCombatMessage1();
                pEffect->setNextTime(30);
                pEffect->setDelay(30);
                pEffect->setDeadline(60);
                pEffect->setRelicName(RelicName);

                // Attach the effect to the Zone
                (pZone->getObjectRegistry()).registerObject(pEffect);
                pZone->addEffect(pEffect);
            }
        }
        */

        /*		if (pGamePlayer->isSlayer() &&
                        pCreature->isMonster() &&
                        (pCreature->getMonsterType() == 371 ||
                         pCreature->getMonsterType() == 372 ||
                         pCreature->getMonsterType() == 371))
                    return;
                if (pGamePlayer->isVampire() &&
                        pCreature->isMonster() &&
                        (pCreature->getMonsterType() == 373 ||
                         pCreature->getMonsterType() == 374))
                    return;
        */
        if (!isAbleToUseObjectSkill(pCreature, SKILL_ATTACK_MELEE))
            return;

        if (pCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);

            if (pSlayer->isFlag(Effect::EFFECT_CLASS_SNIPING_MODE)) {
                g_Sniping.checkRevealRatio(pSlayer, 20, 10);
            }

            Item* pItem = pSlayer->getWearItem(Slayer::WEAR_RIGHTHAND);
            if (pItem != NULL) {
                if (isArmsWeapon(pItem)) {
                    SkillHandler* pSkillHandler = g_pSkillHandlerManager->getSkillHandler(SKILL_ATTACK_ARMS);
                    Assert(pSkillHandler != NULL);

#ifdef __PROFILE_SKILLS__
                    beginProfileEx(SkillTypes2String[SKILL_ATTACK_ARMS]);
                    pSkillHandler->execute(pSlayer, pPacket->getObjectID());
                    endProfileEx(SkillTypes2String[SKILL_ATTACK_ARMS]);
#else
                    pSkillHandler->execute(pSlayer, pPacket->getObjectID());
#endif
                } else {
                    SkillHandler* pSkillHandler = g_pSkillHandlerManager->getSkillHandler(SKILL_ATTACK_MELEE);
                    Assert(pSkillHandler != NULL);

#ifdef __PROFILE_SKILLS__
                    beginProfileEx(SkillTypes2String[SKILL_ATTACK_MELEE]);
                    pSkillHandler->execute(pSlayer, pPacket->getObjectID());
                    endProfileEx(SkillTypes2String[SKILL_ATTACK_MELEE]);
#else
                    pSkillHandler->execute(pSlayer, pPacket->getObjectID());
#endif
                }
            } else {
                SkillHandler* pSkillHandler = g_pSkillHandlerManager->getSkillHandler(SKILL_ATTACK_MELEE);
                Assert(pSkillHandler != NULL);

#ifdef __PROFILE_SKILLS__
                beginProfileEx(SkillTypes2String[SKILL_ATTACK_MELEE]);
                pSkillHandler->execute(pSlayer, pPacket->getObjectID());
                endProfileEx(SkillTypes2String[SKILL_ATTACK_MELEE]);
#else
                pSkillHandler->execute(pSlayer, pPacket->getObjectID());
#endif
            }
        } else if (pCreature->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);

            if (pVampire->isFlag(Effect::EFFECT_CLASS_INVISIBILITY)) {
                addVisibleCreature(pZone, pVampire, true);
            }

            SkillHandler* pSkillHandler = g_pSkillHandlerManager->getSkillHandler(SKILL_ATTACK_MELEE);
            Assert(pSkillHandler != NULL);

#ifdef __PROFILE_SKILLS__
            beginProfileEx(SkillTypes2String[SKILL_ATTACK_MELEE]);
            pSkillHandler->execute(pVampire, pPacket->getObjectID());
            endProfileEx(SkillTypes2String[SKILL_ATTACK_MELEE]);
#else
            pSkillHandler->execute(pVampire, pPacket->getObjectID());
#endif
        } else if (pCreature->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);

            SkillHandler* pSkillHandler = g_pSkillHandlerManager->getSkillHandler(SKILL_ATTACK_MELEE);
            Assert(pSkillHandler != NULL);

#ifdef __PROFILE_SKILLS__
            beginProfileEx(SkillTypes2String[SKILL_ATTACK_MELEE]);
            pSkillHandler->execute(pOusters, pPacket->getObjectID());
            endProfileEx(SkillTypes2String[SKILL_ATTACK_MELEE]);
#else
            pSkillHandler->execute(pOusters, pPacket->getObjectID());
#endif
        }
    } catch (Throwable& t) {
        cout << "CGAttackHandle error: " << t.toString();
    } catch (...) {
    }

#endif

    __END_DEBUG
    __END_DEBUG_EX __END_CATCH
}
