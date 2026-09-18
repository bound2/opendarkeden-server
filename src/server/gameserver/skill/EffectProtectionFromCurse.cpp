//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectProtectionFromCurse.cpp
// Written by  : excel96
// Description : Effect created by ProtectionFromCurse that raises curse resistance.
//////////////////////////////////////////////////////////////////////////////

#include "EffectProtectionFromCurse.h"

#include "Creature.h"
#include "DB.h"
#include "GCRemoveEffect.h"
#include "Slayer.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectProtectionFromCurse::EffectProtectionFromCurse(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectProtectionFromCurse::affect(Creature* pCreature)

{
    __BEGIN_TRY

    Resist_t resist = pCreature->getResist(MAGIC_DOMAIN_CURSE);
    resist = min(MAX_RESIST, (int)(resist + m_Resist));
    pCreature->setResist(MAGIC_DOMAIN_CURSE, resist);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectProtectionFromCurse::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectProtectionFromCurse::unaffect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectProtectionFromCurse::unaffect()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    pCreature->removeFlag(Effect::EFFECT_CLASS_PROTECTION_FROM_CURSE);

    Resist_t resist = pCreature->getResist(MAGIC_DOMAIN_CURSE);
    resist = max(0, (int)(resist - m_Resist));
    pCreature->setResist(MAGIC_DOMAIN_CURSE, resist);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    // Tells clients that the effect is gone.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_PROTECTION_FROM_CURSE);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);

    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectProtectionFromCurse::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

    {__BEGIN_TRY __END_CATCH}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectProtectionFromCurse::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "EffectProtectionFromCurse(" << "ObjectID:" << getObjectID() << ")";

    return msg.toString();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectProtectionFromCurseLoader::load(Creature* pCreature)

{
    __BEGIN_TRY __END_CATCH
}
