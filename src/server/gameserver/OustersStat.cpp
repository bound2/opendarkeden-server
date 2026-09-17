//////////////////////////////////////////////////////////////////////////////
// FileName 	: OustersStat.cpp
// Description	: Ousters stat computation: the castle skills, the all-stat recalculation and the item, option and blood bible contributions to it.
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
// 아우스터스
//
//////////////////////////////////////////////////////////////////////////////

void Ousters::initCastleSkill() {
    __BEGIN_TRY

    removeAllCastleSkill();

    if (!getZone()->isHolyLand())
        return;

    list<CastleInfo*> pCastleInfoList = g_pCastleInfoManager->getGuildCastleInfos(getGuildID());
    if (pCastleInfoList.empty())
        return;

    list<CastleInfo*>::iterator itr = pCastleInfoList.begin();

    for (; itr != pCastleInfoList.end(); itr++) {
        SkillType_t CastleSkillType = g_pCastleInfoManager->getCastleSkillType((*itr)->getZoneID(), getGuildID());
        if (CastleSkillType == SKILL_MAX)
            continue;

        SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(CastleSkillType);
        Assert(pSkillInfo != NULL);

        Turn_t Delay = pSkillInfo->getMaxDelay();

        OustersCastleSkillSlot* pCastleSkillSlot = new OustersCastleSkillSlot();

        pCastleSkillSlot->setName(m_Name);
        pCastleSkillSlot->setSkillType(CastleSkillType);
        pCastleSkillSlot->setExpLevel(1);
        pCastleSkillSlot->setInterval(Delay);
        pCastleSkillSlot->setRunTime();

        addSkill(pCastleSkillSlot);
    }

    __END_CATCH
}

void Ousters::initAllStat(int numPartyMember)

