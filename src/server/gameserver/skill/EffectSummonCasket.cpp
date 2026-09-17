//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectSummonCasket.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectSummonCasket.h"

#include "Creature.h"
#include "GCRemoveEffect.h"
#include "Monster.h"
#include "Player.h"
#include "Vampire.h"
#include "ZoneUtil.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectSummonCasket::EffectSummonCasket(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);
    m_Type = 0;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSummonCasket::affect(Creature* pCreature)

{
    __BEGIN_TRY

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSummonCasket::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSummonCasket::unaffect()

{
    __BEGIN_TRY


    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    Assert(pCreature != NULL);

    if (pCreature->isSlayer()) {
        throw Error("EffectTransfromToWolf::unaffect() : Slayer cannot transfrom to wolf!");
    }

    unaffect(pCreature);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSummonCasket::unaffect(Creature* pCreature)

{
    __BEGIN_TRY


    Assert(pCreature != NULL);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    // 원래 모습으로 되돌리고, 플래그를 제거한다.
    pCreature->removeFlag(Effect::EFFECT_CLASS_CASKET);

    // 능력치를 원래대로 되돌린다.
    if (pCreature->isVampire()) {
        Vampire* pTargetVampire = dynamic_cast<Vampire*>(pCreature);
        VAMPIRE_RECORD prev;

        pTargetVampire->getVampireRecord(prev);
        pTargetVampire->initAllStat();
        pTargetVampire->sendRealWearingInfo();
        pTargetVampire->sendModifyInfo(prev);

        //(int)pTargetVampire->getDefense() << endl; 		cout << "Prev Protection : " << (int)prev.Protection << "
    }

    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_CASKET);
    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSummonCasket::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

    {__BEGIN_TRY __END_CATCH}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectSummonCasket::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectSummonCasket(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
