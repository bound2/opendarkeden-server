////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionActivatePortal.cpp
// Written By  :
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "ActionActivatePortal.h"

#include "GCMoveOK.h"
#include "GCNPCResponse.h"
#include "GCSystemMessage.h"
#include "GCUpdateInfo.h"
#include "GQuestManager.h"
#include "GamePlayer.h"
#include "IncomingPlayerManager.h"
#include "LogClient.h"
#include "Ousters.h"
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
void ActionActivatePortal::read(PropertyBuffer& pb)

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
// 액션을 실행한다.
////////////////////////////////////////////////////////////////////////////////
void ActionActivatePortal::execute(Creature* pNPC, Creature* pCreature)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    Assert(pCreature != NULL);
    Assert(pCreature->isPC());

    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pCreature->getPlayer());
    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);

    bool bTransport = true;

    if (bTransport) {
        if (m_ZoneID == 1410) {
            if (pCreature->isFlag(Effect::EFFECT_CLASS_CAN_ENTER_GDR_LAIR)) {
                Effect* pEffect = pCreature->findEffect(Effect::EFFECT_CLASS_CAN_ENTER_GDR_LAIR);
                if (pEffect != NULL)
                    pEffect->setDeadline(0);
            }

            if (pCreature->isSlayer()) {
                Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
                Assert(pSlayer != NULL);

                // 오토바이를 타고 있으면 오토바이에서 내린다.
                if (pSlayer->hasRideMotorcycle()) {
                    pSlayer->getOffMotorcycle();
                }
            }

            if (pCreature->isOusters()) {
                Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);
                Assert(pOusters != NULL);

                // 실프 타고 있으면 내려준다
                if (pOusters->isFlag(Effect::EFFECT_CLASS_SUMMON_SYLPH)) {
                    Effect* pEffect = pOusters->findEffect(Effect::EFFECT_CLASS_SUMMON_SYLPH);
                    if (pEffect != NULL)
                        pEffect->setDeadline(0);
                }
            }

            if (pCreature->isVampire()) {
                Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);
                Assert(pVampire != NULL);

                if (pVampire->isFlag(Effect::EFFECT_CLASS_TRANSFORM_TO_BAT)) {
                    addUntransformCreature(pVampire->getZone(), pVampire, true);
                }
            }
        }

        if (pNPC != NULL) {
            pPC->getGQuestManager()->illegalWarp();
        }

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
string ActionActivatePortal::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ActionActivatePortal(" << "ZoneID:" << (int)m_ZoneID << ",X:" << (int)m_X << ",Y:" << (int)m_Y << ")";
    return msg.toString();

    __END_CATCH
}