{
    __BEGIN_TRY

    BASIC_ATTR attr;
    Creature::CreatureClass CClass = getCreatureClass();

    m_Resist[MAGIC_DOMAIN_NO_DOMAIN] = 0;
    m_Resist[MAGIC_DOMAIN_POISON] = 0;
    m_Resist[MAGIC_DOMAIN_ACID] = 0;
    m_Resist[MAGIC_DOMAIN_CURSE] = 0;
    m_Resist[MAGIC_DOMAIN_BLOOD] = 0;

    // BloodBible 관련 보너스 수치들 초기화
    m_ConsumeMPRatio = 0;
    m_GamblePriceRatio = 0;
    m_PotionPriceRatio = 0;
    m_MagicBonusDamage = 0;
    m_PhysicBonusDamage = 0;
    m_MagicDamageReduce = 0;
    m_PhysicDamageReduce = 0;

    //////////////////////////////////////////////////////////////////////////////
    // 제일 먼저 기본 능력치를 초기화시키고,
    // 기본 능력치에 영향을 주는 이펙트를 검사한다.
    //////////////////////////////////////////////////////////////////////////////
    m_STR[ATTR_CURRENT] = m_STR[ATTR_MAX] = m_STR[ATTR_BASIC];
    m_DEX[ATTR_CURRENT] = m_DEX[ATTR_MAX] = m_DEX[ATTR_BASIC];
    m_INT[ATTR_CURRENT] = m_INT[ATTR_MAX] = m_INT[ATTR_BASIC];

    //////////////////////////////////////////////////////////////////////////////
    // 능력치 계산을 위한 파라미터들을 초기화한다.
    //////////////////////////////////////////////////////////////////////////////
    attr.nSTR = m_STR[ATTR_CURRENT];
    attr.nDEX = m_DEX[ATTR_CURRENT];
    attr.nINT = m_INT[ATTR_CURRENT];
    attr.pWeapon = getWearItem(WEAR_RIGHTHAND);
    attr.nLevel = m_Level;

    m_HPStealRatio = 0;
    m_MPStealRatio = 0;

    m_HPStealAmount = 0;
    m_MPStealAmount = 0;

    m_HPRegen = 0;
    m_MPRegen = 0;
    m_Luck = m_BaseLuck;
    //	cout << getName() << "의 기본 행운 : " << m_Luck << endl;

    m_FireDamage = 0;
    m_WaterDamage = 0;
    m_EarthDamage = 0;

    m_SilverResist = 0;

    m_ElementalFire = 0;
    m_ElementalWater = 0;
    m_ElementalEarth = 0;
    m_ElementalWind = 0;

    m_PassiveRatio = 0;

    ////////////////////////////////////////////////////////////
    // 부가적인 능력치들을 다시 계산한다.
    ////////////////////////////////////////////////////////////
    m_HP[ATTR_MAX] = computeHP(CClass, &attr);
    m_HP[ATTR_BASIC] = 0;
    m_MP[ATTR_MAX] = computeMP(CClass, &attr);
    m_MP[ATTR_BASIC] = 0;
    m_ToHit[ATTR_CURRENT] = computeToHit(CClass, &attr);
    m_ToHit[ATTR_MAX] = 0;
    m_Defense[ATTR_CURRENT] = computeDefense(CClass, &attr);
    m_Defense[ATTR_MAX] = 0;
    m_Protection[ATTR_CURRENT] = computeProtection(CClass, &attr);
    m_Protection[ATTR_MAX] = 0;
    m_Damage[ATTR_CURRENT] = computeMinDamage(CClass, &attr);
    m_Damage[ATTR_MAX] = computeMaxDamage(CClass, &attr);
    m_Damage[ATTR_BASIC] = 0;
    m_AttackSpeed[ATTR_CURRENT] = computeAttackSpeed(CClass, &attr);
    m_AttackSpeed[ATTR_MAX] = 0;
    m_CriticalRatio[ATTR_CURRENT] = computeCriticalRatio(CClass, &attr);
    m_CriticalRatio[ATTR_MAX] = 0;

    int RaceWarHPBonus = 0;

    if (m_pZone->isHolyLand() || m_pZone->isLevelWarZone()) {
        RaceWarHPBonus = getPercentValue(m_HP[ATTR_MAX], g_pVariableManager->getRaceWarHPBonus());
    }

    int DragonEyeHPBonus = 0;
    if (isFlag(Effect::EFFECT_CLASS_DRAGON_EYE)) {
        // HP 보너스는 두배
        DragonEyeHPBonus = m_HP[ATTR_MAX];
    }

    //////////////////////////////////////////////////////////////////////////////
    // 일단 기어 체크 변수를 초기화해서 모든 기어를 안 입은 것으로 간주하고 시작한다.
    //////////////////////////////////////////////////////////////////////////////
    bool pOldRealWearingCheck[OUSTERS_WEAR_MAX]; // by sigi. 2002.10.31
    for (int i = 0; i < OUSTERS_WEAR_MAX; i++) {
        pOldRealWearingCheck[i] = m_pRealWearingCheck[i];
        m_pRealWearingCheck[i] = false;
    }

    //////////////////////////////////////////////////////////////////////////////
    // Blood Bible 각각의 보너스 옵션을 받는다.
    //////////////////////////////////////////////////////////////////////////////
    /*	if ( m_pZone->isHolyLand() && !g_pWarSystem->hasActiveRaceWar() )
        {
            const BloodBibleBonusHashMap& bloodBibleBonus = g_pBloodBibleBonusManager->getBloodBibleBonuses();
            BloodBibleBonusHashMapConstItor itr;
            for (itr=bloodBibleBonus.begin(); itr!=bloodBibleBonus.end(); itr++)
            {
                if ( itr->second->getRace() == RACE_OUSTERS )
                {
                    OptionTypeList optionTypes = itr->second->getOptionTypeList();
                    OptionTypeListConstItor optionItr;

                    for ( optionItr = optionTypes.begin(); optionItr != optionTypes.end(); optionItr++ )
                    {
                        computeOptionStat( *optionItr );
                    }
                }
            }
        }*/

    if (g_pSweeperBonusManager->isAble(getZoneID()) &&
        g_pLevelWarZoneInfoManager->isCreatureBonusZone(this, getZoneID())) {
        const SweeperBonusHashMap& sweeperBonuses = g_pSweeperBonusManager->getSweeperBonuses();

        SweeperBonusHashMapConstItor itr = sweeperBonuses.begin();
        SweeperBonusHashMapConstItor endItr = sweeperBonuses.end();

        for (; itr != endItr; itr++) {
            if (itr->second->getRace() == RACE_OUSTERS &&
                itr->second->getLevel() == g_pLevelWarZoneInfoManager->getCreatureLevelGrade(this)) {
                OptionTypeList optionTypes = itr->second->getOptionTypeList();
                OptionTypeListConstItor optionItr;

                for (optionItr = optionTypes.begin(); optionItr != optionTypes.end(); optionItr++) {
                    computeOptionStat(*optionItr);
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // 기본적으로 가지고 있는 옵션들을 계산한다.
    //////////////////////////////////////////////////////////////////////////////
    forward_list<DefaultOptionSetType_t>::iterator itr = m_DefaultOptionSet.begin();
    for (; itr != m_DefaultOptionSet.end(); itr++) {
        DefaultOptionSetInfo* pDefaultOptionSetInfo = de::gameContext().optionSets().getDefaultOptionSetInfo((*itr));
        if (pDefaultOptionSetInfo != NULL) {
            const list<OptionType_t>& optionList = pDefaultOptionSetInfo->getOptionTypeList();
            list<OptionType_t>::const_iterator citr;
            for (citr = optionList.begin(); citr != optionList.end(); citr++) {
                computeOptionStat(*citr);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // 펫이 주는 보너스를 계산한다.
    //////////////////////////////////////////////////////////////////////////////
    if (m_pPetInfo != NULL) {
        if (m_pPetInfo->getPetAttr() != 0xff)
            computeOptionClassStat((OptionClass)m_pPetInfo->getPetAttr(), (int)m_pPetInfo->getPetAttrLevel());
        if (m_pPetInfo->getPetOption() != 0)
            computeOptionStat(m_pPetInfo->getPetOption());
    }

    if (isFlag(Effect::EFFECT_CLASS_GROUND_BLESS)) {
        EffectGroundBless* pEffect = dynamic_cast<EffectGroundBless*>(findEffect(Effect::EFFECT_CLASS_GROUND_BLESS));

        if (pEffect != NULL) {
            int bonus = pEffect->getBonus();
            m_STR[ATTR_CURRENT] += bonus;
            m_DEX[ATTR_CURRENT] += bonus;
            m_INT[ATTR_CURRENT] += bonus;
            m_STR[ATTR_MAX] += bonus;
            m_DEX[ATTR_MAX] += bonus;
            m_INT[ATTR_MAX] += bonus;

            computeStatOffset();
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // for 가 두번인 이유는 아이템으로 올라간 능력치에 의해서
    // 입을 수 있게 되는 아이템을 체크하기 위해서이다.
    //////////////////////////////////////////////////////////////////////////////
    for (int j = 0; j < OUSTERS_WEAR_MAX; j++) {
        int wearCount = 0;
        for (int i = 0; i < OUSTERS_WEAR_MAX; i++) {
            Item* pItem = m_pWearItem[i];
            // 현재 포인트에 아이템이 있고
            // 그것에 대한 체크를 아직 하지 않았다면...
            if (pItem != NULL && m_pRealWearingCheck[i] == false) {
                // 만일 진짜루 입을 수 있는 아이템이라면 능력치를 올려준다.
                if (isRealWearing(pItem)) {
                    computeItemStat(pItem);

                    // 양손 무기라면, 체크를 두번 하지 않도록
                    // 왼쪽, 오른쪽 모두 체크 변수를 세팅
                    if (isTwohandWeapon(pItem)) {
                        m_pRealWearingCheck[WEAR_LEFTHAND] = true;
                        m_pRealWearingCheck[WEAR_RIGHTHAND] = true;
                    } else
                        m_pRealWearingCheck[i] = true;

                    wearCount++;
                }
            }
        }

        if (wearCount == 0) // by sigi. 2002.10.30
            break;
    }

    bool zaps[4] = {false, false, false, false};
    for (int i = WEAR_ZAP1; i <= WEAR_ZAP4; ++i) {
        Item* pItem = m_pWearItem[i];
        if (pItem != NULL && m_pRealWearingCheck[i] == true && pItem->getItemClass() == Item::ITEM_CLASS_CORE_ZAP &&
            pItem->getItemType() < 4) {
            zaps[pItem->getItemType()] = true;
        }
    }
    if (zaps[0] && zaps[1] && zaps[2] && zaps[3]) {
        computeOptionStat(182); // 모저 9
        computeOptionStat(185); // 모능 3
    }

    applyBloodBibleSign();

    // by sigi. 2002.11.6
    bool bSendPacket = false;

    if (m_pPlayer != NULL) {
        bSendPacket = (dynamic_cast<GamePlayer*>(m_pPlayer)->getPlayerStatus() == GPS_NORMAL);
    }

    // 일단 위에서 다 입었는데..
    // 능력치에 따라서 복장이 적용이 안되는 아이템은 복장 정보를 없앤다.
    // by sigi. 2002.10.30
    for (int i = 0; i < OUSTERS_WEAR_MAX; i++)
    // int i=WEAR_COAT;
    {
        if (m_pRealWearingCheck[i]) {
            // by sigi. 2002.10.31
            if (pOldRealWearingCheck[i] == false) {
                Item* pItem = m_pWearItem[i];
                if (pItem != NULL) {
                    Item::ItemClass IClass = pItem->getItemClass();
                    ItemType_t IType = pItem->getItemType();

                    Color_t color = getItemShapeColor(pItem);
                    addShape(IClass, IType, color);

                    if (bSendPacket) {
                        GCChangeShape pkt;
                        pkt.setObjectID(getObjectID());
                        pkt.setItemClass(IClass);
                        pkt.setItemType(IType);
                        pkt.setOptionType(pItem->getFirstOptionType());
                        pkt.setAttackSpeed(m_AttackSpeed[ATTR_CURRENT]);

                        m_pZone->broadcastPacket(m_X, m_Y, &pkt, this);
                    }
                }
            }
        } else {
            Item* pItem = m_pWearItem[i];
            if (pItem != NULL) {
                removeShape(pItem->getItemClass(), bSendPacket);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // HP, MP 스틸 확률을 계산해 둔다.
    //////////////////////////////////////////////////////////////////////////////
    if (hasRankBonus(RankBonus::RANK_BONUS_LIFE_ABSORB)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_LIFE_ABSORB);
        Assert(pRankBonus != NULL);

        int StealBonus = pRankBonus->getPoint();

        m_HPStealAmount += StealBonus;
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_SOUL_ABSORB)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_SOUL_ABSORB);
        Assert(pRankBonus != NULL);

        int StealBonus = pRankBonus->getPoint();

        m_MPStealAmount += StealBonus;
    }

    m_HPStealRatio = computeStealRatio(CClass, m_HPStealAmount, &attr);
    m_MPStealRatio = computeStealRatio(CClass, m_MPStealAmount, &attr);
    // cout << getName() << " HPSteal : " << (int)m_HPStealAmount << endl;

    //////////////////////////////////////////////////////////////////////////////
    // 부가적인 능력치를 직접 수정하는 이펙트를 검사한다.
    //////////////////////////////////////////////////////////////////////////////
    if (isFlag(Effect::EFFECT_CLASS_DOOM)) {
        EffectDoom* pDoom = dynamic_cast<EffectDoom*>(findEffect(Effect::EFFECT_CLASS_DOOM));
        if (pDoom != NULL) {
            int DefensePenalty = getPercentValue(m_Defense[ATTR_CURRENT], pDoom->getDefensePenalty());
            int ProtectionPenalty = getPercentValue(m_Protection[ATTR_CURRENT], pDoom->getProtectionPenalty());

            m_Defense[ATTR_CURRENT] = max(0, m_Defense[ATTR_CURRENT] - DefensePenalty);
            m_Protection[ATTR_CURRENT] = max(0, m_Protection[ATTR_CURRENT] - ProtectionPenalty);
        }
    }
    if (isFlag(Effect::EFFECT_CLASS_SEDUCTION)) {
        EffectSeduction* pSeduction = dynamic_cast<EffectSeduction*>(findEffect(Effect::EFFECT_CLASS_SEDUCTION));
        if (pSeduction != NULL) {
            int ToHitPenalty = getPercentValue(m_ToHit[ATTR_CURRENT], pSeduction->getToHitPenalty());
            int DamagePenalty1 = getPercentValue(m_Damage[ATTR_CURRENT], pSeduction->getDamagePenalty());
            int DamagePenalty2 = getPercentValue(m_Damage[ATTR_MAX], pSeduction->getDamagePenalty());

            m_ToHit[ATTR_CURRENT] = max(0, m_ToHit[ATTR_CURRENT] - ToHitPenalty);
            m_Damage[ATTR_CURRENT] = max(0, m_Damage[ATTR_CURRENT] - DamagePenalty1);
            m_Damage[ATTR_MAX] = max(0, m_Damage[ATTR_MAX] - DamagePenalty2);
        }
    }
    if (isFlag(Effect::EFFECT_CLASS_DEATH)) {
        EffectDeath* pDeath = dynamic_cast<EffectDeath*>(findEffect(Effect::EFFECT_CLASS_DEATH));
        if (pDeath != NULL) {
            for (int i = 0; i < MAGIC_DOMAIN_MAX; i++) {
                m_Resist[i] -= pDeath->getResistPenalty();
                //				if ( m_Resist[i] < 0 ) m_Resist[i] = 0;
            }
        }
    }
    if (isFlag(Effect::EFFECT_CLASS_EVADE)) {
        EffectEvade* pEffect = dynamic_cast<EffectEvade*>(findEffect(Effect::EFFECT_CLASS_EVADE));

        if (pEffect != NULL) {
            if (attr.pWeapon != NULL && attr.pWeapon->getItemClass() == Item::ITEM_CLASS_OUSTERS_CHAKRAM)
                m_Defense[ATTR_CURRENT] += pEffect->getBonus();
            else
                pEffect->setDeadline(0);
        }
    }

    if (isFlag(Effect::EFFECT_CLASS_CROSS_GUARD)) {
        EffectCrossGuard* pEffect = dynamic_cast<EffectCrossGuard*>(findEffect(Effect::EFFECT_CLASS_CROSS_GUARD));

        if (pEffect != NULL) {
            if (attr.pWeapon != NULL && attr.pWeapon->getItemClass() == Item::ITEM_CLASS_OUSTERS_CHAKRAM)
                m_Protection[ATTR_CURRENT] += pEffect->getBonus();
            else
                pEffect->setDeadline(0);
        }
    }

    if (isFlag(Effect::EFFECT_CLASS_BLUNTING)) {
        EffectBlunting* pBlunting = dynamic_cast<EffectBlunting*>(findEffect(Effect::EFFECT_CLASS_BLUNTING));
        if (pBlunting != NULL) {
            if (attr.pWeapon != NULL && attr.pWeapon->getItemClass() == Item::ITEM_CLASS_OUSTERS_CHAKRAM) {
                int DefensePenalty = pBlunting->getDefensePenalty();
                m_Defense[ATTR_CURRENT] = max(0, m_Defense[ATTR_CURRENT] - DefensePenalty);
            } else
                pBlunting->setDeadline(0);
        }
    }

    /*	if ( isFlag( Effect::EFFECT_CLASS_HANDS_OF_FIRE ) )
        {
            //cout << getName() << " 핸즈오브파이어 붙었당" << endl;
            EffectHandsOfFire* pEffect =
       dynamic_cast<EffectHandsOfFire*>(findEffect(Effect::EFFECT_CLASS_HANDS_OF_FIRE));

            if ( pEffect != NULL )
            {
                if ( attr.pWeapon == NULL || attr.pWeapon->getItemClass() != Item::ITEM_CLASS_OUSTERS_WRISTLET )
                    pEffect->setDeadline(0);
                else
                {
                    m_FireDamage += pEffect->getBonus();
                    //cout << getName() << " FireDamage : " << m_FireDamage << endl;
                }
            }
        }*/

    if (isFlag(Effect::EFFECT_CLASS_RING_OF_FLARE)) {
        EffectRingOfFlare* pEffect = dynamic_cast<EffectRingOfFlare*>(findEffect(Effect::EFFECT_CLASS_RING_OF_FLARE));

        if (pEffect != NULL) {
            if (attr.pWeapon == NULL || attr.pWeapon->getItemClass() != Item::ITEM_CLASS_OUSTERS_WRISTLET)
                pEffect->setDeadline(0);
        }
    }

    if (isFlag(Effect::EFFECT_CLASS_WATER_BARRIER)) {
        EffectWaterBarrier* pEffect = dynamic_cast<EffectWaterBarrier*>(findEffect(Effect::EFFECT_CLASS_WATER_BARRIER));

        if (pEffect != NULL) {
            if (attr.pWeapon == NULL || attr.pWeapon->getItemClass() != Item::ITEM_CLASS_OUSTERS_WRISTLET)
                pEffect->setDeadline(0);
        }
    }

    /*	if ( isFlag( Effect::EFFECT_CLASS_GNOMES_WHISPER ) )
        {
            EffectGnomesWhisper* pEffect =
       dynamic_cast<EffectGnomesWhisper*>(findEffect(Effect::EFFECT_CLASS_GNOMES_WHISPER));

            if ( pEffect != NULL )
            {
                if ( attr.pWeapon == NULL || attr.pWeapon->getItemClass() != Item::ITEM_CLASS_OUSTERS_WRISTLET )
                    pEffect->setDeadline(0);
            }
        }*/

    if (isFlag(Effect::EFFECT_CLASS_SUMMON_SYLPH)) {
        int ProtectionBonus = decore::summonSylphProtectionBonus(getLevel());
        int ResistBonus = decore::summonSylphResistBonus(getLevel());

        m_Protection[ATTR_CURRENT] += ProtectionBonus;

        m_Resist[MAGIC_DOMAIN_NO_DOMAIN] += ResistBonus;
        m_Resist[MAGIC_DOMAIN_POISON] += ResistBonus;
        m_Resist[MAGIC_DOMAIN_ACID] += ResistBonus;
        m_Resist[MAGIC_DOMAIN_CURSE] += ResistBonus;
        m_Resist[MAGIC_DOMAIN_BLOOD] += ResistBonus;
    }

    if (isFlag(Effect::EFFECT_CLASS_INTIMATE_GRAIL)) {
        EffectIntimateGrail* pIntimateGrail =
            dynamic_cast<EffectIntimateGrail*>(findEffect(Effect::EFFECT_CLASS_INTIMATE_GRAIL));
        if (pIntimateGrail != NULL) {
            int ratio = decore::intimateGrailRatio(pIntimateGrail->getSkillLevel());
            m_Defense[ATTR_CURRENT] -= getPercentValue(m_Defense[ATTR_CURRENT], ratio);
            m_HP[ATTR_CURRENT] -= getPercentValue(m_HP[ATTR_CURRENT], ratio);
            m_HP[ATTR_MAX] -= getPercentValue(m_HP[ATTR_MAX], ratio);
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // 패시브 기술을 계산한다.
    //////////////////////////////////////////////////////////////////////////////
    OustersSkillSlot* pHideSight = getSkill(SKILL_HIDE_SIGHT);
    if (pHideSight != NULL && attr.pWeapon != NULL &&
        attr.pWeapon->getItemClass() == Item::ITEM_CLASS_OUSTERS_CHAKRAM) {
        SkillLevel_t level = pHideSight->getExpLevel();

        m_ToHit[ATTR_CURRENT] += decore::hideSightToHitBonus(level);
    }


    ///////////////////////////////////////////////////////////////////////////////
    // 계급 보너스를 계산한다.
    ///////////////////////////////////////////////////////////////////////////////
    if (hasRankBonus(RankBonus::RANK_BONUS_WOOD_SKIN)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_WOOD_SKIN);
        Assert(pRankBonus != NULL);

        int ProtectionBonus = pRankBonus->getPoint();

        m_Protection[ATTR_CURRENT] = min(OUSTERS_MAX_PROTECTION, m_Protection[ATTR_CURRENT] + ProtectionBonus);
        m_Protection[ATTR_MAX] = min(OUSTERS_MAX_PROTECTION, m_Protection[ATTR_MAX] + ProtectionBonus);
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_WIND_SENSE)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_WIND_SENSE);
        Assert(pRankBonus != NULL);

        int DefenseBonus = pRankBonus->getPoint();

        m_Defense[ATTR_CURRENT] = min(OUSTERS_MAX_DEFENSE, m_Defense[ATTR_CURRENT] + DefenseBonus);
        m_Defense[ATTR_MAX] = min(OUSTERS_MAX_DEFENSE, m_Defense[ATTR_MAX] + DefenseBonus);
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_HOMING_EYE)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_HOMING_EYE);
        Assert(pRankBonus != NULL);

        int ToHitBonus = pRankBonus->getPoint();

        m_ToHit[ATTR_CURRENT] = min(OUSTERS_MAX_TOHIT, m_ToHit[ATTR_CURRENT] + ToHitBonus);
        m_ToHit[ATTR_MAX] = min(OUSTERS_MAX_TOHIT, m_ToHit[ATTR_MAX] + ToHitBonus);
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_LIFE_ENERGY)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_LIFE_ENERGY);
        Assert(pRankBonus != NULL);

        int HPBonus = pRankBonus->getPoint();

        m_HP[ATTR_MAX] = min(OUSTERS_MAX_HP, m_HP[ATTR_MAX] + HPBonus);
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_SOUL_ENERGY)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_SOUL_ENERGY);
        Assert(pRankBonus != NULL);

        int MPBonus = pRankBonus->getPoint();

        m_MP[ATTR_MAX] = min(OUSTERS_MAX_MP, m_MP[ATTR_MAX] + MPBonus);
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_STONE_MAUL)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_STONE_MAUL);
        Assert(pRankBonus != NULL);

        int DamageBonus = pRankBonus->getPoint();

        m_Damage[ATTR_CURRENT] = min(OUSTERS_MAX_DAMAGE, m_Damage[ATTR_CURRENT] + DamageBonus);
        m_Damage[ATTR_MAX] = min(OUSTERS_MAX_DAMAGE, m_Damage[ATTR_MAX] + DamageBonus);
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_SWIFT_ARM)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_SWIFT_ARM);
        Assert(pRankBonus != NULL);

        int AttackSpeedBonus = pRankBonus->getPoint();

        m_AttackSpeed[ATTR_CURRENT] += AttackSpeedBonus;
        m_AttackSpeed[ATTR_MAX] += AttackSpeedBonus;
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_FIRE_ENDOW)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_FIRE_ENDOW);
        Assert(pRankBonus != NULL);

        int FireDamageBonus = pRankBonus->getPoint();

        m_FireDamage += FireDamageBonus;
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_WATER_ENDOW)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_WATER_ENDOW);
        Assert(pRankBonus != NULL);

        int WaterDamageBonus = pRankBonus->getPoint();

        m_WaterDamage += WaterDamageBonus;
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_EARTH_ENDOW)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_EARTH_ENDOW);
        Assert(pRankBonus != NULL);

        int EarthDamageBonus = pRankBonus->getPoint();

        m_EarthDamage += EarthDamageBonus;
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_ANTI_ACID_SKIN)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_ANTI_ACID_SKIN);
        Assert(pRankBonus != NULL);

        int ResistBonus = pRankBonus->getPoint();

        m_Resist[MAGIC_DOMAIN_ACID] += ResistBonus;
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_ANTI_BLOODY_SKIN)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_ANTI_BLOODY_SKIN);
        Assert(pRankBonus != NULL);

        int ResistBonus = pRankBonus->getPoint();

        m_Resist[MAGIC_DOMAIN_BLOOD] += ResistBonus;
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_ANTI_CURSE_SKIN)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_ANTI_CURSE_SKIN);
        Assert(pRankBonus != NULL);

        int ResistBonus = pRankBonus->getPoint();

        m_Resist[MAGIC_DOMAIN_CURSE] += ResistBonus;
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_ANTI_POISON_SKIN)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_ANTI_POISON_SKIN);
        Assert(pRankBonus != NULL);

        int ResistBonus = pRankBonus->getPoint();

        m_Resist[MAGIC_DOMAIN_POISON] += ResistBonus;
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_ANTI_SILVER_DAMAGE_SKIN)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_ANTI_SILVER_DAMAGE_SKIN);
        Assert(pRankBonus != NULL);

        int ResistBonus = pRankBonus->getPoint();

        m_SilverResist += ResistBonus;
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_BLESS_OF_NATURE)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_BLESS_OF_NATURE);
        Assert(pRankBonus != NULL);

        int MPAmount = pRankBonus->getPoint();

        m_ConsumeMPRatio -= MPAmount;
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_MYSTIC_RULE)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_MYSTIC_RULE);
        Assert(pRankBonus != NULL);

        int LuckBonus = pRankBonus->getPoint();

        m_Luck += LuckBonus;
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_POWER_OF_SPIRIT)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_POWER_OF_SPIRIT);
        Assert(pRankBonus != NULL);

        int ProtectionBonus = getPercentValue(m_Protection[ATTR_CURRENT], pRankBonus->getPoint());

        m_Protection[ATTR_CURRENT] = min(OUSTERS_MAX_PROTECTION, m_Protection[ATTR_CURRENT] + ProtectionBonus);
        m_Protection[ATTR_MAX] = min(OUSTERS_MAX_PROTECTION, m_Protection[ATTR_MAX] + ProtectionBonus);
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_WIND_OF_SPIRIT)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_WIND_OF_SPIRIT);
        Assert(pRankBonus != NULL);

        int DefenseBonus = getPercentValue(m_Defense[ATTR_CURRENT], pRankBonus->getPoint());

        m_Defense[ATTR_CURRENT] = min(OUSTERS_MAX_DEFENSE, m_Defense[ATTR_CURRENT] + DefenseBonus);
        m_Defense[ATTR_MAX] = min(OUSTERS_MAX_DEFENSE, m_Defense[ATTR_MAX] + DefenseBonus);
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_PIXIES_EYES)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_PIXIES_EYES);
        Assert(pRankBonus != NULL);

        int ToHitBonus = getPercentValue(m_ToHit[ATTR_CURRENT], pRankBonus->getPoint());

        m_ToHit[ATTR_CURRENT] = min(OUSTERS_MAX_TOHIT, m_ToHit[ATTR_CURRENT] + ToHitBonus);
        m_ToHit[ATTR_MAX] = min(OUSTERS_MAX_TOHIT, m_ToHit[ATTR_MAX] + ToHitBonus);
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_GROUND_OF_SPIRIT)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_GROUND_OF_SPIRIT);
        Assert(pRankBonus != NULL);

        int MPBonus = getPercentValue(m_MP[ATTR_CURRENT], pRankBonus->getPoint());
        // edit by Coffee 2007-5-20 錦攣침쥣轟掘MP BUG
        m_MPStealAmount += MPBonus;
        m_MPStealRatio = computeStealRatio(CClass, m_MPStealAmount, &attr);
        // m_MP[ATTR_CURRENT]  = min(OUSTERS_MAX_MP, m_MP[ATTR_CURRENT] + MPBonus);
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_FIRE_OF_SPIRIT)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_FIRE_OF_SPIRIT);
        Assert(pRankBonus != NULL);

        int CriticalRatioBonus = getPercentValue(m_CriticalRatio[ATTR_CURRENT], pRankBonus->getPoint());

        m_CriticalRatio[ATTR_CURRENT] = m_CriticalRatio[ATTR_CURRENT] + CriticalRatioBonus;
        m_CriticalRatio[ATTR_MAX] = m_CriticalRatio[ATTR_MAX] + CriticalRatioBonus;
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_SALAMANDERS_KNOWLEDGE)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_SALAMANDERS_KNOWLEDGE);
        Assert(pRankBonus != NULL);

        m_ElementalFire += pRankBonus->getPoint();
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_UNDINES_KNOWLEDGE)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_UNDINES_KNOWLEDGE);
        Assert(pRankBonus != NULL);

        m_ElementalWater += pRankBonus->getPoint();
    }

    if (hasRankBonus(RankBonus::RANK_BONUS_GNOMES_KNOWLEDGE)) {
        RankBonus* pRankBonus = getRankBonus(RankBonus::RANK_BONUS_GNOMES_KNOWLEDGE);
        Assert(pRankBonus != NULL);

        m_ElementalEarth += pRankBonus->getPoint();
    }

    // -_- %로 적용되는 스킬은 마지막에 적용시킨다.
    if (isFlag(Effect::EFFECT_CLASS_SHARP_CHAKRAM)) {
        EffectSharpChakram* pEffect = dynamic_cast<EffectSharpChakram*>(findEffect(Effect::EFFECT_CLASS_SHARP_CHAKRAM));

        if (pEffect != NULL) {
            int bonus = pEffect->getBonus();
            m_ToHit[ATTR_CURRENT] += getPercentValue(m_ToHit[ATTR_CURRENT], bonus);
        }
    }
    if (isFlag(Effect::EFFECT_CLASS_REACTIVE_ARMOR)) {
        EffectReactiveArmor* pEffect =
            dynamic_cast<EffectReactiveArmor*>(findEffect(Effect::EFFECT_CLASS_REACTIVE_ARMOR));

        if (pEffect != NULL) {
            bool unaffect = false;
            if (getSkill(SKILL_REACTIVE_ARMOR) != NULL) {
                SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SKILL_REACTIVE_ARMOR);
                if (pSkillInfo != NULL && !satisfySkillRequire(pSkillInfo)) {
                    unaffect = true;
                }
            }

            if (unaffect) {
                pEffect->setDeadline(0);
            } else {
                int bonus = pEffect->getBonus();
                m_Protection[ATTR_CURRENT] += bonus;
                m_Defense[ATTR_CURRENT] += bonus;
            }
        }
    }

    // HP,MP의 현재치를 HP,MP의 최고치를 넘는 경우
    // 현재치를 최고치값으로 set
    /*    if (m_HP[ATTR_CURRENT] > m_HP[ATTR_MAX])
        {
            m_HP[ATTR_CURRENT] = m_HP[ATTR_MAX];
        }
        if (m_MP[ATTR_CURRENT] > m_MP[ATTR_MAX])
        {
            m_MP[ATTR_CURRENT] = m_MP[ATTR_MAX];
        }
    */
    // 패시브 스킬 초기화
    bool bCanUsePassive = false;
    if (hasSkill(SKILL_FIRE_OF_SOUL_STONE) != NULL) {
        SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SKILL_FIRE_OF_SOUL_STONE);
        Assert(pSkillInfo != NULL);

        if (satisfySkillRequire(pSkillInfo)) {
            bCanUsePassive = true;

            m_PassiveSkillMap[SKILL_FIRE_OF_SOUL_STONE].first = true;
            m_PassiveSkillMap[SKILL_FIRE_OF_SOUL_STONE].second = decore::fireOfSoulStonePoint(getSTR(), getDEX());
        }
    }

    if (!bCanUsePassive) {
        m_PassiveSkillMap[SKILL_FIRE_OF_SOUL_STONE].first = false;
        m_PassiveSkillMap[SKILL_FIRE_OF_SOUL_STONE].second = 0;
    }

    bCanUsePassive = false;
    if (hasSkill(SKILL_ICE_OF_SOUL_STONE) != NULL) {
        SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SKILL_ICE_OF_SOUL_STONE);
        Assert(pSkillInfo != NULL);

        if (satisfySkillRequire(pSkillInfo)) {
            bCanUsePassive = true;

            m_PassiveSkillMap[SKILL_ICE_OF_SOUL_STONE].first = true;
            m_PassiveSkillMap[SKILL_ICE_OF_SOUL_STONE].second = decore::iceOfSoulStonePoint(getDEX());
        }
    }

    if (!bCanUsePassive) {
        m_PassiveSkillMap[SKILL_ICE_OF_SOUL_STONE].first = false;
        m_PassiveSkillMap[SKILL_ICE_OF_SOUL_STONE].second = 0;
    }

    bCanUsePassive = false;
    if (hasSkill(SKILL_SAND_OF_SOUL_STONE) != NULL) {
        SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SKILL_SAND_OF_SOUL_STONE);
        Assert(pSkillInfo != NULL);

        if (satisfySkillRequire(pSkillInfo)) {
            bCanUsePassive = true;

            m_PassiveSkillMap[SKILL_SAND_OF_SOUL_STONE].first = true;
            m_PassiveSkillMap[SKILL_SAND_OF_SOUL_STONE].second = decore::sandOfSoulStonePoint(getSTR(), getDEX());
        }
    }

    if (!bCanUsePassive) {
        m_PassiveSkillMap[SKILL_SAND_OF_SOUL_STONE].first = false;
        m_PassiveSkillMap[SKILL_SAND_OF_SOUL_STONE].second = 0;
    }

    bCanUsePassive = false;
    if (hasSkill(SKILL_BLOCK_HEAD) != NULL) {
        SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SKILL_BLOCK_HEAD);
        Assert(pSkillInfo != NULL);

        if (satisfySkillRequire(pSkillInfo)) {
            bCanUsePassive = true;

            m_PassiveSkillMap[SKILL_BLOCK_HEAD].first = true;
            m_PassiveSkillMap[SKILL_BLOCK_HEAD].second = decore::blockHeadPoint(getDEX());
        }
    }

    if (!bCanUsePassive) {
        m_PassiveSkillMap[SKILL_BLOCK_HEAD].first = false;
        m_PassiveSkillMap[SKILL_BLOCK_HEAD].second = 0;
    }

    bCanUsePassive = false;
    if (hasSkill(SKILL_BLESS_FIRE) != NULL) {
        SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SKILL_BLESS_FIRE);
        Assert(pSkillInfo != NULL);

        if (satisfySkillRequire(pSkillInfo)) {
            bCanUsePassive = true;

            m_PassiveSkillMap[SKILL_BLESS_FIRE].first = true;
            m_PassiveSkillMap[SKILL_BLESS_FIRE].second = decore::blessFirePoint(getSTR(), getDEX());
        }
    }

    if (!bCanUsePassive) {
        m_PassiveSkillMap[SKILL_BLESS_FIRE].first = false;
        m_PassiveSkillMap[SKILL_BLESS_FIRE].second = 0;
    }

    bCanUsePassive = false;
    if (hasSkill(SKILL_WATER_SHIELD) != NULL) {
        SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SKILL_WATER_SHIELD);
        Assert(pSkillInfo != NULL);

        if (satisfySkillRequire(pSkillInfo)) {
            bCanUsePassive = true;

            m_PassiveSkillMap[SKILL_WATER_SHIELD].first = true;
            m_PassiveSkillMap[SKILL_WATER_SHIELD].second = 0;
        }
    }

    if (!bCanUsePassive) {
        m_PassiveSkillMap[SKILL_WATER_SHIELD].first = false;
        m_PassiveSkillMap[SKILL_WATER_SHIELD].second = 0;
    }

    bCanUsePassive = false;
    if (hasSkill(SKILL_SAND_CROSS) != NULL) {
        SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SKILL_SAND_CROSS);
        Assert(pSkillInfo != NULL);

        if (satisfySkillRequire(pSkillInfo)) {
            bCanUsePassive = true;

            m_PassiveSkillMap[SKILL_SAND_CROSS].first = true;
            m_PassiveSkillMap[SKILL_SAND_CROSS].second = decore::sandCrossPoint(getSTR(), getDEX());
        }
    }

    if (!bCanUsePassive) {
        m_PassiveSkillMap[SKILL_SAND_CROSS].first = false;
        m_PassiveSkillMap[SKILL_SAND_CROSS].second = 0;
    }

    m_PassiveRatio = getElementalSum();

    for (int i = WEAR_STONE1; i <= WEAR_STONE3; ++i) {
        Item* pItem = getWearItem((WearPart)i);

        if (pItem == NULL || pItem->getItemClass() != Item::ITEM_CLASS_OUSTERS_STONE)
            continue;
        OustersStone* pOustersStone = dynamic_cast<OustersStone*>(pItem);
        Assert(pOustersStone != NULL);

        m_PassiveRatio += pOustersStone->getElemental();
    }

    if (RaceWarHPBonus > 0) {
        m_HP[ATTR_MAX] = min(OUSTERS_MAX_HP, m_HP[ATTR_MAX] + RaceWarHPBonus);
    }

    if (DragonEyeHPBonus > 0) {
        m_HP[ATTR_MAX] = min(OUSTERS_MAX_HP, m_HP[ATTR_MAX] + DragonEyeHPBonus);
    }

    initCastleSkill();

    //	cout << "불 : " << m_ElementalFire << endl;
    //	cout << "물 : " << m_ElementalWater << endl;
    //	cout << "대지 : " << m_ElementalEarth << endl;

    //	cout << getName() << "의 Luck : " << m_Luck << endl;

    /*	cout << getName() << ":" << endl;
        for ( int i=0; i<MAGIC_DOMAIN_MAX; ++i )
        {
            cout << "저항 " << i << " : " << m_Resist[i] << endl;
        }

        cout << "물리공격력 " << m_PhysicBonusDamage << endl;
        cout << "물리방어력 " << m_PhysicDamageReduce << endl;
        cout << "마법공격력 " << m_MagicBonusDamage << endl;
        cout << "마법방어력 " << m_MagicDamageReduce << endl;*/

    __END_CATCH
}

