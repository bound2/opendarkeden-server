////////////////////////////////////////////////////////////////////////////////
// Filename    : ConditionEnterHolyLand.cpp
// Written By  :
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "ConditionEnterHolyLand.h"

#include "DB.h"
#include "FlagSet.h"
#include "GamePlayer.h"
#include "PacketUtil.h"
#include "PaySystem.h"
#include "PlayerCreature.h"
#include "VariableManager.h"
#include "WarSystem.h"
#include "Zone.h"
#include "ZoneGroupManager.h"
#include "ZoneInfoManager.h"
#include "ZoneUtil.h"

////////////////////////////////////////////////////////////////////////////////
// is satisfied?
////////////////////////////////////////////////////////////////////////////////
bool ConditionEnterHolyLand::isSatisfied(Creature* pCreature1, Creature* pCreature2, void* pParam) const

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
        // During a race war, if the number of participants is limited.
        if (g_pWarSystem->hasActiveRaceWar() && g_pVariableManager->isActiveRaceWarLimiter()) {
            Zone* pZone = getZoneByZoneID(m_TargetZoneID);
            Assert(pZone != NULL);

            // When entering Adam's holy land
            if (!pZone->isHolyLand()) {
                return true;
            }

            PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature2);

            // The PC must have joined the war.
            return pPC->isFlag(Effect::EFFECT_CLASS_RACE_WAR_JOIN_TICKET);
        }

        // When no war is going on, entry is free.
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void ConditionEnterHolyLand::read(PropertyBuffer& propertyBuffer)

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
string ConditionEnterHolyLand::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ConditionEnterHolyLand(" << "TargetZoneID:" << (int)m_TargetZoneID << ")";
    return msg.toString();

    __END_CATCH
}
