////////////////////////////////////////////////////////////////////////////////
// Filename    : ConditionEnterMasterLair.cpp
// Written By  :
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "ConditionEnterMasterLair.h"

#include "DB.h"
#include "FlagSet.h"
#include "GamePlayer.h"
#include "MasterLairManager.h"
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
bool ConditionEnterMasterLair::isSatisfied(Creature* pCreature1, Creature* pCreature2, void* pParam) const

{
    if (!g_pVariableManager->isActiveMasterLair()) {
        return false;
    }

    Assert(pCreature2 != NULL);
    Assert(pCreature2->isPC());


    bool bPayPlay = false;

    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pCreature2->getPlayer());
    Assert(pGamePlayer != NULL);

    bPayPlay = true;

    // 돈 낸 사람만 마스터 레어에 들어갈 수 있다.
    if (bPayPlay) {
        // 존을 찾는다.
        Zone* pZone = getZoneByZoneID(m_TargetZoneID);
        Assert(pZone != NULL);

        // 마스터 레어가 아니면 체크할 필요가 없는거다.
        if (!pZone->isMasterLair()) {
            return true;
        }

        MasterLairManager* pMasterLairManager = pZone->getMasterLairManager();
        Assert(pMasterLairManager != NULL);

        if (pMasterLairManager->enterCreature(pCreature2)) {
            // 출입 가능
            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void ConditionEnterMasterLair::read(PropertyBuffer& propertyBuffer)

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
string ConditionEnterMasterLair::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ConditionEnterMasterLair(" << "TargetZoneID:" << (int)m_TargetZoneID << ")";
    return msg.toString();

    __END_CATCH
}