int Ousters::getBloodBibleSignOpenNum() const {
    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(m_pPlayer);

    int openNumLimit = 6;
    if (!pGamePlayer->isPayPlaying() && !pGamePlayer->isPremiumPlay()) {
        openNumLimit = 2;
    }

    Fame_t fame = getFame();

    if (!g_pWarSystem->canApplyBloodBibleSign())
        return 0;

    return decore::oustersBloodBibleSignOpenNum(fame, openNumLimit);
}

//////////////////////////////////////////////////////////////////////////////
// STR, DEX, INT의 경우
// CURRENT = 기본 수치 + 아이템 수치 + 마법 수치
// MAX     = 기본 수치 + 아이템 수치
// BASIC   = 기본 수치
//
// HP, MP의 경우
// CURRENT = 현재 수치
// MAX     = 현재 맥스
// BASIC   = 아이템에 의한 변화 수치
//
// Defense, Protection, ToHit의 경우
// CURRENT = 현재 수치
// MAX     = 아이템에 의한 변화 수치
//
// Damage의 경우
// CURRENT = Min 데미지
// MAX     = Max 데미지
// BASIC   = 아이템에 의한 변화 수치
//////////////////////////////////////////////////////////////////////////////
void Ousters::computeStatOffset()

{
    __BEGIN_TRY

    Creature::CreatureClass CClass = getCreatureClass();
    BASIC_ATTR cur_attr;

    cur_attr.nSTR = m_STR[ATTR_CURRENT];
    cur_attr.nDEX = m_DEX[ATTR_CURRENT];
    cur_attr.nINT = m_INT[ATTR_CURRENT];
    cur_attr.nLevel = m_Level;

    // 세로워진 STR, DEX, INT로 새로 계산을 한 다음
    // 아이템 또는 마법 수치를 더한다.
    m_HP[ATTR_MAX] = computeHP(CClass, &cur_attr);
    m_HP[ATTR_MAX] += m_HP[ATTR_BASIC];

    m_MP[ATTR_MAX] = computeMP(CClass, &cur_attr);
    m_MP[ATTR_MAX] += m_MP[ATTR_BASIC];

    m_ToHit[ATTR_CURRENT] = computeToHit(CClass, &cur_attr);
    m_ToHit[ATTR_CURRENT] += m_ToHit[ATTR_MAX];

    m_Defense[ATTR_CURRENT] = computeDefense(CClass, &cur_attr);
    m_Defense[ATTR_CURRENT] += m_Defense[ATTR_MAX];

    m_Protection[ATTR_CURRENT] = computeProtection(CClass, &cur_attr);
    m_Protection[ATTR_CURRENT] += m_Protection[ATTR_MAX];

    m_Damage[ATTR_CURRENT] = computeMinDamage(CClass, &cur_attr);
    m_Damage[ATTR_MAX] = computeMaxDamage(CClass, &cur_attr);

    m_Damage[ATTR_CURRENT] += m_Damage[ATTR_BASIC];
    m_Damage[ATTR_MAX] += m_Damage[ATTR_BASIC];

    m_AttackSpeed[ATTR_CURRENT] = computeAttackSpeed(CClass, &cur_attr);
    m_AttackSpeed[ATTR_CURRENT] += m_AttackSpeed[ATTR_MAX];

    m_CriticalRatio[ATTR_CURRENT] = computeCriticalRatio(CClass, &cur_attr);
    m_CriticalRatio[ATTR_CURRENT] += m_CriticalRatio[ATTR_MAX];

    __END_CATCH
}

