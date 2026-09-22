//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectComa.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectComa.h"

#include "AlignmentManager.h"
#include "Assert.h"
#include "Event.h"
#include "EventResurrect.h"
#include "GCCreatureDied.h"
#include "GCGetOffMotorCycle.h"
#include "GamePlayer.h"
#include "IncomingPlayerManager.h"
#include "ItemFactoryManager.h"
#include "ItemUtil.h"
#include "Ousters.h"
#include "ResurrectLocationManager.h"
#include "Slayer.h"
#include "SlayerCorpse.h"
#include "Thread.h"
#include "Tile.h"
#include "Vampire.h"
#include "VampireCorpse.h"
#include "Zone.h"
#include "ZoneGroupManager.h"
#include "ZoneInfo.h"
#include "ZoneInfoManager.h"
#include "ZonePlayerManager.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

EffectComa::EffectComa(Creature* pCreature)

{
    __BEGIN_TRY

    setTarget(pCreature);

    __END_CATCH
}

EffectComa::~EffectComa()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

void EffectComa::affect()

{
    __BEGIN_TRY

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);

    affect(pCreature);

    __END_CATCH
}

void EffectComa::affect(Creature* pCreature)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectComa::unaffect(Creature* pDeadCreature)

{
    __BEGIN_TRY

    // cout << "EffectComa " << "unaffect BEGIN(Creature)" << endl;

    Assert(pDeadCreature != NULL);
    // Assert(pDeadCreature->isDead());

    // Clear the effect flag.
    pDeadCreature->removeFlag(Effect::EFFECT_CLASS_COMA);
    // Code that forces the creature to die once it gets here.
    if (pDeadCreature->isSlayer()) {
        Slayer* pSlayer = dynamic_cast<Slayer*>(pDeadCreature);
        pSlayer->setHP(0, ATTR_CURRENT);
    } else if (pDeadCreature->isVampire()) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pDeadCreature);
        pVampire->setHP(0, ATTR_CURRENT);
    } else if (pDeadCreature->isOusters()) {
        Ousters* pOusters = dynamic_cast<Ousters*>(pDeadCreature);
        pOusters->setHP(0, ATTR_CURRENT);
    }

    // cout << "EffectComa " << "unaffect END(Creature)" << endl;

    __END_CATCH
}

void EffectComa::unaffect()

{
    __BEGIN_TRY

    // cout << "EffectComa " << "unaffect BEGIN" << endl;

    Creature* pCreature = dynamic_cast<Creature*>(m_pTarget);
    unaffect(pCreature);

    // cout << "EffectComa " << "unaffect END" << endl;

    __END_CATCH
}

void EffectComa::create(const string& ownerID)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectComa::destroy(const string& ownerID)

{
    __BEGIN_TRY
    __END_CATCH
}

void EffectComa::save(const string& ownerID)

{
    __BEGIN_TRY
    __END_CATCH
}

// The start time is when the effect was created, which is when the player died.
// Adding 5 seconds to it lets canResurrect reject anyone who has been dead for
// less than 5 seconds.
void EffectComa::setStartTime(void) {
    getCurrentTime(m_StartTime);
    m_StartTime.tv_sec += 5;
}

bool EffectComa::canResurrect(void) {
    Timeval currentTime;
    getCurrentTime(currentTime);

    if (m_StartTime < currentTime) {
        return true;
    }

    return false;
}

string EffectComa::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectComa(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}

void EffectComaLoader::load(Creature* pCreature)

{
    __BEGIN_TRY __END_CATCH
}
