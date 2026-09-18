//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectIceOfSoulStone.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectIceOfSoulStone.h"

#include "GCRemoveEffect.h"
#include "Ousters.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectIceOfSoulStone::EffectIceOfSoulStone(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectIceOfSoulStone::unaffect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);

    // Turns off the flag.
    pCreature->removeFlag(Effect::EFFECT_CLASS_ICE_OF_SOUL_STONE);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);


    // Tells clients to remove the effect.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_ICE_OF_SOUL_STONE);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectIceOfSoulStone::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectIceOfSoulStone::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectIceOfSoulStone(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
