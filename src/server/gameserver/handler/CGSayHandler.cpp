//////////////////////////////////////////////////////////////////////////////
// Filename    : CGSayHandler.cc
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGSay.h"

#ifdef __GAME_SERVER__
#include "GCSay.h"
#include "GamePlayer.h"
#include "LogNameManager.h"
#include "Vampire.h"
#include "Zone.h"
#include "ZoneUtil.h"
#include "gm/CommandGating.h"
#include "gm/GMCommands.h"
#include "skill/Sniping.h"
#endif


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGSayHandler::execute(CGSay* pPacket, Player* pPlayer) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    try {
        GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);

        // �������� Ŭ���̾�Ʈ�� �����ϹǷ� GC- ��Ŷ�� ����ؾ� �Ѵ�.
        GCSay gcSay;

        Creature* pCreature = pGamePlayer->getCreature();

        // ũ��ó �̸��� �޽����� ��Ŷ�� �����Ѵ�.
        gcSay.setObjectID(pCreature->getObjectID());
        gcSay.setColor(pPacket->getColor());

        string msg = pPacket->getMessage();

        gcSay.setMessage(msg);

        bool Success = true;

        size_t i = msg.find_first_of('*', 0);

        if (i == 0) {
            Success = false;

            const de::gm::CommandContext command{pCreature, pGamePlayer, msg, static_cast<int>(i),
                                                 de::gm::permissionOf(pCreature)};

            // A broadcast prefix answers the message on its own; anything
            // else is looked up among the operator commands.
            if (!de::gm::broadcastCommands().dispatch(command))
                de::gm::operatorCommands().dispatch(command);
        }

        // ä�� �α׸� �����. by sigi. 2002.10.30
        if (LogNameManager::getInstance().isExist(pCreature->getName())) {
            filelog("chatLog.txt", "[Say] %s> %s", pCreature->getName().c_str(), msg.c_str());
        }

        // invisibility���¸� Ǭ��.
        if (pCreature->isFlag(Effect::EFFECT_CLASS_INVISIBILITY)) {
            Zone* pZone = pCreature->getZone();
            Assert(pZone);
            addVisibleCreature(pZone, pCreature, true);
        }

        if (pCreature->isFlag(Effect::EFFECT_CLASS_SNIPING_MODE)) {
            g_Sniping.checkRevealRatio(pCreature, 20, 10);
        }

        bool isVampire = false;
        // �����̾ ����, �����̸� ���Ҽ�����.
        if (pCreature->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);

            if (pVampire->isFlag(Effect::EFFECT_CLASS_TRANSFORM_TO_WOLF) ||
                pVampire->isFlag(Effect::EFFECT_CLASS_TRANSFORM_TO_BAT) ||
                pVampire->isFlag(Effect::EFFECT_CLASS_TRANSFORM_TO_WERWOLF)) {
                Success = false;
            }

            isVampire = true;
        }

        if (pGamePlayer->isPenaltyFlag(PENALTY_TYPE_MUTE)) {
            Success = false;
        }

        if (pCreature->isFlag(Effect::EFFECT_CLASS_MUTE)) {
            Success = false;
        }

        if (Success) {
            // �ֺ� PC�鿡�� ��ε�ĳ��Ʈ�Ѵ�.
            //		pCreature->getZone()->broadcastSayPacket(pCreature->getX() , pCreature->getY() , &gcSay , pCreature,
            // isVampire);
            pCreature->getZone()->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcSay, pCreature);
        }

    } catch (Throwable& t) {
        // cout << t.toString() << endl;
    }

#endif

    __END_DEBUG_EX __END_CATCH
}
