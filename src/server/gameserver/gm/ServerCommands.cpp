//////////////////////////////////////////////////////////////////////////////
// Filename    : ServerCommands.cpp
// Description : GM commands that act on the server as a whole: the save and shutdown schedules,
//               the tunable variables and reloadable tables, the war switch, the world relay
//               and the reports the client sends.
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include <list>

#include "BillingPlayerManager.h"
#include "CastleInfoManager.h"
#include "ClientManager.h"
#include "CombatInfoManager.h"
#include "CreatureUtil.h"
#include "DatabaseError.h"
#include "EffectShutDown.h"
#include "EventReloadInfo.h"
#include "GCNoticeEvent.h"
#include "GCSystemMessage.h"
#include "GGCommand.h"
#include "GamePlayer.h"
#include "GameServerInfoManager.h"
#include "Guild.h"
#include "ItemFactoryManager.h"
#include "ItemUtil.h"
#include "LoginServerManager.h"
#include "MasterLairManager.h"
#include "Monster.h"
#include "NPC.h"
#include "OptionInfo.h"
#include "PCFinder.h"
#include "PacketUtil.h"
#include "Properties.h"
#include "Relic.h"
#include "RelicUtil.h"
#include "Slayer.h"
#include "StringPool.h"
#include "VSDateTime.h"
#include "Vampire.h"
#include "VariableManager.h"
#include "Zone.h"
#include "ZoneGroupManager.h"
#include "ZoneInfoManager.h"
#include "ZonePlayerManager.h"
#include "ZoneUtil.h"
#include "gm/GMCommands.h"
#include "repository/SessionRepository.h"
#include "repository/ZoneInfoRepository.h"

namespace de::gm {

void opcombat(GamePlayer* pGamePlayer, string msg, int i) {
    __BEGIN_TRY

    if (pGamePlayer == NULL)
        return;

    Creature* pCreature = pGamePlayer->getCreature();
    if (pCreature == NULL)
        return;

    filelog("change.txt", "[%s] %s", pCreature->getName().c_str(), msg.c_str());

    //	Creature* pCreature = pGamePlayer->getCreature();
    GCSystemMessage gcSystemMessage;

    // �ӽ÷� ���Ƶа�
    {
        gcSystemMessage.setMessage(g_pStringPool->getString(STRID_DO_NOT_SUPPORT_OLD_WAR));
        pGamePlayer->sendPacket(&gcSystemMessage);

        return;
    }

    StringStream message;

    size_t j = msg.find_first_of(' ', i + 1);
    size_t z = msg.find_first_of(' ', j + 1);

    string set_type = msg.substr(j + 1, z - j - 1);

    if (set_type == "start") {
        if (g_pCombatInfoManager->isCombat() || g_pCombatInfoManager->isSlayerBonus() ||
            g_pCombatInfoManager->isVampireBonus()) {
            cout << "�̹� �������Դϴ�" << endl;
            //			message << "�̹� �������Դϴ�";
            gcSystemMessage.setMessage(g_pStringPool->getString(STRID_COMBAT_ALEADY_START));

            pGamePlayer->sendPacket(&gcSystemMessage);
        } else {
            cout << "������ ���۵Ǿ����ϴ�" << endl;
            //			message << "������ ���۵Ǿ����ϴ�";
            gcSystemMessage.setMessage(g_pStringPool->getString(STRID_COMBAT_START));

            g_pZoneGroupManager->broadcast(&gcSystemMessage);

            // by sigi. 2002.7.5
            for (int i = 0; i < maxRelic; i++) {
                const RelicInfo* pRelicInfo = dynamic_cast<RelicInfo*>(g_pRelicInfoManager->getItemInfo(i));

                int ZoneNum = pRelicInfo->zoneID;

                ZoneInfo* pZoneInfo = NULL;

                try {
                    pZoneInfo = g_pZoneInfoManager->getZoneInfo(ZoneNum);
                } catch (NoSuchElementException&) {
                    throw Error("No zone info for the relic zone.");
                }

                ZoneGroup* pZoneGroup = NULL;

                try {
                    pZoneGroup = g_pZoneGroupManager->getZoneGroup(pZoneInfo->getZoneGroupID());
                } catch (NoSuchElementException&) {
                    throw Error("No zone group for the relic zone.");
                }

                Zone* pZone = pZoneGroup->getZone(ZoneNum);

                pZone->addRelicItem(i);
            }

            g_pCombatInfoManager->setCombat(true);
        }
    } else if (set_type == "end") {
        if (!g_pCombatInfoManager->isCombat() &&
            (g_pCombatInfoManager->isSlayerBonus() || g_pCombatInfoManager->isVampireBonus()))

        {
            cout << "������ �����մϴ�." << endl;
            gcSystemMessage.setMessage(g_pStringPool->getString(STRID_COMBAT_END));
            g_pZoneGroupManager->broadcast(&gcSystemMessage);

            // by sigi. 2002.7.5
            for (int i = 0; i < maxRelic; i++) {
                const RelicInfo* pRelicInfo = dynamic_cast<RelicInfo*>(g_pRelicInfoManager->getItemInfo(i));

                int ZoneNum = pRelicInfo->zoneID;

                ZoneInfo* pZoneInfo = NULL;

                try {
                    pZoneInfo = g_pZoneInfoManager->getZoneInfo(ZoneNum);
                } catch (NoSuchElementException&) {
                    throw Error("No zone info for the relic zone.");
                }

                ZoneGroup* pZoneGroup = NULL;

                try {
                    pZoneGroup = g_pZoneGroupManager->getZoneGroup(pZoneInfo->getZoneGroupID());
                } catch (NoSuchElementException&) {
                    throw Error("No zone group for the relic zone.");
                }

                Zone* pZone = pZoneGroup->getZone(ZoneNum);

                pZone->deleteRelicItem();
                g_pCombatInfoManager->setRelicOwner(i, CombatInfoManager::RELIC_OWNER_NULL);
            }

            g_pCombatInfoManager->computeModify();
            g_pCombatInfoManager->setCombat(false);
        } else {
            cout << "�������� �ƴϰų� ������ �� �����ϴ�." << endl;
            gcSystemMessage.setMessage(g_pStringPool->getString(STRID_CANNOT_END_COMBAT));

            pGamePlayer->sendPacket(&gcSystemMessage);
        }
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void opset(GamePlayer* pGamePlayer, string msg, int i) {
    __BEGIN_TRY

    // [!!!] GGCommand�� ���ؼ� ���Ⱑ ó���ɶ�����
    // pGamePlayer�� NULL�� �� �����Ƿ�.. �� üũ�� �ؾ��Ѵ�! by sigi.2002.12.23

    GCSystemMessage gcSystemMessage;
    StringStream message;

    size_t j = msg.find_first_of(' ', i + 1);
    size_t z = msg.find_first_of(' ', j + 1);

    string set_type = msg.substr(j + 1, z - j - 1);
    string set_value = msg.substr(z + 1);

    if (set_value.length() < 1)
        return;

    if (pGamePlayer != NULL) {
        Creature* pCreature = pGamePlayer->getCreature();
        filelog("change.txt", "%s , %s , %s", pCreature->getName().c_str(), set_type.c_str(), set_value.c_str());
    }


    if (set_type == "star") {
        g_pVariableManager->setStar(atoi(set_value.c_str()));
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_STAR_RATIO), set_value.c_str());
        //	    message << "�� ���� Ȯ���� 1/" << set_value << "�� �����Ǿ����ϴ�";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "event_activate") {
        if (set_value == "start") {
            g_pVariableManager->setEventActivate(1);
            //	    	message << "�̺�Ʈ�� ���۵Ǿ����ϴ�";
            gcSystemMessage.setMessage(g_pStringPool->getString(STRID_EVENT_START));
            filelog("change.txt", "[%s]%s",
                    (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                    gcSystemMessage.toString().c_str());
        } else {
            g_pVariableManager->setEventActivate(0);
            //	    	message << "�̺�Ʈ�� �����Ǿ����ϴ�";
            gcSystemMessage.setMessage(g_pStringPool->getString(STRID_EVENT_END));
            filelog("change.txt", "[%s]%s",
                    (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                    gcSystemMessage.toString().c_str());
        }
    } else if (set_type == "event_ratio") {
        g_pVariableManager->setEventRatio(atoi(set_value.c_str()));
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_EVENT_MONSTER_RATIO), set_value.c_str());
        //	    message << "�̺�Ʈ ���� ���� Ȯ���� ���ؼ� 1/" << set_value << "�� �����Ǿ����ϴ�";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "exp_ratio") {
        int value = atoi(set_value.c_str());
        if (value < 100)
            return;

        g_pVariableManager->setExpRatio(value);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_EXP_RATIO), set_value.c_str());
        //	    message << "����ġ ȹ�����" << set_value << "%�� �����Ǿ����ϴ�";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "item_prob_ratio") {
        int value = atoi(set_value.c_str());
        if (value < value)
            return;

