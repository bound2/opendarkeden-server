//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectWaterBarrier.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectWaterBarrier.h"

#include "GCRemoveEffect.h"
#include "Ousters.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectWaterBarrier::EffectWaterBarrier(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);
    Assert(pCreature->isOusters());

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectWaterBarrier::unaffect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);
    Assert(pCreature->isOusters());

    // Turns off the flag.
    pCreature->removeFlag(Effect::EFFECT_CLASS_WATER_BARRIER);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    Ousters* pTargetOusters = dynamic_cast<Ousters*>(pCreature);
    Assert(pTargetOusters != NULL);

    // Tells clients to remove the effect.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_WATER_BARRIER);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectWaterBarrier::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectWaterBarrier::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectWaterBarrier(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
