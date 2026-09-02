//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectMute.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectMute.h"

#include "Creature.h"
#include "EventMorph.h"
#include "EventRegeneration.h"
#include "GCChangeDarkLight.h"
#include "GCModifyInformation.h"
#include "GCMorph1.h"
#include "GCMorphVampire2.h"
#include "GCRemoveEffect.h"
#include "GamePlayer.h"
#include "Monster.h"
#include "PCFinder.h"
#include "PCManager.h"
#include "Slayer.h"
#include "Vampire.h"
#include "repository/EffectSaveRepository.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectMute::EffectMute(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectMute::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectMute::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectMute::unaffect(Creature* pFromCreature)

{
    __BEGIN_TRY

    Assert(pFromCreature != NULL);

    pFromCreature->removeFlag(Effect::EFFECT_CLASS_MUTE);

    GCRemoveEffect gcRemoveEffect;
    gcRemoveEffect.setObjectID(pFromCreature->getObjectID());
    gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_MUTE);
    pFromCreature->getPlayer()->sendPacket(&gcRemoveEffect);

    destroy(pFromCreature->getName());

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectMute::unaffect()

{
    __BEGIN_TRY

    // cout << "EffectMute" << "unaffect BEGIN" << endl;

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    // cout << "EffectMute" << "unaffect END" << endl;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectMute::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectMute::create(const string& ownerID)

{
    __BEGIN_TRY

    Turn_t currentYearTime;

    getCurrentYearTime(currentYearTime);

    defaultEffectSaveRepository().insertDeadline(EFFECT_TABLE_MUTE, ownerID, currentYearTime, m_Deadline.tv_sec);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectMute::destroy(const string& ownerID)

{
    __BEGIN_TRY

    defaultEffectSaveRepository().deleteDeadline(EFFECT_TABLE_MUTE, ownerID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectMute::save(const string& ownerID)

{
    __BEGIN_TRY

    Turn_t currentYearTime;

    getCurrentYearTime(currentYearTime);

    defaultEffectSaveRepository().updateDeadline(EFFECT_TABLE_MUTE, ownerID, currentYearTime, m_Deadline.tv_sec);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectMute::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "EffectMute(" << "ObjectID:" << getObjectID() << ")";

    return msg.toString();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectMuteLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    vector<DWORD> dayTimes = defaultEffectSaveRepository().loadDeadlines(EFFECT_TABLE_MUTE, pCreature->getName());

    for (size_t r = 0; r < dayTimes.size(); r++) {
        int DayTime = dayTimes[r];

        Timeval currentTime;
        getCurrentTime(currentTime);

        EffectMute* pEffectMute = new EffectMute(pCreature);

        if (currentTime.tv_sec < DayTime) {
            pEffectMute->setDeadline((DayTime - currentTime.tv_sec) * 10);

            pCreature->addEffect(pEffectMute);
            pCreature->setFlag(Effect::EFFECT_CLASS_MUTE);
        } else {
            pEffectMute->destroy(pCreature->getName());
        }
    }

    __END_CATCH
}

EffectMuteLoader* g_pEffectMuteLoader = NULL;
