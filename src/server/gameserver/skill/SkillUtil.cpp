//////////////////////////////////////////////////////////////////////////////
// Filename    : SkillUtil.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "SkillUtil.h"

#include "Monster.h"
#include "Player.h"
#include "SkillInfo.h"
#include "domain/Formulas.h"
// #include "AttrBalanceInfo.h"
#include <math.h>
#include <stdio.h>

#include "AlignmentManager.h"
#include "CreatureUtil.h"
#include "DynamicZone.h"
#include "EffectAirShield.h"
#include "EffectAlignmentRecovery.h"
#include "EffectArmageddon.h"
#include "EffectAuraShield.h"
#include "EffectBlindness.h"
#include "EffectBlockHead.h"
#include "EffectCanEnterGDRLair.h"
#include "EffectDivineSpirits.h"
#include "EffectEnemyErase.h"
#include "EffectExpansion.h"
#include "EffectExplosionWater.h"
#include "EffectFrozenArmor.h"
#include "EffectGrandMasterOusters.h"
#include "EffectGrandMasterSlayer.h"
#include "EffectGrandMasterVampire.h"
#include "EffectHandsOfFire.h"
#include "EffectHymn.h"
#include "EffectIceFieldToCreature.h"
#include "EffectIceOfSoulStone.h"
#include "EffectInstallTurret.h"
#include "EffectMephisto.h"
#include "EffectPrecedence.h"
#include "EffectReactiveArmor.h"
#include "EffectRediance.h"
#include "EffectRequital.h"
#include "EffectRevealer.h"
#include "EffectShareHP.h"
#include "EffectSharpShield.h"
#include "EffectSleep.h"
#include "EffectStoneSkin.h"
#include "EffectStriking.h"
#include "EffectSwordOfThor.h"
#include "EffectWaterBarrier.h"
#include "EventHeadCount.h"
#include "EventItemUtil.h"
#include "GCAddEffect.h"
#include "GCAddEffectToTile.h"
#include "GCAddInjuriousCreature.h"
#include "GCKickMessage.h"
#include "GCLearnSkillReady.h"
#include "GCOtherModifyInfo.h"
#include "GCRemoveEffect.h"
#include "GCRemoveFromGear.h"
#include "GCSkillFailed1.h"
#include "GCSkillFailed2.h"
#include "GCSkillToObjectOK4.h"
#include "GCSkillToObjectOK6.h"
#include "GCStatusCurrentHP.h"
#include "GCSystemMessage.h"
#include "GDRLairManager.h"
#include "GQuestManager.h"
#include "GamePlayer.h"
#include "GameServerInfoManager.h"
#include "HitRoll.h"
#include "ItemFactoryManager.h"
#include "ItemUtil.h"
#include "LogClient.h"
#include "MasterLairInfoManager.h"
#include "OustersEXPInfo.h"
#include "PKZoneInfoManager.h"
#include "PacketUtil.h"
#include "Party.h"
#include "PrecedenceTable.h"
#include "Properties.h"
#include "SkillDomainInfoManager.h"
#include "SkillPropertyManager.h"
#include "SkillUtilInternal.h"
#include "SummonGroundElemental.h"
#include "Thread.h"
#include "VampEXPInfo.h"
#include "VariableManager.h"
#include "Zone.h"
#include "ZoneUtil.h"
#include "mission/EventQuestLootingManager.h"
#include "mission/MonsterKillQuestStatus.h"
#include "mission/QuestManager.h"

//////////////////////////////////////////////////////////////////////////////
// data structure & helper functions
//////////////////////////////////////////////////////////////////////////////
typedef struct DomainStruct {
    int DomainType;
    int DomainLevel;
} SDomain;

class isBig {
public:
    isBig(){};

    bool operator()(const DomainStruct& t, const DomainStruct& b) {
        if (t.DomainLevel > b.DomainLevel)
            return true;
        else
            return false;
    }
};

//////////////////////////////////////////////////////////////////////////////
// À¯·áÈ­Á¸ °æÇèÄ¡ »Ç³ª½º
//////////////////////////////////////////////////////////////////////////////
// const uint g_pVariableManager->getPremiumExpBonusPercent() = 150;


RankExp_t computeRankExp(int myLevel, int otherLevel) // by sigi. 2002.12.31
{
    // The formula lives in de-core; this adapter only supplies
    // the two server-configured percentages.
    return decore::rankExp(myLevel, otherLevel, g_pVariableManager->getVariable(RANK_EXP_GAIN_PERCENT),
                           g_pVariableManager->getPremiumExpBonusPercent());
}

//////////////////////////////////////////////////////////////////////////////
// Creature¸¦ Á×¿´À»¶§ÀÇ È¿°ú
//
// Á×Àº »ç¶÷¿¡°Ô KillCount¸¦ Áõ°¡½ÃÄÑÁØ´Ù. --> °è±Þ °æÇèÄ¡
// by sigi. 2002.8.31
//////////////////////////////////////////////////////////////////////////////
void affectKillCount(Creature* pAttacker, Creature* pDeadCreature) {
    // [Ã³¸®ÇÒ ÇÊ¿ä ¾ø´Â °æ¿ì]
    // °ø°ÝÇÑ »ç¶÷ÀÌ ¾ø°Å³ª
    // Á×Àº¾Ö°¡ ¾ø°Å³ª
    // °ø°ÝÇÑ »ç¶÷ÀÌ »ç¶÷ÀÌ ¾Æ´Ï°Å³ª -_-;
    // Á×Àº¾Ö°¡ Á×Àº°Ô ¾Æ´Ï¸é -_-;
    if (pAttacker == NULL || pDeadCreature == NULL || !pAttacker->isPC() || pDeadCreature->isAlive()) {
        return;
    }

    if (pDeadCreature->isMonster()) {
        Monster* pMonster = dynamic_cast<Monster*>(pDeadCreature);

        if (pMonster->getLastKiller() == 0)
            pMonster->setLastKiller(pAttacker->getObjectID());
    }

    int myLevel = 0;
    int otherLevel = 0;
    // int bonusPercent = 100;

    if (pAttacker->isSlayer()) {
        // ½½·¹ÀÌ¾î°¡ ½½·¹ÀÌ¾î¸¦ Á×ÀÎ °æ¿ì
        if (pDeadCreature->isSlayer())
            return;

        Slayer* pSlayer = dynamic_cast<Slayer*>(pAttacker);
        myLevel = pSlayer->getHighestSkillDomainLevel();

        // ½½·¹ÀÌ¾îÀÏ °æ¿ì ¹«±â¸¦ µé°í ÀÖÁö ¾Ê´Ù¸é ¹«½ÃÇÑ´Ù.
        if (!pSlayer->isRealWearingEx(Slayer::WEAR_RIGHTHAND))
            return;

        // ½½·¹ÀÌ¾î°¡ ¹ìÆÄÀÌ¾î¸¦ Á×ÀÎ °æ¿ì
        if (pDeadCreature->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pDeadCreature);
            otherLevel = pVampire->getLevel();
            // bonusPercent = 150;
        }
        // ½½·¹ÀÌ¾î°¡ ¾Æ¿ì½ºÅÍ½º¸¦ Á×ÀÎ °æ¿ì
        else if (pDeadCreature->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pDeadCreature);
            otherLevel = pOusters->getLevel();
            // bonusPercent = 150;
        }
        // ½½·¹ÀÌ¾î°¡ ¸ó½ºÅÍ¸¦ Á×ÀÎ °æ¿ì
        else if (pDeadCreature->isMonster()) {
            Monster* pMonster = dynamic_cast<Monster*>(pDeadCreature);

            // ¸¶½ºÅÍ´Â MasterLairManager¿¡¼­ Ã³¸®ÇÑ´Ù.
            if (pMonster->isMaster()) {
                // last killÇÑ »ç¶÷Àº °æÇèÄ¡ ÇÑ¹ø ´õ ¸Ô´Â´Ù.
                pSlayer->increaseRankExp(MASTER_KILL_RANK_EXP);
                return;
            }

            otherLevel = pMonster->getLevel();
        } else
            return;
    } else if (pAttacker->isVampire()) {
        // ¹ìÆÄÀÌ¾î°¡ ¹ìÆÄÀÌ¾î¸¦ Á×ÀÎ °æ¿ì
        if (pDeadCreature->isVampire())
            return;

        Vampire* pVampire = dynamic_cast<Vampire*>(pAttacker);
        myLevel = pVampire->getLevel();

        // ¹ìÆÄÀÌ¾î°¡ ½½·¹ÀÌ¾î¸¦ Á×ÀÎ °æ¿ì
        if (pDeadCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pDeadCreature);
            otherLevel = pSlayer->getHighestSkillDomainLevel();
            // bonusPercent = 150;
        }
        // ¹ìÆÄÀÌ¾î°¡ ¾Æ¿ì½ºÅÍ½º¸¦ Á×ÀÎ °æ¿ì
        else if (pDeadCreature->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pDeadCreature);
            otherLevel = pOusters->getLevel();
            // bonusPercent = 150;
        }
        // ¹ìÆÄÀÌ¾î°¡ ¸ó½ºÅÍ¸¦ Á×ÀÎ °æ¿ì
        else if (pDeadCreature->isMonster()) {
            Monster* pMonster = dynamic_cast<Monster*>(pDeadCreature);

            // ¸¶½ºÅÍ´Â MasterLairManager¿¡¼­ Ã³¸®ÇÑ´Ù.
            if (pMonster->isMaster()) {
                // last killÇÑ »ç¶÷Àº °æÇèÄ¡ ÇÑ¹ø ´õ ¸Ô´Â´Ù.
                pVampire->increaseRankExp(MASTER_KILL_RANK_EXP);
                return;
            }

            otherLevel = pMonster->getLevel();
        } else
            return;
    } else if (pAttacker->isOusters()) {
        // ¾Æ¿ì½ºÅÍ½º°¡ ¾Æ¿ì½ºÅÍ½º¸¦ Á×ÀÎ °æ¿ì
        if (pDeadCreature->isOusters())
            return;

        Ousters* pOusters = dynamic_cast<Ousters*>(pAttacker);
        myLevel = pOusters->getLevel();

        // ¾Æ¿ì½ºÅÍ½º°¡°¡ ½½·¹ÀÌ¾î¸¦ Á×ÀÎ °æ¿ì
        if (pDeadCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pDeadCreature);
            otherLevel = pSlayer->getHighestSkillDomainLevel();
            // bonusPercent = 150;
        }
        // ¾Æ¿ì½ºÅÍÁî°¡ ¹ìÆÄÀÌ¾î¸¦ Á×ÀÎ °æ¿ì
        if (pDeadCreature->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pDeadCreature);
            otherLevel = pVampire->getLevel();
            // bonusPercent = 150;
        }
        // ¹ìÆÄÀÌ¾î°¡ ¸ó½ºÅÍ¸¦ Á×ÀÎ °æ¿ì
        else if (pDeadCreature->isMonster()) {
            Monster* pMonster = dynamic_cast<Monster*>(pDeadCreature);

            // ¸¶½ºÅÍ´Â MasterLairManager¿¡¼­ Ã³¸®ÇÑ´Ù.
            if (pMonster->isMaster()) {
                // last killÇÑ »ç¶÷Àº °æÇèÄ¡ ÇÑ¹ø ´õ ¸Ô´Â´Ù.
                pOusters->increaseRankExp(MASTER_KILL_RANK_EXP);
                return;
            }

            otherLevel = pMonster->getLevel();
        } else
            return;
    } else
        return;

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pAttacker);

    if (pDeadCreature->isMonster()) {
        Monster* pMonster = dynamic_cast<Monster*>(pDeadCreature);
        PrecedenceTable* pTable = pMonster->getPrecedenceTable();

        //		if ( pMonster->getLastKiller() == 0 )
        //			pMonster->setLastKiller( pAttacker->getObjectID() );

        if (pMonster->getMonsterType() == 722) {
            if (pPC->getPartyID() == 0) {
                if (!pPC->isFlag(Effect::EFFECT_CLASS_CAN_ENTER_GDR_LAIR)) {
                    EffectCanEnterGDRLair* pEffect = new EffectCanEnterGDRLair(pPC);
                    pEffect->setDeadline(216000);

                    pPC->setFlag(pEffect->getEffectClass());
                    pPC->addEffect(pEffect);

                    pEffect->create(pPC->getName());

                    GCAddEffect gcAddEffect;
                    gcAddEffect.setObjectID(pPC->getObjectID());
                    gcAddEffect.setEffectID(pEffect->getSendEffectClass());
                    gcAddEffect.setDuration(21600);

                    pPC->getZone()->broadcastPacket(pPC->getX(), pPC->getY(), &gcAddEffect);
                }
                //					addSimpleCreatureEffect( pPC, Effect::EFFECT_CLASS_CAN_ENTER_GDR_LAIR, 216000 );
            } else {
                LocalPartyManager* pLPM = pPC->getLocalPartyManager();
                Assert(pLPM != NULL);
                pLPM->shareGDRLairEnter(pPC->getPartyID(), pPC);
            }

            Item* pItem = g_pItemFactoryManager->createItem(Item::ITEM_CLASS_EVENT_ITEM, 29, list<OptionType_t>());
            pMonster->setQuestItem(pItem);
        }

        if (pTable != NULL && pTable->canGainRankExp(pPC)) {
            QuestManager* pQM = pPC->getQuestManager();

            if (pQM != NULL && pQM->killedMonster(pMonster)) {
                pPC->sendCurrentQuestInfo();
            }

            if (g_pEventQuestLootingManager->killed(pPC, pMonster))
                pTable->setQuestHostName(pPC->getName());

            if (pMonster->getQuestItem() == NULL && g_pVariableManager->getVariable(EVENT_NEW_YEAR_2005) != 0) {
                Item* pItem = getNewYear2005Item(getNewYear2005ItemKind(pPC, pMonster));
                pMonster->setQuestItem(pItem);
                if (pItem != NULL)
                    logEventItemCount(pItem);
            }

            if (pMonster->getQuestItem() == NULL && g_pVariableManager->isEventMoonCard()) {
                Item* pItem = getCardItem(getCardKind(pPC, pMonster));
                //				setItemGender( pItem, (pPC->getSex()==FEMALE)?GENDER_FEMALE:GENDER_MALE );
                pMonster->setQuestItem(pItem);
            }

            if (pMonster->getQuestItem() == NULL && g_pVariableManager->isEventLuckyBag()) {
                Item* pItem = getLuckyBagItem(getLuckyBagKind(pPC, pMonster));
                pMonster->setQuestItem(pItem);
            }

            if (pMonster->getQuestItem() == NULL && g_pVariableManager->getVariable(NICKNAME_PEN_EVENT) != 0 &&
                canGiveEventItem(pPC, pMonster)) {
                int value = rand() % 100000;
                if (value < g_pVariableManager->getVariable(NICKNAME_PEN_RATIO)) {
                    Item* pItem =
                        g_pItemFactoryManager->createItem(Item::ITEM_CLASS_EVENT_GIFT_BOX, 22, list<OptionType_t>());
                    pMonster->setQuestItem(pItem);
                }
            }

            if (pMonster->getQuestItem() == NULL && g_pVariableManager->getVariable(CLOVER_EVENT) != 0 &&
                canGiveEventItem(pPC, pMonster)) {
                int value = rand() % 100000;
                if (value < g_pVariableManager->getVariable(CLOVER_RATIO)) {
                    Item* pItem =
                        g_pItemFactoryManager->createItem(Item::ITEM_CLASS_MOON_CARD, 3, list<OptionType_t>());
                    pMonster->setQuestItem(pItem);
                }
            }

            if (pMonster->getQuestItem() == NULL && g_pVariableManager->getVariable(PINE_CAKE_EVENT) != 0 &&
                canGiveEventItem(pPC, pMonster)) {
                int value = rand() % 100000;
                if (value < g_pVariableManager->getVariable(PINE_CAKE_RATIO)) {
                    int value = rand() % 10;
                    int add = 0;
                    if (value < 6)
                        add = 0;
                    else if (value < 9)
                        add = 1;
                    else
                        add = 2;
                    Item* pItem =
                        g_pItemFactoryManager->createItem(Item::ITEM_CLASS_EVENT_ETC, 15 + add, list<OptionType_t>());
                    pMonster->setQuestItem(pItem);
                }
            }

            if (g_pVariableManager->getVariable(NETMARBLE_CARD_EVENT) != 0 && pMonster->getQuestItem() == NULL &&
                g_pVariableManager->getVariable(NETMARBLE_CARD_RATIO) > (rand() % 100000) &&
                canGiveEventItem(pPC, pMonster)) {
                Item* pItem = g_pItemFactoryManager->createItem(Item::ITEM_CLASS_MOON_CARD, 2, list<OptionType_t>());
                pMonster->setQuestItem(pItem);
            }

            if (g_pVariableManager->isHeadCount()) {
                GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pAttacker->getPlayer());
                if (pGamePlayer != NULL) {
                    EventHeadCount* pEvent =
                        dynamic_cast<EventHeadCount*>(pGamePlayer->getEvent(Event::EVENT_CLASS_HEAD_COUNT));
                    if (pEvent != NULL)
                        pEvent->cutHead();
                }
            }

            if (canGiveEventItem(pPC, pMonster) &&
                rand() % 100000 < g_pVariableManager->getVariable(GOLD_MEDAL_RATIO)) {
                giveGoldMedal(pPC);
            }

            if (pPC->getLevel() - 20 <= pMonster->getLevel() && pPC->getLevel() >= 30 &&
                !GDRLairManager::Instance().isGDRLairZone(pPC->getZoneID()) && !pMonster->isChief() &&
                !pMonster->isMaster()) {
                addOlympicStat(pPC, 1);
            }

            if (pMonster->isChief()) {
                if (canGiveEventItem(pPC, pMonster) && !GDRLairManager::Instance().isGDRLairZone(pPC->getZoneID()))
                    addOlympicStat(pPC, 3);
            }

            if (pMonster->isMaster()) {
                if (canGiveEventItem(pPC, pMonster))
                    addOlympicStat(pPC, 3);
            }

            if (pMonster->getQuestItem() == NULL && canGiveEventItem(pPC, pMonster) &&
                rand() % 100000 < g_pVariableManager->getVariable(OLYMPIC_ITEM_RATIO)) {
                Item* pItem = g_pItemFactoryManager->createItem(Item::ITEM_CLASS_MOON_CARD, 4, list<OptionType_t>());
                pMonster->setQuestItem(pItem);
            }

            if (pMonster->getQuestItem() == NULL && canGiveEventItem(pPC, pMonster) &&
                rand() % 100000 < g_pVariableManager->getVariable(LUCK_CHARM_RATIO)) {
                Item* pItem = g_pItemFactoryManager->createItem(Item::ITEM_CLASS_EVENT_ITEM, 30, list<OptionType_t>());
                pMonster->setQuestItem(pItem);
            }

            if (pMonster->getQuestItem() == NULL && pMonster->getMonsterType() >= 769 &&
                canGiveEventItem(pPC, pMonster) &&
                rand() % 100000 < g_pVariableManager->getVariable(HOURGLASS_RATIO_S)) {
                Item* pItem = g_pItemFactoryManager->createItem(Item::ITEM_CLASS_EFFECT_ITEM, 6, list<OptionType_t>());
                pMonster->setQuestItem(pItem);
            }

            if (pMonster->getQuestItem() == NULL && pMonster->getMonsterType() >= 769 &&
                canGiveEventItem(pPC, pMonster) &&
                rand() % 100000 < g_pVariableManager->getVariable(HOURGLASS_RATIO_M)) {
                Item* pItem = g_pItemFactoryManager->createItem(Item::ITEM_CLASS_EFFECT_ITEM, 5, list<OptionType_t>());
                pMonster->setQuestItem(pItem);
            }

            if (pMonster->getQuestItem() == NULL && pMonster->getMonsterType() >= 769 &&
                canGiveEventItem(pPC, pMonster) &&
                rand() % 100000 < g_pVariableManager->getVariable(HOURGLASS_RATIO_L)) {
                Item* pItem = g_pItemFactoryManager->createItem(Item::ITEM_CLASS_EFFECT_ITEM, 4, list<OptionType_t>());
                pMonster->setQuestItem(pItem);
            }

            if (pMonster->getZone()->isDynamicZone() &&
                pMonster->getZone()->getDynamicZone()->getTemplateZoneID() == 4002 &&
                pMonster->getQuestItem() == NULL && (rand() % 20) == 0) {
                Item* pItem = g_pItemFactoryManager->createItem(Item::ITEM_CLASS_EVENT_ITEM, 31, list<OptionType_t>());
                pMonster->setQuestItem(pItem);
            }

            if (g_pVariableManager->getVariable(PET_FOOD_EVENT) != 0 && pMonster->getQuestItem() == NULL &&
                g_pVariableManager->getVariable(PET_FOOD_RATIO) > rand() % 100000 && canGiveEventItem(pPC, pMonster)) {
                bool isHigher = g_pVariableManager->getVariable(HIGHER_PET_FOOD_RATIO) > rand() % 100;

                int itemClassSeed = rand() % 100;
                int RacePetFoodRatio = g_pVariableManager->getVariable(RACE_PET_FOOD_RATIO);
                int RevivalSetRatio = g_pVariableManager->getVariable(REVIVAL_SET_RATIO);

                Item::ItemClass iClass = Item::ITEM_CLASS_PET_FOOD;
                ItemType_t itemType = 1;

                if (itemClassSeed < RevivalSetRatio) {
                    iClass = Item::ITEM_CLASS_PET_ENCHANT_ITEM;
                    itemType = 13;
                }
                //				bool isRace = g_pVariableManager->getVariable(RACE_PET_FOOD_RATIO) > rand()%100;
                else if (itemClassSeed - RevivalSetRatio < RacePetFoodRatio) {
                    switch (pPC->getCreatureClass()) {
                    case Creature::CREATURE_CLASS_SLAYER:
                        itemType = 6;
                        break;

                    case Creature::CREATURE_CLASS_VAMPIRE:
                        itemType = 10;
                        break;

                    case Creature::CREATURE_CLASS_OUSTERS:
                        itemType = 14;
                        break;

                    default:
                        break;
                    }
                }

                if (iClass == Item::ITEM_CLASS_PET_FOOD && isHigher)
                    ++itemType;
                //				cout << "ÆêÀ½½ÄÀÌº¥Æ®:" << iClass << ", " << itemType << endl;

                Item* pItem = g_pItemFactoryManager->createItem(iClass, itemType, list<OptionType_t>());
                pMonster->setQuestItem(pItem);
            }
        }
    }

    int PartyID = pPC->getPartyID();
    if (PartyID != 0) {
        // ÆÄÆ¼¿¡ °¡ÀÔµÇ¾î ÀÖ´Ù¸é ·ÎÄÃ ÆÄÆ¼ ¸Å´ÏÀú¸¦ ÅëÇØ
        // ÁÖÀ§ÀÇ ÆÄÆ¼¿øµé°ú °æÇèÄ¡¸¦ °øÀ¯ÇÑ´Ù.
        LocalPartyManager* pLPM = pPC->getLocalPartyManager();
        Assert(pLPM != NULL);
        pLPM->shareRankExp(PartyID, pAttacker, otherLevel);
        if (pPC->isAdvanced())
            pLPM->shareAdvancementExp(PartyID, pAttacker, computeCreatureExp(pDeadCreature, 1));
    } else {
        PlayerCreature* pAttackPC = dynamic_cast<PlayerCreature*>(pAttacker);
        if (pAttackPC->isAdvanced()) {
            pAttackPC->increaseAdvancementClassExp(computeCreatureExp(pDeadCreature, 1), true);
        }

        // ÆÄÆ¼¿¡ °¡ÀÔµÇ¾îÀÖÁö ¾Ê´Ù¸é È¥ÀÚ ¿Ã¶ó°£´Ù.
        RankExp_t rankExp = computeRankExp(myLevel, otherLevel);

        if (pDeadCreature->isMonster()) {
            Monster* pMonster = dynamic_cast<Monster*>(pDeadCreature);
            PrecedenceTable* pTable = pMonster->getPrecedenceTable();

            if (pTable != NULL && pTable->canGainRankExp(pPC)) {
                pPC->increaseRankExp(rankExp);
            }
        } else {
            pPC->increaseRankExp(rankExp);
        }
    }
}


bool canGiveSkillExp(Slayer* pSlayer, SkillDomainType_t SkillDomainType, SkillType_t UseSkillType) {
    if (pSlayer == NULL)
        return false;
    if (!pSlayer->isRealWearing(Slayer::WEAR_RIGHTHAND))
        return false;

    Item* pWeapon = pSlayer->getWearItem(Slayer::WEAR_RIGHTHAND);
    if (pWeapon == NULL)
        return false;

    switch (SkillDomainType) {
    case SKILL_DOMAIN_BLADE:
        if (pWeapon->getItemClass() != Item::ITEM_CLASS_BLADE)
            return false;
        break;
    case SKILL_DOMAIN_SWORD:
        if (pWeapon->getItemClass() != Item::ITEM_CLASS_SWORD)
            return false;
        break;
    case SKILL_DOMAIN_GUN:
        if (!isArmsWeapon(pWeapon))
            return false;
        break;
    case SKILL_DOMAIN_HEAL:
        if (pWeapon->getItemClass() != Item::ITEM_CLASS_CROSS)
            return false;
        break;
    case SKILL_DOMAIN_ENCHANT:
        if (pWeapon->getItemClass() != Item::ITEM_CLASS_MACE)
            return false;
        break;
    default:
        return false;
    }

    if (UseSkillType < SKILL_DOUBLE_IMPACT)
        return true;
    SkillInfo* pUseSkillInfo = g_pSkillInfoManager->getSkillInfo(UseSkillType);
    if (SkillDomainType != pUseSkillInfo->getDomainType())
        return false;

    return true;
}

void giveSkillExp(Slayer* pSlayer, SkillType_t SkillType, ModifyInfo& AttackerMI) {
    if (pSlayer == NULL)
        return;

    SkillSlot* pSkillSlot = pSlayer->getSkill(SkillType);
    SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SkillType);
    if (pSkillSlot != NULL && pSkillInfo != NULL) {
        // cout << pSlayer->getName() << "¿¡°Ô " << pSkillInfo->getName() << "½ºÅ³ÀÇ °æÇèÄ¡¸¦ ÁÝ´Ï´Ù." << endl;
        increaseSkillExp(pSlayer, pSkillInfo->getDomainType(), pSkillSlot, pSkillInfo, AttackerMI);
    }
}


