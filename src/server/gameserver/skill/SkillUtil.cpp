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
