//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectHydroConvergence.cpp
// Written by  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectHydroConvergence.h"

#include "DB.h"
#include "GCAddEffect.h"
#include "GCRemoveEffect.h"
#include "GamePlayer.h"
#include "Monster.h"
#include "Ousters.h"
#include "SkillUtil.h"
#include "Zone.h"
#include "ZoneUtil.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectHydroConvergence::EffectHydroConvergence(Creature* pCreature)

{
    __BEGIN_TRY

    m_pTarget = pCreature;
    m_UserOID = 0;
    m_Damage = 0;
    m_Duration = 0;
    m_AttackNum = 0; // ÊÜµ½¹¥»÷´ÎÊý
    m_TrageSaveHP = 0;
    if (!pCreature->isDead() && !pCreature->isOusters()) {
        if (pCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
            m_TrageSaveHP = pSlayer->getHP();
        }
        if (pCreature->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);
            m_TrageSaveHP = pVampire->getHP();
        }
        if (pCreature->isMonster()) {
            Monster* pMonsterAttacker = dynamic_cast<Monster*>(pCreature);
            m_TrageSaveHP = pMonsterAttacker->getHP();
        }
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectHydroConvergence::affect()

{
    __BEGIN_TRY

    setNextTime(10);
    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    HP_t CurrentHP = 0;
    Slayer* pSlayer;
    Vampire* pVampire;
    Monster* pMonsterAttacker;
    if (!pCreature->isDead() && !pCreature->isOusters()) {
        if (pCreature->isSlayer()) {
            pSlayer = dynamic_cast<Slayer*>(pCreature);
            CurrentHP = pSlayer->getHP();
        }
        if (pCreature->isVampire()) {
            pVampire = dynamic_cast<Vampire*>(pCreature);
            CurrentHP = pVampire->getHP();
        }
        if (pCreature->isMonster()) {
            pMonsterAttacker = dynamic_cast<Monster*>(pCreature);
            CurrentHP = pMonsterAttacker->getHP();
        }
        if (CurrentHP < m_TrageSaveHP) {
            // Ôö¼ÓÆäËü¹¥»÷´ÎÊý
            m_AttackNum++;
        }
        // ´´½¨ÉËº¦
        affect(pCreature);
        if (pCreature->isSlayer())
            CurrentHP = pSlayer->getHP();
        if (pCreature->isVampire())
            CurrentHP = pVampire->getHP();
        if (pCreature->isMonster())
            CurrentHP = pMonsterAttacker->getHP();
        // ¼ÇÂ¼µ±Ç°HP
        m_TrageSaveHP = CurrentHP;
        if (m_AttackNum >= 5) {
            setDuration(0);
            setDeadline(0);
        }
    }


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectHydroConvergence::affect(Creature* pCreature)

{
    __BEGIN_TRY

    if (pCreature == NULL)
        return;

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    Ousters* pOusters = dynamic_cast<Ousters*>(pZone->getCreature(m_UserOID));

    GCModifyInformation gcMI, gcAttackerMI;

    if (canAttack(pOusters, pCreature) && !(pZone->getZoneLevel() & COMPLETE_SAFE_ZONE)) {
        Damage_t damage = getDamage();
        if (pCreature->isPC()) {
            PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);

            ::setDamage(pPC, damage, pOusters, SKILL_HYDRO_CONVERGENCE, &gcMI, &gcAttackerMI, true, false);
            pPC->getPlayer()->sendPacket(&gcMI);
        } else if (pCreature->isMonster()) {
            ::setDamage(pCreature, damage, pOusters, SKILL_HYDRO_CONVERGENCE, NULL, &gcAttackerMI, true, false);
        }

        if (pOusters != NULL) {
            computeAlignmentChange(pCreature, damage, pOusters, &gcMI, &gcAttackerMI);
            increaseAlignment(pOusters, pCreature, gcAttackerMI);

            if (pCreature->isDead()) {
                int exp = computeCreatureExp(pCreature, 100, pOusters);
                shareOustersExp(pOusters, exp, gcAttackerMI);
            }

            pOusters->getPlayer()->sendPacket(&gcAttackerMI);
        }
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectHydroConvergence::unaffect(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);
    pCreature->removeFlag(Effect::EFFECT_CLASS_HYDRO_CONVERGENCE);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_HYDRO_CONVERGENCE);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectHydroConvergence::unaffect()

{
    __BEGIN_TRY


    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    // ¶ÁÈ¡5*5·¶Î§ÄÚ¶ÔÏó,²¢´´½¨ÉËº¦
    // ´´½¨Ä¿±êÉËº¦
    affect(pCreature);

    int cx = pCreature->getX();
    int cy = pCreature->getY();
    //
    Zone* pZone = pCreature->getZone();
    //
    for (int i = -2; i <= 2; ++i)
        for (int j = -2; j <= 2; ++j) {
            int tx = cx + i;
            int ty = cy + j;
            if (tx < 0 || ty < 0)
                continue;
            if (!isValidZoneCoord(pZone, tx, ty))
                continue;

            forward_list<Object*>& olist = pZone->getTile(tx, ty).getObjectList();
            forward_list<Object*>::iterator itr = olist.begin();
            for (; itr != olist.end(); ++itr) {
                Object* pObject = *itr;
                if (pObject == NULL || pObject->getObjectClass() != Object::OBJECT_CLASS_CREATURE)
                    continue;

                Creature* pTargetCreature = dynamic_cast<Creature*>(pObject);
                if (pTargetCreature == NULL || pTargetCreature->isOusters() || pTargetCreature == pCreature)
                    continue;
                if (pTargetCreature->isFlag(getEffectClass()))
                    continue;
                // ´´½¨ÉËº¦
                affect(pTargetCreature);
            }
        }
    unaffect(pCreature);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectHydroConvergence::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "EffectHydroConvergence(" << "ObjectID:" << getObjectID() << ")";

    return msg.toString();

    __END_CATCH
}
