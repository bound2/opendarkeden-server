////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionActivateMazeEnter.cpp
// Written By  :
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "ActionActivateMazeEnter.h"

#include "EffectRefiniumTicket.h"
#include "GCMoveOK.h"
#include "GCNPCResponse.h"
#include "GCSystemMessage.h"
#include "GCUpdateInfo.h"
#include "GamePlayer.h"
#include "IncomingPlayerManager.h"
#include "LogClient.h"
#include "PCSlayerInfo2.h"
#include "PCVampireInfo2.h"
#include "PacketUtil.h"
#include "PaySystem.h"
#include "Properties.h"
#include "Slayer.h"
#include "StringPool.h"
#include "Vampire.h"
#include "WeatherManager.h"
#include "Zone.h"
#include "ZoneGroup.h"
#include "ZoneGroupManager.h"
#include "ZoneInfo.h"
#include "ZoneInfoManager.h"
#include "ZonePlayerManager.h"
#include "ZoneUtil.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ActionActivateMazeEnter::read(PropertyBuffer& pb)

{
    __BEGIN_TRY

    try {
        m_ZoneID = pb.getPropertyInt("ZoneID");
        m_X = pb.getPropertyInt("X");
        m_Y = pb.getPropertyInt("Y");
    } catch (NoSuchElementException& nsee) {
        throw Error(nsee.toString());
    }

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
// Execute the action.
////////////////////////////////////////////////////////////////////////////////
void ActionActivateMazeEnter::execute(Creature* pNPC, Creature* pCreature)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    Assert(pCreature != NULL);
    Assert(pCreature->isPC());

    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pCreature->getPlayer());

    bool bTransport = true;

    if (bTransport) {
        EffectRefiniumTicket* pEffect = new EffectRefiniumTicket(pCreature);
        pEffect->setExit(rand() % 6);

        pCreature->setFlag(pEffect->getEffectClass());
        pCreature->addEffect(pEffect);
        transportCreature(pCreature, m_ZoneID, m_X, m_Y, true);
    } else {
        GCNPCResponse response;
        response.setCode(NPC_RESPONSE_QUIT_DIALOGUE);
        pGamePlayer->sendPacket(&response);
    }

    __END_DEBUG
    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// get debug string
////////////////////////////////////////////////////////////////////////////////
string ActionActivateMazeEnter::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ActionActivateMazeEnter(" << "ZoneID:" << (int)m_ZoneID << ",X:" << (int)m_X << ",Y:" << (int)m_Y << ")";
    return msg.toString();

    __END_CATCH
}