//////////////////////////////////////////////////////////////////////////////
// ÀÎÆ®¿¡ µû¶ó ¸¶³ª ¼Ò¸ð·®ÀÌ º¯ÇÏ´Â ¹ìÆÄÀÌ¾î ¸¶¹ýÀÇ ¸¶³ª ¼Ò¸ð·®À» °è»êÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
MP_t decreaseConsumeMP(Vampire* pVampire, SkillInfo* pSkillInfo) {
    Assert(pVampire != NULL);
    Assert(pSkillInfo != NULL);

    // The INT-discount bracket table lives in de-core.
    return decore::vampireSkillConsumeMP(pSkillInfo->getConsumeMP(), pSkillInfo->getLevel(), pVampire->getINT());
}


//////////////////////////////////////////////////////////////////////////////
// ±â¼úÀ» »ç¿ëÇÏ±â À§ÇÑ ÃæºÐÇÑ ¸¶³ª¸¦ °¡Áö°í ÀÖ´Â°¡?
//////////////////////////////////////////////////////////////////////////////
bool hasEnoughMana(Creature* pCaster, int RequiredMP) {
    if (pCaster->isSlayer()) {
        Slayer* pSlayer = dynamic_cast<Slayer*>(pCaster);

        int decreaseRatio = pSlayer->getConsumeMPRatio();
        if (decreaseRatio != 0) {
            RequiredMP += getPercentValue(RequiredMP, decreaseRatio);
        }

        // Sacrifice¸¦ ¾´ »óÅÂ¶ó¸é ¸¶³ª°¡ ¸ðÀÚ¶óµµ HP·Î ´ë½ÅÇÒ ¼ö ÀÖ´Ù.
        if (pSlayer->isFlag(Effect::EFFECT_CLASS_SACRIFICE)) {
            // cout << "RequiredMP : " << (int)RequiredMP << endl;
            int margin = RequiredMP - pSlayer->getMP(ATTR_CURRENT);
            // cout << "margin: " << (int)margin<< endl;

            // ¿ä±¸Ä¡¿¡¼­ ÇöÀç ¼öÄ¡¸¦ »« °ªÀÌ 0ÀÌ»óÀÌ¶ó¸é ,
            // ¿ä±¸Ä¡°¡ ´õ Å©´Ù´Â ¸»ÀÌ´Ù. ÀÌ ¼öÄ¡´Â HP¿¡¼­ Á¦°ÅÇÑ´Ù.
            if (margin > 0) {
                margin = (int)pSlayer->getHP(ATTR_CURRENT) * 2 - (int)margin;
                // cout << "margin: " << (int)margin<< endl;
                if (margin > 0)
                    return true;
            } else {
                return true;
            }
        } else {
            if (pSlayer->getMP(ATTR_CURRENT) >= (MP_t)RequiredMP)
                return true;
        }
    } else if (pCaster->isVampire()) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pCaster);

        // ¹ìÆÄÀÌ¾î´Â HP°¡ °ð MPÀÌ±â ¶§¹®¿¡ ¸¶³ª¸¦ »ç¿ëÇÏ°í,
        // Á×¾î¹ö¸®¸é °ï¶õÇÏ´Ù. ±×·¯¹Ç·Î ±â¼úÀ» »ç¿ëÇÏ°í ³ª¼­
        // HP´Â 1 ÀÌ»óÀÌ¾î¾ß ÇÑ´Ù. ±×·¡¼­ >= ´ë½Å >¸¦ »ç¿ëÇÑ´Ù.

        int decreaseRatio = pVampire->getConsumeMPRatio();
        if (decreaseRatio != 0) {
            RequiredMP += getPercentValue(RequiredMP, decreaseRatio);
        }

        if (pVampire->getHP(ATTR_CURRENT) > (HP_t)RequiredMP)
            return true;
    } else if (pCaster->isOusters()) {
        Ousters* pOusters = dynamic_cast<Ousters*>(pCaster);

        int decreaseRatio = pOusters->getConsumeMPRatio();
        if (decreaseRatio != 0) {
            RequiredMP += getPercentValue(RequiredMP, decreaseRatio);
        }

        if (pOusters->getMP(ATTR_CURRENT) >= (MP_t)RequiredMP)
            return true;
    } else if (pCaster->isMonster()) {
        // ¸ó½ºÅÍ´Â ¹«ÇÑ ¸¶³ª µÇ°Ú´Ù. À½È±È±
        // ³ªÁß¿¡¶óµµ ¸ó½ºÅÍ¿¡ ¸¶¹ý Ä«¿îÆ®³ª ¹¹ ±×·² °ÍÀÌ »ý±æÁöµµ ¸ð¸£Áö.
        // comment by ±è¼º¹Î
        return true;
    } else {
        cerr << "hasEnoughMana() : Invalid Creature Class" << endl;
        Assert(false);
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////
// ÁÖ¾îÁø Æ÷ÀÎÆ®¸¸Å­ÀÇ ¸¶³ª¸¦ ÁÙÀÎ´Ù.
// ´Ü ½½·¹ÀÌ¾î °°Àº °æ¿ì¿¡´Â Sacrifice °°Àº ÀÌÆåÆ®°¡ ºÙ¾îÀÖÀ¸¸é,
// ¸¶³ª°¡ ¸ðÀÚ¶ö °æ¿ì, HP°¡ ´âÀ» ¼öµµ ÀÖ´Ù.
//////////////////////////////////////////////////////////////////////////////
int decreaseMana(Creature* pCaster, int MP, ModifyInfo& info) {
    Assert(pCaster != NULL);

    int RemainHP = 0;
    int RemainMP = 0;

    if (pCaster->isFlag(Effect::EFFECT_CLASS_NO_DAMAGE))
        return 0;

    if (pCaster->isSlayer()) {
        Slayer* pSlayer = dynamic_cast<Slayer*>(pCaster);

        // Magic Brain ÀÌ ÀÖ´Ù¸é MP ¼Ò¸ð·® 25% °¨¼Ò
        if (pSlayer->hasRankBonus(RankBonus::RANK_BONUS_MAGIC_BRAIN)) {
            RankBonus* pRankBonus = pSlayer->getRankBonus(RankBonus::RANK_BONUS_MAGIC_BRAIN);
            Assert(pRankBonus != NULL);

            MP -= getPercentValue(MP, pRankBonus->getPoint());
        }

        // Blood Bible º¸³Ê½º Àû¿ë
        int decreaseRatio = pSlayer->getConsumeMPRatio();
        if (decreaseRatio != 0) {
            // decreaseRatio °ª ÀÚÃ¼°¡ ¸¶ÀÌ³Ê½º °ªÀÌ´Ù.
            MP += getPercentValue(MP, decreaseRatio);
        }

        // sacrifice¸¦ ¾´ »óÅÂ¶ó¸é ¸ÕÀú MP¿¡¼­ ±ï°í, ¸ðÀÚ¶ó¸é HPµµ ±ï´Â´Ù.
        if (pSlayer->isFlag(Effect::EFFECT_CLASS_SACRIFICE)) {
            int margin = (int)MP - (int)pSlayer->getMP(ATTR_CURRENT);

            // ¸¶ÁøÀÌ 0º¸´Ù Å©´Ù´Â ¸»Àº ¿ä±¸Ä¡º¸´Ù ÇöÀç MP°¡ Àû´Ù´Â ¸»ÀÌ´Ù.
            if (margin > 0) {
                // MP¸¦ ±ï°í...
                pSlayer->setMP(0, ATTR_CURRENT);
                // HPµµ ±ï´Â´Ù.
                RemainHP = max(0, (int)(pSlayer->getHP(ATTR_CURRENT) - margin / 2));
                pSlayer->setHP(RemainHP, ATTR_CURRENT);

                info.addShortData(MODIFY_CURRENT_MP, pSlayer->getMP(ATTR_CURRENT));
                info.addShortData(MODIFY_CURRENT_HP, pSlayer->getHP(ATTR_CURRENT));
                return CONSUME_BOTH;
            }

            // sacrifice¸¦ ¾²Áö ¾ÊÀº »óÅÂ¶ó¸é °Á MP¿¡¼­ ±ï´Â´Ù.
            RemainMP = max(0, ((int)pSlayer->getMP(ATTR_CURRENT) - (int)MP));
            pSlayer->setMP(RemainMP, ATTR_CURRENT);

            info.addShortData(MODIFY_CURRENT_MP, pSlayer->getMP(ATTR_CURRENT));
            return CONSUME_MP;
        } else // sacrifice¸¦ ¾²Áö ¾ÊÀº »óÅÂ¶ó¸é °Á MP¿¡¼­ ±ï´Â´Ù.
        {
            RemainMP = max(0, ((int)pSlayer->getMP(ATTR_CURRENT) - (int)MP));
            pSlayer->setMP(RemainMP, ATTR_CURRENT);

            info.addShortData(MODIFY_CURRENT_MP, pSlayer->getMP(ATTR_CURRENT));
            return CONSUME_MP;
        }
    } else if (pCaster->isVampire()) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pCaster);

        // Wisdom of Blood °¡ ÀÖ´Ù¸é HP ¼Ò¸ð·® 10% °¨¼Ò
        if (pVampire->hasRankBonus(RankBonus::RANK_BONUS_WISDOM_OF_BLOOD)) {
            RankBonus* pRankBonus = pVampire->getRankBonus(RankBonus::RANK_BONUS_WISDOM_OF_BLOOD);
            Assert(pRankBonus != NULL);

            MP -= getPercentValue(MP, pRankBonus->getPoint());
        }

        // Blood Bible º¸³Ê½º Àû¿ë
        int decreaseRatio = pVampire->getConsumeMPRatio();
        if (decreaseRatio != 0) {
            // decreaseRatio °ª ÀÚÃ¼°¡ ¸¶ÀÌ³Ê½º °ªÀÌ´Ù.
            MP += getPercentValue(MP, decreaseRatio);
        }

        HP_t currentHP = pVampire->getHP(ATTR_CURRENT);
        RemainHP = max(0, ((int)currentHP - (int)MP));
        pVampire->setHP(RemainHP, ATTR_CURRENT);

        // Mephisto ÀÌÆåÆ® °É·ÁÀÖÀ¸¸é HP 30% ÀÌÇÏÀÏ¶§ Ç®¸°´Ù.
        if (pVampire->isFlag(Effect::EFFECT_CLASS_MEPHISTO)) {
            HP_t maxHP = pVampire->getHP(ATTR_MAX);

            // 33% ... ÄÉÄÉ..
            if (currentHP * 3 < maxHP) {
                Effect* pEffect = pVampire->findEffect(Effect::EFFECT_CLASS_MEPHISTO);
                if (pEffect != NULL) {
                    pEffect->setDeadline(0);
                } else {
                    pVampire->removeFlag(Effect::EFFECT_CLASS_MEPHISTO);
                }
                //				pVampire->getEffectManager()->deleteEffect( Effect::EFFECT_CLASS_MEPHISTO );
            }
        }

        info.addShortData(MODIFY_CURRENT_HP, pVampire->getHP(ATTR_CURRENT));
        return CONSUME_HP;
    } else if (pCaster->isOusters()) {
        Ousters* pOusters = dynamic_cast<Ousters*>(pCaster);

        // Blood Bible º¸³Ê½º Àû¿ë
        int decreaseRatio = pOusters->getConsumeMPRatio();
        if (decreaseRatio != 0) {
            // decreaseRatio °ª ÀÚÃ¼°¡ ¸¶ÀÌ³Ê½º °ªÀÌ´Ù.
            MP += getPercentValue(MP, decreaseRatio);
        }

        RemainMP = max(0, ((int)pOusters->getMP(ATTR_CURRENT) - (int)MP));
        pOusters->setMP(RemainMP, ATTR_CURRENT);

        info.addShortData(MODIFY_CURRENT_MP, pOusters->getMP(ATTR_CURRENT));
        return CONSUME_MP;
    } else if (pCaster->isMonster()) {
        // ¸ó½ºÅÍ´Â ¹«ÇÑ ¸¶³ª µÇ°Ú´Ù. À½È±È±
        // ³ªÁß¿¡¶óµµ ¸ó½ºÅÍ¿¡ ¸¶¹ý Ä«¿îÆ®³ª ¹¹ ±×·² °ÍÀÌ »ý±æÁöµµ ¸ð¸£Áö.
        // comment by ±è¼º¹Î
        cerr << "decreaseMana() : Monster don't have Mana" << endl;
        Assert(false);
    } else {
        cerr << "hasEnoughMana() : Invalid Creature Class" << endl;
        Assert(false);
    }

    return CONSUME_MP;
}

//////////////////////////////////////////////////////////////////////////////
// ½½·¹ÀÌ¾î¿ë ½ºÅ³ÀÇ »çÁ¤°Å¸®¸¦ °è»êÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
Range_t computeSkillRange(SkillSlot* pSkillSlot, SkillInfo* pSkillInfo) {
    Assert(pSkillSlot != NULL);
    Assert(pSkillInfo != NULL);

    // SkillÀÇ Min/Max Range ¸¦ ¹Þ¾Æ¿Â´Ù.
    Range_t SkillMinPoint = pSkillInfo->getMinRange();
    Range_t SkillMaxPoint = pSkillInfo->getMaxRange();

    // Skill LevelÀ» ¹Þ¾Æ¿Â´Ù.
    SkillLevel_t SkillLevel = pSkillSlot->getExpLevel();

    // SkillÀÇ Range¸¦ °è»êÇÑ´Ù.
    Range_t Range = (int)(SkillMinPoint + (SkillMaxPoint - SkillMinPoint) * (double)(SkillLevel * 0.01));

    return Range;
}


//////////////////////////////////////////////////////////////////////////////
// (OX,OY)¿Í (TX,TY) »çÀÌÀÇ °Å¸®¸¦ ±¸ÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
Range_t getDistance(ZoneCoord_t Ox, ZoneCoord_t Oy, ZoneCoord_t Tx, ZoneCoord_t Ty) {
    // Pure geometry — lives in de-core.
    return decore::tileDistance(Ox, Oy, Tx, Ty);
}

//////////////////////////////////////////////////////////////////////////////
// ½ºÅ³À» ¾µ ¼ö ÀÖ´Â Àû´çÇÑ °Å¸®ÀÎ°¡¸¦ °ËÁõÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
bool verifyDistance(Creature* pCreature, ZoneCoord_t X, ZoneCoord_t Y, Range_t Dist) {
    Assert(pCreature != NULL);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    ZoneCoord_t cx = pCreature->getX();
    ZoneCoord_t cy = pCreature->getY();

    ZoneLevel_t AttackerZoneLevel = pZone->getZoneLevel(cx, cy);
    // ZoneLevel_t DefenderZoneLevel = pZone->getZoneLevel(X, Y);

    // ¾Æ´ãÀÇ ¼ºÁö³ª PKÁ¸ ³»ÀÇ ¾ÈÀüÁö´ë¿¡¼­´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    if ((AttackerZoneLevel & SAFE_ZONE) && (g_pPKZoneInfoManager->isPKZone(pZone->getZoneID()) || pZone->isHolyLand()))
        return false;

    // °ø°ÝÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ½½·¹ÀÌ¾î ¾ÈÀüÁö´ë¶ó¸é,
    // ½½·¹ÀÌ¾î°¡ ¾Æ´Ñ ÀÚ´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    if ((AttackerZoneLevel & SLAYER_SAFE_ZONE) && !pCreature->isSlayer())
        return false;
    // °ø°ÝÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ¹ìÆÄÀÌ¾î ¾ÈÀüÁö´ë¶ó¸é,
    // ¹ìÆÄÀÌ¾î°¡ ¾Æ´Ñ ÀÚ´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    else if ((AttackerZoneLevel & VAMPIRE_SAFE_ZONE) && !pCreature->isVampire())
        return false;
    // °ø°ÝÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ¾Æ¿ì½ºÅÍ½º ¾ÈÀüÁö´ë¶ó¸é,
    // ¾Æ¿ì½ºÅÍ½º°¡ ¾Æ´Ñ ÀÚ´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    else if ((AttackerZoneLevel & OUSTERS_SAFE_ZONE) && !pCreature->isOusters())
        return false;
    // ¿ÏÀü ¾ÈÀüÁö´ë¶ó¸é ½½·¹ÀÌ¾îµç ¹ìÆÄÀÌ¾îµç ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    else if (AttackerZoneLevel & COMPLETE_SAFE_ZONE)
        return false;

    // ¹æ¾îÀÚ°¡ ¼­ ÀÖ´Â °÷ÀÌ ¿ÏÀüÁö´ë¶ó¸é ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    //	if (DefenderZoneLevel & COMPLETE_SAFE_ZONE)
    //		return false;

    if ((abs(cx - X) <= Dist) && (abs(cy - Y) <= Dist))
        return true;

    return false;
}


//////////////////////////////////////////////////////////////////////////////
// ½ºÅ³À» ¾µ ¼ö ÀÖ´Â Àû´çÇÑ °Å¸®ÀÎ°¡¸¦ °ËÁõÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
bool verifyDistance(Creature* pCreature, Creature* pTargetCreature, Range_t Dist) {
    Assert(pCreature != NULL);
    Assert(pTargetCreature != NULL);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    int ox = pCreature->getX();
    int oy = pCreature->getY();
    int tx = pTargetCreature->getX();
    int ty = pTargetCreature->getY();

    ZoneLevel_t AttackerZoneLevel = pZone->getZoneLevel(ox, oy);
    ZoneLevel_t DefenderZoneLevel = pZone->getZoneLevel(tx, ty);

    // ¾Æ´ãÀÇ ¼ºÁö³ª PKÁ¸ ³»ÀÇ ¾ÈÀüÁö´ë¿¡¼­´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    if ((AttackerZoneLevel & SAFE_ZONE) && (g_pPKZoneInfoManager->isPKZone(pZone->getZoneID()) || pZone->isHolyLand()))
        return false;

    // °ø°ÝÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ½½·¹ÀÌ¾î ¾ÈÀüÁö´ë¶ó¸é,
    // ½½·¹ÀÌ¾î°¡ ¾Æ´Ñ ÀÚ´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    if ((AttackerZoneLevel & SLAYER_SAFE_ZONE) && !pCreature->isSlayer())
        return false;
    // °ø°ÝÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ¹ìÆÄÀÌ¾î ¾ÈÀüÁö´ë¶ó¸é,
    // ¹ìÆÄÀÌ¾î°¡ ¾Æ´Ñ ÀÚ´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    else if ((AttackerZoneLevel & VAMPIRE_SAFE_ZONE) && !pCreature->isVampire())
        return false;
    // °ø°ÝÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ¾Æ¿ì½ºÅÍ½º ¾ÈÀüÁö´ë¶ó¸é,
    // ¾Æ¿ì½ºÅÍ½º°¡ ¾Æ´Ñ ÀÚ´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    else if ((AttackerZoneLevel & OUSTERS_SAFE_ZONE) && !pCreature->isOusters())
        return false;
    // ¿ÏÀü ¾ÈÀüÁö´ë¶ó¸é ½½·¹ÀÌ¾îµç ¹ìÆÄÀÌ¾îµç ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    else if (AttackerZoneLevel & COMPLETE_SAFE_ZONE)
        return false;

    // ¹æ¾îÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ½½·¹ÀÌ¾î ¾ÈÀüÁö´ëÀÌ°í,
    // ¹æ¾îÀÚ°¡ ½½·¹ÀÌ¾î¶ó¸é ±â¼úÀº ¸ÂÁö ¾Ê´Â´Ù.
    if ((DefenderZoneLevel & SLAYER_SAFE_ZONE) && pTargetCreature->isSlayer())
        return false;
    // ¹æ¾îÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ¹ìÆÄÀÌ¾î ¾ÈÀüÁö´ëÀÌ°í,
    // ¹æ¾îÀÚ°¡ ¹ìÆÄÀÌ¾î¶ó¸é ±â¼úÀº ¸ÂÁö ¾Ê´Â´Ù.
    else if ((DefenderZoneLevel & VAMPIRE_SAFE_ZONE) && pTargetCreature->isVampire())
        return false;
    // ¹æ¾îÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ¾Æ¿ì½ºÅÍ½º ¾ÈÀüÁö´ëÀÌ°í,
    // ¹æ¾îÀÚ°¡ ¾Æ¿ì½ºÅÍ½º¶ó¸é ±â¼úÀº ¸ÂÁö ¾Ê´Â´Ù.
    else if ((DefenderZoneLevel & OUSTERS_SAFE_ZONE) && pTargetCreature->isOusters())
        return false;
    // ¿ÏÀü ¾ÈÀüÁö´ë¶ó¸é ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    else if (DefenderZoneLevel & COMPLETE_SAFE_ZONE)
        return false;

    if ((abs(tx - ox) <= Dist) && (abs(ty - oy) <= Dist))
        return true;

    return false;
}


//////////////////////////////////////////////////////////////////////////////
// ½½·¹ÀÌ¾î¿ë ½ºÅ³ÀÇ ½ÇÇà½Ã°£À» °ËÁõÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
bool verifyRunTime(SkillSlot* pSkillSlot) {
    Assert(pSkillSlot != NULL);

    Timeval CurrentTime;
    Timeval LastTime = pSkillSlot->getRunTime();

    getCurrentTime(CurrentTime);

    if (CurrentTime > LastTime)
        return true;

    return false;
}


//////////////////////////////////////////////////////////////////////////////
// ¹ìÆÄÀÌ¾î¿ë ½ºÅ³ÀÇ ½ÇÇà½Ã°£À» °ËÁõÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
bool verifyRunTime(VampireSkillSlot* pSkillSlot) {
    Assert(pSkillSlot != NULL);

    Timeval CurrentTime;
    Timeval LastTime = pSkillSlot->getRunTime();

    getCurrentTime(CurrentTime);

    if (CurrentTime > LastTime)
        return true;

    return false;
}


//////////////////////////////////////////////////////////////////////////////
// ¾Æ¿ì½ºÅÍ½º¿ë ½ºÅ³ÀÇ ½ÇÇà½Ã°£À» °ËÁõÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
bool verifyRunTime(OustersSkillSlot* pSkillSlot) {
    Assert(pSkillSlot != NULL);

    Timeval CurrentTime;
    Timeval LastTime = pSkillSlot->getRunTime();

    getCurrentTime(CurrentTime);

    if (CurrentTime > LastTime)
        return true;

    return false;
}


