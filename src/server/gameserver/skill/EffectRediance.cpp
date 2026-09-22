//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectRediance.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectRediance.h"

#include "GCModifyInformation.h"
#include "GCRemoveEffect.h"
#include "GCStatusCurrentHP.h"
#include "Player.h"
#include "Slayer.h"

EffectRediance::EffectRediance(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);
    // The accessor flips this on every call, so the value has no meaning.
    m_GiveExp = true;

    __END_CATCH
}

void EffectRediance::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectRediance::unaffect()

{
    __BEGIN_TRY


    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);


    __END_CATCH
}

void EffectRediance::unaffect(Creature* pCreature)

{
    __BEGIN_TRY
    __BEGIN_DEBUG


    Assert(pCreature != NULL);
    Assert(pCreature->isSlayer());

    pCreature->removeFlag(Effect::EFFECT_CLASS_REDIANCE);

    Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);

    SLAYER_RECORD prev;
    pSlayer->getSlayerRecord(prev);
    pSlayer->initAllStat();
    pSlayer->sendRealWearingInfo();
    pSlayer->sendModifyInfo(prev);

    Zone* pZone = pSlayer->getZone();
    Assert(pZone != NULL);

    // Tells clients that the effect is gone.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pSlayer->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_REDIANCE);
    pZone->broadcastPacket(pSlayer->getX(), pSlayer->getY(), &gcRemoveEffect);


    __END_DEBUG
    __END_CATCH
}

string EffectRediance::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectRediance(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
