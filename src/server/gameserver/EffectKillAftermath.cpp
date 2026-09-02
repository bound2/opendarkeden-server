//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectKillAftermath.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectKillAftermath.h"

#include "Creature.h"
#include "GamePlayer.h"
#include "Ousters.h"
#include "Slayer.h"
#include "Vampire.h"
#include "repository/EffectSaveRepository.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectKillAftermath::EffectKillAftermath(Creature* pCreature)

{
    __BEGIN_TRY

    // 서버 전용 Effect이다. by sigi. 2002.11.14
    m_bBroadcastingEffect = false;

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectKillAftermath::~EffectKillAftermath()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectKillAftermath::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectKillAftermath::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectKillAftermath::unaffect(Creature* pFromCreature)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    // cout << "EffectKillAftermath" << "unaffect BEGIN" << endl;

    Assert(pFromCreature != NULL);
    pFromCreature->removeFlag(Effect::EFFECT_CLASS_KILL_AFTERMATH);
    destroy(pFromCreature->getName());

    // cout << "EffectKillAftermath" << "unaffect END" << endl;

    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectKillAftermath::unaffect()

{
    __BEGIN_TRY

    // cout << "EffectKillAftermath" << "unaffect BEGIN" << endl;

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    // cout << "EffectKillAftermath" << "unaffect END" << endl;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectKillAftermath::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectKillAftermath::create(const string& ownerID)

{
    __BEGIN_TRY

    Turn_t currentYearTime;

    getCurrentYearTime(currentYearTime);

    defaultEffectSaveRepository().insertDeadline(EFFECT_TABLE_KILL_AFTERMATH, ownerID, currentYearTime,
                                                 m_Deadline.tv_sec);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectKillAftermath::destroy(const string& ownerID)

{
    __BEGIN_TRY

    defaultEffectSaveRepository().deleteDeadline(EFFECT_TABLE_KILL_AFTERMATH, ownerID);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectKillAftermath::save(const string& ownerID)

{
    __BEGIN_TRY

    Turn_t currentYearTime;

    getCurrentYearTime(currentYearTime);

    defaultEffectSaveRepository().updateDeadline(EFFECT_TABLE_KILL_AFTERMATH, ownerID, currentYearTime,
                                                 m_Deadline.tv_sec);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectKillAftermath::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "EffectKillAftermath(" << "ObjectID:" << getObjectID() << ")";

    return msg.toString();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectKillAftermathLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    if (pCreature == NULL) {
        // cout << "EffectKillAftermathLoader : 크리쳐가 널입니다." << endl;
        return;
    }

    vector<DWORD> dayTimes =
        defaultEffectSaveRepository().loadDeadlines(EFFECT_TABLE_KILL_AFTERMATH, pCreature->getName());

    for (size_t r = 0; r < dayTimes.size(); r++) {
        if (pCreature->isSlayer()) {
            int DayTime = dayTimes[r];

            Timeval currentTime;
            getCurrentTime(currentTime);

            EffectKillAftermath* pEffectKillAftermath = new EffectKillAftermath(pCreature);

            if (currentTime.tv_sec < DayTime) {
                pEffectKillAftermath->setDeadline((DayTime - currentTime.tv_sec) * 10);

                Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
                pSlayer->setFlag(Effect::EFFECT_CLASS_KILL_AFTERMATH);

                EffectManager* pEffectManager = pSlayer->getEffectManager();
                pEffectManager->addEffect(pEffectKillAftermath);
            } else {
                pEffectKillAftermath->setDeadline(0);

                Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
                pSlayer->setFlag(Effect::EFFECT_CLASS_KILL_AFTERMATH);

                EffectManager* pEffectManager = pSlayer->getEffectManager();
                pEffectManager->addEffect(pEffectKillAftermath);
            }
        } else if (pCreature->isVampire()) {
            int DayTime = dayTimes[r];

            Timeval currentTime;
            getCurrentTime(currentTime);

            EffectKillAftermath* pEffectKillAftermath = new EffectKillAftermath(pCreature);

            if (currentTime.tv_sec < DayTime) {
                pEffectKillAftermath->setDeadline((DayTime - currentTime.tv_sec) * 10);

                Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);
                pVampire->setFlag(Effect::EFFECT_CLASS_KILL_AFTERMATH);

                EffectManager* pEffectManager = pVampire->getEffectManager();
                pEffectManager->addEffect(pEffectKillAftermath);
            } else {
                pEffectKillAftermath->setDeadline(0);

                Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);
                pVampire->setFlag(Effect::EFFECT_CLASS_KILL_AFTERMATH);

                EffectManager* pEffectManager = pVampire->getEffectManager();
                pEffectManager->addEffect(pEffectKillAftermath);
            }
        } else if (pCreature->isOusters()) {
            int DayTime = dayTimes[r];

            Timeval currentTime;
            getCurrentTime(currentTime);

            EffectKillAftermath* pEffectKillAftermath = new EffectKillAftermath(pCreature);

            if (currentTime.tv_sec < DayTime) {
                pEffectKillAftermath->setDeadline((DayTime - currentTime.tv_sec) * 10);

                Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);
                pOusters->setFlag(Effect::EFFECT_CLASS_KILL_AFTERMATH);

                EffectManager* pEffectManager = pOusters->getEffectManager();
                pEffectManager->addEffect(pEffectKillAftermath);
            } else {
                pEffectKillAftermath->setDeadline(0);

                Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);
                pOusters->setFlag(Effect::EFFECT_CLASS_KILL_AFTERMATH);

                EffectManager* pEffectManager = pOusters->getEffectManager();
                pEffectManager->addEffect(pEffectKillAftermath);
            }
        }
    }

    __END_CATCH
}

EffectKillAftermathLoader* g_pEffectKillAftermathLoader = NULL;