void Ousters::computeItemStat(Item* pItem)

{
    __BEGIN_TRY

    //	if (isOustersWeapon(pItem->getItemClass()))
    if (pItem->getItemClass() == Item::ITEM_CLASS_OUSTERS_CHAKRAM) {
        // 무기라면 무기가 가지는 속도 파라미터를 더한다.
        ItemInfo* pItemInfo = g_pItemInfoManager->getItemInfo(pItem->getItemClass(), pItem->getItemType());
        m_AttackSpeed[ATTR_CURRENT] += pItemInfo->getSpeed();
        m_AttackSpeed[ATTR_MAX] += pItemInfo->getSpeed();
    }

    if (pItem->getItemClass() == Item::ITEM_CLASS_CORE_ZAP) {
        CoreZapInfo* pItemInfo =
            dynamic_cast<CoreZapInfo*>(g_pItemInfoManager->getItemInfo(pItem->getItemClass(), pItem->getItemType()));
        if (pItemInfo != NULL) {
            computeOptionClassStat(pItemInfo->getOptionClass(), pItem->getGrade());
        }
    }

    Elemental_t point = 0;
    ElementalType type = ELEMENTAL_MAX;

    if (pItem->getItemClass() == Item::ITEM_CLASS_OUSTERS_WRISTLET) {
        OustersWristlet* pWristlet = dynamic_cast<OustersWristlet*>(pItem);
        Assert(pWristlet != NULL);

        point = pWristlet->getElemental();
        type = pWristlet->getElementalType();
    } else if (pItem->getItemClass() == Item::ITEM_CLASS_OUSTERS_STONE) {
        OustersStone* pStone = dynamic_cast<OustersStone*>(pItem);
        Assert(pStone != NULL);

        point = pStone->getElemental();
        type = pStone->getElementalType();
    }

    if (point != 0 && type != ELEMENTAL_MAX) {
        switch (type) {
        case ELEMENTAL_FIRE:
            m_ElementalFire += point;
            break;
        case ELEMENTAL_WATER:
            m_ElementalWater += point;
            break;
        case ELEMENTAL_EARTH:
            m_ElementalEarth += point;
            break;
        case ELEMENTAL_WIND:
            m_ElementalWind += point;
            break;
        default:
            break;
        }
    }

    m_Protection[ATTR_CURRENT] += pItem->getProtectionBonus();
    m_Protection[ATTR_MAX] += pItem->getProtectionBonus();

    m_Defense[ATTR_CURRENT] += pItem->getDefenseBonus();
    m_Defense[ATTR_MAX] += pItem->getDefenseBonus();

    m_ToHit[ATTR_CURRENT] += pItem->getToHitBonus();
    m_ToHit[ATTR_MAX] += pItem->getToHitBonus();

    m_Luck += pItem->getLuck();

    //	if (pItem->getOptionType()) computeOptionStat(pItem);
    // 부가적인 옵션들
    const list<OptionType_t>& optionType = pItem->getOptionTypeList();
    list<OptionType_t>::const_iterator itr;
    for (itr = optionType.begin(); itr != optionType.end(); itr++) {
        computeOptionStat(*itr);
    }

    // Item 자체의 defaultOption을 적용시킨다.
    const list<OptionType_t>& defaultOptions = pItem->getDefaultOptions();
    list<OptionType_t>::const_iterator iOptions;

    for (iOptions = defaultOptions.begin(); iOptions != defaultOptions.end(); iOptions++) {
        computeOptionStat(*iOptions);
    }

    __END_CATCH
}

