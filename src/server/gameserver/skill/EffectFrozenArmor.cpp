//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectFrozenArmor.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectFrozenArmor.h"

#include "GCRemoveEffect.h"
#include "Ousters.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectFrozenArmor::EffectFrozenArmor(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);
    Assert(pCreature->isOusters());

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectFrozenArmor::unaffect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);
    Assert(pCreature->isOusters());

    // Turns off the flag.
    pCreature->removeFlag(Effect::EFFECT_CLASS_FROZEN_ARMOR);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    Ousters* pTargetOusters = dynamic_cast<Ousters*>(pCreature);
    Assert(pTargetOusters != NULL);

    // Tells clients to remove the effect.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_FROZEN_ARMOR);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectFrozenArmor::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectFrozenArmor::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectFrozenArmor(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
