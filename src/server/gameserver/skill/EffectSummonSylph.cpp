//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectSummonSylph.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectSummonSylph.h"

#include "GCRemoveEffect.h"
#include "Ousters.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectSummonSylph::EffectSummonSylph(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);
    Assert(pCreature->isOusters());

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSummonSylph::unaffect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);
    Assert(pCreature->isOusters());

    // Turns off the flag.
    pCreature->removeFlag(Effect::EFFECT_CLASS_SUMMON_SYLPH);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    Ousters* pTargetOusters = dynamic_cast<Ousters*>(pCreature);
    Assert(pTargetOusters != NULL);

    OUSTERS_RECORD prev;

    pTargetOusters->getOustersRecord(prev);
    pTargetOusters->initAllStat();
    pTargetOusters->sendRealWearingInfo();
    pTargetOusters->sendModifyInfo(prev);

    // Tells clients to remove the effect.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(getSendEffectClass());
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSummonSylph::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectSummonSylph::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectSummonSylph(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
