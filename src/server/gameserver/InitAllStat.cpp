//////////////////////////////////////////////////////////////////////////////
// Filename    : InitAllStat.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "AbilityBalance.h"
#include "BloodBibleBonus.h"
#include "BloodBibleBonusManager.h"
#include "BloodBibleSignInfo.h"
#include "CastleInfoManager.h"
#include "CombatInfoManager.h"
#include "CoreZap.h"
#include "DefaultOptionSetInfo.h"
#include "Effect.h"
#include "GCAddEffect.h"
#include "GCBloodBibleSignInfo.h"
#include "GCChangeShape.h"
#include "GCModifyInformation.h"
#include "GCRemoveEffect.h"
#include "GameContext.h"
#include "GamePlayer.h"
#include "HolyLandRaceBonus.h"
#include "ItemInfo.h"
#include "ItemInfoManager.h"
#include "ItemUtil.h"
#include "LevelWarZoneInfoManager.h"
#include "Monster.h"
#include "OptionInfo.h"
#include "Ousters.h"
#include "PetInfo.h"
#include "Player.h"
#include "RankBonus.h"
#include "SkillInfo.h"
#include "Slayer.h"
#include "SweeperBonus.h"
#include "SweeperBonusManager.h"
#include "Vampire.h"
#include "VariableManager.h"
#include "WarSystem.h"
#include "Zone.h"
#include "domain/Formulas.h"
#include "item/OustersStone.h"
#include "item/OustersWristlet.h"
#include "skill/CastleSkillSlot.h"
#include "skill/EffectBerserker.h"
#include "skill/EffectBless.h"
#include "skill/EffectBlunting.h"
#include "skill/EffectChargingPower.h"
#include "skill/EffectConcealment.h"
#include "skill/EffectCrossGuard.h"
#include "skill/EffectDancingSword.h"
#include "skill/EffectDeath.h"
#include "skill/EffectDoom.h"
#include "skill/EffectEvade.h"
#include "skill/EffectExpansion.h"
#include "skill/EffectExtreme.h"
#include "skill/EffectGhostBlade.h"
#include "skill/EffectGnomesWhisper.h"
#include "skill/EffectGroundBless.h"
#include "skill/EffectHandsOfFire.h"
#include "skill/EffectHolyArmor.h"
#include "skill/EffectIntimateGrail.h"
#include "skill/EffectMephisto.h"
#include "skill/EffectMindControl.h"
#include "skill/EffectObservingEye.h"
#include "skill/EffectParalyze.h"
#include "skill/EffectPotentialExplosion.h"
#include "skill/EffectProtectionFromAcid.h"
#include "skill/EffectProtectionFromBlood.h"
#include "skill/EffectProtectionFromCurse.h"
#include "skill/EffectProtectionFromPoison.h"
#include "skill/EffectReactiveArmor.h"
#include "skill/EffectRediance.h"
#include "skill/EffectRingOfFlare.h"
#include "skill/EffectSeduction.h"
#include "skill/EffectSharpChakram.h"
#include "skill/EffectStriking.h"
#include "skill/EffectSummonCasket.h"
#include "skill/EffectTransformToBat.h"
#include "skill/EffectTransformToWerwolf.h"
#include "skill/EffectTransformToWolf.h"
#include "skill/EffectWaterBarrier.h"
#include "skill/EffectWhitsuntide.h"
#include "skill/OustersCastleSkillSlot.h"
#include "skill/VampireCastleSkillSlot.h"