//////////////////////////////////////////////////////////////////////////////
// °¢ Á¸ÀÇ PK Á¤Ã¥¿¡ µû¶ó, PK°¡ µÇ´À³Ä ¾È µÇ´À³Ä¸¦ Á¤ÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
bool verifyPK(Creature* pAttacker, Creature* pDefender) {
    Zone* pZone = pDefender->getZone();
    Assert(pZone != NULL);

    if (pDefender != NULL && pAttacker != NULL) {
        if (pZone->getZoneID() == 1412 || pZone->getZoneID() == 1413) {
            if (pDefender->isPC() && pAttacker->isPC())
                return false;
        }

        if (pDefender->getCreatureClass() == pAttacker->getCreatureClass() && pAttacker->isPC()) {
            // Á¸ ·¹º§ÀÌ PK°¡ ¾È µÇ´Â °÷ÀÌ¶ó¸é °ø°ÝÇÒ ¼ö ¾ø´Ù.
            if (pZone->getZoneLevel() == NO_PK_ZONE)
                return false;

            // °°Àº ÆÄÆ¼¿ø³¢¸®´Â °ø°ÝÇÒ ¼ö ¾ø´Ù.
            int PartyID1 = pAttacker->getPartyID();
            int PartyID2 = pDefender->getPartyID();
            if (PartyID1 != 0 && PartyID1 == PartyID2)
                return false;

            if (pDefender->isFlag(Effect::EFFECT_CLASS_HAS_FLAG) ||
                pDefender->isFlag(Effect::EFFECT_CLASS_HAS_SWEEPER)) {
                if (pAttacker->isPC() && !dynamic_cast<PlayerCreature*>(pAttacker)->hasEnemy(pDefender->getName()))
                    return false;
            }
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// ±â¼úÀ» »ç¿ëÇÒ ¼ö ÀÖ´Â Á¸ÀÎ°¡?
// (¼¿ÇÁ ±â¼úÀÏ °æ¿ì, Á¸ ·¹º§À» Ã¼Å©ÇÏ´Â ÇÔ¼ö´Ù...)
//////////////////////////////////////////////////////////////////////////////
bool checkZoneLevelToUseSkill(Creature* pCaster) {
    Assert(pCaster != NULL);

    if (pCaster->isFlag(Effect::EFFECT_CLASS_PLEASURE_EXPLOSION))
        return false;

    Zone* pZone = pCaster->getZone();
    Assert(pZone != NULL);

    ZoneCoord_t cx = pCaster->getX();
    ZoneCoord_t cy = pCaster->getY();
    ZoneLevel_t ZoneLevel = pZone->getZoneLevel(cx, cy);

    // ¾ÈÀüÁö´ë¿¡¼­´Â ¼¿ÇÁ ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    if ((ZoneLevel & SAFE_ZONE)) // && pZone->isHolyLand() )
        return false;

    if (pCaster->isFlag(Effect::EFFECT_CLASS_REFINIUM_TICKET))
        return false;

    /*	// ½½·¹ÀÌ¾î ¾ÈÀüÁö´ë¿¡¼­´Â ½½·¹ÀÌ¾î¸¸ÀÌ ±â¼úÀ» »ç¿ëÇÒ ¼ö ÀÖ´Ù.
        if ((ZoneLevel & SLAYER_SAFE_ZONE) && !pCaster->isSlayer())
            return false;
        // ¸¶Âù°¡Áö·Î ¹ìÆÄÀÌ¾î ¾ÈÀüÁö´ë¿¡¼­´Â ¹ìÆÄÀÌ¾î¸¸ÀÌ ±â¼úÀ» »ç¿ëÇÒ ¼ö ÀÖ´Ù.
        else if ((ZoneLevel & VAMPIRE_SAFE_ZONE) && !pCaster->isVampire())
            return false;
        // ÅëÇÕ ¾ÈÀüÁö´ë¿¡¼­´Â ´©±¸µµ ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
        else if (ZoneLevel & COMPLETE_SAFE_ZONE)
            return false;
    */
    return true;
}

//////////////////////////////////////////////////////////////////////////////
// X, Y¿¡ ¼­ ÀÖ´Â Å©¸®ÃÄ°¡ ÀÓÀÇÀÇ ±â¼ú¿¡ ¿µÇâÀ» ¹Þ´ÂÁö Ã¼Å©ÇÏ´Â ÇÔ¼ö´Ù.
//////////////////////////////////////////////////////////////////////////////
bool checkZoneLevelToHitTarget(Creature* pTargetCreature) {
    Assert(pTargetCreature != NULL);

    Zone* pZone = pTargetCreature->getZone();
    Assert(pZone != NULL);

    ZoneCoord_t tx = pTargetCreature->getX();
    ZoneCoord_t ty = pTargetCreature->getY();
    ZoneLevel_t ZoneLevel = pZone->getZoneLevel(tx, ty);

    // ½½·¹ÀÌ¾î ¾ÈÀüÁö´ë¿¡¼­ ½½·¹ÀÌ¾î´Â ±â¼ú¿¡ ¸ÂÁö ¾Ê´Â´Ù.
    if ((ZoneLevel & SLAYER_SAFE_ZONE) && pTargetCreature->isSlayer())
        return false;
    // ¹ìÆÄÀÌ¾î ¾ÈÀüÁö´ë¿¡¼­ ¹ìÆÄÀÌ¾î´Â ±â¼ú¿¡ ¸ÂÁö ¾Ê´Â´Ù.
    else if ((ZoneLevel & VAMPIRE_SAFE_ZONE) && pTargetCreature->isVampire())
        return false;
    // ¾Æ¿ì½ºÅÍÁî ¾ÈÀüÁö´ë¿¡¼­ ¾Æ¿ì½ºÅÍÁî´Â ±â¼ú¿¡ ¸ÂÁö ¾Ê´Â´Ù.
    else if ((ZoneLevel & OUSTERS_SAFE_ZONE) && pTargetCreature->isOusters())
        return false;
    // ÅëÇÕ ¾ÈÀüÁö´ë¿¡¼­´Â ´©±¸µµ ¸ÂÁö ¾Ê´Â´Ù.
    else if (ZoneLevel & COMPLETE_SAFE_ZONE)
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// ¼ºÇâÀ» º¯°æÇÑ´Ù.
// ±â¼úÀ» »ç¿ëÇÏ°Å³ª, PK¸¦ ÇÒ ¶§ »ý±â´Â ¼ºÇâ º¯È­¸¦ °è»êÇÏ´Â ÇÔ¼ö´Ù.
//////////////////////////////////////////////////////////////////////////////
void computeAlignmentChange(Creature* pTargetCreature, Damage_t Damage, Creature* pAttacker, ModifyInfo* pMI,
                            ModifyInfo* pAttackerMI) {
    Assert(pTargetCreature != NULL);

    // PKÁ¸¿¡¼­´Â ¼ºÇâÀÌ º¯ÇÏÁö ¾Ê´Â´Ù.
    if (g_pPKZoneInfoManager->isPKZone(pAttacker->getZoneID()))
        return;

    Zone* pZone = pTargetCreature->getZone();
    Assert(pZone != NULL);

    bool bSameRace = false;

    // °ø°ÝÀÚ°¡ ÀÖ´Ù¸é, °°Àº Á¾Á·Àº ¾Æ´ÑÁö Ã¼Å©ÇÑ´Ù.
    if (pAttacker != NULL) {
        // ÀÌº¥Æ® °æ±âÀå¿¡¼­´Â ¼ºÇâÀÌ ¾È¹Ù²î°Ô µÇ´Â ÄÚµåÀÌ´Ù.
        // ZoneInfo¿¡ ³Ö°í, Zone¿¡¼­ ÀÐÀ» ¼ö ÀÖ°Ô ÇÏ¸é ÁÁ°ÚÁö¸¸,
        // °©ÀÚ±â ¶³¾îÁø ÀÏÀÌ¶ó ±ÍÂú´Ù´Â ÀÌÀ¯·Î ÇÏµå ÄÚµùÀÌ´Ù. - -;
        // 2002.8.21. by sigi
        // int zoneID = pAttacker->getZone()->getZoneID();

        // zoneID==1005 || zoneID==1006)

        bSameRace = isSameRace(pTargetCreature, pAttacker);

        bool bPKOlympic = false;

        switch (pAttacker->getZoneID()) {
        case 1122:
        case 1131:
        case 1132:
        case 1133:
        case 1134:
            bPKOlympic = true;
            break;
        default:
            break;
        }

        // °°Àº Á¾Á·ÀÌ ¾Æ´Ï¸é ¿Ã¸²ÇÈ ±Ý¸Þ´Þ~
        if (!bSameRace && bPKOlympic && pTargetCreature->isPC() && pTargetCreature->isDead() &&
            !GDRLairManager::Instance().isGDRLairZone(pTargetCreature->getZoneID())) {
            PlayerCreature* pAttackPC = dynamic_cast<PlayerCreature*>(pAttacker);
            if (pAttackPC->getLevel() - 10 <= pTargetCreature->getLevel() &&
                !pTargetCreature->isFlag(Effect::EFFECT_CLASS_PK_COUNTED)) {
                addSimpleCreatureEffect(pTargetCreature, Effect::EFFECT_CLASS_PK_COUNTED, 6000, false);
                addOlympicStat(pAttackPC, 2);
            }
        }

        if (pZone->isPKZone())
            return;
    }

    // °°Àº Á¾Á·ÀÌ¶ó¸é ¼ºÇâ¿¡ º¯È­°¡ »ý±æ ¼ö ÀÖ´Ù.
    if (bSameRace) {
        PlayerCreature* pAttackPC = dynamic_cast<PlayerCreature*>(pAttacker);
        PlayerCreature* pTargetPC = dynamic_cast<PlayerCreature*>(pTargetCreature);

        string AttackName = pAttackPC->getName();
        string TargetName = pTargetPC->getName();

        Alignment_t AttackAlignment = pAttackPC->getAlignment();
        Alignment_t TargetAlignment = pTargetPC->getAlignment();

        Alignment_t ModifyAlignment = 0;

        // °¨¼ÒÇÏ´Â °ÍÀÎÁö Áõ°¡ÇÏ´Â °ÍÀÎÁö ¾Ë¾ÆµÐ´Ù.
        bool bdecrease = false;
        if (pTargetPC->isDead()) {
            ModifyAlignment = g_pAlignmentManager->getMultiplier(AttackAlignment, TargetAlignment); // Damage* 2

            if (ModifyAlignment < 0) {
                ModifyAlignment = ModifyAlignment * 10;
                bdecrease = true;
            } else if (ModifyAlignment > 0) {
                // (ÇÇ»ìÀÚ ·¹º§) / (»ìÇØÀÚ ·¹º§) * (±âÁ¸ ¼ºÇâ È¹µæ·®) :// max = (±âÁ¸ ¼ºÇâ È¹µæ·®)

                if (pAttackPC->getLevel() - 10 <= pTargetPC->getLevel() &&
                    !pTargetPC->isFlag(Effect::EFFECT_CLASS_PUNISH_COUNTED)) {
                    addOlympicStat(pAttackPC, 7);
                    addSimpleCreatureEffect(pTargetPC, Effect::EFFECT_CLASS_PUNISH_COUNTED, 6000, false);
                }

                if (pAttackPC->isSlayer()) {
                    Slayer* pAttackSlayer = dynamic_cast<Slayer*>(pAttackPC);
                    Slayer* pTargetSlayer = dynamic_cast<Slayer*>(pTargetPC);

                    SkillLevel_t AttackLevel = pAttackSlayer->getHighestSkillDomainLevel();
                    SkillLevel_t TargetLevel = pTargetSlayer->getHighestSkillDomainLevel();

                    if (AttackLevel > TargetLevel && AttackLevel != 0) {
                        ModifyAlignment = (Alignment_t)(ModifyAlignment * ((float)TargetLevel / (float)AttackLevel));
                    }
                } else if (pAttackPC->isVampire()) {
                    Vampire* pAttackVampire = dynamic_cast<Vampire*>(pAttackPC);
                    Vampire* pTargetVampire = dynamic_cast<Vampire*>(pTargetPC);

                    Level_t AttackLevel = pAttackVampire->getLevel();
                    Level_t TargetLevel = pTargetVampire->getLevel();

                    if (AttackLevel > TargetLevel && AttackLevel != 0) {
                        ModifyAlignment = (Alignment_t)(ModifyAlignment * ((float)TargetLevel / (float)AttackLevel));
                    }
                } else if (pAttackPC->isOusters()) {
                    Ousters* pAttackOusters = dynamic_cast<Ousters*>(pAttackPC);
                    Ousters* pTargetOusters = dynamic_cast<Ousters*>(pTargetPC);

                    Level_t AttackLevel = pAttackOusters->getLevel();
                    Level_t TargetLevel = pTargetOusters->getLevel();

                    if (AttackLevel > TargetLevel && AttackLevel != 0) {
                        ModifyAlignment = (Alignment_t)(ModifyAlignment * ((float)TargetLevel / (float)AttackLevel));
                    }
                }
            }

            pTargetPC->setPK(true);
        }

        Alignment_t ResultAlignment = AttackAlignment + ModifyAlignment;

        ResultAlignment = max(-10000, ResultAlignment);
        ResultAlignment = min(10000, ResultAlignment);

        EffectManager* pAttackEffectManager = pAttackPC->getEffectManager();
        EffectManager* pTargetEffectManager = pTargetPC->getEffectManager();

        // ¼ºÇâ¿¡ °ü°è ¾øÀÌ Á¤´ç¹æÀ§¿¡ ÇØ´çµÇÁö ¾Ê´Â »ç¶÷À» ¶§¸®¸é ¹«Á¶°Ç »ó´ë¹æ¿¡°Ô Á¤´ç¹æÀ§ ±ÇÇÑÀ» ÁØ´Ù.
        if (!pAttackPC->hasEnemy(TargetName) && g_pAlignmentManager->getAlignmentType(TargetAlignment) >= NEUTRAL) {
            GCAddInjuriousCreature gcAddInjuriousCreature;
            gcAddInjuriousCreature.setName(AttackName);
            pTargetPC->getPlayer()->sendPacket(&gcAddInjuriousCreature);

            // °ø°Ý´çÇÏ´Â »ç¶÷¿¡°Ô ¼±°øÀÚ ¸®½ºÆ®¿¡ Ãß°¡ÇÏ°í
            // 5ºÐ µÚ¿¡ »ç¶óÁø´Ù´Â ÀÌÆåÆ®¸¦ ºÙÀÎ´Ù.
            pTargetPC->addEnemy(AttackName);

            EffectEnemyErase* pEffectEnemyErase = new EffectEnemyErase(pTargetPC);
            pEffectEnemyErase->setDeadline(3000);
            pEffectEnemyErase->setEnemyName(AttackName);
            pEffectEnemyErase->create(TargetName);
            pTargetEffectManager->addEffect(pEffectEnemyErase);
        }

        // »ó´ë°¡ ³ª¿¡°Ô Á¤´ç¹æÀ§ÀÇ ´ë»óÀÌ°í »ó´ë¸¦ Á×¿´À» °æ¿ì ÀÌÆåÆ®¸¦ Áö¿öÁØ´Ù.
        if (pAttackPC->hasEnemy(TargetName) && pTargetPC->isDead()) {
            EffectEnemyErase* pAttackerEffect =
                (EffectEnemyErase*)pAttackEffectManager->findEffect(Effect::EFFECT_CLASS_ENEMY_ERASE, TargetName);

            if (pAttackerEffect != NULL) {
                // ¼±°øÀÚ ¸®½ºÆ®¿¡ ÀÖ´Ù´Â ¸»Àº ¼±°øÀÚ¸¦ Áö¿öÁÖ´Â ÀÌÆåÆ®°¡ ¹«Á¶°Ç ÀÖ´Ù´Â ¾ê±âÀÌ´Ù. µû¶ó¼­ NULLÀÌ µÉ ¼ö
                // ¾ø´Ù.
                Assert(pAttackerEffect != NULL);
                Assert(pAttackerEffect->getEffectClass() == Effect::EFFECT_CLASS_ENEMY_ERASE);
                // Áö¿öÁØ´Ù.
                pAttackerEffect->setDeadline(0);
            }
        }

        // ¼±°øÀÚÀÇ ¸®½ºÆ®¿¡ ¹æ¾îÀÚÀÇ ÀÌ¸§ÀÌ ÀÖ°í, ÀÚ½ÅÀÇ ¼ºÇâÀÌ Good ¶Ç´Â Neutral ÀÌ¶ó¸é Á¤´ç¹æÀ§·Î ÀÎÁ¤ÇÏ°í, ¼ºÇâÀÌ
        // ¶³¾îÁöÁö´Â ¾Ê°Ô ÇÑ´Ù.
        if (!(bdecrease && pAttackPC->hasEnemy(TargetName) &&
              g_pAlignmentManager->getAlignmentType(AttackAlignment) >= NEUTRAL)) {
            // ¿Ã¶ó°¡µç ³»·Á°¡µç ¸ÕÀú ¼ÂÆÃÀ» ÇØ ³õ¾Æ¾ß ÇÑ´ç.
            // ¸ÕÀú ¼ÂÆÃÀ» ÇØ ³õ´Â´Ù.
            if (pAttackerMI && ModifyAlignment != 0) {
                if (pAttackPC->isSlayer()) {
                    Slayer* pSlayer = dynamic_cast<Slayer*>(pAttackPC);

                    pAttackerMI->addShortData(MODIFY_ALIGNMENT, ResultAlignment);
                    pSlayer->setAlignment(ResultAlignment);

                    WORD AlignmentSaveCount = pSlayer->getAlignmentSaveCount();
                    if (AlignmentSaveCount > ALIGNMENT_SAVE_PERIOD) {
                        char pField[80];
                        sprintf(pField, "ALIGNMENT=%d", ResultAlignment);
                        pSlayer->tinysave(pField);

                        AlignmentSaveCount = 0;
                    } else
                        AlignmentSaveCount++;

                    pSlayer->setAlignmentSaveCount(AlignmentSaveCount);
                } else if (pAttackPC->isVampire()) {
                    Vampire* pVampire = dynamic_cast<Vampire*>(pAttackPC);

                    pAttackerMI->addShortData(MODIFY_ALIGNMENT, ResultAlignment);
                    pVampire->setAlignment(ResultAlignment);

                    WORD AlignmentSaveCount = pVampire->getAlignmentSaveCount();
                    if (AlignmentSaveCount > ALIGNMENT_SAVE_PERIOD) {
                        char pField[80];
                        sprintf(pField, "ALIGNMENT=%d", ResultAlignment);
                        pVampire->tinysave(pField);

                        AlignmentSaveCount = 0;
                    } else
                        AlignmentSaveCount++;

                    pVampire->setAlignmentSaveCount(AlignmentSaveCount);
                } else if (pAttackPC->isOusters()) {
                    Ousters* pOusters = dynamic_cast<Ousters*>(pAttackPC);

                    pAttackerMI->addShortData(MODIFY_ALIGNMENT, ResultAlignment);
                    pOusters->setAlignment(ResultAlignment);

                    WORD AlignmentSaveCount = pOusters->getAlignmentSaveCount();
                    if (AlignmentSaveCount > ALIGNMENT_SAVE_PERIOD) {
                        char pField[80];
                        sprintf(pField, "ALIGNMENT=%d", ResultAlignment);
                        pOusters->tinysave(pField);

                        AlignmentSaveCount = 0;
                    } else
                        AlignmentSaveCount++;

                    pOusters->setAlignmentSaveCount(AlignmentSaveCount);
                }
            }

            // ¼ºÇâÀÌ °¨¼ÒµÉ¶§ ÀÌÆåÆ®¸¦ ÅëÇÏ¿© 10¹è¸¦ ÁÙÀÎ´ÙÀ½ ¼­¼­È÷ È¸º¹½ÃÅ°´Â ¹æ¹ýÀÌ´Ù.
            if (bdecrease) {
                // ¸¸¾à ¹æ¾îÀÚÀÇ ¼±°øÀÚ ¸®½ºÆ®¿¡ ³» ÀÌ¸§ÀÌ ÀÖ´Ù¸é, °ø°ÝÀÚ´Â ³ª»Û³ÑÀÌ´Ù.
                // ¼±°øÀÚÀÇ ¸®½ºÆ®¿¡ ÀÌ¸§ÀÌ ÀÖ´Ù´Â °ÍÀº ¾ÆÁ÷ ÀÌÆåÆ®°¡ ºÙ¾îÀÖ´Ù´Â ¾ê±âÀÌ´Ù.
                // ¼ºÇâÀ» È¸º¹½ÃÅ°´Â ÀÌÆåÆ®´Â ÇÑ¼ø°£¿¡ ÇÏ³ª ÀÌÇÏ·Î Á¸ÀçÇÒ ¼ö ÀÖ´Ù. Áßº¹µÇÁö ¾Ê´Â´Ù.
                EffectAlignmentRecovery* pAttackerEffect =
                    (EffectAlignmentRecovery*)pAttackEffectManager->findEffect(Effect::EFFECT_CLASS_ALIGNMENT_RECOVERY);
                // ÀÌÆåÆ®¸¦ ¹Þ¾Æ¿Í¼­ °ªÀ» ´Ù½Ã ¼ÂÆÃÇÑ´Ù.
                // ¾Æ¸¶µµ ¼±°øÀÚÀÇ ÀÌ¸§¿¡ ³»°¡ ÀÖÀ¸¹Ç·Î ÀÌÆåÆ®´Â ÇÊ½Ã ÀÖÀ» °ÍÀÌ´Ù.
                // ÇÏ³ª µ¿±â°¡ ±úÁú ¼ö ÀÖ´Â »óÈ²ÀÌ ±úÁú ¼ö ÀÖÀ¸¹Ç·Î, µ¥µå¶óÀÎÀ» ¾à°£ ±æ°Ô Àâµµ·Ï ÇÑ´Ù.

                if (pAttackerEffect != NULL) {
                    // ¾ó¸¶³ª È¸º¹½ÃÅ³ °Í °ÍÀÎ°¡?
                    Alignment_t Amount = abs(ModifyAlignment / 10 * 9);

                    // ¾ó¸¶¾¿ È¸º¹½ÃÅ³ °ÍÀÎ°¡? 10¾¿
                    Alignment_t Quantity = 10;

                    // È¸º¹ ÁÖ±â´Â ¾ó¸¶ÀÎ°¡? 30ÃÊ
                    int DelayProvider = 300;

                    // ¸î¹ø È¸º¹½ÃÅ³ °ÍÀÎ°¡?
                    double temp = (double)((double)Amount / (double)Quantity);
                    int Period = (uint)floor(temp);

                    // ´Ù È¸º¹½ÃÅ°´Âµ¥ °É¸®´Â ½Ã°£Àº ¾ó¸¶ÀÎ°¡?
                    Turn_t Deadline = Period * DelayProvider;

                    pAttackerEffect->setQuantity(Quantity);
                    pAttackerEffect->setPeriod(Period);
                    pAttackerEffect->setDeadline(Deadline);
                    pAttackerEffect->setDelay(DelayProvider);
                } else {
                    // ¾ø´Ù¸é ÃÖÃÊ·Î ¼±°øÇÏ´Â °ÍÀÌ´Ù »õ ÀÌÆåÆ®¸¦ »ý¼ºÇØ¼­ ºÙÀÌ°í 5ºÐ°£ Áö¼Ó µÉ °ÍÀÌ´Ù.
                    // ¹æ¾îÀÚ¿¡°Ô »ç¶óÁö´Â ÀÌÆåÆ®¸¦ ºÙ¿©¾ß ÇÔÀ» ÀØÁö ¸»¾Æ¾ß ÇÑ´Ù.
                    // »ç¶óÁö´Â °ÍÀº »ó´ëÀÇ ÀÌÆåÆ® ¸Þ´ÏÁ®¿¡ ¼ÓÇØÀÖ´Ù.
                    // 30ÃÊ¸¶´Ù 10¾¿ ¼ºÇâÀ» È¸º¹½ÃÅ°´Â ÀÌÆåÆ®¸¦ °ø°ÝÀÚ¿¡°Ô ºÙÀÎ´Ù.

                    // ¾ó¸¶³ª È¸º¹½ÃÅ³ °Í °ÍÀÎ°¡?
                    Alignment_t Amount = abs(ModifyAlignment / 10 * 9);

                    // ¾ó¸¶¾¿ È¸º¹½ÃÅ³ °ÍÀÎ°¡? 10¾¿
                    Alignment_t Quantity = 10;

                    // È¸º¹ ÁÖ±â´Â ¾ó¸¶ÀÎ°¡? 30ÃÊ
                    int DelayProvider = 300;

                    // ¸î¹ø È¸º¹½ÃÅ³ °ÍÀÎ°¡?
                    double temp = (double)((double)Amount / (double)Quantity);
                    int Period = (uint)floor(temp);

                    // ´Ù È¸º¹½ÃÅ°´Âµ¥ °É¸®´Â ½Ã°£Àº ¾ó¸¶ÀÎ°¡?
                    Turn_t Deadline = Period * DelayProvider;

                    // ¸ÕÀú È¸º¹ ÀÌÆåÆ®¸¦ ºÙÀÎ´Ù.
                    EffectAlignmentRecovery* pEffectAlignmentRecovery = new EffectAlignmentRecovery();

                    pEffectAlignmentRecovery->setTarget(pAttackPC);
                    pEffectAlignmentRecovery->setDeadline(Deadline);
                    pEffectAlignmentRecovery->setDelay(DelayProvider);
                    pEffectAlignmentRecovery->setNextTime(DelayProvider);
                    pEffectAlignmentRecovery->setQuantity(Quantity);
                    pEffectAlignmentRecovery->setPeriod(Period);

                    pAttackEffectManager->addEffect(pEffectAlignmentRecovery);
                }

                // ¹æ¾îÀÚ¿¡°Ô ºÙ¾îÀÖ´Â ÀÌÆåÆ®ÀÇ µ¥µå¶óÀÎÀ» ´Ù½Ã ¼ÂÆÃ ÇØ ÁÖ¾î¾ß ÇÑ´Ù.
                EffectEnemyErase* pDefenderEffect =
                    (EffectEnemyErase*)pTargetEffectManager->findEffect(Effect::EFFECT_CLASS_ENEMY_ERASE, AttackName);

                if (pDefenderEffect != NULL) {
                    // ¼±°øÀÚ ¸®½ºÆ®¿¡ ÀÖ´Ù´Â ¸»Àº ¼±°øÀÚ¸¦ Áö¿öÁÖ´Â ÀÌÆåÆ®°¡ ¹«Á¶°Ç ÀÖ´Ù´Â ¾ê±âÀÌ´Ù. µû¶ó¼­ NULLÀÌ µÉ
                    // ¼ö ¾ø´Ù.
                    Assert(pDefenderEffect != NULL);
                    Assert(pDefenderEffect->getEffectClass() == Effect::EFFECT_CLASS_ENEMY_ERASE);
                    // 5ºÐÀ¸·Î ¼ÂÆÃ
                    pDefenderEffect->setDeadline(36000);
                    pDefenderEffect->save(TargetName);
                }
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////////
// ½½·¹ÀÌ¾î ¹× ¹ìÆÄÀÌ¾î°¡ ¸÷À» Á×ÀÏ ¶§ ¼ºÇâÀ» ¾à°£¾¿ È¸º¹½ÃÅ²´Ù.
//////////////////////////////////////////////////////////////////////////////
void increaseAlignment(Creature* pCreature, Creature* pEnemy, ModifyInfo& mi) {
    Assert(pCreature != NULL);
    Assert(pEnemy != NULL);

    // PKÁ¸¿¡¼­´Â ¼ºÇâÀ» ¾È ¿Ã·ÁÁØ´Ù.
    if (g_pPKZoneInfoManager->isPKZone(pCreature->getZoneID()))
        return;

    // ´ÙÀÌ³ª¹Í Á¸ ¾È¿¡¼­´Â ¼ºÇâÀ» ¾È ¿Ã·ÁÁØ´Ù.
    if (pCreature->getZone() != NULL && pCreature->getZone()->isDynamicZone())
        return;

    // ¸ó½ºÅÍ°¡ ¾ÆÁ÷ »ì¾ÆÀÖÀ» °æ¿ì¿¡´Â ¼ºÇâÀÌ º¯È­µÇÁö ¾Ê´Â´Ù.
    if (!pEnemy->isDead())
        return;

    // ÀûÀÌ NPCÀÌ°Å³ª, µ¿Á·³¢¸® °ø°ÝÇÏ´Â °æ¿ì¿¡´Â ¼ºÇâÀ» Áõ°¡½ÃÅ°Áö ¾Ê´Â´Ù.
    if (pEnemy->isNPC())
        return;
    if (pCreature->isSlayer() && pEnemy->isSlayer())
        return;
    if (pCreature->isVampire() && pEnemy->isVampire())
        return;
    if (pCreature->isOusters() && pEnemy->isOusters())
        return;

    Alignment_t OldAlignValue = 0;
    Alignment_t NewAlignValue = 0;

    if (pCreature->isSlayer()) {
        Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);

        // ÇöÀç ¼ºÇâ °ªÀ» ÀÐ¾î¿Â´Ù.
        OldAlignValue = pSlayer->getAlignment();

        // ¼ºÇâÀÌ 0ÀÌ»óÀÎ °æ¿ì¿¡´Â ¸ó½ºÅÍ¸¦ Á×¿©µµ ¼ºÇâÀÇ º¯È­°¡ ¾ø´Ù.
        if (OldAlignValue > 0)
            return;

        // ¿Ã¶ó°¥ ¼ºÇâÀÇ ¼öÄ¡¸¦ °è»êÇÑ´Ù.
        if (pEnemy->isMonster()) {
            Monster* pMonster = dynamic_cast<Monster*>(pEnemy);
            Assert(pMonster != NULL);
            NewAlignValue = max(0, (int)(pMonster->getLevel() / 10));
        } else if (pEnemy->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pEnemy);
            Assert(pVampire != NULL);
            NewAlignValue = max(0, (int)(pVampire->getLevel() / 5));
        } else if (pEnemy->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pEnemy);
            Assert(pOusters != NULL);
            NewAlignValue = max(0, (int)(pOusters->getLevel() / 5));
        }

        NewAlignValue = OldAlignValue + NewAlignValue;

        if (OldAlignValue != NewAlignValue) {
            // ÆÐÅ¶¿¡´Ù ¼ºÇâÀÌ ¹Ù²î¾ú´Ù°í ¾Ë·ÁÁØ´Ù.
            mi.addShortData(MODIFY_ALIGNMENT, NewAlignValue);

            WORD AlignmentSaveCount = pSlayer->getAlignmentSaveCount();
            if (AlignmentSaveCount > ALIGNMENT_SAVE_PERIOD) {
                pSlayer->saveAlignment(NewAlignValue);
                AlignmentSaveCount = 0;
            } else {
                pSlayer->setAlignment(NewAlignValue);
                AlignmentSaveCount++;
            }

            pSlayer->setAlignmentSaveCount(AlignmentSaveCount);
        }
    } else if (pCreature->isVampire()) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);

        // ÇöÀç ¼ºÇâ °ªÀ» ÀÐ¾î¿Â´Ù.
        OldAlignValue = pVampire->getAlignment();

        // ¼ºÇâÀÌ 0ÀÌ»óÀÎ °æ¿ì¿¡´Â ¸ó½ºÅÍ¸¦ Á×¿©µµ ¼ºÇâÀÇ º¯È­°¡ ¾ø´Ù.
        if (OldAlignValue > 0)
            return;

        // ¿Ã¶ó°¥ ¼ºÇâÀÇ ¼öÄ¡¸¦ °è»êÇÑ´Ù.
        NewAlignValue = 0;
        if (pEnemy->isMonster()) {
            Monster* pMonster = dynamic_cast<Monster*>(pEnemy);
            Assert(pMonster != NULL);
            NewAlignValue = max(0, (int)(pMonster->getLevel() / 10));
        } else if (pEnemy->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pEnemy);
            Assert(pSlayer != NULL);
            NewAlignValue = max(0, (int)(pSlayer->getSTR(ATTR_BASIC) + pSlayer->getDEX(ATTR_BASIC) +
                                         pSlayer->getINT(ATTR_BASIC) + pSlayer->getSkillDomainLevelSum()) /
                                       5);
        } else if (pEnemy->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pEnemy);
            Assert(pOusters != NULL);
            NewAlignValue = max(0, (int)(pOusters->getLevel() / 5));
        }

        NewAlignValue = OldAlignValue + NewAlignValue;

        if (OldAlignValue != NewAlignValue) {
            // ÆÐÅ¶¿¡´Ù ¼ºÇâÀÌ ¹Ù²î¾ú´Ù°í ¾Ë·ÁÁØ´Ù.
            mi.addShortData(MODIFY_ALIGNMENT, NewAlignValue);

            WORD AlignmentSaveCount = pVampire->getAlignmentSaveCount();
            if (AlignmentSaveCount > ALIGNMENT_SAVE_PERIOD) {
                pVampire->saveAlignment(NewAlignValue);
                AlignmentSaveCount = 0;
            } else {
                pVampire->setAlignment(NewAlignValue);
                AlignmentSaveCount++;
            }
        }
    } else if (pCreature->isOusters()) {
        Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);

        // ÇöÀç ¼ºÇâ °ªÀ» ÀÐ¾î¿Â´Ù.
        OldAlignValue = pOusters->getAlignment();

        // ¼ºÇâÀÌ 0ÀÌ»óÀÎ °æ¿ì¿¡´Â ¸ó½ºÅÍ¸¦ Á×¿©µµ ¼ºÇâÀÇ º¯È­°¡ ¾ø´Ù.
        if (OldAlignValue > 0)
            return;

        // ¿Ã¶ó°¥ ¼ºÇâÀÇ ¼öÄ¡¸¦ °è»êÇÑ´Ù.
        NewAlignValue = 0;
        if (pEnemy->isMonster()) {
            Monster* pMonster = dynamic_cast<Monster*>(pEnemy);
            Assert(pMonster != NULL);
            NewAlignValue = max(0, (int)(pMonster->getLevel() / 10));
        } else if (pEnemy->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pEnemy);
            Assert(pSlayer != NULL);
            NewAlignValue = max(0, (int)(pSlayer->getSTR(ATTR_BASIC) + pSlayer->getDEX(ATTR_BASIC) +
                                         pSlayer->getINT(ATTR_BASIC) + pSlayer->getSkillDomainLevelSum()) /
                                       5);
        } else if (pEnemy->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pEnemy);
            Assert(pVampire != NULL);
            NewAlignValue = max(0, (int)(pVampire->getLevel() / 5));
        }

        NewAlignValue = OldAlignValue + NewAlignValue;

        if (OldAlignValue != NewAlignValue) {
            // ÆÐÅ¶¿¡´Ù ¼ºÇâÀÌ ¹Ù²î¾ú´Ù°í ¾Ë·ÁÁØ´Ù.
            mi.addShortData(MODIFY_ALIGNMENT, NewAlignValue);

            WORD AlignmentSaveCount = pOusters->getAlignmentSaveCount();
            if (AlignmentSaveCount > ALIGNMENT_SAVE_PERIOD) {
                pOusters->saveAlignment(NewAlignValue);
                AlignmentSaveCount = 0;
            } else {
                pOusters->setAlignment(NewAlignValue);
                AlignmentSaveCount++;
            }
        }
    }

    // ¼ºÇâ ´Ü°è°¡ ¹Ù²î¸é ´Ù¸¥ »ç¶÷µé¿¡°Ôµµ ¾Ë·ÁÁà¾ß ÇÑ´Ù.  by sigi. 2002.1.6
    Alignment beforeAlignment = g_pAlignmentManager->getAlignmentType(OldAlignValue);
    Alignment afterAlignment = g_pAlignmentManager->getAlignmentType(NewAlignValue);

    if (beforeAlignment != afterAlignment) {
        GCOtherModifyInfo gcOtherModifyInfo;
        gcOtherModifyInfo.setObjectID(pCreature->getObjectID());
        gcOtherModifyInfo.addShortData(MODIFY_ALIGNMENT, NewAlignValue);

        Zone* pZone = pCreature->getZone();
        Assert(pZone != NULL);
        pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcOtherModifyInfo, pCreature);
    }
}


