//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectGnomesWhisper.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectGnomesWhisper.h"

#include "GCRemoveEffect.h"
#include "Ousters.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectGnomesWhisper::EffectGnomesWhisper(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectGnomesWhisper::unaffect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);

    // Turns off the flag.
    pCreature->removeFlag(Effect::EFFECT_CLASS_GNOMES_WHISPER);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    pZone->updateInvisibleScan(pCreature);


    // Tells clients to remove the effect.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_GNOMES_WHISPER);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectGnomesWhisper::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectGnomesWhisper::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectGnomesWhisper(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
