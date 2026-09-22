//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectIceFieldToCreature.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectIceFieldToCreature.h"

#include "GCRemoveEffect.h"
#include "Ousters.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectIceFieldToCreature::EffectIceFieldToCreature(Creature* pCreature, bool bFrozenArmor)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    setTarget(pCreature);
    m_bFrozenArmor = bFrozenArmor;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectIceFieldToCreature::unaffect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);

    // Turns off the flag.
    pCreature->removeFlag(Effect::EFFECT_CLASS_ICE_FIELD_TO_CREATURE);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);


    // Tells clients to remove the effect.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(getSendEffectClass());
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectIceFieldToCreature::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectIceFieldToCreature::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectIceFieldToCreature(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
