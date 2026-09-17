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

    // 플래그를 끈다.
    pCreature->removeFlag(Effect::EFFECT_CLASS_GNOMES_WHISPER);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    pZone->updateInvisibleScan(pCreature);


    // 이펙트를 삭제하라고 알려준다.
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
