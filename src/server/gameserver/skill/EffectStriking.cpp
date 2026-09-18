//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectStriking.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectStriking.h"

#include "GCRemoveEffect.h"
#include "Player.h"
#include "Slayer.h"

EffectStriking::EffectStriking(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);
    Assert(pCreature->isSlayer());

    setTarget(pCreature);

    __END_CATCH
}

void EffectStriking::affect(Creature* pCreature)

{
    __BEGIN_TRY

    __END_CATCH
}

void EffectStriking::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectStriking::unaffect()

{
    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);
}

void EffectStriking::unaffect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);
    Assert(pCreature->isSlayer());

    // Clear the flag first.
    pCreature->removeFlag(Effect::EFFECT_CLASS_STRIKING);

    // Recalculate the stats and send them.
    Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
    SLAYER_RECORD prev;
    pSlayer->getSlayerRecord(prev);
    pSlayer->initAllStat();
    pSlayer->sendRealWearingInfo();
    pSlayer->sendModifyInfo(prev);

    // Tell the surroundings that the effect is gone.
    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_STRIKING);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);


    __END_CATCH
}

void EffectStriking::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

    {__BEGIN_TRY __END_CATCH}

string EffectStriking::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectStriking(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
