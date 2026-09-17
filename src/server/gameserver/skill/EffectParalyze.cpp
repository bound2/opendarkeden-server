//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectParalyze.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectParalyze.h"

#include "Creature.h"
#include "DB.h"
#include "GCModifyInformation.h"
#include "GCRemoveEffect.h"
#include "GCStatusCurrentHP.h"
#include "Monster.h"
#include "Player.h"
#include "SkillHandler.h"
#include "Slayer.h"
#include "Vampire.h"

EffectParalyze::EffectParalyze(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);

    __END_CATCH
}

void EffectParalyze::affect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    affect(pCreature);

    __END_CATCH
}

void EffectParalyze::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectParalyze::unaffect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);
    pCreature->removeFlag(Effect::EFFECT_CLASS_PARALYZE);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_PARALYZE);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);


    __END_CATCH
}

void EffectParalyze::unaffect()

{
    __BEGIN_TRY


    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);


    __END_CATCH
}

string EffectParalyze::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectParalyze(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