        g_pVariableManager->setItemProbRatio(value);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_ITEM_PROBE_RATIO), set_value.c_str());
        //	    message << "������ ȹ����� " << set_value << "%�� �����Ǿ����ϴ�";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "combat_bonus_time") {
        int bonusTime = atoi(set_value.c_str());
        if (bonusTime < 1 || bonusTime > 14400)
            return;

        g_pVariableManager->setCombatBonusTime(bonusTime);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_COMBAT_BONUS_TIME), set_value.c_str());
        //	    message << "���� �¸� �����ð��� " << set_value << "������ �����Ǿ����ϴ�";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "combat_bonus_slayer_hp_ratio") {
        int bonus = atoi(set_value.c_str());
        if (bonus < 0 || bonus > 100)
            return;

        g_pVariableManager->setCombatSlayerHPBonusRatio(bonus);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_COMBAT_SLAYER_BONUS_HP), set_value.c_str());
        //	    message << "�����̾� ���� HP���ʽ��� +" << set_value << "% �� �����Ǿ����ϴ�";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "combat_bonus_vampire_hp_ratio") {
        int bonus = atoi(set_value.c_str());
        if (bonus < 0 || bonus > 100)
            return;

        g_pVariableManager->setCombatVampireHPBonusRatio(bonus);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_COMBAT_VAMPIRE_BONUS_HP), set_value.c_str());
        //	    message << "�����̾� ���� HP���ʽ��� +" << set_value << "% �� �����Ǿ����ϴ�";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "combat_bonus_slayer_damage") {
        int bonus = atoi(set_value.c_str());
        if (bonus < 0 || bonus > 20)
            return;

        g_pVariableManager->setCombatSlayerDamageBonus(bonus);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_COMBAT_SLAYER_BONUS_DAMAGE), set_value.c_str());
        //	    message << "�����̾� ���� ���������ʽ��� +" << set_value << " �� �����Ǿ����ϴ�";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "combat_bonus_vampire_damage") {
        int bonus = atoi(set_value.c_str());
        if (bonus < 0 || bonus > 20)
            return;

        g_pVariableManager->setCombatVampireDamageBonus(bonus);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_COMBAT_VAMPIRE_BONUS_DAMAGE), set_value.c_str());
        //	    message << "�����̾� ���� ���������ʽ��� +" << set_value << " �� �����Ǿ����ϴ�";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "premium_exp_bonus_percent") {
        int bonus = atoi(set_value.c_str());
        if (bonus < 100 || bonus > 1000)
            return;

        g_pVariableManager->setPremiumExpBonusPercent(bonus);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_PREMIUM_EXP_RATIO), set_value.c_str());
        //	    message << "�����̾� ������� ����ġ ���ʽ��� " << set_value << "% �� �����Ǿ����ϴ�.";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "premium_item_probe_percent") {
        int bonus = atoi(set_value.c_str());
        if (bonus < 100 || bonus > 1000)
            return;

        g_pVariableManager->setPremiumItemProbePercent(bonus);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_PREMIUM_ITEM_PROBE_RATIO), set_value.c_str());
        //	    message << "�����̾� ���� ������ ���� Ȯ���� " << set_value << "% �� �����Ǿ����ϴ�.";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "zone_group_balancing_minute") {
        int minute = atoi(set_value.c_str());
        if (minute <= 0 || minute > 30 * 24 * 60)
            return;

        g_pVariableManager->setZoneGroupBalancingMinute(minute);
        g_pClientManager->setBalanceZoneGroup(minute);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_ZONE_GROUP_BALANCING_TIME), set_value.c_str());
        //	    message << "ZoneGroupBalancing �ֱⰡ " << set_value << "������ �����Ǿ����ϴ�.";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "gamble_item_type_ratio") {
        int ratio = atoi(set_value.c_str());
        if (ratio <= 10 || ratio > 1000)
            return;

        g_pVariableManager->setGambleItemTypeRatio(ratio);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_GAMBLE_ITEM_TYPE_RATIO), set_value.c_str());
        //	    message << "���� ItemTypeȮ���� " << set_value << "%�� �����Ǿ����ϴ�.";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "gamble_item_option_ratio") {
        int ratio = atoi(set_value.c_str());
        if (ratio <= 10 || ratio > 1000)
            return;

        g_pVariableManager->setGambleItemOptionRatio(ratio);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_GAMBLE_ITEM_OPTION_RATIO), set_value.c_str());
        //	    message << "���� ItemOptionȮ���� " << set_value << "%�� �����Ǿ����ϴ�.";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "summon_motorcycle") {
        if (set_value == "on") {
            g_pVariableManager->setSummonMotorcycle(true);
            char msg[100];
            sprintf(msg, g_pStringPool->c_str(STRID_SET_SUMMON_MOTORCYCLE), "ON");
            //			message << "������� ��ȯ ����� ON �Ǿ����ϴ�.";
            gcSystemMessage.setMessage(msg);
            filelog("change.txt", "[%s]%s",
                    (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                    gcSystemMessage.toString().c_str());
        } else if (set_value == "off") {
            g_pVariableManager->setSummonMotorcycle(false);
            char msg[100];
            sprintf(msg, g_pStringPool->c_str(STRID_SET_SUMMON_MOTORCYCLE), "OFF");
            //			message << "������� ��ȯ ����� OFF �Ǿ����ϴ�.";
            gcSystemMessage.setMessage(msg);
            filelog("change.txt", "[%s]%s",
                    (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                    gcSystemMessage.toString().c_str());
        }
    } else if (set_type == "enemy_limit_time") {
        int enemy_limit_time = atoi(set_value.c_str());
        if (enemy_limit_time <= 180 || enemy_limit_time >= 3600)
            return;

        g_pVariableManager->setEnemyLimitTime(enemy_limit_time);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_MONSTER_FORGET_TIME), set_value.c_str());
        //	    message << "������ ���� �� �νĽð���" << set_value << "�ʷ� �����Ǿ����ϴ�.";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "rare_item_ratio") {
        int ratio = atoi(set_value.c_str());
        if (ratio < 0 || ratio > 100000)
            return;

        g_pVariableManager->setRareItemRatio(ratio);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_RARE_ITEM_RATIO), set_value.c_str());
        //		message << "���� ������ ���� Ȯ���� " << set_value << "%�� �����Ǿ����ϴ�.";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "unique_item_ratio") {
        int ratio = atoi(set_value.c_str());
        if (ratio < 0 || ratio > 10000)
            return;

        g_pVariableManager->setUniqueItemRatio(ratio);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_UNIQUE_ITEM_RATIO), set_value.c_str());
        //		message << "����ũ ������ ���� Ȯ���� " << set_value << "/10000 ���� �����Ǿ����ϴ�.";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "active_master_lair") {
        MasterLairManager* pMasterLairManager = NULL;

        if (pGamePlayer != NULL) {
            Creature* pCreature = pGamePlayer->getCreature();
            Zone* pZone = pCreature->getZone();
            pMasterLairManager = pZone->getMasterLairManager();
        }

        if (set_value == "on") {
            g_pVariableManager->setActiveMasterLair(true);
            char msg[100];
            sprintf(msg, g_pStringPool->c_str(STRID_SET_MASTER_LAIR_ACTIVATE), "ON");
            //			message << "������ ���� �̺�Ʈ�� ON �Ǿ����ϴ�.";
            gcSystemMessage.setMessage(msg);
            filelog("change.txt", "[%s]%s",
                    (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                    gcSystemMessage.toString().c_str());

            if (pMasterLairManager != NULL) {
                pMasterLairManager->startEvent();
            }
        } else if (set_value == "off") {
            g_pVariableManager->setActiveMasterLair(false);
            char msg[100];
            sprintf(msg, g_pStringPool->c_str(STRID_SET_MASTER_LAIR_ACTIVATE), "OFF");
            //			message << "������ ���� �̺�Ʈ�� OFF �Ǿ����ϴ�.";
            gcSystemMessage.setMessage(msg);
            filelog("change.txt", "[%s]%s",
                    (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                    gcSystemMessage.toString().c_str());

            if (pMasterLairManager != NULL) {
                pMasterLairManager->stopEvent();
            }
        }
    } else if (set_type == "retry_master_lair") {
        if (set_value == "on") {
            g_pVariableManager->setRetryMasterLair(true);
            char msg[100];
            sprintf(msg, g_pStringPool->c_str(STRID_SET_RETRY_MASTER_LAIR), "ON");
            //			message << "������ ��� �װ� �ٽ� ���Ⱑ ON
            // �Ǿ����ϴ�.";
            gcSystemMessage.setMessage(msg);
            filelog("change.txt", "[%s]%s",
                    (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                    gcSystemMessage.toString().c_str());
        } else if (set_value == "off") {
            g_pVariableManager->setRetryMasterLair(false);
            char msg[100];
            sprintf(msg, g_pStringPool->c_str(STRID_SET_RETRY_MASTER_LAIR), "OFF");
            //			message << "������ ��� �װ� �ٽ� ���Ⱑ OFF
            // �Ǿ����ϴ�.";
            gcSystemMessage.setMessage(msg);
            filelog("change.txt", "[%s]%s",
                    (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                    gcSystemMessage.toString().c_str());
        }
    }

    else if (set_type == "harvest_festival_item_ratio") {
        int ratio = atoi(set_value.c_str());
        if (ratio < 0 || ratio > 10000)
            return;

        g_pVariableManager->setHarvestFestivalItemRatio(ratio);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_HARVEST_FESTIVAL_ITEM_RATIO), set_value.c_str());
        //		message << "�߼� ���� ������ ���� Ȯ���� 1/" << ratio << "��
        // �Ǿ����ϴ�.";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "master_blood_drain_start_hp") {
        int percent = atoi(set_value.c_str());
        if (percent < 0 || percent > 100)
            return;

        g_pVariableManager->setMasterBloodDrainStartHP(percent);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_MASTER_BLOOD_DRAIN_START_HP), set_value.c_str());
        //		message << "�������� ���� ���� HP�� " << percent << "%�� �Ǿ����ϴ�.";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "master_blood_drain_start_bd") {
        int percent = atoi(set_value.c_str());
        if (percent < 0 || percent > 100)
            return;

        g_pVariableManager->setMasterBloodDrainStartBD(percent);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_MASTER_BLOOD_DRAIN_START_BD), set_value.c_str());
        //		message << "�������� ���� ���� ���� ���� Ȯ���� " << percent << "%�� �Ǿ����ϴ�.";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "master_blood_drain_end_hp") {
        int percent = atoi(set_value.c_str());
        if (percent < 0 || percent > 100)
            return;

        g_pVariableManager->setMasterBloodDrainEndHP(percent);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_MASTER_BLOOD_DRAIN_END_HP), set_value.c_str());
        //		message << "�������� ���� ��(?) HP�� " << percent << "%�� �Ǿ����ϴ�.";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "master_blood_drain_end_bd") {
        int percent = atoi(set_value.c_str());
        if (percent < 0 || percent > 100)
            return;

        g_pVariableManager->setMasterBloodDrainEndBD(percent);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_MASTER_BLOOD_DRAIN_END_BD), set_value.c_str());
        //		message << "�������� ���� ��(?) ������ ���� Ȯ���� " << percent << "%�� �Ǿ����ϴ�.";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "chief_monster") {
        if (set_value == "on") {
            g_pVariableManager->setActiveChiefMonster(true);
            char msg[100];
            sprintf(msg, g_pStringPool->c_str(STRID_SET_CHIEF_MONSTER), "ON");
            //			message << "ġ�� ���� ����� ON �Ǿ����ϴ�.";
            gcSystemMessage.setMessage(msg);
            filelog("change.txt", "[%s]%s",
                    (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                    gcSystemMessage.toString().c_str());
        } else if (set_value == "off") {
            g_pVariableManager->setActiveChiefMonster(false);
            char msg[100];
            sprintf(msg, g_pStringPool->c_str(STRID_SET_CHIEF_MONSTER), "OFF");
            //			message << "ġ�� ���� ����� OFF �Ǿ����ϴ�.";
            gcSystemMessage.setMessage(msg);
            filelog("change.txt", "[%s]%s",
                    (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                    gcSystemMessage.toString().c_str());
        }
    } else if (set_type == "chief_monster_rare_item_percent") {
        int ratio = atoi(set_value.c_str());
        if (ratio < 0 || ratio > 100)
            return;

        g_pVariableManager->setChiefMonsterRareItemPercent(ratio);
        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_SET_CHIEF_MONSTER_RARE_ITEM_RATIO), set_value.c_str());
        //		message << "ġ�� ���� ���� ������ ���� Ȯ���� " << set_value << "%�� �����Ǿ����ϴ�.";
        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (set_type == "newbie_transport_to_guild") {
        if (set_value == "on") {
            g_pVariableManager->setNewbieTransportToGuild(true);
            char msg[100];
            sprintf(msg, g_pStringPool->c_str(STRID_SET_NEWBIE_TRANSPORT_TO_GUILD), "ON");
            //			message << "�ɷ�ġ 40¥��, ���� �̵���Ű�� ����� ON �Ǿ����ϴ�.";
            gcSystemMessage.setMessage(msg);
            filelog("change.txt", "[%s]%s",
                    (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                    gcSystemMessage.toString().c_str());
        } else if (set_value == "off") {
            g_pVariableManager->setNewbieTransportToGuild(false);
            char msg[100];
            sprintf(msg, g_pStringPool->c_str(STRID_SET_NEWBIE_TRANSPORT_TO_GUILD), "OFF");
            //			message << "�ɷ�ġ 40¥��, ���� �̵���Ű�� ����� OFF �Ǿ����ϴ�.";
            gcSystemMessage.setMessage(msg);
            filelog("change.txt", "[%s]%s",
                    (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                    gcSystemMessage.toString().c_str());
        }
    } else if (set_type == "xmas_2002") {
        size_t l = set_value.find_first_of(' ', 0);
        size_t m = set_value.find_first_of(' ', l + 1);

        string value_firecracker = set_value.substr(0, l);
        string value_treepart = set_value.substr(l + 1, m - l - 1);
        string value_giftbox = set_value.substr(m + 1);

        g_pVariableManager->setVariable(CHRISTMAS_FIRE_CRACKER_RATIO, value_firecracker);
        const string& msg1 = g_pVariableManager->toString(CHRISTMAS_FIRE_CRACKER_RATIO);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
        gcSystemMessage.setMessage(msg1);

        if (pGamePlayer != NULL)
            pGamePlayer->sendPacket(&gcSystemMessage);

        g_pVariableManager->setVariable(CHRISTMAS_TREE_PART_RATIO, value_treepart);
        const string& msg2 = g_pVariableManager->toString(CHRISTMAS_TREE_PART_RATIO);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
        gcSystemMessage.setMessage(msg2);

        if (pGamePlayer != NULL)
            pGamePlayer->sendPacket(&gcSystemMessage);

        g_pVariableManager->setVariable(CHRISTMAS_GIFT_BOX_RATIO, value_giftbox);
        const string& msg3 = g_pVariableManager->toString(CHRISTMAS_GIFT_BOX_RATIO);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());

        gcSystemMessage.setMessage(msg3);

    } else if (set_type == "ItemTaxRatio") {
        if (pGamePlayer != NULL) {
            Creature* pCreature = pGamePlayer->getCreature();
            Zone* pZone = pCreature->getZone();
            if (set_value.size() != 0) {
                int Tax = atoi(set_value.c_str());
                g_pCastleInfoManager->setItemTaxRatio(pZone, Tax);
            } else {
                gcSystemMessage.setMessage(g_pStringPool->getString(STRID_WRONG_ITEM_TAX_RATIO));
            }
        }
    }

    else {
        // by sigi. 2002.11.19
        VariableType vt = g_pVariableManager->getVariableType(set_type);

        if (vt != VARIABLE_MAX) {
            g_pVariableManager->setVariable(vt, set_value);
            const string& msg = g_pVariableManager->toString(vt);
            gcSystemMessage.setMessage(msg);
            filelog("change.txt", "[%s]%s",
                    (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                    gcSystemMessage.toString().c_str());

            if (vt == PREMIUM_HALF_EVENT) {
                GCNoticeEvent gcNoticeEvent;

                if (g_pVariableManager->getVariable(vt)) {
                    gcNoticeEvent.setCode(NOTICE_EVENT_PREMIUM_HALF_START);
                } else {
                    gcNoticeEvent.setCode(NOTICE_EVENT_PREMIUM_HALF_END);
                }

                // ���������� �Ѹ���.
                getZoneByZoneID(61)->broadcastPacket(&gcNoticeEvent); // ��γ��ϵ�
                getZoneByZoneID(64)->broadcastPacket(&gcNoticeEvent); // ��γ�����
                getZoneByZoneID(1007)->broadcastPacket(
                    &gcNoticeEvent); // �󼾼�����(���丮����)
            } else if (vt == TODAY_IS_HOLYDAY) {
                GCNoticeEvent gcNoticeEvent;
                gcNoticeEvent.setCode(NOTICE_EVENT_HOLYDAY);
                gcNoticeEvent.setParameter(g_pVariableManager->getVariable(vt));

                g_pZoneGroupManager->broadcast(&gcNoticeEvent);
            } else if (vt == CROWN_PRICE) {
                GCNoticeEvent gcNoticeEvent;
                gcNoticeEvent.setCode(NOTICE_EVENT_CROWN_PRICE);
                gcNoticeEvent.setParameter(g_pVariableManager->getVariable(vt));

                g_pZoneGroupManager->broadcast(&gcNoticeEvent);
            }
        } else {
            gcSystemMessage.setMessage(g_pStringPool->getString(STRID_WRONG_VARIABLE_NAME));
        }
    }

    // �ڽſ��Ը� �ý��� �޽��� ������ (�����Ǿ�����)
    if (pGamePlayer != NULL) {
        pGamePlayer->sendPacket(&gcSystemMessage);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void opview(GamePlayer* pGamePlayer, string msg, int i) {
    __BEGIN_TRY

    if (pGamePlayer == NULL)
        return;

    Creature* pCreature = pGamePlayer->getCreature();
    GCSystemMessage gcSystemMessage;
    StringStream message;

    size_t j = msg.find_first_of(' ', i + 1);
    size_t z = msg.find_first_of(' ', j + 1);

    string set_type = msg.substr(j + 1, z - j - 1);

    cout << "modifier : " << pCreature->getName() << " set type : " << set_type << endl;

    if (set_type == "star") {
        message << "star : 1/" << g_pVariableManager->getStar();
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "evnet_activate") {
        if (g_pVariableManager->getEventActivate() == 1) {
            // message << ((const string &) ("�¼����ڽ�����...")) << endl;
        } else {
            // message << "�¼���ֹͣ" << endl;
        }
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "event_ratio") {
        message << "ʱ����ﱬ�� 1/" << g_pVariableManager->getEventRatio();
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "exp_ratio") {
        message << "����ֵ: " << g_pVariableManager->getExpRatio();
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "item_prob_ratio") {
        message << "��Ʒ����: " << g_pVariableManager->getItemProbRatio();
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "combat_bonus_time") {
        message << "ս��ʤ��ά��ʱ��: " << g_pVariableManager->getCombatBonusTime() << "��";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "combat_bonus_slayer_hp_ratio") {
        message << "����ս�� HP��������ֵ: +" << g_pVariableManager->getCombatSlayerHPBonusRatio() << "%";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "combat_bonus_vampire_hp_ratio") {
        message << "��Ѫ��ս�� HP��������ֵ: +" << g_pVariableManager->getCombatVampireHPBonusRatio() << "%";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "combat_bonus_slayer_damage") {
        message << "����ս�� Damage��������ֵ: +" << g_pVariableManager->getCombatSlayerDamageBonus();
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "combat_bonus_vampire_damage") {
        message << "��Ѫ��ս�� Damage��������ֵ: +" << g_pVariableManager->getCombatVampireDamageBonus();
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "premium_exp_bonus_percent") {
        message << "�����û�����ֵ����: " << g_pVariableManager->getPremiumExpBonusPercent() << "%";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "premium_item_probe_percent") {
        message << "���ѵ�ͼ��Ʒ����: " << g_pVariableManager->getPremiumItemProbePercent() << "%";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "zone_group_balancing_minute") {
        message << "ZoneGroupBalancingʱ��: " << g_pVariableManager->getZoneGroupBalancingMinute() << "��";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "zone_group_next_balancing_time") {
        const Timeval& tv = g_pClientManager->getBalanceZoneGroupTime();
        Timeval currentTime;
        getCurrentTime(currentTime);
        message << "�´�ZoneGroupBalancingʱ��: " << (tv.tv_sec - currentTime.tv_sec) / 60 << "�ֺ�";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "gamble_item_type_ratio") {
        message << "�Ĳ�ItemType����: " << g_pVariableManager->getGambleItemTypeRatio() << "%";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "gamble_item_option_ratio") {
        message << "�Ĳ�ItemOption����: " << g_pVariableManager->getGambleItemOptionRatio() << "%";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "summon_motorcycle") {
        message << "Ħ�г��ٻ����� : " << (g_pVariableManager->isSummonMotorcycle() ? "ON" : "OFF");
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "enemy_limit_time") {
        message << "������Чʱ��: " << g_pVariableManager->getEnemyLimitTime() << "��";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "rare_item_ratio") {
        message << "�߼���Ʒ����: " << g_pVariableManager->getRareItemRatio() << "%";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "unique_item_ratio") {
        message << "ϡ����Ʒ����: " << g_pVariableManager->getUniqueItemRatio() << "/10000";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "active_master_lair") {
        message << "�Ϲֻ : " << (g_pVariableManager->isActiveMasterLair() ? "ON" : "OFF");
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "retry_master_lair") {
        message << "���Ϲ�ɱ����,������ս : " << (g_pVariableManager->isRetryMasterLair() ? "ON" : "OFF");
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "harvest_festival_item_ratio") {
        message << "�����ף������: 1/" << g_pVariableManager->getHarvestFestivalItemRatio();
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "master_blood_drain_start_hp") {
        message << "��ʼ��Ѫʱ��HP: " << g_pVariableManager->getMasterBloodDrainStartHP() << "%";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "master_blood_drain_start_bd") {
        message << "��ʼ��Ѫʱ����Ѫ��: " << g_pVariableManager->getMasterBloodDrainStartBD() << "%";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "master_blood_drain_end_hp") {
        message << "������Ѫʱ(?)��HP: " << g_pVariableManager->getMasterBloodDrainEndHP() << "%";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "master_blood_drain_end_bd") {
        message << "������Ѫʱ(?)��Ѫ��: " << g_pVariableManager->getMasterBloodDrainEndBD() << "%";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "chief_monster") {
        message << "ˢ��BOSS���� : " << (g_pVariableManager->isActiveChiefMonster() ? "ON" : "OFF");
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "chief_monster_rare_item_percent") {
        message << "BOSS����߼���Ʒ����: " << g_pVariableManager->getChiefMonsterRareItemPercent()
                << "%";
        gcSystemMessage.setMessage(message.toString());
    } else if (set_type == "newbie_transport_to_guild") {
        message << "�ƶ�������ֵΪ40���л�:" << (g_pVariableManager->isNewbieTransportToGuild() ? "ON" : "OFF");
        gcSystemMessage.setMessage(message.toString());
    } else {
        // by sigi. 2002.11.19
        VariableType vt = g_pVariableManager->getVariableType(set_type);

        if (vt != VARIABLE_MAX) {
            const string& msg = g_pVariableManager->toString(vt);
            gcSystemMessage.setMessage(msg);
        } else {
            gcSystemMessage.setMessage(g_pStringPool->getString(STRID_WRONG_VARIABLE_NAME));
        }
    }

    // �ڽſ��Ը� �ý��� �޽��� ������ (�����Ǿ�����)
    pGamePlayer->sendPacket(&gcSystemMessage);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void opload(GamePlayer* pGamePlayer, string msg, int i) {
    __BEGIN_TRY

    GCSystemMessage gcSystemMessage;
    StringStream message;

    size_t j = msg.find_first_of(' ', i + 1);
    size_t z = msg.find_first_of(' ', j + 1);

    string load_type = msg.substr(j + 1, z - j - 1);
    string load_value = "0";

    if (z != string::npos)
        load_value = msg.substr(z + 1);

    if (load_type.length() < 1)
        return;

    if (pGamePlayer != NULL) {
        Creature* pCreature = pGamePlayer->getCreature();
        filelog("change.txt", "[LOAD] %s , %s , %s", pCreature->getName().c_str(), load_type.c_str(),
                load_value.c_str());
    }

    EventReloadInfo* pEvent = NULL;

    if (load_type == "master_lair_info") {
        pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::MASTER_LAIR_INFO);
        //		StringStream msg;
        //		msg << "������ ���� ������ load�մϴ�.";
        gcSystemMessage.setMessage(g_pStringPool->getString(STRID_LOAD_MASTER_LAIR_INFO));
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (load_type == "monster_info") {
        //	string MonsterName = load_value;
        string MonsterName = "all";
        int SpriteType = 0;

        // �ϴ� '�̸�'���� ���� SpriteType�� ã�ƺ���.
        //		int SpriteType = g_pMonsterInfoManager->getSpriteTypeByName( MonsterName );

        //		if (SpriteType==0)
        //		{
        // �̸����� ���ٸ�.. �� ��ü�� ����(SpriteType)�ΰ�?
        //			SpriteType = atoi( MonsterName.c_str() );
        //		}

        bool bExist = true;
        //		try
        //		{
        //			g_pMonsterInfoManager->getMonsterTypeBySprite(SpriteType);
        //		} catch (Throwable&) {
        //			bExist = false;
        //		}

        //		StringStream msg;
        char msg[100];

        if (bExist || MonsterName == "all") {
            // all�� ���� SpriteType�� 0�̴�.
            pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::MONSTER_INFO, SpriteType);

            if (SpriteType != 0) {
                sprintf(msg, g_pStringPool->c_str(STRID_LOAD_MONSTER_INFO), load_value.c_str());
                //				msg << "���� ����(" << load_value.c_str() << ")�� load�մϴ�.";
            } else {
                sprintf(msg, g_pStringPool->c_str(STRID_LOAD_ALL_MONSTER_INFO));
                //				msg << "��� ���� ������ load�մϴ�.";
            }
        } else {
            sprintf(msg, g_pStringPool->c_str(STRID_LOAD_WRONG_MONSTER_INFO), load_value.c_str());
            //			msg << "����(" << load_value.c_str() << ")�� �߸� �����Ǿ����ϴ�.";
        }

        gcSystemMessage.setMessage(msg);
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (load_type == "monster_ai") {
        pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::MONSTER_AI);
        //		StringStream msg;
        //		msg << "���� AI����(DirectiveSet)�� load�մϴ�.";
        gcSystemMessage.setMessage(g_pStringPool->getString(STRID_LOAD_DIRECTIVESET));
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (load_type == "zone_info") {
        pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::ZONE_INFO);
        //		StringStream msg;
        //		msg << "�� ������ load�մϴ�.";
        gcSystemMessage.setMessage(g_pStringPool->getString(STRID_LOAD_ZONE));
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (load_type == "zone") {
        int zoneID = atoi(load_value.c_str());

        pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::ZONE, zoneID);

        //		StringStream msg;
        //		msg << "���� load�մϴ�.";
        gcSystemMessage.setMessage(g_pStringPool->getString(STRID_LOAD_ZONE));
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (load_type == "log_user") {
        pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::LOG_USER_INFO);
        //		StringStream msg;
        //		msg << "LogUser ������ load�մϴ�.";
        gcSystemMessage.setMessage(g_pStringPool->getString(STRID_LOAD_LOG_USER));
        filelog("change.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()),
                gcSystemMessage.toString().c_str());
    } else if (load_type == "item_info") {
        Item::ItemClass ItemClass = g_pItemFactoryManager->getItemClassByName(load_value);

        // ItemClass�� MAX��� �̸����δ� �� ã�Ҵٴ� ���̴�.
        // �� ��쿡�� ������ Ŭ������ ���ڷ� �ٷ� ������ �ʾҴ��� �˻��ؾ� �Ѵ�.
        if (ItemClass == Item::ITEM_CLASS_MAX) {
            int temp = atoi(load_value.c_str());
            if (temp < 0 || temp >= Item::ITEM_CLASS_MAX) {
                return;
            } else {
                ItemClass = (Item::ItemClass)(temp);
            }
        }

        pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::ITEM_INFO, ItemClass);

        //		StringStream msg;
        //		msg << load_value << " Info�� load�մϴ�.";

        char msg[100];
        sprintf(msg, g_pStringPool->c_str(STRID_LOAD_ITEM_INFO), load_value.c_str());
        gcSystemMessage.setMessage(msg);
    } else if (load_type == "option_info") {
        pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::OPTION_INFO);
        //	StringStream msg;
        //	msg << "OptionInfo ������ load�մϴ�.";
        gcSystemMessage.setMessage(g_pStringPool->getString(STRID_LOAD_OPTION_INFO));
    } else if (load_type == "rank_bonus_info") {
        pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::RANK_BONUS_INFO);
        //		StringStream msg;
        //		msg << "RankBonusInfo ������ load�մϴ�.";
        gcSystemMessage.setMessage(g_pStringPool->getString(STRID_LOAD_RANK_BONUS_INFO));
    } else if (load_type == "string_pool") {
        pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::STRING_POOL);
        gcSystemMessage.setMessage("reload StringPool");
    } else if (load_type == "war_schedule_info") {
        ZoneID_t zoneID = atoi(load_value.c_str());

        // ���� �ȵ� ���� ���� Creature�� �ִ� ��
        if (zoneID == 0) {
            if (pGamePlayer != NULL) {
                Creature* pCreature = pGamePlayer->getCreature();
                Assert(pCreature != NULL);

                Zone* pZone = pCreature->getZone();
                Assert(pZone != NULL);

                zoneID = pZone->getZoneID();
            }
        }

        pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::WAR_SCHEDULE_INFO, zoneID);

        StringStream msg;
        msg << g_pStringPool->getString(STRID_LOAD_WAR_SCHEDULE_INFO);

        if (!g_pVariableManager->isWarActive()) {
            msg << g_pStringPool->getString(STRID_WAR_OFF);
        }

        gcSystemMessage.setMessage(msg.toString().c_str());

    } else if (load_type == "blood_bible_owner") {
        pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::BLOOD_BIBLE_OWNER);

        //		StringStream msg;
        //		msg << "BloodBibleOwner ������ load�մϴ�.";
        gcSystemMessage.setMessage(g_pStringPool->getString(STRID_LOAD_BLOOD_BIBLE_OWNER_INFO));
    } else if (load_type == "sweeper_owner") {
        int level = atoi(load_value.c_str());
        pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::SWEEPER_OWNER, level);

        //		StringStream msg;
        //		msg << "BloodBibleOwner ������ load�մϴ�.";
        gcSystemMessage.setMessage(g_pStringPool->getString(STRID_LOAD_BLOOD_BIBLE_OWNER_INFO));
    } else if (load_type == "race_war_pc_limit") {
        pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::RACE_WAR_PC_LIMIT);

        //		StringStream msg;
        //		msg << "RaceWarPCLimit ������ load�մϴ�.";
        gcSystemMessage.setMessage(g_pStringPool->getString(STRID_LOAD_RACE_WAR_PC_LIMIT_INFO));
    } else if (load_type == "npc") {
        ZoneID_t zoneID = (ZoneID_t)(atoi(load_value.c_str()));
        Zone* pZone = getZoneByZoneID(zoneID);
        if (pZone != NULL) {
            // �� �ܿ��� NPC �� ������� ���� ������ -_-a
            CastleInfo* pCastleInfo = g_pCastleInfoManager->getCastleInfo(zoneID);

            if (pCastleInfo != NULL)
                pZone->loadNPCs(pCastleInfo->getRace());
        }

    } else if (load_type == "goods_list_info") {
        pEvent = new EventReloadInfo(pGamePlayer, EventReloadInfo::GOODS_LIST_INFO);

        gcSystemMessage.setMessage(g_pStringPool->getString(STRID_LOAD_GOODS_LIST_INFO));
    }
    /*	else if ( load_type == "quest_info" )
        {
            pEvent = new EventReloadInfo( pGamePlayer, EventReloadInfo::QUEST_INFO );

            gcSystemMessage.setMessage( g_pStringPool->getString( STRID_LOAD_QUEST_INFO ) );
        }*/
    else {
        gcSystemMessage.setMessage(g_pStringPool->getString(STRID_WRONG_VARIABLE_NAME));
    }

    if (pEvent != NULL) {
        g_pClientManager->addEvent(pEvent);
    }

    // �ڽſ��Ը� �ý��� �޽��� ������ (�����Ǿ�����)
    if (pGamePlayer != NULL) {
        pGamePlayer->sendPacket(&gcSystemMessage);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void opsave(GamePlayer* pGamePlayer, string msg, int i) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

        GCSystemMessage gcSystemMessage;

    gcSystemMessage.setMessage(g_pStringPool->getString(STRID_SAVE_YOUR_DATA));

    // A SQL failure arrives as END_DB's DatabaseError, which __END_DEBUG_EX
    // below does not match, so it is swallowed here (the text is in
    // DBError.log). On an empty table the read answers false and the 0
    // below stands, so the loop runs zero times.
    int maxZoneGroupID = 0;
    try {
        defaultZoneInfoRepository().loadMaxZoneGroupID(maxZoneGroupID);
    } catch (const DatabaseError&) {
        return;
    }

    BYTE GroupCount = maxZoneGroupID + 1;
    for (int i = 1; i < GroupCount; i++) {
        ZoneGroup* pZoneGroup = NULL;

        try {
            pZoneGroup = g_pZoneGroupManager->getZoneGroup(i);
        } catch (NoSuchElementException&) {
            throw Error("Critical Error : ZoneInfoManager has no such zone group.");
        }

        ZonePlayerManager* pZonePlayerManager = pZoneGroup->getZonePlayerManager();

        pZonePlayerManager->broadcastPacket(&gcSystemMessage);
        pZonePlayerManager->save();
    }

    __END_DEBUG_EX __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void opwall(GamePlayer* pGamePlayer, string msg, int i) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

        size_t j = msg.find_first_of(' ', i + 1);

    GCSystemMessage gcSystemMessage;

    gcSystemMessage.setType(SYSTEM_MESSAGE_OPERATOR);
    gcSystemMessage.setMessage(msg.substr(j + 1, msg.size() - j - 1).c_str());

    if (pGamePlayer != NULL) {
        Creature* pCreature = pGamePlayer->getCreature();
        filelog("change.txt", "[Wall]%s, %s", pCreature->getName().c_str(), gcSystemMessage.getMessage().c_str());
    }

    g_pZoneGroupManager->broadcast(&gcSystemMessage);

    __END_DEBUG_EX __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void opshutdown(GamePlayer* pGamePlayer, string msg, int i) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

        // �ƹ� zone���� ���̸� �Ǵµ�..
        // multithread����.. lock�� �ɰ� effect�� �ٿ��� �Ѵ�.
        size_t j = msg.find_first_of(' ', i + 1);
    Turn_t dTime = atoi(msg.substr(j + 1, msg.size() - j - 1).c_str());

    if (pGamePlayer != NULL) {
        Creature* pCreature = pGamePlayer->getCreature();
        filelog("change.txt", "[ShutDown]%s, %d", pCreature->getName().c_str(), (int)dTime);
    }

    EffectShutDown* pEffectShutDown = new EffectShutDown();
    pEffectShutDown->setNextTime(100);
    pEffectShutDown->setDelay(100);
    pEffectShutDown->setDeadline(dTime);


    if (pGamePlayer != NULL) {
        Creature* pCreature = pGamePlayer->getCreature();

        Zone* pZone = pCreature->getZone();
        (pZone->getObjectRegistry()).registerObject(pEffectShutDown);

        pZone->addEffect(pEffectShutDown);
    } else {
        int ZoneNum = 1003;

        ZoneInfo* pZoneInfo = NULL;
        try {
            pZoneInfo = g_pZoneInfoManager->getZoneInfo(ZoneNum);
        } catch (NoSuchElementException&) {
            throw Error("Shutdown requested, but the zone has no zone info.");
        }

        ZoneGroup* pZoneGroup = NULL;

        try {
            pZoneGroup = g_pZoneGroupManager->getZoneGroup(pZoneInfo->getZoneGroupID());
        } catch (NoSuchElementException&) {
            throw Error("Shutdown requested, but the zone group is missing.");
        }

        Zone* pZone = pZoneGroup->getZone(ZoneNum);

        (pZone->getObjectRegistry()).registerObject(pEffectShutDown);

        pZone->addEffect_LOCKING(pEffectShutDown);
    }

    __END_DEBUG_EX __END_CATCH
}

