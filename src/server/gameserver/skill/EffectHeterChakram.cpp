//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectHeterChakram.cpp
// Written by  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectHeterChakram.h"

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
EffectHeterChakram::EffectHeterChakram(Creature* pCreature)

{
    __BEGIN_TRY

    m_pTarget = pCreature;
    m_UserOID = 0;
    m_Damage = 0;
    m_Duration = 0;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectHeterChakram::affect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    setNextTime(10);
    // 			// Increases the number of other attacks
    // 			//m_TrageSaveHP = CurrentHP;
    // Deals the damage.
    affect(pCreature);

    //	}


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectHeterChakram::affect(Creature* pCreature)

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

            ::setDamage(pPC, damage, pOusters, SKILL_HETER_CHAKRAM, &gcMI, &gcAttackerMI, true, false);
            pPC->getPlayer()->sendPacket(&gcMI);
        } else if (pCreature->isMonster()) {
            ::setDamage(pCreature, damage, pOusters, SKILL_HETER_CHAKRAM, NULL, &gcAttackerMI, true, false);
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
void EffectHeterChakram::unaffect(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);
    pCreature->removeFlag(Effect::EFFECT_CLASS_HETER_CHAKRAM);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_HETER_CHAKRAM);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectHeterChakram::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);

    // Damage the target.
    affect(pCreature);

    //  Reads the objects within the 5x5 area and deals damage to them.


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
                //  Deals the damage.
                affect(pTargetCreature);
            }
        }

    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectHeterChakram::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "EffectHeterChakram(" << "ObjectID:" << getObjectID() << ")";

    return msg.toString();

    __END_CATCH
}
