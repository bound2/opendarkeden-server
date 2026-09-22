//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectReactiveArmor.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectReactiveArmor.h"

#include "GCRemoveEffect.h"
#include "Ousters.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectReactiveArmor::EffectReactiveArmor(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);
    Assert(pCreature->isOusters());

    setTarget(pCreature);
    m_DamageReduce = 0;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectReactiveArmor::unaffect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);
    Assert(pCreature->isOusters());

    // Turns off the flag.
    pCreature->removeFlag(Effect::EFFECT_CLASS_REACTIVE_ARMOR);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    Ousters* pTargetOusters = dynamic_cast<Ousters*>(pCreature);
    Assert(pTargetOusters != NULL);

    pTargetOusters->initAllStatAndSend();

    // Tells clients to remove the effect.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_REACTIVE_ARMOR);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectReactiveArmor::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectReactiveArmor::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectReactiveArmor(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
