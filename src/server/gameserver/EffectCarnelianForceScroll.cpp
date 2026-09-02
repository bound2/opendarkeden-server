//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectCarnelianForceScroll.cpp
// Written by  : bezz
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectCarnelianForceScroll.h"

#include "GCRemoveEffect.h"
#include "PlayerCreature.h"
#include "Timeval.h"
#include "Zone.h"
#include "repository/EffectSaveRepository.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectCarnelianForceScroll::EffectCarnelianForceScroll(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectCarnelianForceScroll::affect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    affect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectCarnelianForceScroll::affect(Creature* pCreature)

{
    __BEGIN_TRY

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
    Assert(pPC != NULL);

    Zone* pZone = pPC->getZone();
    Assert(pZone != NULL);

    ObjectRegistry& objectregister = pZone->getObjectRegistry();
    objectregister.registerObject(this);

    // 모저 9옵션 적용
    pPC->addEffectOption(getObjectID(), 182);
    pPC->initAllStatAndSend();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectCarnelianForceScroll::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectCarnelianForceScroll::unaffect(Creature* pCreature)

{
    __BEGIN_TRY

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
    Assert(pPC != NULL);

    Zone* pZone = pPC->getZone();
    Assert(pZone != NULL);

    pPC->removeFlag(getEffectClass());
    pPC->removeEffectOption(getObjectID());
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
void EffectCarnelianForceScroll::create(const string& ownerID)

{
    __BEGIN_TRY

    Timeval currentTime;
    getCurrentTime(currentTime);

    Timeval remainTime = timediff(m_Deadline, currentTime);
    Turn_t remainTurn = remainTime.tv_sec * 10 + remainTime.tv_usec / 100000;

    defaultEffectSaveRepository().insertRemain(EFFECT_TABLE_CARNELIAN_FORCE_SCROLL, ownerID, remainTurn);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectCarnelianForceScroll::destroy(const string& ownerID)

{
    __BEGIN_TRY

    defaultEffectSaveRepository().deleteRemain(EFFECT_TABLE_CARNELIAN_FORCE_SCROLL, ownerID);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectCarnelianForceScroll::save(const string& ownerID)

{
    __BEGIN_TRY

    Timeval currentTime;
    getCurrentTime(currentTime);

    Timeval remainTime = timediff(m_Deadline, currentTime);
    Turn_t remainTurn = remainTime.tv_sec * 10 + remainTime.tv_usec / 100000;

    defaultEffectSaveRepository().updateRemain(EFFECT_TABLE_CARNELIAN_FORCE_SCROLL, ownerID, remainTurn);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectCarnelianForceScroll::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectCarnelianForceScroll(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectCarnelianForceScrollLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    DWORD storedRemainTurn = 0;

    if (defaultEffectSaveRepository().loadRemain(EFFECT_TABLE_CARNELIAN_FORCE_SCROLL, pCreature->getName(),
                                                 storedRemainTurn)) {
        Turn_t remainTurn = storedRemainTurn;

        Timeval currentTime;
        getCurrentTime(currentTime);

        EffectCarnelianForceScroll* pEffect = new EffectCarnelianForceScroll(pCreature);

        pEffect->setDeadline(remainTurn);
        pCreature->addEffect(pEffect);
        pCreature->setFlag(pEffect->getEffectClass());

        PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
        Assert(pPC != NULL);

        Zone* pZone = pPC->getZone();
        Assert(pZone != NULL);

        ObjectRegistry& objectregister = pZone->getObjectRegistry();
        objectregister.registerObject(pEffect);

        // 모저 9옵션 적용
        pPC->addEffectOption(pEffect->getObjectID(), 182);
    }

    __END_CATCH
}

EffectCarnelianForceScrollLoader* g_pEffectCarnelianForceScrollLoader = NULL;
