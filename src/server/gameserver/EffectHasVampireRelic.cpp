//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectHasVampireRelic.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectHasVampireRelic.h"

#include <stdio.h>

#include "Creature.h"
#include "GCAddEffect.h"
#include "GCModifyInformation.h"
#include "GCRemoveEffect.h"
#include "GCStatusCurrentHP.h"
#include "GCSystemMessage.h"
#include "Monster.h"
#include "MonsterCorpse.h"
#include "Player.h"
#include "Slayer.h"
#include "StringPool.h"
#include "Vampire.h"
#include "ZoneGroupManager.h"
#include "ZoneInfoManager.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectHasVampireRelic::EffectHasVampireRelic(Creature* pCreature)

    : EffectHasRelic(pCreature){__BEGIN_TRY

                                    __END_CATCH}
      //////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////
      EffectHasVampireRelic::EffectHasVampireRelic(Item * pItem)

    : EffectHasRelic(pItem) {
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectHasVampireRelic::affect(Creature* pCreature)

{
    __BEGIN_TRY

    // Timeval      nextTime   = getNextTime();
    // Timeval      deadLine   = getDeadline();
    // Turn_t       RemainTime = deadLine.tv_sec - nextTime.tv_sec;
    /*
    StringStream msg;

    if (pCreature->isSlayer())
    {
        Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);

        msg << pSlayer->getName();
    }
    else
    {
        Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);

        msg << pVampire->getName();
    }

    msg << " has taken the Vampire relic.";

    GCSystemMessage gcSystemMessage;
    gcSystemMessage.setMessage(msg.toString());

    g_pZoneGroupManager->broadcast( &gcSystemMessage );
    */
    // Get the zone.
    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    ZoneInfo* pZoneInfo = g_pZoneInfoManager->getZoneInfo(pZone->getZoneID());
    Assert(pZoneInfo != NULL);

    // Announce the location.
    char msg[100];

    const char* race;
    if (pCreature->isSlayer()) {
        race = g_pStringPool->c_str(STRID_SLAYER);
    } else if (pCreature->isVampire()) {
        race = g_pStringPool->c_str(STRID_VAMPIRE);
    } else if (pCreature->isOusters()) {
        race = g_pStringPool->c_str(STRID_OUSTERS);
    }

    sprintf(msg, g_pStringPool->c_str(STRID_HAVING_VAMPIRE_RELIC), pCreature->getName().c_str(), race,
            //                ( pCreature->isSlayer() ? g_pStringPool->c_str( STRID_SLAYER ) : g_pStringPool->c_str(
            //                STRID_VAMPIRE ) ),
            (int)pCreature->getX(), (int)pCreature->getY());

    /*	// Announce the location.
        StringStream msg;
        msg << pCreature->getName() << " (" << ( pCreature->isSlayer() ? "Slayer" : "Vampire" ) << ") at "
            << pZoneInfo->getFullName() << "(" << (int)pCreature->getX() << ", " << (int)pCreature->getY()
            << ") has the Vampire relic."; */

    GCSystemMessage gcSystemMessage;
    gcSystemMessage.setMessage(msg);

    g_pZoneGroupManager->broadcast(&gcSystemMessage);

    setNextTime(m_Tick);

    __END_CATCH
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectHasVampireRelic::affect(Item* pItem)

    {__BEGIN_TRY

         /*
      //Timeval      nextTime   = getNextTime();
      //Timeval      deadLine   = getDeadline();
      //Turn_t       RemainTime = deadLine.tv_sec - nextTime.tv_sec;
      StringStream msg;

      if (pCreature->isSlayer())
      {
          Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);

          msg << pSlayer->getName() << " ";
      }
      else
      {
          Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);

          msg << pVampire->getName() << " ";
      }

      msg << " has the Vampire relic." << endl;

      GCSystemMessage gcSystemMessage;
      gcSystemMessage.setMessage(msg.toString());

      g_pZoneGroupManager->broadcast( &gcSystemMessage );

      setNextTime(m_Tick);
      */

         __END_CATCH}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectHasVampireRelic::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectHasVampireRelic(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
