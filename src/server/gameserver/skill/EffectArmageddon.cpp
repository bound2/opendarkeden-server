//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectArmageddon.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectArmageddon.h"

#include "GCRemoveEffect.h"
#include "GCStatusCurrentHP.h"
#include "Monster.h"
#include "Player.h"
#include "SkillUtil.h"
#include "Slayer.h"
#include "Vampire.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectArmageddon::EffectArmageddon(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectArmageddon::affect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);

    Assert(pCreature != NULL);

    affect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectArmageddon::affect(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    // The tick only rearms the effect; it deals no damage of its own.
    setNextTime(m_Delay);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectArmageddon::unaffect(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);


    // Turns off the flag.
    pCreature->removeFlag(Effect::EFFECT_CLASS_ARMAGEDDON);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    // Tells clients to remove the effect.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_ARMAGEDDON);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectArmageddon::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectArmageddon::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectArmageddon(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectArmageddon::decreaseHP(Damage_t damage) {
    HP_t RemainHP = max(0, m_HP - damage);

    setHP(RemainHP);
    // When the remaining HP is 0, the next EffectManager::heartbeat() drops the effect.
    if (RemainHP == 0)
        setDeadline(0);
}