//////////////////////////////////////////////////////////////////////////////
//
// 공통
//
//////////////////////////////////////////////////////////////////////////////
void PlayerCreature::applyBloodBibleSign() {
    int openNum = getBloodBibleSignOpenNum();
    getBloodBibleSign()->setOpenNum(openNum);
    //	int openNum = getBloodBibleSign()->getOpenNum();

    int applyCount = 0;
    vector<ItemType_t>::iterator bItr = getBloodBibleSign()->getList().begin();
    for (; bItr != getBloodBibleSign()->getList().end(); ++bItr) {
        if (applyCount >= openNum)
            break;
        BloodBibleBonus* pBonus = g_pBloodBibleBonusManager->getBloodBibleBonus(*bItr);
        if (pBonus != NULL) {
            OptionTypeList optionTypes = pBonus->getOptionTypeList();
            OptionTypeListConstItor optionItr;
            //			cout << getName() << "에게 " << pBonus->getName() << " 을 적용합니다." << endl;
            ++applyCount;

            for (optionItr = optionTypes.begin(); optionItr != optionTypes.end(); optionItr++) {
                computeOptionStat(*optionItr);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// 몬스터
//
//////////////////////////////////////////////////////////////////////////////
void Monster::initAllStat(void)

{
    __BEGIN_TRY

    const MonsterInfo* pMonsterInfo = g_pMonsterInfoManager->getMonsterInfo(m_MonsterType);

    Creature::CreatureClass CClass = getCreatureClass();
    BASIC_ATTR attr;

    m_Resist[MAGIC_DOMAIN_NO_DOMAIN] = 0;
    m_Resist[MAGIC_DOMAIN_POISON] = 50;
    m_Resist[MAGIC_DOMAIN_ACID] = 0;
    m_Resist[MAGIC_DOMAIN_CURSE] = 0;
    m_Resist[MAGIC_DOMAIN_BLOOD] = 0;

    ////////////////////////////////////////////////////////////
    // 제일 먼저 기본 능력치를 초기화시키고...
    ////////////////////////////////////////////////////////////
    m_STR = pMonsterInfo->getSTR();
    m_DEX = pMonsterInfo->getDEX();
    m_INT = pMonsterInfo->getINT();

    ////////////////////////////////////////////////////////////
    // 기본 능력에 영향을 주는 이펙트를 검사한다.
    ////////////////////////////////////////////////////////////
    attr.nSTR = m_STR;
    attr.nDEX = m_DEX;
    attr.nINT = m_INT;
    attr.nLevel = pMonsterInfo->getLevel();

    ////////////////////////////////////////////////////////////
    // 부가적인 능력치들을 다시 계산한다.
    ////////////////////////////////////////////////////////////
    m_HP[ATTR_MAX] = computeHP(CClass, &attr, pMonsterInfo->getEnhanceHP());
    m_ToHit = computeToHit(CClass, &attr, pMonsterInfo->getEnhanceToHit());
    m_Defense = computeDefense(CClass, &attr, pMonsterInfo->getEnhanceDefense());
    m_Protection = computeProtection(CClass, &attr, pMonsterInfo->getEnhanceProtection());
    m_Damage[ATTR_CURRENT] = computeMinDamage(CClass, &attr, pMonsterInfo->getEnhanceMinDamage());
    m_Damage[ATTR_MAX] = computeMaxDamage(CClass, &attr, pMonsterInfo->getEnhanceMaxDamage());


    // #ifdef __XMAS_EVENT_CODE__
    //  원래 크리스마스 이벤트로 기획된 것이지만, 앞으로 계속 사용될 것으로
    //  예정된다.
    if (m_MonsterType == 358 || m_MonsterType == 359 || m_MonsterType == 360 || m_MonsterType == 361)
        m_HP[ATTR_MAX] = m_HP[ATTR_MAX] * 10;
    // #endif

    ////////////////////////////////////////////////////////////
    // 부가적인 능력치들을 직접 수정하는 이펙트를 검사한다.
    ////////////////////////////////////////////////////////////
    if (isFlag(Effect::EFFECT_CLASS_DOOM)) {
        EffectDoom* pDoom = dynamic_cast<EffectDoom*>(findEffect(Effect::EFFECT_CLASS_DOOM));
        if (pDoom != NULL) {
            int DefensePenalty = getPercentValue(m_Defense, pDoom->getDefensePenalty());
            int ProtectionPenalty = getPercentValue(m_Protection, pDoom->getProtectionPenalty());

            m_Defense = max(0, m_Defense - DefensePenalty);
            m_Protection = max(0, m_Protection - ProtectionPenalty);
        }
    }
    if (isFlag(Effect::EFFECT_CLASS_SEDUCTION)) {
        EffectSeduction* pSeduction = dynamic_cast<EffectSeduction*>(findEffect(Effect::EFFECT_CLASS_SEDUCTION));
        if (pSeduction != NULL) {
            int ToHitPenalty = getPercentValue(m_ToHit, pSeduction->getToHitPenalty());
            int DamagePenalty1 = getPercentValue(m_Damage[ATTR_CURRENT], pSeduction->getDamagePenalty());
            int DamagePenalty2 = getPercentValue(m_Damage[ATTR_MAX], pSeduction->getDamagePenalty());

            m_ToHit = max(0, m_ToHit - ToHitPenalty);
            m_Damage[ATTR_CURRENT] = max(0, m_Damage[ATTR_CURRENT] - DamagePenalty1);
            m_Damage[ATTR_MAX] = max(0, m_Damage[ATTR_MAX] - DamagePenalty2);
        }
    }
    /*	if (isFlag(Effect::EFFECT_CLASS_PARALYZE))
        {
            EffectParalyze* pParalyze = dynamic_cast<EffectParalyze*>(findEffect(Effect::EFFECT_CLASS_PARALYZE));
            if (pParalyze != NULL)
            {
                int DefensePenalty = getPercentValue(m_Defense, pParalyze->getDefensePenalty());
                m_Defense = max(0, m_Defense - DefensePenalty);
            }
        }*/
    if (isFlag(Effect::EFFECT_CLASS_TRANSFORM_TO_WOLF)) {
        EffectTransformToWolf* pTransformToWolf =
            dynamic_cast<EffectTransformToWolf*>(findEffect(Effect::EFFECT_CLASS_TRANSFORM_TO_WOLF));
        if (pTransformToWolf != NULL) {
            int ToHitBonus = getPercentValue(m_ToHit, 20);
            int MinDamageBonus = getPercentValue(m_Damage[ATTR_CURRENT], 20);
            int MaxDamageBonus = getPercentValue(m_Damage[ATTR_MAX], 20);
            int DefensePenalty = getPercentValue(m_Defense, 50);
            int ProtectionPenalty = getPercentValue(m_Protection, 50);

            m_ToHit = min(VAMPIRE_MAX_TOHIT, m_ToHit + ToHitBonus);
            m_Damage[ATTR_CURRENT] = min(VAMPIRE_MAX_DAMAGE, m_Damage[ATTR_CURRENT] + MinDamageBonus);
            m_Damage[ATTR_MAX] = min(VAMPIRE_MAX_DAMAGE, m_Damage[ATTR_MAX] + MaxDamageBonus);
            m_Defense = max(0, m_Defense - DefensePenalty);
            m_Protection = max(0, m_Protection - ProtectionPenalty);
        }
    }
    if (isFlag(Effect::EFFECT_CLASS_TRANSFORM_TO_BAT)) {
        EffectTransformToBat* pTransformToBat =
            dynamic_cast<EffectTransformToBat*>(findEffect(Effect::EFFECT_CLASS_TRANSFORM_TO_BAT));
        if (pTransformToBat != NULL) {
            int DefensePenalty = getPercentValue(m_Defense, 25);
            int ProtectionPenalty = getPercentValue(m_Protection, 25);

            m_Defense = max(0, m_Defense - DefensePenalty);
            m_Protection = max(0, m_Protection - ProtectionPenalty);
        }
    }
    if (isFlag(Effect::EFFECT_CLASS_BLUNTING)) {
        EffectBlunting* pBlunting = dynamic_cast<EffectBlunting*>(findEffect(Effect::EFFECT_CLASS_BLUNTING));
        if (pBlunting != NULL) {
            int DefensePenalty = pBlunting->getDefensePenalty();
            m_Defense = max(0, m_Defense - DefensePenalty);
        }
    }


    __END_CATCH
}
