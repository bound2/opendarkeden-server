//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectAftermath.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectAftermath.h"

#include "Creature.h"
#include "GamePlayer.h"
#include "Slayer.h"
#include "Vampire.h"
#include "repository/EffectSaveRepository.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectAftermath::EffectAftermath(Creature* pCreature)

{
    __BEGIN_TRY

    // 서버 전용 Effect이다. by sigi. 2002.11.14
    m_bBroadcastingEffect = false;

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectAftermath::~EffectAftermath()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectAftermath::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectAftermath::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectAftermath::unaffect(Creature* pFromCreature)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    // cout << "EffectAftermath" << "unaffect BEGIN" << endl;

    Assert(pFromCreature != NULL);
    pFromCreature->removeFlag(Effect::EFFECT_CLASS_AFTERMATH);
    destroy(pFromCreature->getName());

    // cout << "EffectAftermath" << "unaffect END" << endl;

    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectAftermath::unaffect()

{
    __BEGIN_TRY

    // cout << "EffectAftermath" << "unaffect BEGIN" << endl;

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    // cout << "EffectAftermath" << "unaffect END" << endl;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectAftermath::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectAftermath::create(const string& ownerID)

{
    __BEGIN_TRY

    Turn_t currentYearTime;

    getCurrentYearTime(currentYearTime);

    defaultEffectSaveRepository().insertDeadline(EFFECT_TABLE_AFTERMATH, ownerID, currentYearTime, m_Deadline.tv_sec);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectAftermath::destroy(const string& ownerID)

{
    __BEGIN_TRY

    defaultEffectSaveRepository().deleteDeadline(EFFECT_TABLE_AFTERMATH, ownerID);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectAftermath::save(const string& ownerID)

{
    __BEGIN_TRY

    Turn_t currentYearTime;

    getCurrentYearTime(currentYearTime);

    defaultEffectSaveRepository().updateDeadline(EFFECT_TABLE_AFTERMATH, ownerID, currentYearTime, m_Deadline.tv_sec);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectAftermath::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "EffectAftermath(" << "ObjectID:" << getObjectID() << ")";

    return msg.toString();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectAftermathLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    if (pCreature == NULL || (!pCreature->isSlayer() && !pCreature->isOusters())) {
        // cout << "EffectAftermathLoader : 크리쳐가 널입니다." << endl;
        return;
    }

    vector<DWORD> dayTimes = defaultEffectSaveRepository().loadDeadlines(EFFECT_TABLE_AFTERMATH, pCreature->getName());

    for (size_t r = 0; r < dayTimes.size(); r++) {
        int DayTime = dayTimes[r];

        Timeval currentTime;
        getCurrentTime(currentTime);

        EffectAftermath* pEffectAftermath = new EffectAftermath(pCreature);

        if (currentTime.tv_sec < DayTime) {
            pEffectAftermath->setDeadline((DayTime - currentTime.tv_sec) * 10);

            pCreature->setFlag(Effect::EFFECT_CLASS_AFTERMATH);

            EffectManager* pEffectManager = pCreature->getEffectManager();
            pEffectManager->addEffect(pEffectAftermath);
        } else {
            pEffectAftermath->setDeadline(0);

            pCreature->setFlag(Effect::EFFECT_CLASS_AFTERMATH);

            EffectManager* pEffectManager = pCreature->getEffectManager();
            pEffectManager->addEffect(pEffectAftermath);
        }
    }

    __END_CATCH
}

EffectAftermathLoader* g_pEffectAftermathLoader = NULL;
