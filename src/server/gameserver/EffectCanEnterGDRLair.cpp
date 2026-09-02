//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectCanEnterGDRLair.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectCanEnterGDRLair.h"

#include "Creature.h"
#include "GCModifyInformation.h"
#include "GCRemoveEffect.h"
#include "Monster.h"
#include "Ousters.h"
#include "Player.h"
#include "Slayer.h"
#include "Vampire.h"
#include "repository/EffectSaveRepository.h"

EffectCanEnterGDRLair::EffectCanEnterGDRLair(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);

    __END_CATCH
}

EffectCanEnterGDRLair::~EffectCanEnterGDRLair()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

void EffectCanEnterGDRLair::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectCanEnterGDRLair::unaffect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    __END_CATCH
}

void EffectCanEnterGDRLair::unaffect(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    pCreature->removeFlag(getEffectClass());

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    // 이펙트를 삭제하라고 알려준다.
    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pCreature->getObjectID());
    gcRemoveEffect.addEffectList(getSendEffectClass());

    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcRemoveEffect);

    destroy(pCreature->getName());

    __END_CATCH
}

void EffectCanEnterGDRLair::create(const string& ownerID)

{
    __BEGIN_TRY

    Turn_t currentYearTime;

    getCurrentYearTime(currentYearTime);

    defaultEffectSaveRepository().insertDeadline(EFFECT_TABLE_CAN_ENTER_GDR_LAIR, ownerID, currentYearTime,
                                                 m_Deadline.tv_sec);

    __END_CATCH
}

void EffectCanEnterGDRLair::destroy(const string& ownerID)

{
    __BEGIN_TRY

    defaultEffectSaveRepository().deleteDeadline(EFFECT_TABLE_CAN_ENTER_GDR_LAIR, ownerID);

    __END_CATCH
}

void EffectCanEnterGDRLair::save(const string& ownerID)

{
    __BEGIN_TRY

    Turn_t currentYearTime;

    getCurrentYearTime(currentYearTime);

    defaultEffectSaveRepository().updateDeadline(EFFECT_TABLE_CAN_ENTER_GDR_LAIR, ownerID, currentYearTime,
                                                 m_Deadline.tv_sec);

    __END_CATCH
}

string EffectCanEnterGDRLair::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectCanEnterGDRLair(" << ")";
    return msg.toString();

    __END_CATCH
}

void EffectCanEnterGDRLairLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    // Assert(pCreature != NULL);
    if (pCreature == NULL) {
        return;
    }

    vector<DWORD> dayTimes =
        defaultEffectSaveRepository().loadDeadlines(EFFECT_TABLE_CAN_ENTER_GDR_LAIR, pCreature->getName());

    for (size_t r = 0; r < dayTimes.size(); r++) {
        int DayTime = dayTimes[r];

        Timeval currentTime;
        getCurrentTime(currentTime);

        EffectCanEnterGDRLair* pEffectCanEnterGDRLair = new EffectCanEnterGDRLair(pCreature);

        EffectManager* pEffectManager = pCreature->getEffectManager();

        if (currentTime.tv_sec < DayTime) {
            pEffectCanEnterGDRLair->setDeadline((DayTime - currentTime.tv_sec) * 10);
        } else {
            pEffectCanEnterGDRLair->setDeadline(100);
        }

        pEffectManager->addEffect(pEffectCanEnterGDRLair);
        pCreature->setFlag(pEffectCanEnterGDRLair->getEffectClass());
    }

    __END_CATCH
}

EffectCanEnterGDRLairLoader* g_pEffectCanEnterGDRLairLoader = NULL;
