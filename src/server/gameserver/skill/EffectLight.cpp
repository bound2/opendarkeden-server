//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectLight.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectLight.h"

#include "Creature.h"
#include "DB.h"
#include "GCModifyInformation.h"
#include "GCRemoveEffect.h"
#include "Player.h"
#include "Slayer.h"
#include "repository/EffectSaveRepository.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectLight member methods
//////////////////////////////////////////////////////////////////////////////

EffectLight::EffectLight(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);

    __END_CATCH
}

void EffectLight::affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectLight::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectLight::unaffect(Creature* pCreature)

{
    __BEGIN_TRY

    // cout << "EffectLight " << "unaffect BEGIN" << endl;

    Assert(pCreature != NULL);
    Assert(pCreature->isSlayer());

    Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);

    if (pSlayer != NULL) {
        Zone* pZone = pSlayer->getZone();

        Sight_t NewSight = 0;

        if (pSlayer->isFlag(Effect::EFFECT_CLASS_YELLOW_POISON_TO_CREATURE)) {
            NewSight = 3;
        } else {
            NewSight = 13;
        }

        pZone->updateScan(pSlayer, pSlayer->getSight(), NewSight);
        pSlayer->setSight(NewSight);

        pSlayer->removeFlag(Effect::EFFECT_CLASS_LIGHT);

        Player* pPlayer = pSlayer->getPlayer();

        GCModifyInformation _GCModifyInformation;

        _GCModifyInformation.addShortData(MODIFY_VISION, NewSight);

        pPlayer->sendPacket(&_GCModifyInformation);

        // 이펙트가 사라졌다고 알려준다.
        GCRemoveEffect gcRemoveEffect;
        gcRemoveEffect.setObjectID(pSlayer->getObjectID());
        gcRemoveEffect.addEffectList(Effect::EFFECT_CLASS_LIGHT);
        pZone->broadcastPacket(pSlayer->getX(), pSlayer->getY(), &gcRemoveEffect);

        destroy(pSlayer->getName());
    }

    // cout << "EffectLight " << "unaffect END" << endl;

    __END_CATCH
}

void EffectLight::unaffect()

{
    __BEGIN_TRY

    Slayer* pSlayer = dynamic_cast<Slayer*>(m_pTarget);
    unaffect(pSlayer);

    __END_CATCH
}

void EffectLight::unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectLight::create(const string& ownerID)

{
    __BEGIN_TRY

    Timeval currentTime;
    getCurrentTime(currentTime);

    Turn_t currentYearTime;
    getCurrentYearTime(currentYearTime);

    defaultEffectSaveRepository().insertCreatureEffect(CREATURE_EFFECT_LIGHT, ownerID, currentYearTime,
                                                       m_Deadline.tv_sec, 0, (int)m_OldSight);

    __END_CATCH
}

void EffectLight::destroy(const string& ownerID)

{
    __BEGIN_TRY

    defaultEffectSaveRepository().deleteCreatureEffect(CREATURE_EFFECT_LIGHT, ownerID);

    __END_CATCH
}

void EffectLight::save(const string& ownerID)

{
    __BEGIN_TRY

    Turn_t currentYearTime;
    getCurrentYearTime(currentYearTime);

    defaultEffectSaveRepository().updateCreatureEffect(CREATURE_EFFECT_LIGHT, ownerID, currentYearTime,
                                                       m_Deadline.tv_sec, 0, (int)m_OldSight);

    __END_CATCH
}

string EffectLight::toString() const throw() {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectLight(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}

void EffectLightLoader::load(Creature* pCreature)

{
    __BEGIN_TRY

    if (pCreature == NULL) {
        // cout << "EffectLightLoader : 크리쳐가 널입니다." << endl;
        return;
    }

    vector<CreatureEffectRow> rows =
        defaultEffectSaveRepository().loadCreatureEffects(CREATURE_EFFECT_LIGHT, pCreature->getName());

    for (size_t r = 0; r < rows.size(); r++) {
        if (pCreature->isSlayer()) {
            Turn_t YearTime = rows[r].yearTime;
            int DayTime = rows[r].dayTime;

            Turn_t currentYearTime;

            Timeval currentTime;

            getCurrentYearTime(currentYearTime);

            getCurrentTime(currentTime);

            int leftTime = ((YearTime - currentYearTime) * 24 * 60 * 60 + (DayTime - currentTime.tv_sec)) * 10;
            EffectLight* pEffect = new EffectLight(pCreature);
            if (leftTime > 0) {
                pEffect->setDeadline(leftTime);
                pCreature->setFlag(Effect::EFFECT_CLASS_LIGHT);
                pCreature->addEffect(pEffect);
            } else {
                pEffect->destroy(pCreature->getName());
                SAFE_DELETE(pEffect);
            }
        }
    }

    __END_CATCH
}

EffectLightLoader* g_pEffectLightLoader = NULL;