//////////////////////////////////////////////////////////////////////////////
// ÆÄÆ¼ °ü·Ã ½½·¹ÀÌ¾î °æÇèÄ¡ °è»ê ÇÔ¼ö
//////////////////////////////////////////////////////////////////////////////
void shareAttrExp(Slayer* pSlayer, Damage_t Damage, BYTE STRMultiplier, BYTE DEXMultiplier, BYTE INTMultiplier,
                  ModifyInfo& _ModifyInfo) {
    Assert(pSlayer != NULL);

    // PKÁ¸ ¾È¿¡¼­´Â °æÇèÄ¡¸¦ ÁÖÁö ¾Ê´Â´Ù.
    if (g_pPKZoneInfoManager->isPKZone(pSlayer->getZoneID()))
        return;

    // ´ÙÀÌ³ª¹Í Á¸ ¾È¿¡¼­´Â °æÇèÄ¡¸¦ ÁÖÁö ¾Ê´Â´Ù.
    if (pSlayer->getZone() != NULL && pSlayer->getZone()->isDynamicZone())
        return;

    // À¯·áÈ­ Á¸¿¡¼­´Â °æÇèÄ¡¸¦ ´õ ¹Þ´Â´Ù.
    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pSlayer->getPlayer());
    Assert(pGamePlayer != NULL);

    if (pGamePlayer->isPremiumPlay() ||
        pGamePlayer->isFamilyFreePass()) // pZone->isPayPlay() || pZone->isPremiumZone() )
    {
        Damage = getPercentValue(Damage, g_pVariableManager->getPremiumExpBonusPercent());
        STRMultiplier = getPercentValue(STRMultiplier, g_pVariableManager->getPremiumExpBonusPercent());
        DEXMultiplier = getPercentValue(DEXMultiplier, g_pVariableManager->getPremiumExpBonusPercent());
        INTMultiplier = getPercentValue(INTMultiplier, g_pVariableManager->getPremiumExpBonusPercent());
    }

    if (pGamePlayer->isPCRoomPlay()) {
        Damage = getPercentValue(Damage, g_pVariableManager->getPCRoomExpBonusPercent());
        STRMultiplier = getPercentValue(STRMultiplier, g_pVariableManager->getPCRoomExpBonusPercent());
        DEXMultiplier = getPercentValue(DEXMultiplier, g_pVariableManager->getPCRoomExpBonusPercent());
        INTMultiplier = getPercentValue(INTMultiplier, g_pVariableManager->getPCRoomExpBonusPercent());
    }

    int PartyID = pSlayer->getPartyID();
    if (PartyID != 0) {
        // ÆÄÆ¼¿¡ °¡ÀÔµÇ¾î ÀÖ´Ù¸é ·ÎÄÃ ÆÄÆ¼ ¸Å´ÏÀú¸¦ ÅëÇØ
        // ÁÖÀ§ÀÇ ÆÄÆ¼¿øµé°ú °æÇèÄ¡¸¦ °øÀ¯ÇÑ´Ù.
        LocalPartyManager* pLPM = pSlayer->getLocalPartyManager();
        Assert(pLPM != NULL);
        pLPM->shareAttrExp(PartyID, pSlayer, Damage, STRMultiplier, DEXMultiplier, INTMultiplier, _ModifyInfo);
    } else {
        // ÆÄÆ¼¿¡ °¡ÀÔµÇ¾îÀÖÁö ¾Ê´Ù¸é È¥ÀÚ ¿Ã¶ó°£´Ù.
        divideAttrExp(pSlayer, Damage, STRMultiplier, DEXMultiplier, INTMultiplier, _ModifyInfo);
    }
}

//////////////////////////////////////////////////////////////////////////////
// ÆÄÆ¼ °ü·Ã ¹ìÆÄÀÌ¾î °æÇèÄ¡ °è»ê ÇÔ¼ö
//////////////////////////////////////////////////////////////////////////////
void shareVampExp(Vampire* pVampire, Exp_t Point, ModifyInfo& _ModifyInfo) {
    Assert(pVampire != NULL);
    if (Point <= 0)
        return;

    // PKÁ¸ ¾È¿¡¼­´Â °æÇèÄ¡¸¦ ¹ÞÁö ¾Ê´Â´Ù.
    if (g_pPKZoneInfoManager->isPKZone(pVampire->getZoneID()))
        return;

    // ´ÙÀÌ³ª¹Í Á¸ ¾È¿¡¼­´Â °æÇèÄ¡¸¦ ¾È ¿Ã·ÁÁØ´Ù.
    if (pVampire->getZone() != NULL && pVampire->getZone()->isDynamicZone())
        return;

    // À¯·áÈ­ Á¸¿¡¼­´Â °æÇèÄ¡¸¦ ´õ ¹Þ´Â´Ù.
    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pVampire->getPlayer());
    Assert(pGamePlayer != NULL);

    if (pGamePlayer->isPremiumPlay() ||
        pGamePlayer->isFamilyFreePass()) // pZone->isPayPlay() || pZone->isPremiumZone() )
    {
        Point = getPercentValue(Point, g_pVariableManager->getPremiumExpBonusPercent());
    }

    if (pGamePlayer->isPCRoomPlay()) // pZone->isPayPlay() || pZone->isPremiumZone() )
    {
        Point = getPercentValue(Point, g_pVariableManager->getPCRoomExpBonusPercent());
    }

    int PartyID = pVampire->getPartyID();
    if (PartyID != 0) {
        // ÆÄÆ¼¿¡ °¡ÀÔµÇ¾î ÀÖ´Ù¸é ·ÎÄÃ ÆÄÆ¼ ¸Å´ÏÀú¸¦ ÅëÇØ
        // ÁÖÀ§ÀÇ ÆÄÆ¼¿øµé°ú °æÇèÄ¡¸¦ °øÀ¯ÇÑ´Ù.
        LocalPartyManager* pLPM = pVampire->getLocalPartyManager();
        Assert(pLPM != NULL);
        pLPM->shareVampireExp(PartyID, pVampire, Point, _ModifyInfo);
    } else {
        // ÆÄÆ¼¿¡ °¡ÀÔµÇ¾îÀÖÁö ¾Ê´Ù¸é È¥ÀÚ ¿Ã¶ó°£´Ù.
        increaseVampExp(pVampire, Point, _ModifyInfo);
    }
}

//////////////////////////////////////////////////////////////////////////////
// ÆÄÆ¼ °ü·Ã ¾Æ¿ì½ºÅÍ½º °æÇèÄ¡ °è»ê ÇÔ¼ö
//////////////////////////////////////////////////////////////////////////////
void shareOustersExp(Ousters* pOusters, Exp_t Point, ModifyInfo& _ModifyInfo) {
    Assert(pOusters != NULL);
    if (Point <= 0)
        return;

    // PKÁ¸ ¾È¿¡¼­´Â °æÇèÄ¡¸¦ ¹ÞÁö ¾Ê´Â´Ù.
    if (g_pPKZoneInfoManager->isPKZone(pOusters->getZoneID()))
        return;

    // ´ÙÀÌ³ª¹Í Á¸ ¾È¿¡¼­´Â °æÇèÄ¡¸¦ ¾È ¿Ã·ÁÁØ´Ù.
    if (pOusters->getZone() != NULL && pOusters->getZone()->isDynamicZone())
        return;

    // À¯·áÈ­ Á¸¿¡¼­´Â °æÇèÄ¡¸¦ ´õ ¹Þ´Â´Ù.
    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pOusters->getPlayer());
    Assert(pGamePlayer != NULL);

    if (pGamePlayer->isPremiumPlay() || pGamePlayer->isFamilyFreePass()) {
        Point = getPercentValue(Point, g_pVariableManager->getPremiumExpBonusPercent());
    }

    if (pGamePlayer->isPCRoomPlay()) {
        Point = getPercentValue(Point, g_pVariableManager->getPCRoomExpBonusPercent());
    }

    int PartyID = pOusters->getPartyID();
    if (PartyID != 0) {
        // ÆÄÆ¼¿¡ °¡ÀÔµÇ¾î ÀÖ´Ù¸é ·ÎÄÃ ÆÄÆ¼ ¸Å´ÏÀú¸¦ ÅëÇØ
        // ÁÖÀ§ÀÇ ÆÄÆ¼¿øµé°ú °æÇèÄ¡¸¦ °øÀ¯ÇÑ´Ù.
        LocalPartyManager* pLPM = pOusters->getLocalPartyManager();
        Assert(pLPM != NULL);
        pLPM->shareOustersExp(PartyID, pOusters, Point, _ModifyInfo);
    } else {
        // ÆÄÆ¼¿¡ °¡ÀÔµÇ¾îÀÖÁö ¾Ê´Ù¸é È¥ÀÚ ¿Ã¶ó°£´Ù.
        increaseOustersExp(pOusters, Point, _ModifyInfo);
    }
}

/*void decreaseSTR(Slayer* pSlayer)
{
    StringStream  msg1;

    Attr_t CurSTR = pSlayer->getSTR( ATTR_BASIC );

    // exp level°ú ´É·ÂÄ¡¸¦ ¿Ã·ÁÁØ´Ù.
    CurSTR--;
    pSlayer->setSTR(CurSTR, ATTR_BASIC);
    //_ModifyInfo.addLongData(MODIFY_BASIC_STR, CurSTR);

    // ´ÙÀ½ ·¹º§ÀÇ STRInfo¸¦ ¹Þ¾Æ¿Â´Ù.
    STRBalanceInfo* pAfterSTRInfo = g_pSTRBalanceInfoManager->getSTRBalanceInfo(CurSTR);
    // ÀÌÀü ·¹º§ÀÇ STRInfo¸¦ ¹Þ¾Æ¿Â´Ù.
//	STRBalanceInfo* pBeforeSTRInfo = g_pSTRBalanceInfoManager->getSTRBalanceInfo(CurSTR-1);

    // »õ·Î¿î ¸ñÇ¥ °æÇèÄ¡¸¦ ¼ÂÆÃÇØ Áà¾ß ÇÑ´Ù.
    Exp_t NewGoalExp = pAfterSTRInfo->getGoalExp();
//	Exp_t NewExp = pBeforeSTRInfo->getAccumExp();
    pSlayer->setSTRGoalExp(NewGoalExp);
//	pSlayer->setSTRExp(NewExp);

    // DB¿¡ ¿Ã¶ó°£ ´É·ÂÄ¡¸¦ ÀúÀåÇÑ´Ù.
    msg1 << "STR = " << (int)CurSTR << ", STRGoalExp = " << NewGoalExp;

    pSlayer->tinysave(msg1.toString());

//	cout << "ÈûÀ» ³·Ãä´Ï´Ù." << endl;
}

void decreaseINT(Slayer* pSlayer)
{
    StringStream  msg1;

    Attr_t CurINT = pSlayer->getINT( ATTR_BASIC );

    // exp level°ú ´É·ÂÄ¡¸¦ ¿Ã·ÁÁØ´Ù.
    CurINT--;
    pSlayer->setINT(CurINT, ATTR_BASIC);
    //_ModifyInfo.addLongData(MODIFY_BASIC_INT, CurINT);

    // ´ÙÀ½ ·¹º§ÀÇ INTInfo¸¦ ¹Þ¾Æ¿Â´Ù.
    INTBalanceInfo* pAfterINTInfo = g_pINTBalanceInfoManager->getINTBalanceInfo(CurINT);
    // ÀÌÀü ·¹º§ÀÇ INTInfo¸¦ ¹Þ¾Æ¿Â´Ù.
//	INTBalanceInfo* pBeforeINTInfo = g_pINTBalanceInfoManager->getINTBalanceInfo(CurINT-1);

    // »õ·Î¿î ¸ñÇ¥ °æÇèÄ¡¸¦ ¼ÂÆÃÇØ Áà¾ß ÇÑ´Ù.
    Exp_t NewGoalExp = pAfterINTInfo->getGoalExp();
//	Exp_t NewExp = pBeforeINTInfo->getAccumExp();
    pSlayer->setINTGoalExp(NewGoalExp);
//	pSlayer->setINTExp(NewExp);

    // DB¿¡ ¿Ã¶ó°£ ´É·ÂÄ¡¸¦ ÀúÀåÇÑ´Ù.
    msg1 << "INTE = " << (int)CurINT << ", INTGoalExp = " << NewGoalExp;

    pSlayer->tinysave(msg1.toString());

//	cout << "ÀÎÆ®¸¦ ³·Ãä´Ï´Ù." << endl;
}

void decreaseDEX(Slayer* pSlayer)
{
    StringStream  msg1;

    Attr_t CurDEX = pSlayer->getDEX( ATTR_BASIC );

    // exp level°ú ´É·ÂÄ¡¸¦ ¿Ã·ÁÁØ´Ù.
    CurDEX--;
    pSlayer->setDEX(CurDEX, ATTR_BASIC);
    //_ModifyInfo.addLongData(MODIFY_BASIC_DEX, CurDEX);

    // ´ÙÀ½ ·¹º§ÀÇ DEXInfo¸¦ ¹Þ¾Æ¿Â´Ù.
    DEXBalanceInfo* pAfterDEXInfo = g_pDEXBalanceInfoManager->getDEXBalanceInfo(CurDEX);
    // ÀÌÀü ·¹º§ÀÇ DEXInfo¸¦ ¹Þ¾Æ¿Â´Ù.
//	DEXBalanceInfo* pBeforeDEXInfo = g_pDEXBalanceInfoManager->getDEXBalanceInfo(CurDEX-1);

    // »õ·Î¿î ¸ñÇ¥ °æÇèÄ¡¸¦ ¼ÂÆÃÇØ Áà¾ß ÇÑ´Ù.
    Exp_t NewGoalExp = pAfterDEXInfo->getGoalExp();
//	Exp_t NewExp = pBeforeDEXInfo->getAccumExp();
    pSlayer->setDEXGoalExp(NewGoalExp);
//	pSlayer->setDEXExp(NewExp);

    // DB¿¡ ¿Ã¶ó°£ ´É·ÂÄ¡¸¦ ÀúÀåÇÑ´Ù.
    msg1 << "DEX = " << (int)CurDEX << ", DEXGoalExp = " << NewGoalExp;

    pSlayer->tinysave(msg1.toString());

//	cout << "µ¦½º¸¦ ³·Ãä´Ï´Ù." << endl;
}*/

