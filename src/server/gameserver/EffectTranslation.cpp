
//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectTranslation.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectTranslation.h"

#include "Creature.h"
#include "GCRemoveEffect.h"
#include "Player.h"
#include "Zone.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectTranslation::EffectTranslation(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);
    setBroadcastingEffect();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectTranslation::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectTranslation::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectTranslation::unaffect(Creature* pCreature)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    // cout << "EffectTranslation" << "unaffect BEGIN" << endl;

    Assert(pCreature != NULL);

    // To restore the stats to normal, the flag must be cleared and
    // initAllStat must be called.
    pCreature->removeFlag(Effect::EFFECT_CLASS_TRANSLATION);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_TRANSLATION);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);

    // cout << "EffectTranslation" << "unaffect END" << endl;

    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectTranslation::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectTranslation(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
