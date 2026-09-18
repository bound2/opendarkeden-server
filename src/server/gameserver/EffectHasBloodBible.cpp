//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectHasBloodBible.cpp
// Written by  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EffectHasBloodBible.h"

#include "BloodBible.h"
#include "CastleInfoManager.h"
#include "Creature.h"
#include "GCAddEffect.h"
#include "GCBloodBibleStatus.h"
#include "GCModifyInformation.h"
#include "GCRemoveEffect.h"
#include "GCStatusCurrentHP.h"
#include "GCSystemMessage.h"
#include "HolyLandManager.h"
#include "ItemInfoManager.h"
#include "Monster.h"
#include "MonsterCorpse.h"
#include "Player.h"
#include "ShrineInfoManager.h"
#include "Slayer.h"
#include "Vampire.h"
#include "WarSystem.h"
#include "ZoneGroupManager.h"
#include "ZoneInfoManager.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EffectHasBloodBible::EffectHasBloodBible(Creature* pCreature)

    : EffectHasRelic(pCreature){__BEGIN_TRY

                                    __END_CATCH}

      //////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////
      EffectHasBloodBible::EffectHasBloodBible(Item * pItem)

    : EffectHasRelic(pItem) {
    __BEGIN_TRY

    __END_CATCH
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectHasBloodBible::affect(Creature* pCreature)

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

msg << " has taken the Slayer relic.";

GCSystemMessage gcSystemMessage;
gcSystemMessage.setMessage(msg.toString());

g_pZoneGroupManager->broadcast( &gcSystemMessage );
*/

    // Get the zone.
    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    /*
    ZoneInfo* pZoneInfo = g_pZoneInfoManager->getZoneInfo( pZone->getZoneID() );
    Assert( pZoneInfo != NULL );

    // Announce the location.
    StringStream msg;
    msg << pCreature->getName() << " (" << ( pCreature->isSlayer() ? "Slayer" : "Vampire" ) << ") at "
        << pZoneInfo->getFullName() << "(" << (int)pCreature->getX() << ", " << (int)pCreature->getY()
        << ") has a fragment of the Blood Bible (" << m_PartName << ").";

    GCSystemMessage gcSystemMessage;
    gcSystemMessage.setMessage(msg.toString());

    g_pZoneGroupManager->broadcast( &gcSystemMessage );
    */

    ShrineSet* pShrineSet = g_pShrineInfoManager->getShrineSet(m_Part);
    Assert(pShrineSet != NULL);
    Race_t ShrineRace = pShrineSet->getOwnerRace();

    GCBloodBibleStatus* pGCBBS = new GCBloodBibleStatus;
    pGCBBS->setItemType(m_Part);
    pGCBBS->setZoneID(pZone->getZoneID());
    pGCBBS->setStorage(STORAGE_INVENTORY);
    pGCBBS->setOwnerName(pCreature->getName());
    pGCBBS->setRace(pCreature->getRace());
    pGCBBS->setShrineRace(ShrineRace);
    pGCBBS->setX(pCreature->getX());
    pGCBBS->setY(pCreature->getY());

    //	g_pHolyLandManager->broadcast( pGCBBS );
    g_pZoneGroupManager->broadcast(pGCBBS);
    g_pShrineInfoManager->registerBloodBibleStatus(m_Part, pGCBBS);

    setNextTime(m_Tick);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EffectHasBloodBible::affect(Item* pItem)

{
    __BEGIN_TRY

    if (m_pZone == NULL)
        return;

    /*
    ZoneInfo* pZoneInfo = g_pZoneInfoManager->getZoneInfo( m_pZone->getZoneID() );
    Assert( pZoneInfo != NULL );

    // Announce the location.
    StringStream msg;
    msg << pZoneInfo->getFullName() << "(" << (int)m_X << ", " << (int)m_Y
        << ") has a fragment of the Blood Bible (" << m_PartName << ") on its shrine.";

    GCSystemMessage gcSystemMessage;
    gcSystemMessage.setMessage(msg.toString());

    g_pZoneGroupManager->broadcast( &gcSystemMessage );
    */

    ShrineSet* pShrineSet = g_pShrineInfoManager->getShrineSet(m_Part);
    Assert(pShrineSet != NULL);

    GCBloodBibleStatus* pGCBBS = new GCBloodBibleStatus;
    pGCBBS->setItemType(m_Part);
    pGCBBS->setZoneID(m_pZone->getZoneID());
    pGCBBS->setStorage(STORAGE_CORPSE);
    // pGCBBS->setOwnerName( m_PartName );
    pGCBBS->setRace(pShrineSet->getOwnerRace());
    pGCBBS->setShrineRace(pShrineSet->getOwnerRace());
    pGCBBS->setX(m_X);
    pGCBBS->setY(m_Y);

    // Send the message only while a war is running at the castle.
    if (g_pWarSystem->hasActiveRaceWar()) {
        g_pHolyLandManager->broadcast(pGCBBS);
    }

    g_pShrineInfoManager->registerBloodBibleStatus(m_Part, pGCBBS);

    setNextTime(999999);

    __END_CATCH
}

void EffectHasBloodBible::setPart(int part)

{
    __BEGIN_TRY

    const BloodBibleInfo* pBloodBibleInfo =
        dynamic_cast<const BloodBibleInfo*>(g_pItemInfoManager->getItemInfo(Item::ITEM_CLASS_BLOOD_BIBLE, part));

    if (pBloodBibleInfo != NULL) {
        m_Part = part;
        m_PartName = pBloodBibleInfo->getName();
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
string EffectHasBloodBible::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "EffectHasBloodBible(" << "ObjectID:" << getObjectID() << ")";
    return msg.toString();

    __END_CATCH
}