//////////////////////////////////////////////////////////////////////////////
// ½½·¹ÀÌ¾î ´É·ÂÄ¡ (STR, DEX, INT) °æÇèÄ¡¸¦ °è»êÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
void divideAttrExp(Slayer* pSlayer, Damage_t Damage, BYTE STRMultiplier, BYTE DEXMultiplier, BYTE INTMultiplier,
                   ModifyInfo& _ModifyInfo, int numPartyMember) {
    Assert(pSlayer != NULL);

    // STR Æ÷ÀÎÆ®°¡ Á¦ÀÏ Å©´Ù.
    if (STRMultiplier > DEXMultiplier && STRMultiplier > INTMultiplier) {
        pSlayer->divideAttrExp(ATTR_KIND_STR, Damage, _ModifyInfo);
        // DEX Æ÷ÀÎÆ®°¡ Á¦ÀÏ Å©´Ù.
    } else if (DEXMultiplier > STRMultiplier && DEXMultiplier > INTMultiplier) {
        pSlayer->divideAttrExp(ATTR_KIND_DEX, Damage, _ModifyInfo);
        // INT Æ÷ÀÎÆ®°¡ Á¦ÀÏ Å©´Ù.
    } else if (INTMultiplier > STRMultiplier && INTMultiplier > DEXMultiplier) {
        pSlayer->divideAttrExp(ATTR_KIND_INT, Damage, _ModifyInfo);
    }

    return;

    /*	SkillLevel_t	MaxDomainLevel	= pSlayer->getHighestSkillDomainLevel();
        Attr_t			TotalAttr		= pSlayer->getTotalAttr( ATTR_BASIC );
        Attr_t			TotalAttrBound		= 0;		// ´É·ÂÄ¡ ÃÑÇÕ Á¦ÇÑ
        Attr_t			AttrBound			= 0;		// ´ÜÀÏ ´É·ÂÄ¡ Á¦ÇÑ
        Attr_t			OneAttrExpBound		= 0;		// ÇÑ °³ÀÇ ´É·ÂÄ¡¿¡¸¸ °æÇèÄ¡ ÁÖ´Â ´É·ÂÄ¡ ÃÑÇÕ °æ°è°ª

        // ½½·¹ÀÌ¾î ´É·ÂÄ¡´Â µµ¸ÞÀÎ ·¹º§ 100ÀÌÀü¿¡´Â ÃÑÇÕ 300À¸·Î Á¦ÇÑ µÈ´Ù.(±âÁ¸Ã³·³ 50, 200, 50 À¸·Î..)¶ÇÇÑ ±× ÀÌÈÄÀÇ
       °æÇèÄ¡´Â ´©ÀûµÇÁö ¾Ê´Â´Ù.
        // ±×¸®°í µµ¸ÞÀÎ ·¹º§ ÀÌ 100À» ³Ñ¾î¼­¸é ´Ù½Ã ´É·ÂÄ¡ °æÇèÄ¡°¡ ´©ÀûµÇ¾î ´É·ÂÄ¡°¡ ¿Ã¶ó°¡±â ½ÃÀÛÇÑ´Ù.
        // µµ¸ÞÀÎ ·¹º§ÀÌ 100 ¾Æ·¡·Î µµ·Î ¶³¾îÁ³¾îµµ ´É·ÂÄ¡ ÃÑÇÕÀÌ 300À» ³Ñ¾úÀ» °æ¿ì 300ÀÇ Á¦ÇÑÀ» ¹ÞÁö ¾Ê´Â´Ù.

        if ( MaxDomainLevel <= SLAYER_BOUND_LEVEL && TotalAttr <= SLAYER_BOUND_ATTR_SUM )
        {
            TotalAttrBound	= SLAYER_BOUND_ATTR_SUM;		// 300
            AttrBound		= SLAYER_BOUND_ATTR;			// 200
            OneAttrExpBound	= SLAYER_BOUND_ONE_EXP_ATTR;	// 200
        }
        else
        {
            TotalAttrBound	= SLAYER_MAX_ATTR_SUM;			// 435
            AttrBound		= SLAYER_MAX_ATTR;				// 295
            OneAttrExpBound	= SLAYER_ONE_EXP_ATTR;			// 400
        }

        // ÇöÀçÀÇ ½½·¹ÀÌ¾î ´É·ÂÄ¡¸¦ ÀúÀåÇÑ´Ù.
        SLAYER_RECORD prev;
        pSlayer->getSlayerRecord(prev);

        // ½Ã°£´ë¿¡ µû¶ó ¿Ã¶ó°¡´Â °æÇèÄ¡°¡ ´Þ¶óÁø´Ù.
        Damage = (Damage_t)getPercentValue(Damage, AttrExpTimebandFactor[getZoneTimeband(pSlayer->getZone())]);

        // VariableManager¿¡ ÀÇÇÑ PointÁõ°¡Ä¡¸¦ °è»êÇÑ´Ù.
        if(g_pVariableManager->getExpRatio()>100 && g_pVariableManager->getEventActivate() == 1)
            Damage = getPercentValue(Damage, g_pVariableManager->getExpRatio());

        Exp_t STRPoint = max(1, Damage * STRMultiplier / 10);
        Exp_t DEXPoint = max(1, Damage * DEXMultiplier / 10);
        Exp_t INTPoint = max(1, Damage * INTMultiplier / 10);

        // ÇöÀç ¼ø¼ö ´É·ÂÄ¡¸¦ ¹Þ´Â´Ù.
        Attr_t CurSTR = pSlayer->getSTR(ATTR_BASIC);
        Attr_t CurDEX = pSlayer->getDEX(ATTR_BASIC);
        Attr_t CurINT = pSlayer->getINT(ATTR_BASIC);
        Attr_t CurSUM = CurSTR + CurDEX + CurINT;

        // ´É·Â ÇÕÀÌ 200 ÀÌ»óÀÎ »ç¶÷µéÀº ¾²´Â °è¿­¿¡ µû¶ó ´É·Â¿¡ ¹Ù·Î Àû¿ë µÈ´Ù.
        // ³ª¸ÓÁö ¹èºÐÀº ¹«½Ã ÇÏ°Ô µÈ´Ù.
        // ÀÌ·¸°Ô µÇ¾úÀ»¶§, °è¿­·¾¿¡¸¸ ÇÁ¸® ÇÏ´Ù¸é ´É·ÂÄ¡¸¦ ¾î´ÀÁ¤µµ ÀÚÀ¯·Ó°Ô ¿Ã¸± ¼ö ÀÖ°Ô µÈ´Ù.
        if( CurSUM >= OneAttrExpBound ) {
            // ¾î´À ¸ÖÆ¼ÇÃ¶óÀÌ¾î°¡ °¡Àå Å«Áö Á¶»ç ÇÑ´Ù.
            // STR Æ÷ÀÎÆ®°¡ Á¦ÀÏ Å©´Ù.
            if( STRMultiplier > DEXMultiplier && STRMultiplier > INTMultiplier ) {
                DEXPoint = 0;
                DEXMultiplier = 0;
                INTPoint = 0;
                INTMultiplier = 0;
            // DEX Æ÷ÀÎÆ®°¡ Á¦ÀÏ Å©´Ù.
            } else if ( DEXMultiplier > STRMultiplier && DEXMultiplier > INTMultiplier ) {
                STRPoint = 0;
                STRMultiplier = 0;
                INTPoint = 0;
                INTMultiplier = 0;

            // INT Æ÷ÀÎÆ®°¡ Á¦ÀÏ Å©´Ù.
            } else if ( INTMultiplier > STRMultiplier && INTMultiplier > DEXMultiplier ) {
                STRPoint = 0;
                STRMultiplier = 0;
                DEXPoint = 0;
                DEXMultiplier = 0;
            }
        }

        // Èû °æÇèÄ¡
        Exp_t CurSTRGoalExp = max(0, (int)(pSlayer->getSTRGoalExp() - STRPoint     ));
        // µ¦½º °æÇèÄ¡
        Exp_t CurDEXGoalExp = max(0, (int)(pSlayer->getDEXGoalExp() - DEXPoint     ));
        // ÀÎÆ® °æÇèÄ¡
        Exp_t CurINTGoalExp = max(0, (int)(pSlayer->getINTGoalExp() - INTPoint));

        // STR, DEX, INT °æÇèÄ¡¸¦ ¿Ã¸°´Ù.
        pSlayer->setSTRGoalExp(CurSTRGoalExp);
        pSlayer->setDEXGoalExp(CurDEXGoalExp);
        pSlayer->setINTGoalExp(CurINTGoalExp);

        bool bInitAll = false;

        // °æÇèÄ¡°¡ ´©ÀûµÇ¾î ±âº» ´É·ÂÄ¡°¡ »ó½ÂÇÒ ¶§´Ù...
        if ( STRMultiplier != 0 && CurSTRGoalExp == 0 && CurSTR < AttrBound )
        {
            bool isUp = true;

            // ´É·ÂÄ¡ ÃÑÇÕÀÌ 200À» ³Ñ¾î°¥·Á°í ÇÏ´Â °æ¿ì.
            if (CurSTR + CurDEX + CurINT >= TotalAttrBound )
            {
                isUp= true;

                // ÈûÀÌ ¿À¸¦ °æ¿ì DEX³ª INTÁß ³ôÀº°ÍÀ» ¶³¾îÆ®¸®°í, °°À» °æ¿ì DEX¸¦ ¶³¾îÆ®¸°´Ù.
                if (CurDEX >= CurINT)
                {
                    decreaseDEX(pSlayer);
                }
                else
                {
                    decreaseINT(pSlayer);
                }
            }

            if (isUp)
            {
                StringStream  msg1;

                // exp level°ú ´É·ÂÄ¡¸¦ ¿Ã·ÁÁØ´Ù.
                CurSTR         += 1;
                pSlayer->setSTR(CurSTR, ATTR_BASIC);

                // »õ·Î¿î ·¹º§ÀÇ STRInfo¸¦ ¹Þ¾Æ¿Â´Ù.
                STRBalanceInfo* pNewSTRInfo = g_pSTRBalanceInfoManager->getSTRBalanceInfo(CurSTR);

                // »õ·Î¿î ¸ñÇ¥ °æÇèÄ¡¸¦ ¼ÂÆÃÇØ Áà¾ß ÇÑ´Ù.
                Exp_t NewGoalExp = pNewSTRInfo->getGoalExp();
                pSlayer->setSTRGoalExp(NewGoalExp);

                // DB¿¡ ¿Ã¶ó°£ ´É·ÂÄ¡¸¦ ÀúÀåÇÑ´Ù.
                msg1 << "STR = " << (int)CurSTR << ", STRGoalExp = " << NewGoalExp;

                pSlayer->tinysave(msg1.toString());

                bInitAll = true;
            }
        }

        // °æÇèÄ¡°¡ ´©ÀûµÇ¾î ±âº» ´É·ÂÄ¡°¡ »ó½ÂÇÒ ¶§´Ù...
        if ( DEXMultiplier != 0 && CurDEXGoalExp == 0 && CurDEX < AttrBound )
        {
            bool isUp = true;

            // ´É·ÂÄ¡ ÃÑÇÕÀÌ 200À» ³Ñ¾î°¥·Á°í ÇÏ´Â °æ¿ì.
            if (CurSTR + CurDEX + CurINT >= TotalAttrBound )
            {
                isUp= true;

                // ¹ÎÃ¸ÀÌ ¿À¸¦ °æ¿ì STR³ª INTÁß ³ôÀº°ÍÀ» ¶³¾îÆ®¸®°í, °°À» °æ¿ì STR¸¦ ¶³¾îÆ®¸°´Ù.
                if (CurSTR >= CurINT)
                {
                    decreaseSTR(pSlayer);
                }
                else
                {
                    decreaseINT(pSlayer);
                }
            }

            if (isUp)
            {
                StringStream  msg1;

                // exp level°ú ´É·ÂÄ¡¸¦ ¿Ã·ÁÁØ´Ù.
                CurDEX         += 1;
                pSlayer->setDEX(CurDEX, ATTR_BASIC);

                // »õ·Î¿î ·¹º§ÀÇ DEXInfo¸¦ ¹Þ¾Æ¿Â´Ù.
                DEXBalanceInfo* pNewDEXInfo = g_pDEXBalanceInfoManager->getDEXBalanceInfo(CurDEX);

                // »õ·Î¿î ¸ñÇ¥ °æÇèÄ¡¸¦ ¼ÂÆÃÇØ Áà¾ß ÇÑ´Ù.
                Exp_t NewGoalExp = pNewDEXInfo->getGoalExp();
                pSlayer->setDEXGoalExp(NewGoalExp);

                // DB¿¡ ¿Ã¶ó°£ ´É·ÂÄ¡¸¦ ÀúÀåÇÑ´Ù.
                msg1 << "DEX = " << (int)CurDEX << ", DEXGoalExp = " << NewGoalExp;
                pSlayer->tinysave(msg1.toString());

                bInitAll = true;
            }
        }

        // °æÇèÄ¡°¡ ´©ÀûµÇ¾î ±âº» ´É·ÂÄ¡°¡ »ó½ÂÇÒ ¶§´Ù...
        if ( INTMultiplier != 0 && CurINTGoalExp == 0 && CurINT < AttrBound )
        {
            bool isUp = true;

            // ´É·ÂÄ¡ ÃÑÇÕÀÌ 200À» ³Ñ¾î°¥·Á°í ÇÏ´Â °æ¿ì.
            if (CurSTR + CurDEX + CurINT >= TotalAttrBound )
            {
                isUp= true;

                // Áö½ÄÀÌ ¿À¸¦ °æ¿ì STR³ª DEXÁß ³ôÀº°ÍÀ» ¶³¾îÆ®¸®°í, °°À» °æ¿ì STR¸¦ ¶³¾îÆ®¸°´Ù.
                if (CurSTR >= CurDEX)
                {
                    decreaseSTR(pSlayer);
                }
                else
                {
                    decreaseDEX(pSlayer);
                }
            }

            if (isUp)
            {
                StringStream  msg1;

                // exp level°ú ´É·ÂÄ¡¸¦ ¿Ã·ÁÁØ´Ù.
                CurINT         += 1;
                pSlayer->setINT(CurINT, ATTR_BASIC);
                // »õ·Î¿î ·¹º§ÀÇ INTInfo¸¦ ¹Þ¾Æ¿Â´Ù.
                INTBalanceInfo* pNewINTInfo = g_pINTBalanceInfoManager->getINTBalanceInfo(CurINT);

                // »õ·Î¿î ¸ñÇ¥ °æÇèÄ¡¸¦ ¼ÂÆÃÇØ Áà¾ß ÇÑ´Ù.
                Exp_t NewGoalExp = pNewINTInfo->getGoalExp();
                pSlayer->setINTGoalExp(NewGoalExp);

                // DB¿¡ ¿Ã¶ó°£ ´É·ÂÄ¡¸¦ ÀúÀåÇÑ´Ù.
                msg1 << "INTE = " << (int)CurINT << ", INTGoalExp = " << NewGoalExp;

                pSlayer->tinysave(msg1.toString());

                bInitAll = true;
            }
        }

        // ÆÐÅ¶¿¡´Ù ¹Ù²ï µ¥ÀÌÅÍ¸¦ ÀÔ·ÂÇÑ´Ù.
        // ´É·ÂÄ¡°¡ ÇÕ°è Á¦ÇÑ¿¡ ÀÇÇØ ³»·Á°¥ ¼öµµ ÀÖÀ¸¹Ç·Î ¸ðµç Ã³¸®¸¦ ÇÑ µÚ º¯°æÁ¤º¸¸¦ ³Ö´Â´Ù - by Bezz
        _ModifyInfo.addLongData(MODIFY_STR_EXP, pSlayer->getSTRGoalExp() );//CurSTRExp);
        _ModifyInfo.addLongData(MODIFY_DEX_EXP, pSlayer->getDEXGoalExp() );//CurDEXExp);
        _ModifyInfo.addLongData(MODIFY_INT_EXP, pSlayer->getINTGoalExp() );//CurINTExp);

        // ¿Ã¶ó°£ °æÇèÄ¡¸¦ DB¿¡ ÀúÀåÇÑ´Ù.
        WORD AttrExpSaveCount = pSlayer->getAttrExpSaveCount();
        if (AttrExpSaveCount > ATTR_EXP_SAVE_PERIOD)
        {
            char pField[256];
            sprintf(pField, "STRGoalExp=%ld, DEXGoalExp=%ld, INTGoalExp=%ld",
                                pSlayer->getSTRGoalExp(), pSlayer->getDEXGoalExp(), pSlayer->getINTGoalExp());

            pSlayer->tinysave( pField );

            AttrExpSaveCount = 0;
        }
        else AttrExpSaveCount++;

        pSlayer->setAttrExpSaveCount(AttrExpSaveCount);

        // ±âÁ¸ÀÇ ´É·ÂÄ¡¿Í ºñ±³ÇØ¼­ º¯°æµÈ ´É·ÂÄ¡¸¦ º¸³»ÁØ´Ù.
        if (bInitAll)
        {
            healCreatureForLevelUp(pSlayer, _ModifyInfo, &prev);

            // ·¹º§¾÷ ÀÌÆåÆ®µµ º¸¿©ÁØ´Ù. by sigi. 2002.11.9
            sendEffectLevelUp( pSlayer );

            // ´É·ÂÄ¡ ÇÕÀÌ 40ÀÌ°í, ¾ßÀü»ç·ÉºÎÀÌ¸é µýµ¥·Î º¸³½´Ù.  by sigi. 2002.11.7
            if (g_pVariableManager->isNewbieTransportToGuild())
            {
                checkNewbieTransportToGuild(pSlayer);
            }
        }*/
}


