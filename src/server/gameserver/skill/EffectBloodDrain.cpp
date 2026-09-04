//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectBloodDrain.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectBloodDrain.h"

#include "Creature.h"
#include "DB.h"
#include "EventMorph.h"
#include "EventRegeneration.h"
#include "GCChangeDarkLight.h"
#include "GCModifyInformation.h"
#include "GCMorph1.h"
#include "GCMorphVampire2.h"
#include "GamePlayer.h"
#include "Monster.h"
#include "PCFinder.h"
#include "PCManager.h"
#include "Slayer.h"
#include "Vampire.h"
#include "repository/EffectSaveRepository.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectBloodDrain::EffectBloodDrain(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectBloodDrain::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectBloodDrain::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectBloodDrain::unaffect(Creature* pFromCreature)

{
    __BEGIN_TRY

    // cout << "EffectBloodDrain" << "unaffect BEGIN" << endl;
    Assert(pFromCreature != NULL);

    if (pFromCreature->isSlayer()) {
        Player* pPlayer = pFromCreature->getPlayer();

        Assert(pPlayer != NULL);

        // GamePlayer에 Event를 붙여서 heartbeat를 다 수행한후 지워준다.
        // 동기화 문제가 없을까? -_-; 다이어그램 상으론 문제가 없쥐만 -_-;
        GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);

        pGamePlayer->deleteEvent(Event::EVENT_CLASS_REGENERATION);

        EventMorph* pEventMorph = new EventMorph(pGamePlayer);

        pEventMorph->setCreature(pFromCreature);
        pEventMorph->setDeadline(0);

        pGamePlayer->addEvent(pEventMorph);

        EventRegeneration* pEventRegeneration = new EventRegeneration(pGamePlayer);

        pEventRegeneration->setDeadline(10 * 10);
        pGamePlayer->addEvent(pEventRegeneration);

        destroy(pFromCreature->getName());
    } else {
        // 시야 복구.
        Assert(pFromCreature->isOusters());

        Player* pPlayer = pFromCreature->getPlayer();
        Assert(pPlayer != NULL);

        pFromCreature->removeFlag(Effect::EFFECT_CLASS_BLOOD_DRAIN);

        Sight_t oldSight = pFromCreature->getSight();
        Sight_t newSight = pFromCreature->getEffectedSight();

        if (oldSight != newSight) {
            GCModifyInformation gcMI;
            pFromCreature->setSight(newSight);
            pFromCreature->getZone()->updateScan(pFromCreature, oldSight, pFromCreature->getSight());
            gcMI.addShortData(MODIFY_VISION, pFromCreature->getSight());
            pFromCreature->getPlayer()->sendPacket(&gcMI);

            GCChangeDarkLight gcChangeDarkLight;
            gcChangeDarkLight.setDarkLevel(13);
            gcChangeDarkLight.setLightLevel(min(6, (int)newSight));
            pFromCreature->getPlayer()->sendPacket(&gcChangeDarkLight);
        }

        // DB에서 지워뿐다.
        destroy(pFromCreature->getName());
    }

    // cout << "EffectBloodDrain" << "unaffect END" << endl;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectBloodDrain::unaffect()

{
    __BEGIN_TRY

    // cout << "EffectBloodDrain" << "unaffect BEGIN" << endl;

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    // cout << "EffectBloodDrain" << "unaffect END" << endl;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectBloodDrain::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectBloodDrain::create(const string& ownerID)

{
    __BEGIN_TRY

    Turn_t currentYearTime;

    getCurrentYearTime(currentYearTime);

    defaultEffectSaveRepository().insertCreatureEffect(CREATURE_EFFECT_BLOOD_DRAIN, ownerID, currentYearTime,
                                                       m_Deadline.tv_sec, (int)m_Level, 0);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectBloodDrain::destroy(const string& ownerID)

{
    __BEGIN_TRY

    defaultEffectSaveRepository().deleteCreatureEffect(CREATURE_EFFECT_BLOOD_DRAIN, ownerID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectBloodDrain::save(const string& ownerID)

{
    __BEGIN_TRY

    Turn_t currentYearTime;

    getCurrentYearTime(currentYearTime);

    defaultEffectSaveRepository().updateCreatureEffect(CREATURE_EFFECT_BLOOD_DRAIN, ownerID, currentYearTime,
                                                       m_Deadline.tv_sec, m_Level, 0);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectBloodDrain::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "EffectBloodDrain(" << "ObjectID:" << getObjectID() << ")";

    return msg.toString();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectBloodDrainLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);
    if (!pCreature->isSlayer() && !pCreature->isOusters())
        return;

    vector<CreatureEffectRow> rows =
        defaultEffectSaveRepository().loadCreatureEffects(CREATURE_EFFECT_BLOOD_DRAIN, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        int DayTime = rows[r].dayTime;

        Timeval currentTime;
        getCurrentTime(currentTime);

        EffectBloodDrain* pEffectBloodDrain = new EffectBloodDrain(pCreature);

        if (currentTime.tv_sec + 600 < DayTime) {
            pEffectBloodDrain->setDeadline((DayTime - currentTime.tv_sec) * 10);
            pEffectBloodDrain->setLevel(rows[r].level);

            pCreature->addEffect(pEffectBloodDrain);
            pCreature->setFlag(Effect::EFFECT_CLASS_BLOOD_DRAIN);
        } else {
            // pEffectBloodDrain->setDeadline(6000);
            pEffectBloodDrain->setDeadline(6000);
            pEffectBloodDrain->setLevel(rows[r].level);

            pCreature->addEffect(pEffectBloodDrain);
            pCreature->setFlag(Effect::EFFECT_CLASS_BLOOD_DRAIN);
        }
    }

    __END_CATCH
}

EffectBloodDrainLoader* g_pEffectBloodDrainLoader = NULL;
