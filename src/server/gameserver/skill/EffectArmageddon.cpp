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

    // 이펙트를 건 크리쳐를 가져온다.
    // !! 이미 존을 떠났을 수도 있으므로 NULL 이 될 수 있다.
    // by bezz. 2003.1.4


    // 매초 데미지 주는거 잠시 막아놓음. by Sequoia

    setNextTime(m_Delay);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectArmageddon::unaffect(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);


    // 플래그를 끈다.
    pCreature->removeFlag(Effect::EFFECT_CLASS_ARMAGEDDON);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    // 이펙트를 삭제하라고 알려준다.
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
    // 남은 HP가 0일 경우 다음 EffectManager::heartbeat() 에서 이펙트를 날려준다.
    if (RemainHP == 0)
        setDeadline(0);
}