//////////////////////////////////////////////////////////////////////////////
// ½½·¹ÀÌ¾î ±â¼ú °æÇèÄ¡¸¦ °è»êÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
void increaseSkillExp(Slayer* pSlayer, SkillDomainType_t DomainType, SkillSlot* pSkillSlot, SkillInfo* pSkillInfo,
                      ModifyInfo& _ModifyInfo) {
    Assert(pSlayer != NULL);
    Assert(pSkillSlot != NULL);
    Assert(pSkillInfo != NULL);
    // Edit By Coffee 2007-4-16È¥µô¶þ×ªºó¼¼ÄÜ²»ÄÜÉý¼¶ÎÊÌâ
    // if ( pSkillInfo->getLevel() >= 150 ) return;
    // end

    // PKÁ¸ ¾È¿¡¼­´Â °æÇèÄ¡¸¦ ÁÖÁö ¾Ê´Â´Ù.
    if (g_pPKZoneInfoManager->isPKZone(pSlayer->getZoneID()))
        return;

    // ´ÙÀÌ³ª¹Í Á¸ ¾È¿¡¼­´Â °æÇèÄ¡¸¦ ÁÖÁö ¾Ê´Â´Ù.
    if (pSlayer->getZone() != NULL && pSlayer->getZone()->isDynamicZone())
        return;

    // ¸¸¾à NewLevelÀÌ ÇöÀçÀÇ µµ¸ÞÀÎ ·¹º§¿¡¼­ ³ÑÀ» ¼ö ¾ø´Â °æ¿ì¿¡´Â °æÇèÄ¡¸¦ ¿Ã·ÁÁÖÁö ¾Ê´Â´Ù.
    Level_t CurrentLevel = pSkillSlot->getExpLevel();

    // ÇöÀç ½½·¹ÀÌ¾îÀÇ µµ¸ÞÀÎÀ» ¹Þ¾Æ¿Â´Ù.
    Level_t DomainLevel = pSlayer->getSkillDomainLevel(DomainType);

    // µµ¸ÞÀÎÀÇ ´Ü°è¸¦ ¹Þ¾Æ¿Â´Ù.
    SkillGrade Grade = g_pSkillInfoManager->getGradeByDomainLevel(DomainLevel);

    // ÇöÀç ´Ü°è¿¡¼­ + 1 ÇÑ ´Ü°èÀÇ Á¦ÇÑ ·¹º§À» ¹Þ¾Æ¿Â´Ù.
    Level_t LimitLevel = g_pSkillInfoManager->getLimitLevelByDomainGrade(SkillGrade(Grade + 1));

    if (CurrentLevel < LimitLevel) {
        // °æÇèÄ¡¸¦ °è»êÇÑ´Ù.
        Exp_t MaxExp = pSkillInfo->getSubSkill();
        Exp_t OldExp = pSkillSlot->getExp();
        Exp_t NewExp;

        Exp_t plusExp = 1;

        if (g_pVariableManager->getEventActivate() == 1) {
            plusExp = plusExp * g_pVariableManager->getExpRatio() / 100;
            plusExp = getPercentValue(plusExp, g_pVariableManager->getPremiumExpBonusPercent());
            if (pSlayer->isFlag(Effect::EFFECT_CLASS_BONUS_EXP))
                plusExp *= 2;
        }

        // °æÇèÄ¡ µÎ¹è
        if (isAffectExp2X())
            plusExp *= 2;

        NewExp = min(MaxExp, OldExp + plusExp);

        pSkillSlot->setExp(NewExp);

        SkillLevel_t NewLevel = (NewExp * 100 / MaxExp) + 1;
        NewLevel = min((int)NewLevel, 100);

        ulong longData = (((ulong)pSkillSlot->getSkillType()) << 16) | (ulong)(NewExp / 10);
        _ModifyInfo.addLongData(MODIFY_SKILL_EXP, longData);

        // ÄüÆÄÀÌ¾î´Â ³ªÁß¿¡ DB¸¦ ¼öÁ¤ÇØ¾ß ÇÒ °ÍÀÌ´Ù.
        if (CurrentLevel != NewLevel) {
            pSkillSlot->setExpLevel(NewLevel);
            pSkillSlot->save();

            longData = (((ulong)pSkillSlot->getSkillType()) << 16) | (ulong)NewLevel;
            _ModifyInfo.addLongData(MODIFY_SKILL_LEVEL, longData);

            pSlayer->getGQuestManager()->skillLevelUp(pSkillSlot);
        } else {
            WORD SkillExpSaveCount = pSlayer->getSkillExpSaveCount();
            if (SkillExpSaveCount > SKILL_EXP_SAVE_PERIOD) {
                pSkillSlot->save();
                SkillExpSaveCount = 0;
            } else
                SkillExpSaveCount++;
            pSlayer->setSkillExpSaveCount(SkillExpSaveCount);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////
// ½½·¹ÀÌ¾î °è¿­ °æÇèÄ¡¸¦ °è»êÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
bool increaseDomainExp(Slayer* pSlayer, SkillDomainType_t Domain, Exp_t Point, ModifyInfo& _ModifyInfo,
                       Level_t EnemyLevel, int TargetNum) {
    if (pSlayer == NULL || Point == 0 || TargetNum == 0)
        return false;
    if (pSlayer->isAdvanced())
        return false;

    // PK Á¸ ¾È¿¡¼­´Â °æÇèÄ¡¸¦ ÁÖÁö ¾Ê´Â´Ù.
    if (g_pPKZoneInfoManager->isPKZone(pSlayer->getZoneID()))
        return false;

    // ´ÙÀÌ³ª¹Í Á¸ ¾È¿¡¼­´Â °æÇèÄ¡¸¦ ÁÖÁö ¾Ê´Â´Ù.
    if (pSlayer->getZone() != NULL && pSlayer->getZone()->isDynamicZone())
        return false;

    int PartyID = pSlayer->getPartyID();

    if (EnemyLevel != 0) {
        int levelDiff = (int)pSlayer->getLevel() - (int)EnemyLevel;

        //		cout << "enemyLevel : " << (int)EnemyLevel << " , TargetNum : " << TargetNum << endl;

        //		cout << "Point : " << Point << endl;

        if (levelDiff > 50)
            Point = getPercentValue(Point, 30);
        else if (levelDiff > 40)
            Point = getPercentValue(Point, 50);
        else if (levelDiff < -40)
            Point = getPercentValue(Point, 140);
        else if (levelDiff < -30)
            Point = getPercentValue(Point, 130);
        else if (levelDiff < -20)
            Point = getPercentValue(Point, 120);

        //		cout << "after level Point : " << Point << endl;
    }

    if (TargetNum != -1)
        Point = Point * (TargetNum + 1) / 3;

    //	cout << "after target Point : " << Point << endl;

    // ÀÌ¹Ì ÁöÁ¤µÈ domain¿¡ ¸Â´Â ¹«±â¸¦ µé°í ÀÖ´Ù°í °¡Á¤ÇÏ°í..
    // ¹«±â type¿¡ µû¶ó¼­ SkillPoint¸¦ ´Ù¸£°Ô ÁØ´Ù.
    // by sigi. 2002.10.30
    Item* pWeapon = pSlayer->getWearItem(Slayer::WEAR_RIGHTHAND);
    if (pWeapon != NULL) {
        SkillLevel_t DomainLevel = pSlayer->getSkillDomainLevel(Domain);

        Point = computeSkillPointBonus(Domain, DomainLevel, pWeapon, Point);
    }


    // À¯·áÈ­ Á¸¿¡¼­´Â °æÇèÄ¡¸¦ ´õ ¹Þ´Â´Ù.
    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pSlayer->getPlayer());
    Assert(pGamePlayer != NULL);

    if (pGamePlayer->isPremiumPlay() ||
        pGamePlayer->isFamilyFreePass()) // pZone->isPayPlay() || pZone->isPremiumZone() )
    {
        Point = getPercentValueEx(Point, g_pVariableManager->getPremiumExpBonusPercent());
    }

    if (pGamePlayer->isPCRoomPlay()) // pZone->isPayPlay() || pZone->isPremiumZone() )
    {
        Point = getPercentValueEx(Point, g_pVariableManager->getPCRoomExpBonusPercent());
    }

    if (PartyID != 0) {
        LocalPartyManager* pLPM = pSlayer->getLocalPartyManager();
        Assert(pLPM != NULL);

        int nMemberSize = pLPM->getAdjacentMemberSize(PartyID, pSlayer);
        switch (nMemberSize) {
        case 2:
            Point = getPercentValue(Point, 110);
            break;
        case 3:
            Point = getPercentValue(Point, 120);
            break;
        case 4:
            Point = getPercentValue(Point, 130);
            break;
        case 5:
            Point = getPercentValue(Point, 140);
            break;
        case 6:
            Point = getPercentValue(Point, 150);
            break;
        default:
            break;
        }
    }

    // VariableManager¿¡ ÀÇÇÑ PointÁõ°¡Ä¡¸¦ °è»êÇÑ´Ù.
    if (g_pVariableManager->getExpRatio() > 100 && g_pVariableManager->getEventActivate() == 1)
        Point = getPercentValue(Point, g_pVariableManager->getExpRatio());

    if (pSlayer->isFlag(Effect::EFFECT_CLASS_BONUS_EXP))
        Point *= 2;

    // °æÇèÄ¡ µÎ¹è
    if (isAffectExp2X())
        Point *= 2;

    Level_t CurDomainLevel = pSlayer->getSkillDomainLevel(Domain);
    Level_t NewDomainLevel = CurDomainLevel;
    SkillType_t LearnSkillType = g_pSkillInfoManager->getSkillTypeByLevel(Domain, CurDomainLevel);
    Exp_t NewGoalExp = 0;
    bool availiable = false;

    // ÇöÀç ·¹º§¿¡¼­ ¹è¿ï ¼ö ÀÖ´Â ±â¼úÀÌ ÀÖ´ÂÁö º»´Ù.
    if (LearnSkillType != 0) {
        // ¹è¿ï ¼ö ÀÖ´Â ±â¼úÀÌ ÀÖ°í ÀÌ¹Ì ¹è¿î »óÅÂ¶ó¸é Domain °æÇèÄ¡¸¦ ¿Ã·ÁÁØ´Ù.
        if (pSlayer->hasSkill(LearnSkillType)) {
            availiable = true;
        }
    } else {
        availiable = true;
    }

    if (availiable) {
        bool isLevelUp = false;

        // ½Ã°£´ë¿¡ µû¶ó ¿Ã¶ó°¡´Â °æÇèÄ¡°¡ ´Þ¶óÁø´Ù.
        Point = (Exp_t)getPercentValue(Point, DomainExpTimebandFactor[getZoneTimeband(pSlayer->getZone())]);

        // cout << pSlayer->getName() << "¿¡°Ô " << (int)Domain << " µµ¸ÞÀÎÀÇ °æÇèÄ¡¸¦ " << Point << "¸¸Å­ ÁÝ´Ï´Ù." <<
        // endl;

        // º¸»ó¿ë ÄÚµå
        // Point = max(2, (int)getPercentValue(Point, 150));

        // µµ¸ÞÀÎ ¸ñÇ¥ °æÇèÄ¡
        // µµ¸ÞÀÎ ´©Àû °æÇèÄ¡
        Exp_t GoalExp = pSlayer->getGoalExp(Domain);
        //		Exp_t CurrentExp = pSlayer->getSkillDomainExp(Domain);

        // »õ·Î¿î ¸ñÇ¥ °æÇèÄ¡
        NewGoalExp = max(0, (int)(GoalExp - Point));

        // ´©Àû °æÇèÄ¡¿¡´Â ¸ñÇ¥°æÇèÄ¡°¡ ÁÙ¾îµç ¸¸Å­ ¿Ã¶ó°¡¾ß Á¤»óÀÌ´Ù.
        // »õ·Î¿î ´©Àû °æÇèÄ¡
        //		Exp_t DiffExp = max(0, (int)(GoalExp - NewGoalExp));

        //		Exp_t NewExp = 0;

        // ·¹º§ÀÌ ÃÖ°í¿¡ ´ÞÇÑ »ç¶÷ÀÌ¶óµµ °æÇèÄ¡´Â ½×ÀÎ´Ù.
        //		if( DiffExp == 0 && CurDomainLevel >= SLAYER_MAX_DOMAIN_LEVEL ) {
        //			NewExp  = CurrentExp + Point;
        //		} else {
        //			NewExp  = CurrentExp + DiffExp;
        //		}

        // »õ·Î¿î ¸ñÇ¥ °æÇèÄ¡ ¼ÂÆÃ
        // »õ·Î¿î ´©Àû °æÇèÄ¡ ¼ÂÆÃ
        pSlayer->setGoalExp(Domain, NewGoalExp);
        //		pSlayer->setSkillDomainExp(Domain, NewExp);

        // cout << "³²Àº °æÇèÄ¡´Â " << NewGoalExp << endl;

        // ¸ñÇ¥ °æÇèÄ¡°¡ 0 ÀÌ¶ó¸é, ·¹º§¾÷À» ÇÒ ¼ö ÀÖ´Â »óÅÂÀÎ°¡¸¦ °Ë»çÇÑ´Ù.
        if (NewGoalExp == 0 && CurDomainLevel != SLAYER_MAX_DOMAIN_LEVEL) {
            // µµ¸ÞÀÎ ·¹º§À» ¿Ã·ÁÁÖ°í, ±×¿¡ µû¸¥ ±â¼úÀ» ¹è¿ï ¼ö ÀÖ´Ù¸é ±â¼úÀ» ¹è¿ï ¼ö ÀÖ´Ù´Â °ÍÀ» ¾Ë·ÁÁØ´Ù.
            NewDomainLevel = CurDomainLevel + 1;

            // µµ¸ÞÀÎ ÀÎÆ÷ ¸Þ´ÏÁ®¸¦ ¸¸µé¾î¼­ ¸ñÇ¥ °æÇèÄ¡¸¦ ¼ÂÆÃÇÏ°í ·¹º§À» Àç ¼³Á¤ ÇÑ´Ù.
            NewGoalExp = g_pSkillDomainInfoManager->getDomainInfo((SkillDomain)Domain, NewDomainLevel)->getGoalExp();

            pSlayer->setGoalExp(Domain, NewGoalExp);
            pSlayer->setSkillDomainLevel(Domain, NewDomainLevel);

            // cout << "·¹º§¾÷ÇØ¼­ ³²Àº °æÇèÄ¡´Â " << NewGoalExp << endl;

            SkillType_t NewLearnSkillType = g_pSkillInfoManager->getSkillTypeByLevel(Domain, NewDomainLevel);

            // ÇöÀç ·¹º§¿¡¼­ ¹è¿ï ¼ö ÀÖ´Â ±â¼úÀÌ ÀÖ´ÂÁö º»´Ù.
            if (NewLearnSkillType != 0) {
                // ¹è¿ï ¼ö ÀÖ´Â ±â¼úÀÌ ÀÖ°í ÀÌ¹Ì ¹è¿ìÁö ¾ÊÀº »óÅÂ¶ó¸é ±â¼úÀ» ¹è¿ï ¼ö ÀÖ´Ù´Â ÆÐÅ¶À» ³¯¸°´Ù.
                if (pSlayer->hasSkill(NewLearnSkillType) == NULL) {
                    // GCLearnSkillReadyÀÇ m_SkillType¿¡ level upµÈ µµ¸ÞÀÎÀÇ °¡Àå ÃÖ±Ù
                    // ±â¼úÀ» ´ëÀÔÇÑ´Ù. Áï, Å¬¶óÀÌ¾ðÆ® ±× ´ÙÀ½ ½ºÅ³À» ¹è¿ï¼ö ÀÖ´Ù...
                    GCLearnSkillReady readyPacket;
                    readyPacket.setSkillDomainType((SkillDomainType_t)Domain);
                    // send packet
                    pSlayer->getPlayer()->sendPacket(&readyPacket);
                }
            }

            isLevelUp = true;
            // cout << "·¹º§¾÷ ÇÒ ¼ö ÀÖ½À´Ï´Ù." << endl;
        }

        /*		if (DiffExp != 0)
                {
                    switch (Domain)
                    {
                        case SKILL_DOMAIN_BLADE:   _ModifyInfo.addLongData(MODIFY_BLADE_DOMAIN_EXP, NewGoalExp); break;
                        case SKILL_DOMAIN_SWORD:   _ModifyInfo.addLongData(MODIFY_SWORD_DOMAIN_EXP, NewGoalExp); break;
                        case SKILL_DOMAIN_GUN:     _ModifyInfo.addLongData(MODIFY_GUN_DOMAIN_EXP, NewGoalExp); break;
                        case SKILL_DOMAIN_HEAL:    _ModifyInfo.addLongData(MODIFY_HEAL_DOMAIN_EXP, NewGoalExp); break;
                        case SKILL_DOMAIN_ENCHANT: _ModifyInfo.addLongData(MODIFY_ENCHANT_DOMAIN_EXP, NewGoalExp);
           break; default: break;
                    }
                }*/

        Level_t DomainLevelSum = pSlayer->getSkillDomainLevelSum();

        // ·¹º§¾÷ÀÌ µÇ¾úÀ» °æ¿ì, µµ¸ÞÀÎ ÃÑÇÕÀÌ 100À» ³Ñ´Â´Ù¸é ÇöÀç µµ¸ÞÀÎÀ» Á¦¿ÜÇÑ
        // µµ¸ÞÀÎ Áß¿¡¼­ °¡Àå ³ôÀº µµ¸ÞÀÎ ·¹º§À» ¶³¾î¶ß·Á¾ß ÇÑ´Ù.
        if (isLevelUp && DomainLevelSum > SLAYER_MAX_DOMAIN_LEVEL) {
            SDomain ds[SKILL_DOMAIN_VAMPIRE];

            for (int i = 0; i < SKILL_DOMAIN_VAMPIRE; i++) {
                ds[i].DomainType = i;
                ds[i].DomainLevel = pSlayer->getSkillDomainLevel((SkillDomain)i);
            }

            // ÇöÁ¦ µµ¸ÞÀÎÀ» Á¦¿ÜÇÑ °¡Àå Å« ¼ýÀÚ¸¦ Ã£´Â´Ù.
            stable_sort(ds, ds + SKILL_DOMAIN_VAMPIRE, isBig());

            // ¼ÒÆÃÀ» ÇÏ°í ³­ ´ÙÀ½ Á¦ÀÏ À§¿¡ ÀÖ´Â ½ºÆ®·°ÃÄ°¡ °¡Àå ³ôÀº ·¹º§À» °¡Áö°í ÀÖ´Â µµ¸ÞÀÎÀÌ´Ù.
            int j = 0;
            while (ds[j].DomainType == Domain) {
                j++;
                if (j > SKILL_DOMAIN_VAMPIRE) {
                    cerr << "Out of Skill Domain Range!!!" << endl;
                    Assert(false);
                }
            }

            // °á±¹ ds[j]ÀÇ °ªÀº ÇöÀç µµ¸ÞÀÎ°ú °°Áö ¾ÊÀº °¡Àå ³ôÀº ·¹º§ÀÇ µµ¸ÞÀÎÀÌ´Ù.
            SkillDomainType_t DownDomainType = ds[j].DomainType;
            Level_t DownDomainLevel = ds[j].DomainLevel;

            // cout << (int)DownDomainType << "µµ¸ÞÀÎÀÇ µµ¸ÞÀÎ ·¹º§À» ³·Ãä´Ï´Ù." << endl;

            // ÇöÀç µµ¸ÞÀÎ¿¡¼­ ¹è¿ï ¼ö ÀÖ´Â ±â¼úÀÌ ÀÖ´Ù¸é Disable ½ÃÅ²´Ù.
            SkillType_t eraseSkillType = g_pSkillInfoManager->getSkillTypeByLevel(DownDomainType, DownDomainLevel);
            SkillSlot* pESkillSlot = pSlayer->hasSkill(eraseSkillType);
            if (pESkillSlot != NULL) {
                pESkillSlot->setDisable();
            }

            // µµ¸ÞÀÎÀÇ ·¹º§À» ¶³¾îÆ®¸°´Ù.
            DownDomainLevel--;

            // ´Ù¿î µµ¸ÞÀÎÀÇ ·¹º§À» ¼ÂÆÃÇÑ´Ù.
            pSlayer->setSkillDomainLevel(DownDomainType, DownDomainLevel);

            // ´Ù¿î µµ¸ÞÀÎÀÇ ¸ñÇ¥ °æÇèÄ¡¸¦ Ã£¾Æ¿Â´Ù.
            // ´Ù¿î µµ¸ÞÀÎÀÇ ´©Àû °æÇèÄ¡¸¦ Ã£¾Æ¿Â´Ù.
            Exp_t DownDomainGoalExp =
                g_pSkillDomainInfoManager->getDomainInfo((SkillDomain)DownDomainType, DownDomainLevel)->getGoalExp();
            //			Exp_t DownDomainSumExp  = g_pSkillDomainInfoManager->getDomainInfo((SkillDomain)DownDomainType,
            // DownDomainLevel)->getAccumExp();

            // ´Ù¿î ±×·¹ÀÌµåµÈ ¸ñÇ¥ °æÇèÄ¡·Î Àç ¼ÂÆÃÇÑ´Ù.
            // ´Ù¿î ±×·¹ÀÌµå µÇ¾úÀ¸¹Ç·Î ±× ·¹º§¿¡ ¸Â´Â µµ¸ÞÀÎ °æÇèÄ¡¸¦ ¼ÂÆÃÇÑ´Ù.
            pSlayer->setGoalExp(DownDomainType, DownDomainGoalExp);
            //			pSlayer->setSkillDomainExp(DownDomainType, DownDomainSumExp);
            // cout << "·¹º§ : " << (int)DownDomainLevel << endl;
            // cout << "³²Àº°æÇèÄ¡ : " << (int)DownDomainGoalExp << endl;

            StringStream DownSave;
            if (DownDomainType == SKILL_DOMAIN_BLADE) {
                DownSave << "BladeLevel = " << (int)DownDomainLevel << ",BladeGoalExp = " << (int)DownDomainGoalExp;
                _ModifyInfo.addShortData(MODIFY_BLADE_DOMAIN_LEVEL, DownDomainLevel);
                _ModifyInfo.addLongData(MODIFY_BLADE_DOMAIN_EXP, DownDomainGoalExp);
            } else if (DownDomainType == SKILL_DOMAIN_SWORD) {
                DownSave << "SwordLevel = " << (int)DownDomainLevel << ",SwordGoalExp = " << (int)DownDomainGoalExp;
                _ModifyInfo.addLongData(MODIFY_SWORD_DOMAIN_EXP, DownDomainGoalExp);
                _ModifyInfo.addShortData(MODIFY_SWORD_DOMAIN_LEVEL, DownDomainLevel);
            } else if (DownDomainType == SKILL_DOMAIN_GUN) {
                DownSave << "GunLevel = " << (int)DownDomainLevel << ",GunGoalExp = " << (int)DownDomainGoalExp;
                _ModifyInfo.addLongData(MODIFY_GUN_DOMAIN_EXP, DownDomainGoalExp);
                _ModifyInfo.addShortData(MODIFY_GUN_DOMAIN_LEVEL, DownDomainLevel);
            } else if (DownDomainType == SKILL_DOMAIN_ENCHANT) {
                DownSave << "EnchantLevel = " << (int)DownDomainLevel << ",EnchantGoalExp = " << (int)DownDomainGoalExp;
                _ModifyInfo.addShortData(MODIFY_ENCHANT_DOMAIN_LEVEL, DownDomainLevel);
                _ModifyInfo.addLongData(MODIFY_ENCHANT_DOMAIN_EXP, DownDomainGoalExp);
            } else if (DownDomainType == SKILL_DOMAIN_HEAL) {
                DownSave << "HealLevel = " << (int)DownDomainLevel << ",HealGoalExp = " << (int)DownDomainGoalExp;
                _ModifyInfo.addShortData(MODIFY_HEAL_DOMAIN_LEVEL, DownDomainLevel);
                _ModifyInfo.addLongData(MODIFY_HEAL_DOMAIN_EXP, DownDomainGoalExp);
            } else if (DownDomainType == SKILL_DOMAIN_ETC) {
                DownSave << "ETCLevel = " << (int)DownDomainLevel << ",ETCGoalExp = " << (int)DownDomainGoalExp;
                _ModifyInfo.addShortData(MODIFY_ETC_DOMAIN_LEVEL, DownDomainLevel);
            } else {
            }

            // ¶³¾î¶ß¸° µµ¸ÞÀÎ ·¹º§À» ¼¼ÀÌºêÇÑ´Ù.
            pSlayer->tinysave(DownSave.toString());
        }

        WORD DomainExpSaveCount = pSlayer->getDomainExpSaveCount();

        if (Domain == SKILL_DOMAIN_BLADE) {
            StringStream attrsave;
            if (isLevelUp) {
                attrsave << "BladeLevel = " << (int)NewDomainLevel << ",BladeGoalExp = " << (int)NewGoalExp;
                _ModifyInfo.addShortData(MODIFY_BLADE_DOMAIN_LEVEL, NewDomainLevel);
                pSlayer->tinysave(attrsave.toString());
            } else {
                attrsave << "BladeGoalExp = " << (int)NewGoalExp;

                if (DomainExpSaveCount > DOMAIN_EXP_SAVE_PERIOD) {
                    pSlayer->tinysave(attrsave.toString());
                    DomainExpSaveCount = 0;
                } else
                    DomainExpSaveCount++;
            }

            _ModifyInfo.addLongData(MODIFY_BLADE_DOMAIN_EXP, NewGoalExp);
        } else if (Domain == SKILL_DOMAIN_SWORD) {
            StringStream attrsave;
            if (isLevelUp) {
                attrsave << "SwordLevel = " << (int)NewDomainLevel << ",SwordGoalExp = " << (int)NewGoalExp;
                _ModifyInfo.addShortData(MODIFY_SWORD_DOMAIN_LEVEL, NewDomainLevel);
                pSlayer->tinysave(attrsave.toString());
            } else {
                attrsave << "SwordGoalExp = " << (int)NewGoalExp;

                if (DomainExpSaveCount > DOMAIN_EXP_SAVE_PERIOD) {
                    pSlayer->tinysave(attrsave.toString());
                    DomainExpSaveCount = 0;
                } else
                    DomainExpSaveCount++;
            }

            _ModifyInfo.addLongData(MODIFY_SWORD_DOMAIN_EXP, NewGoalExp);
        } else if (Domain == SKILL_DOMAIN_GUN) {
            StringStream attrsave;
            if (isLevelUp) {
                attrsave << "GunLevel = " << (int)NewDomainLevel << ",GunGoalExp = " << (int)NewGoalExp;
                _ModifyInfo.addShortData(MODIFY_GUN_DOMAIN_LEVEL, NewDomainLevel);
                pSlayer->tinysave(attrsave.toString());
            } else {
                attrsave << "GunGoalExp = " << (int)NewGoalExp;

                if (DomainExpSaveCount > DOMAIN_EXP_SAVE_PERIOD) {
                    pSlayer->tinysave(attrsave.toString());
                    DomainExpSaveCount = 0;
                } else
                    DomainExpSaveCount++;
            }

            _ModifyInfo.addLongData(MODIFY_GUN_DOMAIN_EXP, NewGoalExp);
        } else if (Domain == SKILL_DOMAIN_ENCHANT) {
            StringStream attrsave;
            if (isLevelUp) {
                attrsave << "EnchantLevel = " << (int)NewDomainLevel << ",EnchantGoalExp = " << (int)NewGoalExp;
                _ModifyInfo.addShortData(MODIFY_ENCHANT_DOMAIN_LEVEL, NewDomainLevel);
                pSlayer->tinysave(attrsave.toString());
            } else {
                attrsave << "EnchantGoalExp = " << (int)NewGoalExp;

                if (DomainExpSaveCount > DOMAIN_EXP_SAVE_PERIOD) {
                    pSlayer->tinysave(attrsave.toString());
                    DomainExpSaveCount = 0;
                } else
                    DomainExpSaveCount++;
            }

            _ModifyInfo.addLongData(MODIFY_ENCHANT_DOMAIN_EXP, NewGoalExp);
        } else if (Domain == SKILL_DOMAIN_HEAL) {
            StringStream attrsave;
            if (isLevelUp) {
                attrsave << "HealLevel = " << (int)NewDomainLevel << ",HealGoalExp = " << (int)NewGoalExp;
                _ModifyInfo.addShortData(MODIFY_HEAL_DOMAIN_LEVEL, NewDomainLevel);
                pSlayer->tinysave(attrsave.toString());
            } else {
                attrsave << "HealGoalExp = " << (int)NewGoalExp;

                if (DomainExpSaveCount > DOMAIN_EXP_SAVE_PERIOD) {
                    pSlayer->tinysave(attrsave.toString());
                    DomainExpSaveCount = 0;
                } else
                    DomainExpSaveCount++;
            }

            _ModifyInfo.addLongData(MODIFY_HEAL_DOMAIN_EXP, NewGoalExp);
        } else if (Domain == SKILL_DOMAIN_ETC) {
            StringStream attrsave;
            if (isLevelUp) {
                attrsave << "ETCLevel = " << (int)NewDomainLevel << ",ETCGoalExp = " << (int)NewGoalExp;
                _ModifyInfo.addShortData(MODIFY_ETC_DOMAIN_LEVEL, NewDomainLevel);
                pSlayer->tinysave(attrsave.toString());
            } else {
                attrsave << "ETCGoalExp = " << (int)NewGoalExp;

                if (DomainExpSaveCount > DOMAIN_EXP_SAVE_PERIOD) {
                    pSlayer->tinysave(attrsave.toString());
                    DomainExpSaveCount = 0;
                } else
                    DomainExpSaveCount++;
            }

            _ModifyInfo.addLongData(MODIFY_ETC_DOMAIN_EXP, NewGoalExp);
        } else {
        }

        // GrandMasterÀÎ °æ¿ì´Â Effect¸¦ ºÙ¿©ÁØ´Ù.
        // by sigi. 2002.11.8
        if (isLevelUp && DomainLevelSum >= GRADE_GRAND_MASTER_LIMIT_LEVEL) {
            // ÇÏ³ª°¡ 100·¾ ³Ñ°í ¾ÆÁ÷ Effect°¡ ¾È ºÙ¾îÀÖ´Ù¸é..
            if (pSlayer->getHighestSkillDomainLevel() >= GRADE_GRAND_MASTER_LIMIT_LEVEL &&
                !pSlayer->isFlag(Effect::EFFECT_CLASS_GRAND_MASTER_SLAYER)) {
                EffectGrandMasterSlayer* pEffect = new EffectGrandMasterSlayer(pSlayer);
                pEffect->setDeadline(999999);

                pSlayer->getEffectManager()->addEffect(pEffect);

                // affect()¾È¿¡¼­.. Flag°É¾îÁÖ°í, ÁÖÀ§¿¡ broadcastµµ ÇØÁØ´Ù.
                pEffect->affect();
            } else if (pSlayer->getHighestSkillDomainLevel() == 130 || pSlayer->getHighestSkillDomainLevel() == 150) {
                Effect* pEffect = pSlayer->findEffect(Effect::EFFECT_CLASS_GRAND_MASTER_SLAYER);
                if (pEffect != NULL)
                    pEffect->affect();
            }
        }

        pSlayer->setDomainExpSaveCount(DomainExpSaveCount);

        // ¹º°¡ ·¹º§¾÷Çß´Ù¸é Ã¼·ÂÀ» Ã¼¿öÁØ´Ù.
        if (isLevelUp) {
            SLAYER_RECORD prev;
            pSlayer->getSlayerRecord(prev);
            pSlayer->initAllStat();
            healCreatureForLevelUp(pSlayer, _ModifyInfo, &prev);

            // ·¹º§¾÷ ÀÌÆåÆ®µµ º¸¿©ÁØ´Ù. by sigi. 2002.11.9
            sendEffectLevelUp(pSlayer);

            pSlayer->whenQuestLevelUpgrade();

            // cout << "·¹º§¾÷ÇØ¼­ ÀÌÆåÆ®µµ Âï¾îÁá½À´Ï´Ù." << endl;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// ¹ìÆÄÀÌ¾î °æÇèÄ¡¸¦ °è»êÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
void increaseVampExp(Vampire* pVampire, Exp_t Point, ModifyInfo& _ModifyInfo) {
    Assert(pVampire != NULL);
    if (Point <= 0)
        return;
    if (pVampire->isAdvanced())
        return;

    // ¹ÚÁã »óÅÂÀÏ¶§´Â °æÇèÄ¡¸¦ È¹µæÇÏÁö ¸øÇÑ´Ù.
    if (pVampire->isFlag(Effect::EFFECT_CLASS_TRANSFORM_TO_BAT))
        return;

    // ´ÙÀÌ³ª¹Í Á¸ ¾È¿¡¼­´Â °æÇèÄ¡¸¦ ¾È ¿Ã·ÁÁØ´Ù.
    if (pVampire->getZone() != NULL && pVampire->getZone()->isDynamicZone())
        return;

    Level_t curLevel = pVampire->getLevel();

    // VariableManager¿¡ ÀÇÇÑ Áõ°¡
    if (g_pVariableManager->getExpRatio() > 100 && g_pVariableManager->getEventActivate() == 1)
        Point = getPercentValue(Point, g_pVariableManager->getExpRatio());

    if (pVampire->isFlag(Effect::EFFECT_CLASS_BONUS_EXP))
        Point *= 2;

    // °æÇèÄ¡ µÎ¹è
    if (isAffectExp2X())
        Point *= 2;

    // cout << pVampire->getName() << " ¿¡°Ô " << Point << "¸¸Å­ °æÇèÄ¡¸¦ ÁÝ´Ï´Ù." << endl;

    /*	if (curLevel >= VAMPIRE_MAX_LEVEL)
        {
            // ·¹º§ ÇÑ°è¿¡ µµ´ÞÇØµµ °æÇèÄ¡´Â ½×°Ô ÇØÁØ´Ù.
            // by sigi. 2002.8.31
            Exp_t NewExp = pVampire->getExp() + Point;

            WORD ExpSaveCount = pVampire->getExpSaveCount();
            if (ExpSaveCount > VAMPIRE_EXP_SAVE_PERIOD)
            {
                char pField[80];
                sprintf(pField, "Exp=%lu", NewExp);
                pVampire->tinysave(pField);

                ExpSaveCount = 0;
            }
            else ExpSaveCount++;
            pVampire->setExpSaveCount(ExpSaveCount);

            pVampire->setExp( NewExp );

            return;
        }*/

    //	Exp_t OldExp = pVampire->getExp();

    Exp_t OldGoalExp = pVampire->getGoalExp();
    Exp_t NewGoalExp = max(0, (int)(OldGoalExp - Point));

    // ´©Àû °æÇèÄ¡¿¡´Â ¸ñÇ¥ °æÇèÄ¡°¡ ÁÙ¾îµç ¸¸Å­ ÇÃ·¯½º ÇÏ¿©¾ß ÇÑ´Ù.
    //	Exp_t DiffGoalExp = max(0, (int)(OldGoalExp - NewGoalExp));
    //	Exp_t NewExp      = OldExp + DiffGoalExp;

    //	pVampire->setExp(NewExp);
    pVampire->setGoalExp(NewGoalExp);

    //	_ModifyInfo.addLongData(MODIFY_VAMP_GOAL_EXP, NewGoalExp);

    // ¸ñÇ¥ °æÇèÄ¡°¡ 0ÀÌ ¾Æ´Ï°Å³ª, ÇöÀç ·¹º§ÀÌ 115 ÀÌ»óÀÌ¶ó¸é °æÇèÄ¡¸¸ ÀúÀåÇÏ°í,
    // ·¹º§Àº ¿Ã¶ó°¡Áö ¾Ê´Â´Ù.
    /*	if (NewGoalExp > 0 || curLevel >= 115)
        {
            WORD ExpSaveCount = pVampire->getExpSaveCount();

            // °æÇèÄ¡ ¼¼ÀÌºê Ä«¿îÆ®°¡ ÀÏÁ¤ ¼öÄ¡¿¡ ´Ù´Ù¸£¸é ¼¼ÀÌºêÇÏ°í,
            // Ä«¿îÆ®¸¦ ÃÊ±âÈ­½ÃÄÑ ÁØ´Ù.
            if (ExpSaveCount > VAMPIRE_EXP_SAVE_PERIOD)
            {
                StringStream attrsave;
                attrsave << "Exp = " << NewExp << ", GoalExp = " << NewGoalExp;
                pVampire->tinysave(attrsave.toString());

                ExpSaveCount = 0;
            }
            else ExpSaveCount++;

            pVampire->setExpSaveCount(ExpSaveCount);
        }
        // ¸ñÇ¥ °æÇèÄ¡°¡ 0 ÀÌ¶ó¸é ·¹º§ ¾÷ÀÌ´Ù.
        else*/
    if (NewGoalExp > 0 || curLevel == VAMPIRE_MAX_LEVEL) {
        _ModifyInfo.addLongData(MODIFY_VAMP_GOAL_EXP, NewGoalExp);
        WORD ExpSaveCount = pVampire->getExpSaveCount();

        // °æÇèÄ¡ ¼¼ÀÌºê Ä«¿îÆ®°¡ ÀÏÁ¤ ¼öÄ¡¿¡ ´Ù´Ù¸£¸é ¼¼ÀÌºêÇÏ°í,
        // Ä«¿îÆ®¸¦ ÃÊ±âÈ­½ÃÄÑ ÁØ´Ù.
        if (ExpSaveCount > VAMPIRE_EXP_SAVE_PERIOD) {
            // cout << "°æÇèÄ¡¸¦ ÀúÀåÇÕ´Ï´Ù." << endl;

            StringStream attrsave;
            //			attrsave << "Exp = " << NewExp << ", GoalExp = " << NewGoalExp;
            attrsave << "GoalExp = " << NewGoalExp;
            pVampire->tinysave(attrsave.toString());

            ExpSaveCount = 0;
        } else
            ExpSaveCount++;

        pVampire->setExpSaveCount(ExpSaveCount);
    } else {
        // cout << "·¹º§ÀÌ ¿Ã¶ú½À´Ï´Ù." << endl;
        //  ·¹º§ ¾÷!!
        VAMPIRE_RECORD prev;
        pVampire->getVampireRecord(prev);

        curLevel++;

        pVampire->setLevel(curLevel);
        _ModifyInfo.addShortData(MODIFY_LEVEL, curLevel);

        // add bonus point
        Bonus_t bonus = pVampire->getBonus();

        //		if ((pVampire->getSTR(ATTR_BASIC) + pVampire->getDEX(ATTR_BASIC) + pVampire->getINT(ATTR_BASIC) +
        // pVampire->getBonus() - 60) < ((pVampire->getLevel() - 1) * 3))
        {
            // ·¹º§¿¡ »ó°üÄ¡ ¾Ê°í, ¹«Á¶°Ç 3À¸·Î º¯°æµÇ¾ú´Ù.
            // 2001.12.12 ±è¼º¹Î
            bonus += 3;
        }

        pVampire->setBonus(bonus);
        _ModifyInfo.addShortData(MODIFY_BONUS_POINT, bonus);

        //		VampEXPInfo* pBeforeExpInfo = g_pVampEXPInfoManager->getVampEXPInfo(curLevel-1);
        VampEXPInfo* pNextExpInfo = g_pVampEXPInfoManager->getVampEXPInfo(curLevel);
        Exp_t NextGoalExp = pNextExpInfo->getGoalExp();

        pVampire->setGoalExp(NextGoalExp);
        _ModifyInfo.addLongData(MODIFY_VAMP_GOAL_EXP, NextGoalExp);
        // cout << "³²Àº °æÇèÄ¡´Â " << NextGoalExp << " ÀÔ´Ï´Ù." << endl;

        StringStream sav;
        sav << "Level = "
            << (int)curLevel
            //			<< ",Exp = " << (int)pBeforeExpInfo->getAccumExp()
            << ",GoalExp = " << (int)NextGoalExp << ",Bonus = " << (int)bonus;
        pVampire->tinysave(sav.toString());

        // ·¹º§ÀÌ ¿Ã¶ó¼­ »õ·Î ¹è¿ï ¼ö ÀÖ´Â ±â¼úÀÌ »ý°å´Ù¸é ±â¼úÀ» ¹è¿ï ¼ö ÀÖ´Ù°í ¾Ë¸°´Ù.
        SkillType_t NewLearnSkillType = g_pSkillInfoManager->getSkillTypeByLevel(SKILL_DOMAIN_VAMPIRE, curLevel);
        if (NewLearnSkillType != 0) {
            // ¹è¿ï ¼ö ÀÖ´Â ±â¼úÀÌ ÀÖ°í ÀÌ¹Ì ¹è¿ìÁö ¾ÊÀº »óÅÂ¶ó¸é ±â¼úÀ» ¹è¿ï ¼ö ÀÖ´Ù´Â ÆÐÅ¶À» ³¯¸°´Ù.
            if (pVampire->hasSkill(NewLearnSkillType) == NULL) {
                // GCLearnSkillReadyÀÇ m_SkillType¿¡ level upµÈ µµ¸ÞÀÎÀÇ °¡Àå ÃÖ±Ù
                // ±â¼úÀ» ´ëÀÔÇÑ´Ù. Áï, Å¬¶óÀÌ¾ðÆ® ±× ´ÙÀ½ ½ºÅ³À» ¹è¿ï¼ö ÀÖ´Ù...
                GCLearnSkillReady readyPacket;
                readyPacket.setSkillDomainType(SKILL_DOMAIN_VAMPIRE);
                pVampire->getPlayer()->sendPacket(&readyPacket);
            }
        }

        healCreatureForLevelUp(pVampire, _ModifyInfo, &prev);

        // ·¹º§¾÷ ÀÌÆåÆ®µµ º¸¿©ÁØ´Ù. by sigi. 2002.11.9
        sendEffectLevelUp(pVampire);

        pVampire->whenQuestLevelUpgrade();

        // GrandMasterÀÎ °æ¿ì´Â Effect¸¦ ºÙ¿©ÁØ´Ù.
        // 100·¾ ³Ñ°í ¾ÆÁ÷ Effect°¡ ¾È ºÙ¾îÀÖ´Ù¸é..
        // by sigi. 2002.11.9
        if (curLevel >= GRADE_GRAND_MASTER_LIMIT_LEVEL &&
            !pVampire->isFlag(Effect::EFFECT_CLASS_GRAND_MASTER_VAMPIRE)) {
            EffectGrandMasterVampire* pEffect = new EffectGrandMasterVampire(pVampire);
            pEffect->setDeadline(999999);

            pVampire->getEffectManager()->addEffect(pEffect);

            // affect()¾È¿¡¼­.. Flag°É¾îÁÖ°í, ÁÖÀ§¿¡ broadcastµµ ÇØÁØ´Ù.
            pEffect->affect();
        } else if (curLevel == 130 || curLevel == 150) {
            Effect* pEffect = pVampire->findEffect(Effect::EFFECT_CLASS_GRAND_MASTER_VAMPIRE);
            if (pEffect != NULL)
                pEffect->affect();
        }
    }
}


//////////////////////////////////////////////////////////////////////////////
// ¾Æ¿ì½ºÅÍ½º °æÇèÄ¡¸¦ °è»êÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
void increaseOustersExp(Ousters* pOusters, Exp_t Point, ModifyInfo& _ModifyInfo) {
    Assert(pOusters != NULL);
    if (Point <= 0)
        return;
    if (pOusters->isAdvanced())
        return;

    Level_t curLevel = pOusters->getLevel();

    // ´ÙÀÌ³ª¹Í Á¸ ¾È¿¡¼­´Â °æÇèÄ¡¸¦ ¾È ¿Ã·ÁÁØ´Ù.
    if (pOusters->getZone() != NULL && pOusters->getZone()->isDynamicZone())
        return;

    // VariableManager¿¡ ÀÇÇÑ Áõ°¡
    if (g_pVariableManager->getExpRatio() > 100 && g_pVariableManager->getEventActivate() == 1)
        Point = getPercentValue(Point, g_pVariableManager->getExpRatio());

    if (pOusters->isFlag(Effect::EFFECT_CLASS_BONUS_EXP))
        Point *= 2;

    // °æÇèÄ¡ µÎ¹è
    if (isAffectExp2X())
        Point *= 2;

    // ½Ã°£´ë¿¡ µû¶ó ¿Ã¶ó°¡´Â °æÇèÄ¡°¡ ´Þ¶óÁø´Ù.
    Point = (Exp_t)getPercentValue(Point, DomainExpTimebandFactor[getZoneTimeband(pOusters->getZone())]);

    // cout << pOusters->getName() << " ¿¡°Ô " << Point << "¸¸Å­ °æÇèÄ¡¸¦ ÁÝ´Ï´Ù." << endl;

    /*	if (curLevel >= OUSTERS_MAX_LEVEL)
        {
            // ·¹º§ ÇÑ°è¿¡ µµ´ÞÇØµµ °æÇèÄ¡´Â ½×°Ô ÇØÁØ´Ù.
            // by sigi. 2002.8.31
            Exp_t NewExp = pOusters->getExp() + Point;

            WORD ExpSaveCount = pOusters->getExpSaveCount();
            if (ExpSaveCount > OUSTERS_EXP_SAVE_PERIOD)
            {
                char pField[80];
                sprintf(pField, "Exp=%lu", NewExp);
                pOusters->tinysave(pField);

                ExpSaveCount = 0;
            }
            else ExpSaveCount++;
            pOusters->setExpSaveCount(ExpSaveCount);

            pOusters->setExp( NewExp );

            return;
        }

        Exp_t OldExp = pOusters->getExp();
    */
    Exp_t OldGoalExp = pOusters->getGoalExp();
    Exp_t NewGoalExp = max(0, (int)(OldGoalExp - Point));

    // ´©Àû °æÇèÄ¡¿¡´Â ¸ñÇ¥ °æÇèÄ¡°¡ ÁÙ¾îµç ¸¸Å­ ÇÃ·¯½º ÇÏ¿©¾ß ÇÑ´Ù.
    //	Exp_t DiffGoalExp = max(0, (int)(OldGoalExp - NewGoalExp));
    //	Exp_t NewExp      = OldExp + DiffGoalExp;

    //	pOusters->setExp(NewExp);
    pOusters->setGoalExp(NewGoalExp);

    //	_ModifyInfo.addLongData(MODIFY_OUSTERS_EXP, NewExp);

    //	if ( NewGoalExp > 0 )
    if (NewGoalExp > 0 || curLevel == OUSTERS_MAX_LEVEL) {
        WORD ExpSaveCount = pOusters->getExpSaveCount();
        _ModifyInfo.addLongData(MODIFY_OUSTERS_GOAL_EXP, NewGoalExp);

        // °æÇèÄ¡ ¼¼ÀÌºê Ä«¿îÆ®°¡ ÀÏÁ¤ ¼öÄ¡¿¡ ´Ù´Ù¸£¸é ¼¼ÀÌºêÇÏ°í,
        // Ä«¿îÆ®¸¦ ÃÊ±âÈ­½ÃÄÑ ÁØ´Ù.
        if (ExpSaveCount > OUSTERS_EXP_SAVE_PERIOD) {
            StringStream attrsave;
            attrsave << "GoalExp = " << NewGoalExp;
            pOusters->tinysave(attrsave.toString());

            ExpSaveCount = 0;
        } else
            ExpSaveCount++;

        pOusters->setExpSaveCount(ExpSaveCount);
    } else {
        // ·¹º§ ¾÷!!
        OUSTERS_RECORD prev;
        pOusters->getOustersRecord(prev);

        curLevel++;
        pOusters->setLevel(curLevel);

        //		OustersEXPInfo* pBeforeExpInfo = g_pOustersEXPInfoManager->getOustersEXPInfo(curLevel-1);
        OustersEXPInfo* pNextExpInfo = g_pOustersEXPInfoManager->getOustersEXPInfo(curLevel);
        Exp_t NextGoalExp = pNextExpInfo->getGoalExp();

        // add bonus point
        Bonus_t bonus = pOusters->getBonus();
        SkillBonus_t skillBonus = pOusters->getSkillBonus();

        bonus += 3;
        skillBonus += pNextExpInfo->getSkillPointBonus();

        pOusters->setBonus(bonus);
        pOusters->setSkillBonus(skillBonus);

        _ModifyInfo.addShortData(MODIFY_LEVEL, curLevel);
        _ModifyInfo.addShortData(MODIFY_BONUS_POINT, bonus);
        _ModifyInfo.addShortData(MODIFY_SKILL_BONUS_POINT, skillBonus);

        pOusters->setGoalExp(NextGoalExp);
        _ModifyInfo.addLongData(MODIFY_OUSTERS_GOAL_EXP, NextGoalExp);

        StringStream sav;
        sav << "Level = "
            << (int)curLevel
            //			<< ",Exp = " << (int)pBeforeExpInfo->getAccumExp()
            << ",GoalExp = " << (int)NextGoalExp << ",Bonus = " << (int)bonus << ",SkillBonus = " << (int)skillBonus;
        pOusters->tinysave(sav.toString());

        // ·¹º§ÀÌ ¿Ã¶ó¼­ »õ·Î ¹è¿ï ¼ö ÀÖ´Â ±â¼úÀÌ »ý°å´Ù¸é ±â¼úÀ» ¹è¿ï ¼ö ÀÖ´Ù°í ¾Ë¸°´Ù.
        SkillType_t NewLearnSkillType = g_pSkillInfoManager->getSkillTypeByLevel(SKILL_DOMAIN_OUSTERS, curLevel);
        if (NewLearnSkillType != 0) {
            // ¹è¿ï ¼ö ÀÖ´Â ±â¼úÀÌ ÀÖ°í ÀÌ¹Ì ¹è¿ìÁö ¾ÊÀº »óÅÂ¶ó¸é ±â¼úÀ» ¹è¿ï ¼ö ÀÖ´Ù´Â ÆÐÅ¶À» ³¯¸°´Ù.
            if (pOusters->hasSkill(NewLearnSkillType) == NULL) {
                // GCLearnSkillReadyÀÇ m_SkillType¿¡ level upµÈ µµ¸ÞÀÎÀÇ °¡Àå ÃÖ±Ù
                // ±â¼úÀ» ´ëÀÔÇÑ´Ù. Áï, Å¬¶óÀÌ¾ðÆ® ±× ´ÙÀ½ ½ºÅ³À» ¹è¿ï¼ö ÀÖ´Ù...
                GCLearnSkillReady readyPacket;
                readyPacket.setSkillDomainType(SKILL_DOMAIN_OUSTERS);
                pOusters->getPlayer()->sendPacket(&readyPacket);
            }
        }

        healCreatureForLevelUp(pOusters, _ModifyInfo, &prev);

        // ·¹º§¾÷ ÀÌÆåÆ®µµ º¸¿©ÁØ´Ù. by sigi. 2002.11.9
        sendEffectLevelUp(pOusters);

        pOusters->whenQuestLevelUpgrade();

        // GrandMasterÀÎ °æ¿ì´Â Effect¸¦ ºÙ¿©ÁØ´Ù.
        // 100·¾ ³Ñ°í ¾ÆÁ÷ Effect°¡ ¾È ºÙ¾îÀÖ´Ù¸é..
        // by sigi. 2002.11.9
        if (curLevel >= GRADE_GRAND_MASTER_LIMIT_LEVEL &&
            !pOusters->isFlag(Effect::EFFECT_CLASS_GRAND_MASTER_OUSTERS)) {
            EffectGrandMasterOusters* pEffect = new EffectGrandMasterOusters(pOusters);
            pEffect->setDeadline(999999);

            pOusters->getEffectManager()->addEffect(pEffect);

            // affect()¾È¿¡¼­.. Flag°É¾îÁÖ°í, ÁÖÀ§¿¡ broadcastµµ ÇØÁØ´Ù.
            pEffect->affect();
        } else if (curLevel == 130 || curLevel == 150) {
            Effect* pEffect = pOusters->findEffect(Effect::EFFECT_CLASS_GRAND_MASTER_OUSTERS);
            if (pEffect != NULL)
                pEffect->affect();
        }
    }
}


//////////////////////////////////////////////////////////////////////////////
// ½½·¹ÀÌ¾î ¹× ¹ìÆÄÀÌ¾î ¸í¼ºÀ» °è»êÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
void increaseFame(Creature* pCreature, uint amount) {
    if (pCreature == NULL)
        return;

    // PKÁ¸ ¾È¿¡¼­´Â ¸í¼ºÀ» ¿Ã·ÁÁÖÁö ¾Ê´Â´Ù.
    if (g_pPKZoneInfoManager->isPKZone(pCreature->getZoneID()))
        return;

    // ´ÙÀÌ³ª¹Í Á¸¾È¿¡¼­´Â ¸í¼ºÀ» ¿Ã·ÁÁÖÁö ¾Ê´Â´Ù.
    if (pCreature->getZone() != NULL && pCreature->getZone()->isDynamicZone())
        return;

    // ·ÎÄÃ ÆÄÆ¼°¡ Á¸ÀçÇÑ´Ù¸é, ÆÄÆ¼¿øÀÇ ¼ýÀÚ¿¡ µû¶ó¼­ ¿Ã¶ó°¡´Â ¼öÄ¡°¡ º¯ÇÑ´Ù.
    int PartyID = pCreature->getPartyID();
    if (PartyID != 0) {
        LocalPartyManager* pLPM = pCreature->getLocalPartyManager();
        Assert(pLPM != NULL);

        int nMemberSize = pLPM->getAdjacentMemberSize(PartyID, pCreature);
        switch (nMemberSize) {
        case 2:
            amount = getPercentValue(amount, 120);
            break;
        case 3:
            amount = getPercentValue(amount, 140);
            break;
        case 4:
            amount = getPercentValue(amount, 160);
            break;
        case 5:
            amount = getPercentValue(amount, 180);
            break;
        case 6:
            amount = getPercentValue(amount, 200);
            break;
        default:
            break;
        }
    }

    StringStream attrsave;

    if (pCreature->isSlayer()) {
        Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
        Fame_t OldFame = pSlayer->getFame();
        Fame_t NewFame = OldFame + amount;

        NewFame = min(2000000000, (int)NewFame);

        if (NewFame != OldFame) {
            WORD FameSaveCount = pSlayer->getFameSaveCount();
            if (FameSaveCount > FAME_SAVE_PERIOD) {
                attrsave << "Fame = " << (int)NewFame;
                pSlayer->tinysave(attrsave.toString());

                FameSaveCount = 0;
            } else
                FameSaveCount++;

            pSlayer->setFameSaveCount(FameSaveCount);

            // ¼¼ÀÌºêÇÏµç ¾È ÇÏµç, ¸í¼ºÄ¡ ¼¼ÆÃÀº ÇØÁà¾ß ÇÑ´Ù.
            pSlayer->setFame(NewFame);
        }
    } else if (pCreature->isVampire()) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);
        Fame_t OldFame = pVampire->getFame();
        Fame_t NewFame = OldFame + amount;

        NewFame = min(2000000000, (int)NewFame);

        if (NewFame != OldFame) {
            WORD FameSaveCount = pVampire->getFameSaveCount();
            if (FameSaveCount > FAME_SAVE_PERIOD) {
                attrsave << "Fame = " << (int)NewFame;
                pVampire->tinysave(attrsave.toString());

                FameSaveCount = 0;
            } else
                FameSaveCount++;

            pVampire->setFameSaveCount(FameSaveCount);

            // ¼¼ÀÌºêÇÏµç ¾È ÇÏµç, ¸í¼ºÄ¡ ¼¼ÆÃÀº ÇØÁà¾ß ÇÑ´Ù.
            pVampire->setFame(NewFame);
        }
    } else if (pCreature->isOusters()) {
        Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);
        Fame_t OldFame = pOusters->getFame();
        Fame_t NewFame = OldFame + amount;

        NewFame = min(2000000000, (int)NewFame);

        if (NewFame != OldFame) {
            WORD FameSaveCount = pOusters->getFameSaveCount();
            if (FameSaveCount > FAME_SAVE_PERIOD) {
                attrsave << "Fame = " << (int)NewFame;
                pOusters->tinysave(attrsave.toString());

                FameSaveCount = 0;
            } else
                FameSaveCount++;

            pOusters->setFameSaveCount(FameSaveCount);

            // ¼¼ÀÌºêÇÏµç ¾È ÇÏµç, ¸í¼ºÄ¡ ¼¼ÆÃÀº ÇØÁà¾ß ÇÑ´Ù.
            pOusters->setFame(NewFame);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////
// ÁöÁ¤µÈ ÁÂÇ¥ ÁÖÀ§ÀÇ ½ºÇÃ·¡½Ã µ¥¹ÌÁö¸¦ ¸ÂÀ» Å©¸®ÃÄ¸¦ »Ì¾Æ¿Â´Ù.
//////////////////////////////////////////////////////////////////////////////
int getSplashVictims(Zone* pZone, int cx, int cy, Creature::CreatureClass CClass, list<Creature*>& creatureList,
                     int splash) {
    VSRect rect(0, 0, pZone->getWidth() - 1, pZone->getHeight() - 1);

    // ÇØ´ç Å©¸®ÃÄ°¡ ½½·¹ÀÌ¾î¶ó¸é, ±× ½½·¹ÀÌ¾î¸¸ ¸Â°í,
    // ÁÖÀ§ÀÇ ´Ù¸¥ ½½·¹ÀÌ¾îµéÀº ¸ÂÁö ¾Ê´Â´Ù.
    if (CClass == Creature::CREATURE_CLASS_SLAYER) {
        if (rect.ptInRect(cx, cy)) {
            Tile& rTile = pZone->getTile(cx, cy);

            if (rTile.hasCreature(Creature::MOVE_MODE_WALKING)) {
                Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_WALKING);
                if (pCreature->getCreatureClass() == CClass) {
                    creatureList.push_back(pCreature);
                }
            }
            // ÇöÀç·Î¼­´Â ³¯¾Æ´Ù´Ï´Â ½½·¹ÀÌ¾î´Â ¾øÁö¸¸...
            if (rTile.hasCreature(Creature::MOVE_MODE_FLYING)) {
                Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_FLYING);
                if (pCreature->getCreatureClass() == CClass) {
                    creatureList.push_back(pCreature);
                }
            }
        }

        return (int)creatureList.size();
    }

    vector<Creature*> creatureVector;
    vector<int> pickedVector;

    for (int i = 0; i < 9; i++) {
        int tilex = cx + dirMoveMask[i].x;
        int tiley = cy + dirMoveMask[i].y;

        if (rect.ptInRect(tilex, tiley)) {
            Tile& rTile = pZone->getTile(tilex, tiley);

            if (rTile.hasCreature(Creature::MOVE_MODE_WALKING)) {
                Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_WALKING);

                if (CClass == Creature::CREATURE_CLASS_MAX) {
                    // CREATURE_CLASS_MAX°¡ ÆÄ¶ó¹ÌÅÍ·Î ³Ñ¾î¿À´Â °æ¿ì¿¡´Â ¹«Á¶°Ç ´õÇÏÀÚ.
                    creatureVector.push_back(pCreature);
                } else if (pCreature->getCreatureClass() == CClass) {
                    // ¾Æ´Ñ °æ¿ì¿¡´Â CreatureClass°¡ °°Àº °æ¿ì¿¡¸¸ ´õÇÑ´Ù.
                    creatureVector.push_back(pCreature);
                }
            }

            if (rTile.hasCreature(Creature::MOVE_MODE_FLYING)) {
                Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_FLYING);
                if (CClass == Creature::CREATURE_CLASS_MAX) {
                    // CREATURE_CLASS_MAX°¡ ÆÄ¶ó¹ÌÅÍ·Î ³Ñ¾î¿À´Â °æ¿ì¿¡´Â ¹«Á¶°Ç ´õÇÏÀÚ.
                    creatureVector.push_back(pCreature);
                } else if (pCreature->getCreatureClass() == CClass) {
                    // ¾Æ´Ñ °æ¿ì¿¡´Â CreatureClass°¡ °°Àº °æ¿ì¿¡¸¸ ´õÇÑ´Ù.
                    creatureVector.push_back(pCreature);
                }
            }
        }
    }

    // ½ºÇÃ·¡½Ã µ¥¹ÌÁö¸¦ ÀÔÈú ³ðµéÀÇ ¼ýÀÚº¸´Ù ÇöÀç ÀÖ´Â Å©¸®ÃÄ°¡ Àû´Ù¸é,
    // ¸ðµÎ ½ºÇÃ·¡½Ã µ¥¹ÌÁö¸¦ ÀÔÈ÷¸é µÈ´Ù.
    if ((int)creatureVector.size() <= splash) {
        for (int i = 0; i < (int)creatureVector.size(); i++) {
            creatureList.push_back(creatureVector[i]);
        }
    }
    // ½ºÇÃ·¡½Ã µ¥¹ÌÁö¸¦ ÀÔÈú ³ðº¸´Ù ÇöÀç Á¸ÀçÇÏ´Â Å©¸®ÃÄµéÀÌ ¸¹´Ù¸é,
    // ÀÌ Áß¿¡ splash ¼ýÀÚ¸¸Å­ÀÇ Å©¸®ÃÄ¸¦ ÀÓÀÇ·Î »Ì¾Æ¾ß ÇÑ´Ù.
    else {
        // Á¦ÀÏ Ã³À½¿¡ 6³ðÀÌ ÀÖ°í, ÀÌ Áß¿¡ 4³ðÀ» »Ì¾Æ¾ß ÇÑ´Ù°í
        // °¡Á¤ÇÏ¸é, size = 6ÀÌ µÈ´Ù.
        // Indexes ¹è¿­¿¡´Â (0, 1, 2, 3, 4, 5, -1...)ÀÌ µé¾î°£´Ù.
        // ÀÌ Áß¿¡ 2¸¦ »Ì¾Ò´Ù°í °¡Á¤ÇÏÀÚ.
        // ±×·¯¸é ÀÌ ¹è¿­¿¡¼­ 2¸¦ Á¦°ÅÇØ Áà¾ß ÇÑ´Ù.
        // µÚ¿¡¼­ºÎÅÍ ¾ÕÀ¸·Î ÇÑÄ­¾¿ ¿Å°ÜÁà¾ß ÇÑ´Ù.
        // (0, 1, 3, 4, 5, 5...)
        // ±× ´ÙÀ½ »çÀÌÁî¸¦ ÁÙÀÌ°í, ´Ù½Ã ±× Áß¿¡¼­ ÇÏ³ª¸¦ ·£´ýÀ¸·Î
        // »Ì¾Æ°¡¸é °ãÄ¡Áö ¾Ê´Â Å©¸®ÃÄÀÇ ¸®½ºÆ®¸¦ ¾òÀ» ¼ö ÀÖ´Ù.
        std::vector<int> Indexes(creatureVector.size(), -1);
        int i;
        int size = creatureVector.size();
        for (i = 0; i < size; i++) {
            Indexes[i] = i;
        }

        for (i = 0; i < splash; i++) {
            int index = rand() % size;
            int realIndex = Indexes[index];
            creatureList.push_back(creatureVector[realIndex]);

            for (int m = index + 1; m < size; m++) {
                Indexes[m - 1] = Indexes[m];
            }

            size--;
        }
    }

    return (int)creatureList.size();
}


//////////////////////////////////////////////////////////////////////////////
// ÁöÁ¤µÈ ÁÂÇ¥ ÁÖÀ§ÀÇ Å©¸®Ã³¸¦ Ã£¾Æ¼­ ³Ñ°ÜÁØ´Ù.
//////////////////////////////////////////////////////////////////////////////
int getSplashVictims(Zone* pZone, int cx, int cy, Creature::CreatureClass CClass, list<Creature*>& creatureList,
                     int splash, int range) {
    VSRect rect(0, 0, pZone->getWidth() - 1, pZone->getHeight() - 1);

    vector<Creature*> creatureVector;
    vector<int> pickedVector;

    for (int y = -range; y <= range; y++) {
        for (int x = -range; x <= range; x++) {
            int tilex = cx + x;
            int tiley = cy + y;
            ;

            if (rect.ptInRect(tilex, tiley)) {
                Tile& rTile = pZone->getTile(tilex, tiley);

                if (rTile.hasCreature(Creature::MOVE_MODE_WALKING)) {
                    Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_WALKING);

                    if (CClass == Creature::CREATURE_CLASS_MAX) {
                        // CREATURE_CLASS_MAX°¡ ÆÄ¶ó¹ÌÅÍ·Î ³Ñ¾î¿À´Â °æ¿ì¿¡´Â ¹«Á¶°Ç ´õÇÏÀÚ.
                        creatureVector.push_back(pCreature);
                    } else if (pCreature->getCreatureClass() == CClass) {
                        // ¾Æ´Ñ °æ¿ì¿¡´Â CreatureClass°¡ °°Àº °æ¿ì¿¡¸¸ ´õÇÑ´Ù.
                        creatureVector.push_back(pCreature);
                    }
                }

                if (rTile.hasCreature(Creature::MOVE_MODE_FLYING)) {
                    Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_FLYING);
                    if (CClass == Creature::CREATURE_CLASS_MAX) {
                        // CREATURE_CLASS_MAX°¡ ÆÄ¶ó¹ÌÅÍ·Î ³Ñ¾î¿À´Â °æ¿ì¿¡´Â ¹«Á¶°Ç ´õÇÏÀÚ.
                        creatureVector.push_back(pCreature);
                    } else if (pCreature->getCreatureClass() == CClass) {
                        // ¾Æ´Ñ °æ¿ì¿¡´Â CreatureClass°¡ °°Àº °æ¿ì¿¡¸¸ ´õÇÑ´Ù.
                        creatureVector.push_back(pCreature);
                    }
                }
            }
        }
    }

    // ½ºÇÃ·¡½Ã µ¥¹ÌÁö¸¦ ÀÔÈú ³ðµéÀÇ ¼ýÀÚº¸´Ù ÇöÀç ÀÖ´Â Å©¸®ÃÄ°¡ Àû´Ù¸é,
    // ¸ðµÎ ½ºÇÃ·¡½Ã µ¥¹ÌÁö¸¦ ÀÔÈ÷¸é µÈ´Ù.
    if ((int)creatureVector.size() <= splash) {
        for (int i = 0; i < (int)creatureVector.size(); i++) {
            creatureList.push_back(creatureVector[i]);
        }
    }
    // ½ºÇÃ·¡½Ã µ¥¹ÌÁö¸¦ ÀÔÈú ³ðº¸´Ù ÇöÀç Á¸ÀçÇÏ´Â Å©¸®ÃÄµéÀÌ ¸¹´Ù¸é,
    // ÀÌ Áß¿¡ splash ¼ýÀÚ¸¸Å­ÀÇ Å©¸®ÃÄ¸¦ ÀÓÀÇ·Î »Ì¾Æ¾ß ÇÑ´Ù.
    else {
        // Á¦ÀÏ Ã³À½¿¡ 6³ðÀÌ ÀÖ°í, ÀÌ Áß¿¡ 4³ðÀ» »Ì¾Æ¾ß ÇÑ´Ù°í
        // °¡Á¤ÇÏ¸é, size = 6ÀÌ µÈ´Ù.
        // Indexes ¹è¿­¿¡´Â (0, 1, 2, 3, 4, 5, -1...)ÀÌ µé¾î°£´Ù.
        // ÀÌ Áß¿¡ 2¸¦ »Ì¾Ò´Ù°í °¡Á¤ÇÏÀÚ.
        // ±×·¯¸é ÀÌ ¹è¿­¿¡¼­ 2¸¦ Á¦°ÅÇØ Áà¾ß ÇÑ´Ù.
        // µÚ¿¡¼­ºÎÅÍ ¾ÕÀ¸·Î ÇÑÄ­¾¿ ¿Å°ÜÁà¾ß ÇÑ´Ù.
        // (0, 1, 3, 4, 5, 5...)
        // ±× ´ÙÀ½ »çÀÌÁî¸¦ ÁÙÀÌ°í, ´Ù½Ã ±× Áß¿¡¼­ ÇÏ³ª¸¦ ·£´ýÀ¸·Î
        // »Ì¾Æ°¡¸é °ãÄ¡Áö ¾Ê´Â Å©¸®ÃÄÀÇ ¸®½ºÆ®¸¦ ¾òÀ» ¼ö ÀÖ´Ù.
        std::vector<int> Indexes(creatureVector.size(), -1);
        int i;
        int size = creatureVector.size();
        for (i = 0; i < size; i++) {
            Indexes[i] = i;
        }

        for (i = 0; i < splash; i++) {
            int index = rand() % size;
            int realIndex = Indexes[index];
            creatureList.push_back(creatureVector[realIndex]);

            for (int m = index + 1; m < size; m++) {
                Indexes[m - 1] = Indexes[m];
            }

            size--;
        }
    }

    return (int)creatureList.size();
}

