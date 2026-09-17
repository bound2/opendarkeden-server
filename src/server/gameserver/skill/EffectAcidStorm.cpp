#include "EffectAcidStorm.h"

#include "EffectStormAcid.h"
#include "GCAddEffect.h"
#include "GCModifyInformation.h"
#include "GCStatusCurrentHP.h"
#include "GamePlayer.h"
#include "Monster.h"
#include "SkillUtil.h"
#include "Slayer.h"
#include "Vampire.h"
#include "Zone.h"

EffectAcidStorm::EffectAcidStorm(Zone* pZone, ZoneCoord_t zoneX, ZoneCoord_t zoneY)

{
    __BEGIN_TRY


    m_pZone = pZone;
    m_X = zoneX;
    m_Y = zoneY;

    m_bVampire = false;

    __END_CATCH
}


bool EffectAcidStorm::affectCreature(Creature* pTargetCreature, bool bAffectByMove)

{
    __BEGIN_TRY


    Assert(pTargetCreature != NULL);

    // 상대에게 이미 poison 이펙트가 걸려져 있는 경우에는 걸리지 않는다.
    if (pTargetCreature->isFlag(Effect::EFFECT_CLASS_STORM_ACID)) {
        return false;
    }

    Zone* pZone = pTargetCreature->getZone();

    Creature* pAttacker = pZone->getCreature(m_UserObjectID);
    // 상대방에게 미칠 독 데미지를 계산한다.
    int StormDamage = computeMagicDamage(pTargetCreature, m_Damage, SKILL_ACID_STORM, m_bVampire, pAttacker);

    if (StormDamage > 0) {
        //  포이즌 이펙트를 생성해서, 타겟 크리쳐에 붙이고, 플래그를 켜준다.
        EffectStormAcid* pEffectStormAcid = new EffectStormAcid(pTargetCreature);
        pEffectStormAcid->setLevel(m_Level);
        pEffectStormAcid->setPoint(StormDamage / 3);
        pEffectStormAcid->setDeadline(16); // 이부분 바꿔야 한다.
        pEffectStormAcid->setTick(5);      // 이부분도 바꿔야 한다.
        pEffectStormAcid->setUserObjectID(m_UserObjectID);
        pEffectStormAcid->affect(pTargetCreature);
        pTargetCreature->addEffect(pEffectStormAcid);
        pTargetCreature->setFlag(Effect::EFFECT_CLASS_STORM_ACID);

        // 이펙트가 붙었다고 주변에 알려준다.
        GCAddEffect gcAddEffect;
        gcAddEffect.setObjectID(pTargetCreature->getObjectID());
        gcAddEffect.setEffectID(Effect::EFFECT_CLASS_STORM_ACID);
        gcAddEffect.setDuration(m_Duration);
        pZone->broadcastPacket(pTargetCreature->getX(), pTargetCreature->getY(), &gcAddEffect);
    }


    return true;

    __END_CATCH
}


void EffectAcidStorm::affect()

{
    __BEGIN_TRY

    __END_CATCH
}

void EffectAcidStorm::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectAcidStorm::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectAcidStorm::unaffect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectAcidStorm::unaffect()

{
    __BEGIN_TRY

    Tile& tile = m_pZone->getTile(m_X, m_Y);
    tile.deleteEffect(m_ObjectID);

    __END_CATCH
}

void EffectAcidStorm::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObjbect)

    {__BEGIN_TRY __END_CATCH}

string EffectAcidStorm::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "EffectAcidStorm(" << "ObjectID:" << getObjectID() << ")";

    return msg.toString();

    __END_CATCH
}
