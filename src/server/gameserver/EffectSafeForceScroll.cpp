//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectSafeForceScroll.cpp
// Written by  : bezz
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectSafeForceScroll.h"

#include "GCRemoveEffect.h"
#include "PlayerCreature.h"
#include "Timeval.h"
#include "Zone.h"
#include "repository/EffectSaveRepository.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectSafeForceScroll::EffectSafeForceScroll(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSafeForceScroll::affect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    affect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSafeForceScroll::affect(Creature* pCreature)

{
    __BEGIN_TRY

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
    Assert(pPC != NULL);

    pPC->initAllStatAndSend();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSafeForceScroll::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSafeForceScroll::unaffect(Creature* pCreature)

{
    __BEGIN_TRY

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
    Assert(pPC != NULL);

    Zone* pZone = pPC->getZone();
    Assert(pZone != NULL);

    pPC->removeFlag(getEffectClass());
    pPC->initAllStatAndSend();

    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(getEffectClass());
    pZone->broadcastPacket(pPC->getX(), pPC->getY(), &gcRemoveEffect);

    destroy(pPC->getName());

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSafeForceScroll::create(const string& ownerID)

{
    __BEGIN_TRY

    Timeval currentTime;
    getCurrentTime(currentTime);

    Timeval remainTime = timediff(m_Deadline, currentTime);
    Turn_t remainTurn = remainTime.tv_sec * 10 + remainTime.tv_usec / 100000;

    defaultEffectSaveRepository().insertRemain(EFFECT_TABLE_SAFE_FORCE_SCROLL, ownerID, remainTurn);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSafeForceScroll::destroy(const string& ownerID)

{
    __BEGIN_TRY

    defaultEffectSaveRepository().deleteRemain(EFFECT_TABLE_SAFE_FORCE_SCROLL, ownerID);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSafeForceScroll::save(const string& ownerID)

{
    __BEGIN_TRY

    Timeval currentTime;
    getCurrentTime(currentTime);

    Timeval remainTime = timediff(m_Deadline, currentTime);
    Turn_t remainTurn = remainTime.tv_sec * 10 + remainTime.tv_usec / 100000;

    defaultEffectSaveRepository().updateRemain(EFFECT_TABLE_SAFE_FORCE_SCROLL, ownerID, remainTurn);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectSafeForceScroll::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectSafeForceScroll(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectSafeForceScrollLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    DWORD storedRemainTurn = 0;

    if (defaultEffectSaveRepository().loadRemain(EFFECT_TABLE_SAFE_FORCE_SCROLL, pCreature->getName(),
                                                 storedRemainTurn)) {
        Turn_t remainTurn = storedRemainTurn;

        Timeval currentTime;
        getCurrentTime(currentTime);

        EffectSafeForceScroll* pEffect = new EffectSafeForceScroll(pCreature);

        pEffect->setDeadline(remainTurn);
        pCreature->addEffect(pEffect);
        pCreature->setFlag(pEffect->getEffectClass());
    }

    __END_CATCH
}

EffectSafeForceScrollLoader* g_pEffectSafeForceScrollLoader = NULL;