//////////////////////////////////////////////////////////////////////////////
// ´É·ÂÄ¡°¡ ÇÏ³ª¶óµµ »ó½ÂÇßÀ» ¶§, HP¿Í MP¸¦ ¸¸¶¥À¸·Î Ã¤¿öÁÖ´Â ÇÔ¼ö´Ù.
// ½½·¹ÀÌ¾î¿ë -- 2002.01.14 ±è¼º¹Î
//////////////////////////////////////////////////////////////////////////////
void healCreatureForLevelUp(Slayer* pSlayer, ModifyInfo& _ModifyInfo, SLAYER_RECORD* prev) {
    // ´É·ÂÄ¡¸¦ Àç°è»êÇÑ´Ù.
    pSlayer->initAllStat();

    // ´É·ÂÄ¡°¡ »ó½ÂÇßÀ¸´Ï ¹«¾ð°¡ ºÎ°¡ÀûÀÎ ´É·ÂÄ¡°¡ º¯ÇßÀ¸¹Ç·Î º¸³»ÁØ´Ù.
    pSlayer->sendRealWearingInfo();
    pSlayer->addModifyInfo(*prev, _ModifyInfo);

    if (pSlayer->isDead())
        return;

    // ´É·ÂÄ¡°¡ ÇÏ³ª¶óµµ »ó½ÂÇß´Ù¸é HP¿Í MP¸¦ ¸¸¶¥À¸·Î Ã¤¿öÁØ´Ù.
    HP_t OldHP = pSlayer->getHP(ATTR_CURRENT);
    HP_t OldMP = pSlayer->getMP(ATTR_CURRENT);

    // ¸¸¶¥ Ã¤¿ì±â...
    pSlayer->setHP(pSlayer->getHP(ATTR_MAX), ATTR_CURRENT);
    pSlayer->setMP(pSlayer->getMP(ATTR_MAX), ATTR_CURRENT);

    HP_t NewHP = pSlayer->getHP(ATTR_CURRENT);
    HP_t NewMP = pSlayer->getMP(ATTR_CURRENT);

    // HP°¡ ¹Ù²î¾ú´Ù¸é...
    if (OldHP != NewHP) {
        _ModifyInfo.addShortData(MODIFY_CURRENT_HP, NewHP);

        // ¹Ù²ï Ã¼·ÂÀ» ÁÖÀ§¿¡ ºê·ÎµåÄ³½ºÆÃÇØÁØ´Ù.
        GCStatusCurrentHP gcStatusCurrentHP;
        gcStatusCurrentHP.setObjectID(pSlayer->getObjectID());
        gcStatusCurrentHP.setCurrentHP(NewHP);
        Zone* pZone = pSlayer->getZone();
        Assert(pZone != NULL);
        pZone->broadcastPacket(pSlayer->getX(), pSlayer->getY(), &gcStatusCurrentHP, pSlayer);
    }

    // MP°¡ ¹Ù²î¾ú´Ù¸é...
    if (OldMP != NewMP) {
        _ModifyInfo.addShortData(MODIFY_CURRENT_MP, NewMP);
    }

    //	pSlayer->sendModifyInfo(*prev);
}

