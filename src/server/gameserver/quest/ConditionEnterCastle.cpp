////////////////////////////////////////////////////////////////////////////////
// Filename    : ConditionEnterCastle.cpp
// Written By  :
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "ConditionEnterCastle.h"

#include "CastleInfoManager.h"
#include "DB.h"
#include "FlagSet.h"
#include "GamePlayer.h"
#include "PacketUtil.h"
#include "PaySystem.h"
#include "PlayerCreature.h"
#include "VariableManager.h"
#include "Zone.h"
#include "ZoneGroupManager.h"
#include "ZoneInfoManager.h"
#include "ZoneUtil.h"

////////////////////////////////////////////////////////////////////////////////
// is satisfied?
////////////////////////////////////////////////////////////////////////////////
bool ConditionEnterCastle::isSatisfied(Creature* pCreature1, Creature* pCreature2, void* pParam) const

{
    // A check for whether a war is in progress still has to be added.

    Assert(pCreature2 != NULL);
    Assert(pCreature2->isPC());


    bool bPayPlay = false;

    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pCreature2->getPlayer());
    Assert(pGamePlayer != NULL);

    bPayPlay = true;

    // Only someone who has paid can enter the castle.
    if (bPayPlay) {
        // Find the zone.
        Zone* pZone = getZoneByZoneID(m_TargetZoneID);
        Assert(pZone != NULL);

        // If it is not a castle, there is nothing to check.
        if (!pZone->isCastle()) {
            return true;
        }

        // During a war between races, everyone must be let through regardless of race.
        // During a war within a race, other races must not be let through.
        // In peacetime entry goes through an NPC, so nobody may be let through.
        // All of this has to be added when the war system is built.
        PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature2);

        return g_pCastleInfoManager->canPortalActivate(m_TargetZoneID, pPC);
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void ConditionEnterCastle::read(PropertyBuffer& propertyBuffer)

{
    try {
        // read turn
        m_TargetZoneID = propertyBuffer.getPropertyInt("TargetZoneID");
    } catch (NoSuchElementException& nsee) {
        throw Error(nsee.toString());
    }
}

////////////////////////////////////////////////////////////////////////////////
// get debug string
////////////////////////////////////////////////////////////////////////////////
string ConditionEnterCastle::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ConditionEnterCastle(" << "TargetZoneID:" << (int)m_TargetZoneID << ")";
    return msg.toString();

    __END_CATCH
}
