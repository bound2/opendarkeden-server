//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectJabbingVein.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectJabbingVein.h"

#include "GCRemoveEffect.h"
#include "Ousters.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectJabbingVein::EffectJabbingVein(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectJabbingVein::unaffect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);

    // Turns off the flag.
    pCreature->removeFlag(Effect::EFFECT_CLASS_JABBING_VEIN);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);


    // Tells clients to remove the effect.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_JABBING_VEIN);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectJabbingVein::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectJabbingVein::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectJabbingVein(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