//////////////////////////////////////////////////////////////////////////////
// ´É·ÂÄ¡°¡ »ó½ÂÇßÀ» ¶§, HP¸¦ ¸¸¶¥À¸·Î Ã¤¿öÁÖ´Â ÇÔ¼ö´Ù. ¹ìÆÄÀÌ¾î¿ë
// -- 2002.01.14 ±è¼º¹Î
//////////////////////////////////////////////////////////////////////////////
void healCreatureForLevelUp(Vampire* pVampire, ModifyInfo& _ModifyInfo, VAMPIRE_RECORD* prev) {
    // ´É·ÂÄ¡¸¦ Àç°è»êÇÑ´Ù.
    pVampire->initAllStat();

    // ´É·ÂÄ¡°¡ »ó½ÂÇßÀ¸´Ï ¹«¾ð°¡ ºÎ°¡ÀûÀÎ ´É·ÂÄ¡°¡ º¯ÇßÀ¸¹Ç·Î º¸³»ÁØ´Ù.
    pVampire->sendRealWearingInfo();
    pVampire->addModifyInfo(*prev, _ModifyInfo);

    if (pVampire->isDead())
        return;

    HP_t OldHP = pVampire->getHP(ATTR_CURRENT);

    // ¸¸¶¥ Ã¤¿ì±â...
    pVampire->setHP(pVampire->getHP(ATTR_MAX), ATTR_CURRENT);

    HP_t NewHP = pVampire->getHP(ATTR_CURRENT);

    // HP°¡ ¹Ù²î¾ú´Ù¸é...
    if (OldHP != NewHP) {
        _ModifyInfo.addShortData(MODIFY_CURRENT_HP, NewHP);

        // ¹Ù²ï Ã¼·ÂÀ» ÁÖÀ§¿¡ ºê·ÎµåÄ³½ºÆÃÇØÁØ´Ù.
        GCStatusCurrentHP gcStatusCurrentHP;
        gcStatusCurrentHP.setObjectID(pVampire->getObjectID());
        gcStatusCurrentHP.setCurrentHP(NewHP);
        Zone* pZone = pVampire->getZone();
        Assert(pZone != NULL);
        pZone->broadcastPacket(pVampire->getX(), pVampire->getY(), &gcStatusCurrentHP, pVampire);
    }

    //	pVampire->sendModifyInfo(*prev);
}

//////////////////////////////////////////////////////////////////////////////
// ±â¼ú ½ÇÆÐ½Ã ÆÐÅ¶À» ³¯¸°´Ù.
// ÀÏ¹ÝÀûÀÎ ½ÇÆÐ (È÷Æ®·Ñ ½ÇÆÐÇß´Ù´ø°¡, ¸¶³ª°¡ ¾ø´Ù´ø°¡...)ÀÏ °æ¿ì,
// º»ÀÎ°ú ±×°ÍÀ» º¸´Â ÀÌµé¿¡°Ô ÆÐÅ¶À» ³¯¸°´Ù.
//////////////////////////////////////////////////////////////////////////////
void executeSkillFailNormal(Creature* pCreature, SkillType_t SkillType, Creature* pTargetCreature, BYTE Grade) {
    Assert(pCreature != NULL);

    if (pCreature->isPC()) {
        GCSkillFailed1 gcSkillFailed1;
        gcSkillFailed1.setSkillType(SkillType);
        gcSkillFailed1.setGrade(Grade);
        (pCreature->getPlayer())->sendPacket(&gcSkillFailed1);
    }

    GCSkillFailed2 gcSkillFailed2;
    gcSkillFailed2.setSkillType(SkillType);
    gcSkillFailed2.setObjectID(pCreature->getObjectID());
    gcSkillFailed2.setGrade(Grade);

    // ObjectSkillÀÏ °æ¿ì, »ó´ë¹æÀÇ OID°¡ Á¸ÀçÇÑ´Ù¸é ÆÐÅ¶¿¡´Ù ½Ç¾î¼­ º¸³»ÁØ´Ù.
    // ¼¿ÇÁ ½ºÅ³ÀÌ³ª Å¸ÀÏ ½ºÅ³ÀÎ °æ¿ì¿¡´Â NULL·Î parameter°¡ ³Ñ¾î¿À´Â °ÍÀÌ Á¤»óÀÌ´Ù.
    // (Å¬¶óÀÌ¾ðÆ®¿¡¼­´Â ¼¿ÇÁ³ª Å¸ÀÏ ½ºÅ³ÀÌ ½ÇÆÐÇØ¼­ ³¯¾Æ¿À´Â GCSkillFailed2ÀÏ °æ¿ì¿¡´Â,
    // TargetObjectID¸¦ ÀÐÁöµµ ¾Ê´Â´Ù.)
    if (pTargetCreature != NULL) {
        gcSkillFailed2.setTargetObjectID(pTargetCreature->getObjectID());
    } else {
        gcSkillFailed2.setTargetObjectID(0);
    }

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcSkillFailed2, pCreature);
}

//////////////////////////////////////////////////////////////////////////////
// ±â¼ú ½ÇÆÐ½Ã ÆÐÅ¶À» ³¯¸°´Ù.
// ½ºÅ³ÀÇ °á°ú¸¦ 2¹ø ³¯·ÁÁà¾ß µÈ´Ù.
// ¶ó¹Ù ¸¸µé±â¿¡ ´ëÇÑ °Í ÇÏ³ª ÇÏ°í
// Èí¿µ¿¡ °üÇÑ °Í ÇÏ³ª.
// ±×·¡¼­ Ã³À½¿¡ Á¶°Ç Ã¼Å©ÇÏ´Ù°¡ ½ÇÆÐÇÒ °æ¿ì¿¡
// SkillFail ÆÐÅ¶À» 2¹ø º¸³»ÁØ´Ù.
//////////////////////////////////////////////////////////////////////////////
void executeAbsorbSoulSkillFail(Creature* pCreature, SkillType_t SkillType, ObjectID_t TargetObjectID, bool bBroadcast,
                                bool bSendTwice) {
    Assert(pCreature != NULL);

    // Å¬¶óÀÌ¾ðÆ®¿¡ ¶ôÀÌ °É·ÁÀÖÀ¸¸é ½ºÅ³ »ç¿ëÇÑ º»ÀÎ¿¡°Ô´Â °ËÁõ ÆÐÅ¶À» 2¹ø º¸³»Áà¾ß µÈ´Ù.
    if (pCreature->isPC()) {
        GCSkillFailed1 gcSkillFailed1;
        gcSkillFailed1.setSkillType(SkillType);
        (pCreature->getPlayer())->sendPacket(&gcSkillFailed1);
        if (bSendTwice)
            (pCreature->getPlayer())->sendPacket(&gcSkillFailed1);
    }

    if (bBroadcast) {
        GCSkillFailed2 gcSkillFailed2;
        gcSkillFailed2.setSkillType(SkillType);
        gcSkillFailed2.setObjectID(pCreature->getObjectID());
        gcSkillFailed2.setTargetObjectID(TargetObjectID);

        Zone* pZone = pCreature->getZone();
        Assert(pZone != NULL);

        pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcSkillFailed2, pCreature);
    }
}

//////////////////////////////////////////////////////////////////////////////
// ±â¼ú ½ÇÆÐ½Ã ÆÐÅ¶À» ³¯¸°´Ù.
// ÀÏ¹ÝÀûÀÎ ½ÇÆÐ (È÷Æ®·Ñ ½ÇÆÐÇß´Ù´ø°¡, ¸¶³ª°¡ ¾ø´Ù´ø°¡...)ÀÏ °æ¿ì,
// º»ÀÎ°ú ±×°ÍÀ» º¸´Â ÀÌµé¿¡°Ô ÆÐÅ¶À» ³¯¸°´Ù.
//////////////////////////////////////////////////////////////////////////////
void executeSkillFailNormalWithGun(Creature* pCreature, SkillType_t SkillType, Creature* pTargetCreature,
                                   BYTE RemainBullet) {
    Assert(pCreature != NULL);

    if (pCreature->isPC()) {
        GCSkillFailed1 gcSkillFailed1;
        gcSkillFailed1.setSkillType(SkillType);
        gcSkillFailed1.addShortData(MODIFY_BULLET, RemainBullet);
        (pCreature->getPlayer())->sendPacket(&gcSkillFailed1);
    }

    GCSkillFailed2 gcSkillFailed2;
    gcSkillFailed2.setSkillType(SkillType);
    gcSkillFailed2.setObjectID(pCreature->getObjectID());

    // ObjectSkillÀÏ °æ¿ì, »ó´ë¹æÀÇ OID°¡ Á¸ÀçÇÑ´Ù¸é ÆÐÅ¶¿¡´Ù ½Ç¾î¼­ º¸³»ÁØ´Ù.
    // ¼¿ÇÁ ½ºÅ³ÀÌ³ª Å¸ÀÏ ½ºÅ³ÀÎ °æ¿ì¿¡´Â NULL·Î parameter°¡ ³Ñ¾î¿À´Â °ÍÀÌ Á¤»óÀÌ´Ù.
    // (Å¬¶óÀÌ¾ðÆ®¿¡¼­´Â ¼¿ÇÁ³ª Å¸ÀÏ ½ºÅ³ÀÌ ½ÇÆÐÇØ¼­ ³¯¾Æ¿À´Â GCSkillFailed2ÀÏ °æ¿ì¿¡´Â,
    // TargetObjectID¸¦ ÀÐÁöµµ ¾Ê´Â´Ù.)
    if (pTargetCreature != NULL) {
        gcSkillFailed2.setTargetObjectID(pTargetCreature->getObjectID());
    } else {
        gcSkillFailed2.setTargetObjectID(0);
    }

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    pZone->broadcastPacket(pCreature->getX(), pCreature->getY(), &gcSkillFailed2, pCreature);
}

//////////////////////////////////////////////////////////////////////////////
// ±â¼ú ½ÇÆÐ½Ã ÆÐÅ¶À» ³¯¸°´Ù.
// ¿¹¿ÜÀûÀÎ ½ÇÆÐ (NPC¸¦ °ø°ÝÇß´Ù´ø°¡...)
// º»ÀÎ¿¡°Ô¸¸ ÆÐÅ¶À» ³¯¸°´Ù.
//////////////////////////////////////////////////////////////////////////////
void executeSkillFailException(Creature* pCreature, SkillType_t SkillType, BYTE Grade) {
    // by sigi. 2002.5.8
    //	Assert(pCreature != NULL);

    if (pCreature != NULL && pCreature->isPC()) {
        GCSkillFailed1 gcSkillFailed1;
        gcSkillFailed1.setSkillType(SkillType);
        gcSkillFailed1.setGrade(Grade);

        Player* pPlayer = pCreature->getPlayer();
        pPlayer->sendPacket(&gcSkillFailed1);
    }
}

//////////////////////////////////////////////////////////////////////////////
// ´É·ÂÄ¡°¡ »ó½ÂÇßÀ» ¶§, HP, MP¸¦ ¸¸¶¥À¸·Î Ã¤¿öÁÖ´Â ÇÔ¼ö´Ù. ¾Æ¿ì½ºÅÍ½º¿ë
// -- 2003.04.19 by bezz
//////////////////////////////////////////////////////////////////////////////
void healCreatureForLevelUp(Ousters* pOusters, ModifyInfo& _ModifyInfo, OUSTERS_RECORD* prev) {
    // ´É·ÂÄ¡¸¦ Àç°è»êÇÑ´Ù.
    pOusters->initAllStat();

    // ´É·ÂÄ¡°¡ »ó½ÂÇßÀ¸´Ï ¹«¾ð°¡ ºÎ°¡ÀûÀÎ ´É·ÂÄ¡°¡ º¯ÇßÀ¸¹Ç·Î º¸³»ÁØ´Ù.
    pOusters->sendRealWearingInfo();
    pOusters->addModifyInfo(*prev, _ModifyInfo);

    if (pOusters->isDead())
        return;

    HP_t OldHP = pOusters->getHP(ATTR_CURRENT);
    MP_t OldMP = pOusters->getMP(ATTR_CURRENT);

    // ¸¸¶¥ Ã¤¿ì±â...
    pOusters->setHP(pOusters->getHP(ATTR_MAX), ATTR_CURRENT);
    pOusters->setMP(pOusters->getMP(ATTR_MAX), ATTR_CURRENT);

    HP_t NewHP = pOusters->getHP(ATTR_CURRENT);
    MP_t NewMP = pOusters->getMP(ATTR_CURRENT);

    // HP°¡ ¹Ù²î¾ú´Ù¸é...
    if (OldHP != NewHP) {
        _ModifyInfo.addShortData(MODIFY_CURRENT_HP, NewHP);

        // ¹Ù²ï Ã¼·ÂÀ» ÁÖÀ§¿¡ ºê·ÎµåÄ³½ºÆÃÇØÁØ´Ù.
        GCStatusCurrentHP gcStatusCurrentHP;
        gcStatusCurrentHP.setObjectID(pOusters->getObjectID());
        gcStatusCurrentHP.setCurrentHP(NewHP);
        Zone* pZone = pOusters->getZone();
        Assert(pZone != NULL);
        pZone->broadcastPacket(pOusters->getX(), pOusters->getY(), &gcStatusCurrentHP, pOusters);
    }

    if (OldMP != NewMP)
        _ModifyInfo.addShortData(MODIFY_CURRENT_MP, NewMP);

    //	pOusters->sendModifyInfo(*prev);
}


//----------------------------------------------------------------------
// Set Direction To Creature
//----------------------------------------------------------------------
// ´Ù¸¥ Creature¸¦ ÇâÇØ¼­ ¹Ù¶óº»´Ù.
//----------------------------------------------------------------------
// 8¹æÇâ¿¡ µû¸¥ ±âÁØÀÌ µÇ´Â ±â¿ï±â : °¡·Î/¼¼·Î ºñÀ²°ú °ü·Ã
//----------------------------------------------------------------------
const float BASIS_DIRECTION_LOW = 0.35f;
const float BASIS_DIRECTION_HIGH = 3.0f;

Dir_t getDirectionToPosition(int originX, int originY, int destX, int destY) {
    int stepX = destX - originX, stepY = destY - originY;

    // 0ÀÏ ¶§ check
    float k = (stepX == 0) ? 0 : (float)(stepY) / stepX; // ±â¿ï±â

    //--------------------------------------------------
    // ¹æÇâÀ» Á¤ÇØ¾ß ÇÑ´Ù.
    //--------------------------------------------------
    if (stepY == 0) {
        // XÃà
        // - -;;
        if (stepX == 0)
            return DOWN;
        else if (stepX > 0)
            return RIGHT;
        else
            return LEFT;
    } else if (stepY < 0) // UPÂÊÀ¸·Î
    {
        // yÃà À§
        if (stepX == 0) {
            return UP;
        }
        // 1»çºÐ¸é
        else if (stepX > 0) {
            if (k < -BASIS_DIRECTION_HIGH)
                return UP;
            else if (k <= -BASIS_DIRECTION_LOW)
                return RIGHTUP;
            else
                return RIGHT;
        }
        // 2»çºÐ¸é
        else {
            if (k > BASIS_DIRECTION_HIGH)
                return UP;
            else if (k >= BASIS_DIRECTION_LOW)
                return LEFTUP;
            else
                return LEFT;
        }
    }
    // ¾Æ·¡ÂÊ
    else {
        // yÃà ¾Æ·¡
        if (stepX == 0) {
            return DOWN;
        }
        // 4»çºÐ¸é
        else if (stepX > 0) {
            if (k > BASIS_DIRECTION_HIGH)
                return DOWN;
            else if (k >= BASIS_DIRECTION_LOW)
                return RIGHTDOWN;
            else
                return RIGHT;
        }
        // 3»çºÐ¸é
        else {
            if (k < -BASIS_DIRECTION_HIGH)
                return DOWN;
            else if (k <= -BASIS_DIRECTION_LOW)
                return LEFTDOWN;
            else
                return LEFT;
        }
    }
}


Exp_t computeSkillPointBonus(SkillDomainType_t Domain, SkillLevel_t DomainLevel, Item* pWeapon, Exp_t Point) {
    Assert(pWeapon != NULL);

    ItemType_t itemType = pWeapon->getItemType();
    ItemType_t bestItemType =
        g_pSkillDomainInfoManager->getDomainInfo((SkillDomain)Domain, DomainLevel)->getBestItemType();

    Exp_t newPoint;

    if (pWeapon->isUnique()) {
        newPoint = getPercentValue(Point, 120);
    } else if (itemType <= bestItemType) {
        newPoint = Point * (itemType + 1) / (bestItemType + 1);
    } else {
        Exp_t Point120 = getPercentValue(Point, 120);
        newPoint = Point * itemType / (bestItemType + 1);

        newPoint = min(Point120, newPoint);
    }

    // cout << "skillPoint: " << (int)itemType << " / " << (int)bestItemType
    //	 << ", " << (int)Point << " --> " << (int)newPoint << endl;

    // by sigi. 2002.11.5
    newPoint = max(1, (int)newPoint);

    return newPoint;
}


// Á¡°ú Á¡»çÀÌ¸¦ °É¾î¼­ °¥ ¼ö ÀÖ´Â°¡? ( Å©¸®ÃÄ·Î ¸·Èù °æ¿ì´Â Á¦¿Ü )
bool isPassLine(Zone* pZone, ZoneCoord_t sX, ZoneCoord_t sY, ZoneCoord_t eX, ZoneCoord_t eY, bool blockByCreature) {
    list<TPOINT> tpList;

    if (pZone == NULL)
        return false;

    // µÎ Á¡»çÀÌÀÇ Áø¼±À» ÀÌ·ç´Â Á¡µéÀ» ±¸ÇÑ´Ù.
    getLinePoint(sX, sY, eX, eY, tpList);

    if (tpList.empty())
        return false;

    VSRect rect(0, 0, pZone->getWidth() - 1, pZone->getHeight() - 1);

    list<TPOINT>::const_iterator itr = tpList.begin();
    TPOINT prev = (*itr);
    for (; itr != tpList.end(); ++itr) {
        TPOINT tp = (*itr);
        if (!rect.ptInRect(tp.x, tp.y))
            return false;

        if (tp.x == sX && tp.y == sY) {
            // ½ÃÀÛÁ¡Àº Ã¼Å© ¾ÈÇÑ´Ù.
            continue;
        }

        Tile& tile = pZone->getTile(tp.x, tp.y);

        if (blockByCreature) {
            if (tile.isGroundBlocked())
                return false;
        } else if (tile.isFixedGroundBlocked()) {
            return false;
        }

        // ´ë°¢¼±À¸·Î ¹Ù²ï °æ¿ì, ÇÑÂÊ ¹æÇâÀ¸·Î¸¸ °¥¼ö ÀÖ¾îµµ °¡´ÉÇÏ´Ù.
        // (1,1) -> (2,2) ÀÎ °æ¿ì, (1,2) ³ª (2,1) µÑ Áß¿¡ ÇÏ³ª¸¸ Áö³ª°¥ ¼ö ÀÖ¾îµµ Áö³ª°¥ ¼ö ÀÖ´Ù°í º»´Ù.
        if (prev.x != tp.x && prev.y != tp.y) {
            if (!rect.ptInRect(tp.x, prev.y))
                return false;
            if (!rect.ptInRect(prev.x, tp.y))
                return false;

            Tile& tile1 = pZone->getTile(tp.x, prev.y);
            Tile& tile2 = pZone->getTile(prev.x, tp.y);

            if (tile1.isFixedGroundBlocked() && tile2.isFixedGroundBlocked()) {
                return false;
            }
        }

        prev = tp;
    }

    return true;
}

// µÎ Á¡»çÀÌÀÇ Áø¼±À» ÀÌ·ç´Â Á¡µéÀ» ±¸ÇÑ´Ù.
void getLinePoint(ZoneCoord_t sX, ZoneCoord_t sY, ZoneCoord_t eX, ZoneCoord_t eY, list<TPOINT>& tpList) {
    int xLength = abs(sX - eX);
    int yLength = abs(sY - eY);

    if (xLength == 0 && yLength == 0)
        return;

    if (xLength > yLength) {
        if (sX > eX) {
            int tmpX = sX;
            sX = eX;
            eX = tmpX;
            int tmpY = sY;
            sY = eY;
            eY = tmpY;
        }

        float yStep = (float)(eY - sY) / (float)(eX - sX);

        for (int i = sX; i <= eX; i++) {
            TPOINT pt;
            pt.x = i;
            pt.y = sY + (int)(yStep * (float)(i - sX));

            tpList.push_back(pt);
        }
    } else {
        if (sY > eY) {
            int tmpX = sX;
            sX = eX;
            eX = tmpX;
            int tmpY = sY;
            sY = eY;
            eY = tmpY;
        }

        float xStep = (float)(eX - sX) / (float)(eY - sY);

        for (int i = sY; i <= eY; i++) {
            TPOINT pt;
            pt.x = sX + (int)(xStep * (float)(i - sY));
            pt.y = i;

            tpList.push_back(pt);
        }
    }
}

ElementalType getElementalTypeFromString(const string& type) {
    if (type == "Fire")
        return ELEMENTAL_FIRE;
    else if (type == "Water")
        return ELEMENTAL_WATER;
    else if (type == "Earth")
        return ELEMENTAL_EARTH;
    else if (type == "Wind")
        return ELEMENTAL_WIND;

    return ELEMENTAL_MAX;
}


//////////////////////////////////////////////////////////////////////////////
// °ø°ÝÇÒ ¼ö ÀÖ´Â°¡?
// ¹«Àû »óÅÂ³ª non PK ¸¦ À§ÇØ¼­ °ø°ÝÇÒ ¼ö ÀÖ´ÂÁö¸¦ Ã¼Å©ÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
bool canAttack(Creature* pAttacker, Creature* pDefender) {
    Assert(pDefender != NULL);

    // ¹«Àû »óÅÂ Ã¼Å©
    if (pDefender->isFlag(Effect::EFFECT_CLASS_NO_DAMAGE))
        return false;

    // Attacker °¡ NULL ÀÌ¸é °Á true
    // Á¨Àå ¸Õ°¡ ±ò²ûÇÏ°Ô °íÄ¡±â ¹Ù¶÷ Effect¿¡¼­ Ã¼Å©ÇÒ¶§ Attacker °¡ NULL ÀÌ µÉ ¼ö ÀÖ´Ù.
    if (pAttacker == NULL)
        return true;

    // °ÔÀÓ¼­¹ö¿¡ PK ¼³Á¤ÀÌ µÇ¾ú´Â°¡?
    static bool bNonPK =
        g_pGameServerInfoManager
            ->getGameServerInfo(1, g_pConfig->getPropertyInt("ServerID"), g_pConfig->getPropertyInt("WorldID"))
            ->isNonPKServer();
    // bool canPK = bNonPK || GDRLairManager::Instance().isGDRLairZone( pAttacker->getZoneID() );

    // non PK Ã¼Å©
    if (bNonPK && pAttacker->isPC() && pDefender->isPC())
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////////
// add by Coffee 2007-6-9
// Ôö¼ÓÐÂ¼¼ÄÜÈý×åÊ¹ÓÃÐÂ¼¼ÄÜ ¿Û³ý¼¼ÄÜ¿¨ÑéÖ¤
//////////////////////////////////////////////////////////////////////////
bool useSkillCrad(Creature* pCreature) {
    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pCreature->getPlayer());
    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pGamePlayer->getCreature());

    CoordInven_t InvenX = 0;
    CoordInven_t InvenY = 0;
    ItemType_t fitItem = 0; // ËÄÒ¶²Ý
    if (pCreature->isSlayer()) {
        fitItem = 5;
    } else if (pCreature->isVampire()) {
        fitItem = 6;
    } else if (pCreature->isOusters()) {
        fitItem = 7;
    }
    if (fitItem != 0) {
        Item* pItem = pPC->getInventory()->findItem(Item::ITEM_CLASS_MOON_CARD, fitItem, InvenX, InvenY);
        if (pItem == NULL) {
            GCSystemMessage gcSystemMessage1;
            gcSystemMessage1.setMessage("Ê¹ÓÃ¸Ã¼¼ÄÜÐèÒª¼¼ÄÜ¿¨!");
            gcSystemMessage1.setType(SYSTEM_MESSAGE_OPERATOR);
            pGamePlayer->sendPacket(&gcSystemMessage1);
            return false;
        }
        ItemNum_t OldNum = pItem->getNum();
        if (OldNum == 1) {
            pPC->getInventory()->deleteItem(pItem->getObjectID());
            pItem->destroy();
            SAFE_DELETE(pItem);
        } else {
            OldNum--;
            pItem->setNum((pItem->getNum() - 1));
            pItem->save(pPC->getName(), STORAGE_INVENTORY, 0, InvenX, InvenY);
        }

    } else {
        GCSystemMessage gcSystemMessage1;
        gcSystemMessage1.setMessage("Ê¹ÓÃ¸Ã¼¼ÄÜÐèÒª¼¼ÄÜ¿¨!");
        gcSystemMessage1.setType(SYSTEM_MESSAGE_OPERATOR);
        pGamePlayer->sendPacket(&gcSystemMessage1);
        return false;
    }
    return true;
}
