//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectPeace.cpp
// Written by  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectPeace.h"

#include "DB.h"
#include "DarkLightInfo.h"
#include "GCChangeDarkLight.h"
#include "GCModifyInformation.h"
#include "GCRemoveEffect.h"
#include "Monster.h"
#include "MonsterInfo.h"
#include "Peace.h"
#include "Player.h"
#include "Vampire.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectPeace::EffectPeace(Creature* pCreature, ObjectID_t PeaceCreatureID)

{
    __BEGIN_TRY

    setTarget(pCreature);
    m_PeaceCreatureID = PeaceCreatureID;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectPeace::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectPeace::affect(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);
    Assert(!pCreature->isSlayer()); // Slayers are not affected.
    Assert(!pCreature->isNPC());    // NPCs are not affected either.

    if (pCreature->isMonster()) {
        Monster* pMonster = dynamic_cast<Monster*>(pCreature);

        pMonster->deleteEnemy(pCreature->getObjectID());
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectPeace::unaffect(Creature* pCreature)

{
    __BEGIN_TRY
    __BEGIN_DEBUG


    pCreature->removeFlag(Effect::EFFECT_CLASS_PEACE);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    // Tells clients that the effect is gone.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_PEACE);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);


    __END_DEBUG
    __END_CATCH
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectPeace::unaffect()

{
    __BEGIN_TRY
    __BEGIN_DEBUG


    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);


    __END_DEBUG
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectPeace::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

    {__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectPeace::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "EffectPeace(" << "ObjectID:" << getObjectID() << ")";

    return msg.toString();

    __END_CATCH
}