void oplog(GamePlayer* pPlayer, string msg, int i) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

        size_t j = msg.find_first_of(' ', i + 1);
    size_t k = msg.find_first_of(' ', j + 1);
    uint sec = 0;
    string name;

    if (k == string::npos) {
        name = msg.substr(j + 1, msg.size() - j - 1).c_str();
    } else {
        name = trim(msg.substr(j + 1, k - j - 1));
        sec = (uint)atoi(msg.substr(k + 1, msg.size() - k - 1).c_str());
    }

    Creature* pTargetCreature = NULL;

    __ENTER_CRITICAL_SECTION((*g_pPCFinder))

    pTargetCreature = g_pPCFinder->getCreature_LOCKED(name);
    if (pTargetCreature == NULL) {
        return;
    }

    GamePlayer* pTargetGamePlayer = dynamic_cast<GamePlayer*>(pTargetCreature->getPlayer());

    if (pTargetGamePlayer == NULL) {
        return;
    }

    if (sec == 0)
        sec = 600;

    if (pTargetGamePlayer->startPacketLog(sec)) {
        char msg[100];
        sprintf(msg, "%s�ڼ�¼PacketLog(%u��)", name.c_str(), sec);

        GCSystemMessage gcMsg;
        gcMsg.setMessage(msg);
        pPlayer->sendPacket(&gcMsg);
    }

    __LEAVE_CRITICAL_SECTION((*g_pPCFinder))

    __END_DEBUG_EX __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void opworld(GamePlayer* pGamePlayer, string msg, int i, bool bSameWorldOnly) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

        size_t j = msg.find_first_of(' ', i + 1);
    string command = msg.substr(j + 1, msg.size() - j - 1).c_str();

    // packet
    GGCommand ggCommand;
    ggCommand.setCommand(command);


    // �� server�� ������.
    HashMapGameServerInfo** pGameServerInfos = g_pGameServerInfoManager->getGameServerInfos();


    static int myWorldID = g_pConfig->getPropertyInt("WorldID");
    static int myServerID = g_pConfig->getPropertyInt("ServerID");

    int maxWorldID = g_pGameServerInfoManager->getMaxWorldID();
    int maxServerGroupID = g_pGameServerInfoManager->getMaxServerGroupID();


    for (int worldID = 1; worldID < maxWorldID; worldID++) {
        for (int groupID = 0; groupID < maxServerGroupID; groupID++) {
            HashMapGameServerInfo& gameServerInfo = pGameServerInfos[worldID][groupID];

            if (!gameServerInfo.empty()) {
                HashMapGameServerInfo::const_iterator itr = gameServerInfo.begin();
                for (; itr != gameServerInfo.end(); itr++) {
                    GameServerInfo* pGameServerInfo = itr->second;

                    if (pGameServerInfo->getWorldID() == myWorldID) {
                        // ���� ������ �ƴ� ��쿡��..(������ ó�������Ƿ�)
                        if (pGameServerInfo->getGroupID() == myServerID) {
                        } else {
                            g_pLoginServerManager->sendPacket(pGameServerInfo->getIP(), pGameServerInfo->getUDPPort(),
                                                              &ggCommand);
                        }
                    }
                    // �ٸ� World�� ���. ���� world���� �Ѹ��°� �ƴ϶��..
                    else if (!bSameWorldOnly) {
                        g_pLoginServerManager->sendPacket(pGameServerInfo->getIP(), pGameServerInfo->getUDPPort(),
                                                          &ggCommand);
                    }
                }
            }
        }
    }

    __END_DEBUG_EX __END_CATCH
}

