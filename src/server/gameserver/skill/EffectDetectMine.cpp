//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectDetectMine.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectDetectMine.h"

#include "Creature.h"
#include "DB.h"
#include "Slayer.h"

EffectDetectMine::EffectDetectMine(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);

    __END_CATCH
}

void EffectDetectMine::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectDetectMine::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectDetectMine::unaffect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);

    pCreature->removeFlag(Effect::EFFECT_CLASS_REVEALER);
    // Removes the mines that were visible through the effect.
    Zone* pZone = pCreature->getZone();
    pZone->updateMineScan(pCreature);


    __END_CATCH
}

void EffectDetectMine::unaffect()

{
    __BEGIN_TRY


    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);


    __END_CATCH
}

void EffectDetectMine::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

    {__BEGIN_TRY __END_CATCH}

string EffectDetectMine::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectDetectMine(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
