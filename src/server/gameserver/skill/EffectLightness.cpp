
//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectLightness.cpp
// Written by  : excel96
// Description : Effect created by Lightness that changes the creature sight.
//////////////////////////////////////////////////////////////////////////////

#include "EffectLightness.h"

#include "Creature.h"
#include "GCChangeDarkLight.h"
#include "GCModifyInformation.h"
#include "GCRemoveEffect.h"
#include "Player.h"
#include "Zone.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectLightness::EffectLightness(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectLightness::unaffect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectLightness::unaffect()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    pCreature->removeFlag(Effect::EFFECT_CLASS_LIGHTNESS);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    // Tells clients that the effect is gone.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_LIGHTNESS);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);

    Player* pPlayer = pCreature->getPlayer();
    if (pPlayer != NULL) {
        GCModifyInformation gcMI;

        Sight_t oldSight = pCreature->getSight();
        Sight_t newSight = pCreature->getEffectedSight();

        if (oldSight != newSight) {
            pCreature->setSight(newSight);
            gcMI.addShortData(MODIFY_VISION, newSight);
            pPlayer->sendPacket(&gcMI);
        }

        GCChangeDarkLight gcChangeDarkLight;
        gcChangeDarkLight.setDarkLevel(pZone->getDarkLevel());
        gcChangeDarkLight.setLightLevel(pZone->getLightLevel());

        pPlayer->sendPacket(&gcChangeDarkLight);
    }


    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectLightness::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "EffectLightness(" << "ObjectID:" << getObjectID() << ")";

    return msg.toString();

    __END_CATCH
}