void opbillingdisconnect() {}

void opbugreport(Creature* pCreature, GamePlayer* pGamePlayer, string msg, int i) {
    size_t j = msg.find_first_of(' ', i + 1);
    if (j == string::npos) {
        filelog("bugreport.log", "%s", msg.c_str());
        return;
    }

    string report = msg.substr(j + 1, msg.size() - j - 1);
    if ((j = report.find_first_of('\'')) != string::npos)
        report[j] = '_';
    if ((j = report.find_first_of('\\')) != string::npos)
        report[j] = '_';

    try {
        defaultSessionRepository().insertBugReport(pGamePlayer->getID(), pCreature->getName(), report);
        // ���� �̻��Ѱ� ������ ��������
    } catch (...) {
        filelog("bugreport.log", "%s", msg.c_str());
    }
}

void opcrashreport(Creature* pCreature, GamePlayer* pGamePlayer, string msg, int i) {
    size_t j = msg.find_first_of(' ', i + 1);
    size_t k = msg.find_first_of(' ', j + 13);
    size_t l = msg.find_first_of(' ', k + 1);
    size_t m = msg.find_first_of(' ', l + 1);
    if (j == string::npos || k == string::npos || l == string::npos || m == string::npos) {
        filelog("CrashReport.log", "[%s] %s", pGamePlayer->getID().c_str(), msg.c_str());
        return;
    }

    string ExecutableTime = msg.substr(j + 1, k - j - 1);
    string Version = msg.substr(k + 1, l - k - 1);
    string Address = msg.substr(l + 1, m - l - 1);
    string Message = msg.substr(m + 1, msg.size() - m - 1);

    cout << ExecutableTime << endl;
    cout << Version << endl;
    cout << Address << endl;
    cout << Message << endl;

    try {
        defaultSessionRepository().insertCrashLog(pGamePlayer->getID(), pCreature->getName(), ExecutableTime, Version,
                                                  Address, Message);
        // ���� �̻��Ѱ� ������ ��������
    } catch (...) {
        filelog("CrashReport.log", "%s", msg.c_str());
    }
}

} // namespace de::gm