void Ousters::computeOptionStat(Item* pItem)

{
    __BEGIN_TRY

    // 부가적인 옵션들
    const list<OptionType_t>& optionType = pItem->getOptionTypeList();
    list<OptionType_t>::const_iterator itr;
    for (itr = optionType.begin(); itr != optionType.end(); itr++) {
        computeOptionStat(*itr);
    }

    // Item 자체의 defaultOption을 적용시킨다.
    const list<OptionType_t>& defaultOptions = pItem->getDefaultOptions();
    list<OptionType_t>::const_iterator iOptions;

    for (iOptions = defaultOptions.begin(); iOptions != defaultOptions.end(); iOptions++) {
        computeOptionStat(*iOptions);
    }

    __END_CATCH
}

void Ousters::computeOptionClassStat(OptionClass OClass, int PlusPoint) {
    switch (OClass) {
    case OPTION_STR:
        m_STR[ATTR_CURRENT] += PlusPoint;
        m_STR[ATTR_MAX] += PlusPoint;
        computeStatOffset();
        break;
    case OPTION_DEX:
        m_DEX[ATTR_CURRENT] += PlusPoint;
        m_DEX[ATTR_MAX] += PlusPoint;
        computeStatOffset();
        break;
    case OPTION_INT:
        m_INT[ATTR_CURRENT] += PlusPoint;
        m_INT[ATTR_MAX] += PlusPoint;
        computeStatOffset();
        break;
    case OPTION_HP:
        m_HP[ATTR_MAX] += PlusPoint;
        m_HP[ATTR_BASIC] += PlusPoint;
        break;
    case OPTION_MP:
        m_MP[ATTR_MAX] += PlusPoint;
        m_MP[ATTR_BASIC] += PlusPoint;
        break;
    case OPTION_HP_STEAL:
        m_HPStealAmount += PlusPoint;
        break;
    case OPTION_MP_STEAL:
        m_MPStealAmount += PlusPoint;
        break;
    case OPTION_HP_REGEN:
        m_HPRegen += PlusPoint;
        break;
    case OPTION_MP_REGEN:
        m_MPRegen += PlusPoint;
        break;
    case OPTION_TOHIT:
        m_ToHit[ATTR_CURRENT] += PlusPoint;
        m_ToHit[ATTR_MAX] += PlusPoint;
        break;
    case OPTION_DEFENSE:
        m_Defense[ATTR_CURRENT] += PlusPoint;
        m_Defense[ATTR_MAX] += PlusPoint;
        break;
    case OPTION_DAMAGE:
        m_Damage[ATTR_CURRENT] += PlusPoint;
        m_Damage[ATTR_MAX] += PlusPoint;
        m_Damage[ATTR_BASIC] += PlusPoint;
        break;
    case OPTION_PROTECTION:
        m_Protection[ATTR_CURRENT] += PlusPoint;
        m_Protection[ATTR_MAX] += PlusPoint;
        break;
    case OPTION_POISON:
        m_Resist[MAGIC_DOMAIN_POISON] += PlusPoint;
        break;
    case OPTION_ACID:
        m_Resist[MAGIC_DOMAIN_ACID] += PlusPoint;
        break;
    case OPTION_CURSE:
        m_Resist[MAGIC_DOMAIN_CURSE] += PlusPoint;
        break;
    case OPTION_BLOOD:
        m_Resist[MAGIC_DOMAIN_BLOOD] += PlusPoint;
        break;
    case OPTION_VISION:
        break;
    case OPTION_ATTACK_SPEED:
        m_AttackSpeed[ATTR_CURRENT] += PlusPoint;
        m_AttackSpeed[ATTR_MAX] += PlusPoint;
        break;
    case OPTION_CRITICAL_HIT:
        m_CriticalRatio[ATTR_CURRENT] += PlusPoint;
        m_CriticalRatio[ATTR_MAX] += PlusPoint;
        break;

    case OPTION_ALL_ATTR:
        m_STR[ATTR_CURRENT] += PlusPoint;
        m_STR[ATTR_MAX] += PlusPoint;

        m_DEX[ATTR_CURRENT] += PlusPoint;
        m_DEX[ATTR_MAX] += PlusPoint;

        m_INT[ATTR_CURRENT] += PlusPoint;
        m_INT[ATTR_MAX] += PlusPoint;

        computeStatOffset();
        break;

    case OPTION_ALL_RES:
        m_Resist[MAGIC_DOMAIN_POISON] += PlusPoint;
        m_Resist[MAGIC_DOMAIN_ACID] += PlusPoint;
        m_Resist[MAGIC_DOMAIN_CURSE] += PlusPoint;
        m_Resist[MAGIC_DOMAIN_BLOOD] += PlusPoint;
        break;

    case OPTION_LUCK:
        m_Luck += PlusPoint;
        break;

    case OPTION_CONSUME_MP:
        m_ConsumeMPRatio += PlusPoint;
        break;

    case OPTION_MAGIC_DAMAGE:
        m_MagicBonusDamage += PlusPoint;
        break;

    case OPTION_PHYSIC_DAMAGE:
        m_PhysicBonusDamage += PlusPoint;
        break;

    case OPTION_GAMBLE_PRICE:
        m_GamblePriceRatio += PlusPoint;
        break;

    case OPTION_POTION_PRICE:
        m_PotionPriceRatio += PlusPoint;
        break;
    case OPTION_PHYSIC_PRO:
        m_PhysicDamageReduce += PlusPoint;
        break;
    case OPTION_MAGIC_PRO:
        m_MagicDamageReduce += PlusPoint;
        break;


    default:
        break;
    }
}

