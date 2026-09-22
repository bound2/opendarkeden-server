#include "EffectBloodyStorm.h"

#include "EffectStormBloody.h"
#include "GCAddEffect.h"
#include "GCModifyInformation.h"
#include "GCStatusCurrentHP.h"
#include "GamePlayer.h"
#include "Monster.h"
#include "SkillUtil.h"
#include "Slayer.h"
#include "Vampire.h"
#include "Zone.h"

EffectBloodyStorm::EffectBloodyStorm(Zone* pZone, ZoneCoord_t zoneX, ZoneCoord_t zoneY)

{
    __BEGIN_TRY


    m_UserObjectID = 0;
    m_pZone = pZone;
    m_X = zoneX;
    m_Y = zoneY;
    m_bVampire = false;

    __END_CATCH
}


bool EffectBloodyStorm::affectCreature(Creature* pTargetCreature, bool bAffectByMove)

{
    __BEGIN_TRY


    Assert(pTargetCreature != NULL);

    // Not applied if the target already has the acid storm effect.
    if (pTargetCreature->isFlag(Effect::EFFECT_CLASS_STORM_BLOODY)) {
        return false;
    }

    Zone* pZone = pTargetCreature->getZone();

    Creature* pAttacker = pZone->getCreature(m_UserObjectID);
    // Computes the magic damage the storm deals to the target.
    int StormDamage = computeMagicDamage(pTargetCreature, m_Damage, SKILL_BLOODY_STORM, m_bVampire, pAttacker);

    if (StormDamage > 0) {
        // Create the effect, attach it to the target creature and set the flag.
        EffectStormBloody* pEffectStormBloody = new EffectStormBloody(pTargetCreature);
        pEffectStormBloody->setLevel(m_Level);
        pEffectStormBloody->setPoint(StormDamage / 3);
        pEffectStormBloody->setDeadline(16); // This part needs to be changed.
        pEffectStormBloody->setTick(5);      // This part needs to be changed too.
        pEffectStormBloody->setUserObjectID(m_UserObjectID);
        pEffectStormBloody->affect(pTargetCreature);
        pTargetCreature->addEffect(pEffectStormBloody);
        pTargetCreature->setFlag(Effect::EFFECT_CLASS_STORM_BLOODY);

        // Tells the surroundings that the effect was attached.
        GCAddEffect gcAddEffect;
        gcAddEffect.setObjectID(pTargetCreature->getObjectID());
        gcAddEffect.setEffectID(Effect::EFFECT_CLASS_STORM_BLOODY);
        gcAddEffect.setDuration(m_Duration);
        pZone->broadcastPacket(pTargetCreature->getX(), pTargetCreature->getY(), &gcAddEffect);
    }


    return true;

    __END_CATCH
}


void EffectBloodyStorm::affect()

{
    __BEGIN_TRY

    __END_CATCH
}

void EffectBloodyStorm::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectBloodyStorm::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectBloodyStorm::unaffect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectBloodyStorm::unaffect()

{
    __BEGIN_TRY

    Tile& tile = m_pZone->getTile(m_X, m_Y);
    tile.deleteEffect(m_ObjectID);

    __END_CATCH
}

void EffectBloodyStorm::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObjbect)

    {__BEGIN_TRY __END_CATCH}

string EffectBloodyStorm::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "EffectBloodyStorm(" << "ObjectID:" << getObjectID() << ")";

    return msg.toString();

    __END_CATCH
}
