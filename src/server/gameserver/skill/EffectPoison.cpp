//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectPoison.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectPoison.h"

#include "DB.h"
#include "GCModifyInformation.h"
#include "GCRemoveEffect.h"
#include "GCStatusCurrentHP.h"
#include "Monster.h"
#include "Ousters.h"
#include "Player.h"
#include "SkillUtil.h"
#include "Slayer.h"
#include "Vampire.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectPoison::EffectPoison(Creature* pCreature)

{
    __BEGIN_TRY

    m_UserObjectID = 0;
    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectPoison::affect()

{
    __BEGIN_TRY


    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    affect(pCreature);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectPoison::affect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    // Get the skill user.
    // It may be NULL: the creature may already have left the zone.
    // by bezz. 2003.1.4
    Creature* pCastCreature = pZone->getCreature(m_UserObjectID);

    // EffectPoison currently reaches a creature only when the player walks
    // over the EffectGreenPoison that GreenPoison leaves on a tile.
    // EffectGreenPoison already sets the damage with resistance applied
    // before it attaches EffectPoison, so the damage must not be computed
    // a second time here.
    Damage_t PoisonDamage = m_Point;

    if (!(pZone->getZoneLevel() & COMPLETE_SAFE_ZONE)
        // Invincibility check.
        && canAttack(pCastCreature, pCreature)) {
        if (pCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);

            GCModifyInformation gcMI;
            ::setDamage(pSlayer, PoisonDamage, pCastCreature, SKILL_GREEN_POISON, &gcMI);
            pSlayer->getPlayer()->sendPacket(&gcMI);
        } else if (pCreature->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);

            GCModifyInformation gcMI;
            ::setDamage(pVampire, PoisonDamage, pCastCreature, SKILL_GREEN_POISON, &gcMI);
            pVampire->getPlayer()->sendPacket(&gcMI);
        } else if (pCreature->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);

            GCModifyInformation gcMI;
            ::setDamage(pOusters, PoisonDamage, pCastCreature, SKILL_GREEN_POISON, &gcMI);
            pOusters->getPlayer()->sendPacket(&gcMI);
        } else if (pCreature->isMonster()) {
            Monster* pMonster = dynamic_cast<Monster*>(pCreature);

            ::setDamage(pMonster, PoisonDamage, pCastCreature, SKILL_GREEN_POISON);
        }

        // Handles the kill count when the caster kills pCreature.
        // by sigi. 2002.9.9
    }

    setNextTime(m_Tick);


    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectPoison::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectPoison::unaffect(Creature* pCreature)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    pCreature->removeFlag(Effect::EFFECT_CLASS_POISON);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    // Tells clients that the effect is gone.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_POISON);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);

    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectPoison::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectPoison::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

    {__BEGIN_TRY __END_CATCH}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectPoison::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectPoison(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