void Ousters::computeOptionStat(OptionType_t OptionType)

{
    __BEGIN_TRY

    OptionInfo* pOptionInfo = g_pOptionInfoManager->getOptionInfo(OptionType);
    computeOptionClassStat(pOptionInfo->getClass(), pOptionInfo->getPlusPoint());
    /*	OptionClass   OClass        = pOptionInfo->getClass();

        switch (OClass)
        {
            case OPTION_STR:
                m_STR[ATTR_CURRENT] += pOptionInfo->getPlusPoint();
                m_STR[ATTR_MAX]     += pOptionInfo->getPlusPoint();
                computeStatOffset();
                break;
            case OPTION_DEX:
                m_DEX[ATTR_CURRENT] += pOptionInfo->getPlusPoint();
                m_DEX[ATTR_MAX]     += pOptionInfo->getPlusPoint();
                computeStatOffset();
                break;
            case OPTION_INT:
                m_INT[ATTR_CURRENT] += pOptionInfo->getPlusPoint();
                m_INT[ATTR_MAX]     += pOptionInfo->getPlusPoint();
                computeStatOffset();
                break;
            case OPTION_HP:
                m_HP[ATTR_MAX]   += pOptionInfo->getPlusPoint();
                m_HP[ATTR_BASIC] += pOptionInfo->getPlusPoint();
                break;
            case OPTION_MP:
                m_MP[ATTR_MAX]   += pOptionInfo->getPlusPoint();
                m_MP[ATTR_BASIC] += pOptionInfo->getPlusPoint();
                break;
            case OPTION_HP_STEAL:
                m_HPStealAmount += pOptionInfo->getPlusPoint();
                break;
            case OPTION_MP_STEAL:
                m_MPStealAmount += pOptionInfo->getPlusPoint();
                break;
            case OPTION_HP_REGEN:
                m_HPRegen += pOptionInfo->getPlusPoint();
                break;
            case OPTION_MP_REGEN:
                m_MPRegen += pOptionInfo->getPlusPoint();
                break;
            case OPTION_TOHIT:
                m_ToHit[ATTR_CURRENT] += pOptionInfo->getPlusPoint();
                m_ToHit[ATTR_MAX]     += pOptionInfo->getPlusPoint();
                break;
            case OPTION_DEFENSE:
                m_Defense[ATTR_CURRENT] += pOptionInfo->getPlusPoint();
                m_Defense[ATTR_MAX]     += pOptionInfo->getPlusPoint();
                break;
            case OPTION_DAMAGE:
                m_Damage[ATTR_CURRENT] += pOptionInfo->getPlusPoint();
                m_Damage[ATTR_MAX]     += pOptionInfo->getPlusPoint();
                m_Damage[ATTR_BASIC]   += pOptionInfo->getPlusPoint();
                break;
            case OPTION_PROTECTION:
                m_Protection[ATTR_CURRENT] += pOptionInfo->getPlusPoint();
                m_Protection[ATTR_MAX]     += pOptionInfo->getPlusPoint();
                break;
            case OPTION_POISON:
                m_Resist[MAGIC_DOMAIN_POISON] += pOptionInfo->getPlusPoint();
                break;
            case OPTION_ACID:
                m_Resist[MAGIC_DOMAIN_ACID] += pOptionInfo->getPlusPoint();
                break;
            case OPTION_CURSE:
                m_Resist[MAGIC_DOMAIN_CURSE] += pOptionInfo->getPlusPoint();
                break;
            case OPTION_BLOOD:
                m_Resist[MAGIC_DOMAIN_BLOOD] += pOptionInfo->getPlusPoint();
                break;
            case OPTION_VISION:
                break;
            case OPTION_ATTACK_SPEED:
                m_AttackSpeed[ATTR_CURRENT] += pOptionInfo->getPlusPoint();
                m_AttackSpeed[ATTR_MAX]     += pOptionInfo->getPlusPoint();
                break;
            case OPTION_CRITICAL_HIT:
                m_CriticalRatio[ATTR_CURRENT] += pOptionInfo->getPlusPoint();
                m_CriticalRatio[ATTR_MAX]     += pOptionInfo->getPlusPoint();
                break;

            case OPTION_ALL_ATTR:
                m_STR[ATTR_CURRENT] += pOptionInfo->getPlusPoint();
                m_STR[ATTR_MAX]     += pOptionInfo->getPlusPoint();

                m_DEX[ATTR_CURRENT] += pOptionInfo->getPlusPoint();
                m_DEX[ATTR_MAX]     += pOptionInfo->getPlusPoint();

                m_INT[ATTR_CURRENT] += pOptionInfo->getPlusPoint();
                m_INT[ATTR_MAX]     += pOptionInfo->getPlusPoint();

                computeStatOffset();
                break;

            case OPTION_ALL_RES:
                m_Resist[MAGIC_DOMAIN_POISON] += pOptionInfo->getPlusPoint();
                m_Resist[MAGIC_DOMAIN_ACID] += pOptionInfo->getPlusPoint();
                m_Resist[MAGIC_DOMAIN_CURSE] += pOptionInfo->getPlusPoint();
                m_Resist[MAGIC_DOMAIN_BLOOD] += pOptionInfo->getPlusPoint();
                break;

            case OPTION_LUCK:
                m_Luck += pOptionInfo->getPlusPoint();
                break;

            case OPTION_CONSUME_MP:
                m_ConsumeMPRatio = pOptionInfo->getPlusPoint();
                break;

            case OPTION_MAGIC_DAMAGE:
                m_MagicBonusDamage = pOptionInfo->getPlusPoint();
                break;

            case OPTION_PHYSIC_DAMAGE:
                m_PhysicBonusDamage = pOptionInfo->getPlusPoint();
                break;

            case OPTION_GAMBLE_PRICE:
                m_GamblePriceRatio = pOptionInfo->getPlusPoint();
                break;

            case OPTION_POTION_PRICE:
                m_PotionPriceRatio = pOptionInfo->getPlusPoint();
                break;

            default:
                break;
        }*/

    __END_CATCH
}

