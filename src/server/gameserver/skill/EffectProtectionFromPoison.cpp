//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectProtectionFromPoison.cpp
// Written by  : excel96
// Description : Effect created by ProtectionFromPoison that raises poison resistance.
//////////////////////////////////////////////////////////////////////////////

#include "EffectProtectionFromPoison.h"

#include "Creature.h"
#include "DB.h"
#include "GCRemoveEffect.h"
#include "Slayer.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectProtectionFromPoison::EffectProtectionFromPoison(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectProtectionFromPoison::affect(Creature* pCreature)

{
    __BEGIN_TRY

    Resist_t resist = pCreature->getResist(MAGIC_DOMAIN_POISON);
    resist = min(MAX_RESIST, (int)(resist + m_Resist));
    pCreature->setResist(MAGIC_DOMAIN_POISON, resist);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectProtectionFromPoison::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectProtectionFromPoison::unaffect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectProtectionFromPoison::unaffect()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    pCreature->removeFlag(Effect::EFFECT_CLASS_PROTECTION_FROM_POISON);

    Resist_t resist = pCreature->getResist(MAGIC_DOMAIN_POISON);
    resist = max(0, (int)(resist - m_Resist));
    pCreature->setResist(MAGIC_DOMAIN_POISON, resist);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    // Tells clients that the effect is gone.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_PROTECTION_FROM_POISON);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);

    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectProtectionFromPoison::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

    {__BEGIN_TRY __END_CATCH}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectProtectionFromPoison::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectProtectionFromPoison(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectProtectionFromPoisonLoader::load(Creature* pCreature)

{
    __BEGIN_TRY __END_CATCH
}
