//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectHandsOfFire.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectHandsOfFire.h"

#include "GCRemoveEffect.h"
#include "Ousters.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectHandsOfFire::EffectHandsOfFire(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);
    Assert(pCreature->isOusters());

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectHandsOfFire::unaffect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);
    Assert(pCreature->isOusters());

    // Turns off the flag.
    pCreature->removeFlag(Effect::EFFECT_CLASS_HANDS_OF_FIRE);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    Ousters* pTargetOusters = dynamic_cast<Ousters*>(pCreature);
    Assert(pTargetOusters != NULL);


    pTargetOusters->initAllStat();

    // Tells clients to remove the effect.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_HANDS_OF_FIRE);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectHandsOfFire::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectHandsOfFire::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectHandsOfFire(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