void Ousters::addModifyInfo(const OUSTERS_RECORD& prev, ModifyInfo& pkt) const

{
    __BEGIN_TRY

    if (prev.pSTR[ATTR_CURRENT] != m_STR[ATTR_CURRENT])
        pkt.addShortData(MODIFY_CURRENT_STR, m_STR[ATTR_CURRENT]);
    if (prev.pSTR[ATTR_MAX] != m_STR[ATTR_MAX])
        pkt.addShortData(MODIFY_MAX_STR, m_STR[ATTR_MAX]);
    if (prev.pSTR[ATTR_BASIC] != m_STR[ATTR_BASIC])
        pkt.addShortData(MODIFY_BASIC_STR, m_STR[ATTR_BASIC]);

    if (prev.pDEX[ATTR_CURRENT] != m_DEX[ATTR_CURRENT])
        pkt.addShortData(MODIFY_CURRENT_DEX, m_DEX[ATTR_CURRENT]);
    if (prev.pDEX[ATTR_MAX] != m_DEX[ATTR_MAX])
        pkt.addShortData(MODIFY_MAX_DEX, m_DEX[ATTR_MAX]);
    if (prev.pDEX[ATTR_BASIC] != m_DEX[ATTR_BASIC])
        pkt.addShortData(MODIFY_BASIC_DEX, m_DEX[ATTR_BASIC]);

    if (prev.pINT[ATTR_CURRENT] != m_INT[ATTR_CURRENT])
        pkt.addShortData(MODIFY_CURRENT_INT, m_INT[ATTR_CURRENT]);
    if (prev.pINT[ATTR_MAX] != m_INT[ATTR_MAX])
        pkt.addShortData(MODIFY_MAX_INT, m_INT[ATTR_MAX]);
    if (prev.pINT[ATTR_BASIC] != m_INT[ATTR_BASIC])
        pkt.addShortData(MODIFY_BASIC_INT, m_INT[ATTR_BASIC]);

    if (prev.pHP[ATTR_CURRENT] != m_HP[ATTR_CURRENT])
        pkt.addShortData(MODIFY_CURRENT_HP, m_HP[ATTR_CURRENT]);
    if (prev.pHP[ATTR_MAX] != m_HP[ATTR_MAX])
        pkt.addShortData(MODIFY_MAX_HP, m_HP[ATTR_MAX]);

    if (prev.pMP[ATTR_CURRENT] != m_MP[ATTR_CURRENT])
        pkt.addShortData(MODIFY_CURRENT_MP, m_MP[ATTR_CURRENT]);
    if (prev.pMP[ATTR_MAX] != m_MP[ATTR_MAX])
        pkt.addShortData(MODIFY_MAX_MP, m_MP[ATTR_MAX]);

    if (prev.pDamage[ATTR_CURRENT] != m_Damage[ATTR_CURRENT])
        pkt.addShortData(MODIFY_MIN_DAMAGE, m_Damage[ATTR_CURRENT]);
    if (prev.pDamage[ATTR_MAX] != m_Damage[ATTR_MAX])
        pkt.addShortData(MODIFY_MAX_DAMAGE, m_Damage[ATTR_MAX]);

    if (prev.Defense != m_Defense[ATTR_CURRENT])
        pkt.addShortData(MODIFY_DEFENSE, m_Defense[ATTR_CURRENT]);
    if (prev.Protection != m_Protection[ATTR_CURRENT])
        pkt.addShortData(MODIFY_PROTECTION, m_Protection[ATTR_CURRENT]);
    if (prev.ToHit != m_ToHit[ATTR_CURRENT])
        pkt.addShortData(MODIFY_TOHIT, m_ToHit[ATTR_CURRENT]);
    if (prev.AttackSpeed != m_AttackSpeed[ATTR_CURRENT])
        pkt.addShortData(MODIFY_ATTACK_SPEED, m_AttackSpeed[ATTR_CURRENT]);

    // by sigi. 2002.9.10
    if (prev.Rank != getRank()) {
        pkt.addShortData(MODIFY_RANK, getRank());
        pkt.addLongData(MODIFY_RANK_EXP, getRankGoalExp());
    }

    __END_CATCH
}

void Ousters::sendModifyInfo(const OUSTERS_RECORD& prev) const

{
    __BEGIN_TRY

    GCModifyInformation gcModifyInformation;
    addModifyInfo(prev, gcModifyInformation);
    m_pPlayer->sendPacket(&gcModifyInformation);

    BloodBibleSignInfo* pInfo = getBloodBibleSign();
    GCBloodBibleSignInfo gcInfo;
    gcInfo.setSignInfo(pInfo);
    //	cout << "open num : " << pInfo->getOpenNum() << endl;;
    m_pPlayer->sendPacket(&gcInfo);

    __END_CATCH
}

void Ousters::initAllStatAndSend() {
    OUSTERS_RECORD prev;
    getOustersRecord(prev);
    initAllStat();
    sendModifyInfo(prev);
}
