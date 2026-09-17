//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectDarkBluePoison.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectDarkBluePoison.h"

#include "GCModifyInformation.h"
#include "GCRemoveEffect.h"
#include "Monster.h"
#include "Player.h"
#include "SkillHandler.h"
#include "Slayer.h"
#include "Vampire.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectDarkBluePoison::EffectDarkBluePoison(Creature* pCreature)

{
    __BEGIN_TRY

    m_Level = 0;
    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectDarkBluePoison::affect()

{
    __BEGIN_TRY


    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    affect(pCreature);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectDarkBluePoison::affect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);

    if (canAttack(NULL, pCreature)) {
        // 슬레이어일 경우에만 독 데미지가 존재한다.
        if (pCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
            Assert(pSlayer != NULL);


            MP_t CurrentMP = pSlayer->getMP(ATTR_CURRENT);
            MP_t RemainMP = max(0, (int)CurrentMP - m_Damage);

            pSlayer->setMP(RemainMP, ATTR_CURRENT);

            GCModifyInformation gcMI;
            gcMI.addShortData(MODIFY_CURRENT_MP, RemainMP);

            Player* pPlayer = pCreature->getPlayer();
            Assert(pPlayer != NULL);
            pPlayer->sendPacket(&gcMI);
        } else if (pCreature->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);
            Assert(pOusters != NULL);


            // 아우스터즈는 1/2만 깎인다.
            MP_t CurrentMP = pOusters->getMP(ATTR_CURRENT);
            MP_t RemainMP = max(0, (int)CurrentMP - (m_Damage / 2));

            pOusters->setMP(RemainMP, ATTR_CURRENT);

            GCModifyInformation gcMI;
            gcMI.addShortData(MODIFY_CURRENT_MP, RemainMP);

            Player* pPlayer = pCreature->getPlayer();
            Assert(pPlayer != NULL);
            pPlayer->sendPacket(&gcMI);
        }
    }

    setNextTime(m_Tick);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectDarkBluePoison::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectDarkBluePoison::unaffect(Creature* pCreature)

{
    __BEGIN_TRY
    __BEGIN_DEBUG


    Assert(pCreature != NULL);

    // 크리쳐에게서 플래그를 제거한다.
    pCreature->removeFlag(Effect::EFFECT_CLASS_DARKBLUE_POISON);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    if (pCreature->isPC()) {
        GCRemoveEffect gcRemoveEffect;
        gcRemoveEffect.setObjectID(pCreature->getObjectID());
        gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_DARKBLUE_POISON);

        Player* pPlayer = pCreature->getPlayer();
        Assert(pPlayer != NULL);
        pPlayer->sendPacket(&gcRemoveEffect);
    }


    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectDarkBluePoison::unaffect()

{
    __BEGIN_TRY


    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectDarkBluePoison::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

    {__BEGIN_TRY __END_CATCH}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectDarkBluePoison::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectDarkBluePoison(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
