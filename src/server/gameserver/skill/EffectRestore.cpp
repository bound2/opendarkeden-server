//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectRestore.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectRestore.h"

#include "DB.h"
#include "Monster.h"
#include "Slayer.h"
#include "Vampire.h"
#include "repository/EffectSaveRepository.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectRestore::EffectRestore(Creature* pCreature)

{
    __BEGIN_TRY

    // 서버 전용 Effect이다. by sigi. 2002.11.14
    m_bBroadcastingEffect = false;

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectRestore::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectRestore::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectRestore::unaffect(Creature* pCreature)

{
    __BEGIN_TRY

    // cout << "EffectRestore" << "unaffect BEGIN" << endl;

    Assert(pCreature != NULL);
    destroy(pCreature->getName());

    // cout << "EffectRestore" << "unaffect END" << endl;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectRestore::unaffect()

{
    __BEGIN_TRY

    // cout << "EffectRestore" << "unaffect BEGIN" << endl;

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    // cout << "EffectRestore" << "unaffect END" << endl;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectRestore::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectRestore::create(const string& ownerID)

{
    __BEGIN_TRY

    Turn_t currentYearTime;

    getCurrentYearTime(currentYearTime);

    defaultEffectSaveRepository().insertDeadline(EFFECT_TABLE_RESTORE, ownerID, currentYearTime, m_Deadline.tv_sec);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectRestore::destroy(const string& ownerID)

{
    __BEGIN_TRY

    defaultEffectSaveRepository().deleteDeadline(EFFECT_TABLE_RESTORE, ownerID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectRestore::save(const string& ownerID)

{
    __BEGIN_TRY

    Turn_t currentYearTime;

    getCurrentYearTime(currentYearTime);

    defaultEffectSaveRepository().updateDeadline(EFFECT_TABLE_RESTORE, ownerID, currentYearTime, m_Deadline.tv_sec);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectRestore::toString() const throw() {
    __BEGIN_TRY

    StringStream msg;

    msg << "EffectRestore(" << "ObjectID:" << getObjectID() << ")";

    return msg.toString();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectRestoreLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<DWORD> dayTimes = defaultEffectSaveRepository().loadDeadlines(EFFECT_TABLE_RESTORE, pCreature->getName());

    for (size_t r = 0; r < dayTimes.size(); r++) {
        if (pCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);

            int DayTime = dayTimes[r];

            Timeval currentTime;
            getCurrentTime(currentTime);

            EffectRestore* pEffectRestore = new EffectRestore(pCreature);

            if (currentTime.tv_sec < DayTime) {
                pEffectRestore->setDeadline((DayTime - currentTime.tv_sec) * 10);

                pSlayer->addEffect(pEffectRestore);
                pSlayer->setFlag(Effect::EFFECT_CLASS_RESTORE);
            } else {
                pEffectRestore->setDeadline(6000);

                pSlayer->addEffect(pEffectRestore);
                pSlayer->setFlag(Effect::EFFECT_CLASS_RESTORE);
            }
        }
    }

    __END_CATCH
}

EffectRestoreLoader* g_pEffectRestoreLoader = NULL;
