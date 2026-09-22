//////////////////////////////////////////////////////////////////////////////
// FileName 	: SkillDamage.cpp
// Description	: Damage: the per-race and magic formulas, the to-hit check, and applying the result to a creature.
//////////////////////////////////////////////////////////////////////////////

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
#include "GameContext.h"
#include "GamePlayer.h"
#include "GameServerInfoManager.h"
#include "HitRoll.h"
#include "ItemFactoryManager.h"
#include "ItemUtil.h"
#include "LogClient.h"
#include "MasterLairInfoManager.h"
#include "Monster.h"
#include "OustersEXPInfo.h"
#include "PKZoneInfoManager.h"
#include "PacketUtil.h"
#include "Party.h"
#include "Player.h"
#include "PrecedenceTable.h"
#include "Properties.h"
#include "SkillDomainInfoManager.h"
#include "SkillInfo.h"
#include "SkillPropertyManager.h"
#include "SkillUtil.h"
#include "SkillUtilInternal.h"
#include "SummonGroundElemental.h"
#include "Thread.h"
#include "VampEXPInfo.h"
#include "VariableManager.h"
#include "Zone.h"
#include "ZoneUtil.h"
#include "domain/Formulas.h"
#include "mission/EventQuestLootingManager.h"
#include "mission/MonsterKillQuestStatus.h"
#include "mission/QuestManager.h"

//////////////////////////////////////////////////////////////////////////////
// Computes the final damage from the parameters of the attacker and the target.
//////////////////////////////////////////////////////////////////////////////
Damage_t computeDamage(Creature* pCreature, Creature* pTargetCreature) {
    Assert(pCreature != NULL);
    Assert(pTargetCreature != NULL);

    Damage_t Damage = 0;
    bool bCriticalHit = false;

    try {
        if (pCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
            Assert(pSlayer != NULL);
            Damage = computeSlayerDamage(pSlayer, pTargetCreature, bCriticalHit);
        } else if (pCreature->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);
            Assert(pVampire != NULL);
            Damage = computeVampireDamage(pVampire, pTargetCreature, bCriticalHit);
        } else if (pCreature->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);
            Assert(pOusters != NULL);
            Damage = computeOustersDamage(pOusters, pTargetCreature, bCriticalHit);
        } else if (pCreature->isMonster()) {
            Monster* pMonster = dynamic_cast<Monster*>(pCreature);
            Assert(pMonster != NULL);
            Damage = computeMonsterDamage(pMonster, pTargetCreature, bCriticalHit);
        } else {
            // Anything else is an NPC, so no damage.
            return 0;
        }
    } catch (Throwable& t) {
        cerr << t.toString() << endl;
    }

    return Damage;
}

//////////////////////////////////////////////////////////////////////////////
// Computes the attacker's pure damage.
//////////////////////////////////////////////////////////////////////////////
Damage_t computePureDamage(Creature* pCreature) {
    Damage_t Damage = 0;

    if (pCreature == NULL)
        return Damage;

    if (pCreature->isSlayer()) {
        Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
        Assert(pSlayer != NULL);
        Damage = computePureSlayerDamage(pSlayer);
    } else if (pCreature->isVampire()) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);
        Assert(pVampire != NULL);
        Damage = computePureVampireDamage(pVampire);
    } else if (pCreature->isOusters()) {
        Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);
        Assert(pOusters != NULL);
        Damage = computePureOustersDamage(pOusters);
    } else if (pCreature->isMonster()) {
        Monster* pMonster = dynamic_cast<Monster*>(pCreature);
        Assert(pMonster != NULL);
        Damage = computePureMonsterDamage(pMonster);
    } else {
        // Anything else is an NPC, so no damage.
        return 0;
    }

    return Damage;
}

//////////////////////////////////////////////////////////////////////////////
// Computes the final damage from the parameters of the attacker and the target.
// Same as the function above, except that this one also handles the
// critical hit part internally.
//////////////////////////////////////////////////////////////////////////////
Damage_t computeDamage(Creature* pCreature, Creature* pTargetCreature, int CriticalBonus, bool& bCritical) {
    Assert(pCreature != NULL);
    Assert(pTargetCreature != NULL);

    Damage_t Damage = 0;
    bool bCriticalHit = HitRoll::isCriticalHit(pCreature, CriticalBonus);

    try {
        if (pCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
            Assert(pSlayer != NULL);

            Damage = computeSlayerDamage(pSlayer, pTargetCreature, bCriticalHit);
        } else if (pCreature->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);
            Assert(pVampire != NULL);
            Damage = computeVampireDamage(pVampire, pTargetCreature, bCriticalHit);
        } else if (pCreature->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);
            Assert(pOusters != NULL);
            Damage = computeOustersDamage(pOusters, pTargetCreature, bCriticalHit);
        } else if (pCreature->isMonster()) {
            Monster* pMonster = dynamic_cast<Monster*>(pCreature);
            Assert(pMonster != NULL);
            Damage = computeMonsterDamage(pMonster, pTargetCreature, bCriticalHit);
        } else {
            // Anything else is an NPC, so no damage.
            return 0;
        }
    } catch (Throwable& t) {
        cerr << t.toString() << endl;
    }

    bCritical = bCriticalHit;

    // A critical hit against a monster deals 150% damage.
    if (bCritical && pTargetCreature->isMonster()) {
        Damage = getPercentValue(Damage, 150);
    }

    return Damage;
}

//////////////////////////////////////////////////////////////////////////////
// Returns the final damage, the original damage less protection.
//////////////////////////////////////////////////////////////////////////////
double computeFinalDamage(Damage_t minDamage, Damage_t maxDamage, Damage_t realDamage, Protection_t Protection,
                          bool bCritical) {
    // minDamage/maxDamage are unused by the current formula; the parameters
    // stay for the many call sites. The math lives in de-core.
    return decore::finalDamage(realDamage, Protection, bCritical);
}

//////////////////////////////////////////////////////////////////////////////
// Computes the damage between a Slayer attacker and the target.
//////////////////////////////////////////////////////////////////////////////
Damage_t computeSlayerDamage(Slayer* pSlayer, Creature* pTargetCreature, bool bCritical) {
    Assert(pSlayer != NULL);
    Assert(pTargetCreature != NULL);

    Item* pItem = pSlayer->getWearItem(Slayer::WEAR_RIGHTHAND);
    uint timeband = getZoneTimeband(pSlayer->getZone());
    double FinalDamage = 0;

    // Start from the bare-handed damage.
    Damage_t MinDamage = pSlayer->getDamage(ATTR_CURRENT);
    Damage_t MaxDamage = pSlayer->getDamage(ATTR_MAX);

    // If a weapon is held, add the weapon's min and max to min and max.
    if (pItem != NULL && pSlayer->isRealWearingEx(Slayer::WEAR_RIGHTHAND)) {
        // Slayer::initAllStat() computes the striking damage and sets
        // m_Damage[] there, so it does not have to be added again
        // here.
        MinDamage += pItem->getMinDamage();
        MaxDamage += pItem->getMaxDamage();
    }

    // Computes the actual random damage.
    Damage_t RealDamage = max(1, Random(MinDamage, MaxDamage));

    if (pTargetCreature->isSlayer()) {
        Slayer* pTargetSlayer = dynamic_cast<Slayer*>(pTargetCreature);
        Assert(pTargetSlayer != NULL);

        Protection_t Protection = pTargetSlayer->getProtection();

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else if (pTargetCreature->isVampire()) {
        Vampire* pTargetVampire = dynamic_cast<Vampire*>(pTargetCreature);
        Assert(pTargetVampire != NULL);

        Protection_t Protection = pTargetVampire->getProtection();
        Protection = (Protection_t)getPercentValue(Protection, VampireTimebandFactor[timeband]);

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else if (pTargetCreature->isOusters()) {
        Ousters* pTargetOusters = dynamic_cast<Ousters*>(pTargetCreature);
        Assert(pTargetOusters != NULL);

        Protection_t Protection = pTargetOusters->getProtection();

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else if (pTargetCreature->isMonster()) {
        Monster* pTargetMonster = dynamic_cast<Monster*>(pTargetCreature);
        Assert(pTargetMonster != NULL);

        Protection_t Protection = pTargetMonster->getProtection();
        Protection = (Protection_t)getPercentValue(Protection, MonsterTimebandFactor[timeband]);

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else {
        // Anything else is an NPC, so no damage.
        return 0;
    }

    // The balance adjustment is done in AbilityBalance.cpp.

    return (Damage_t)FinalDamage;
}

//////////////////////////////////////////////////////////////////////////////
// Computes the damage between a Vampire attacker and the target.
//////////////////////////////////////////////////////////////////////////////
Damage_t computeVampireDamage(Vampire* pVampire, Creature* pTargetCreature, bool bCritical) {
    Assert(pVampire != NULL);
    Assert(pTargetCreature != NULL);

    double FinalDamage = 0;
    Damage_t MinDamage = pVampire->getDamage(ATTR_CURRENT);
    Damage_t MaxDamage = pVampire->getDamage(ATTR_MAX);
    uint timeband = getZoneTimeband(pVampire->getZone());

    // Damage from the Vampire's weapon
    Item* pItem = pVampire->getWearItem(Vampire::WEAR_RIGHTHAND);

    // If a weapon is held, add the weapon's min and max to min and max.
    if (pItem != NULL && pVampire->isRealWearingEx(Vampire::WEAR_RIGHTHAND)) {
        MinDamage += pItem->getMinDamage();
        MaxDamage += pItem->getMaxDamage();
    }

    // Computes the actual random damage.
    Damage_t RealDamage = max(1, Random(MinDamage, MaxDamage));

    RealDamage = (Damage_t)getPercentValue(RealDamage, VampireTimebandFactor[timeband]);

    if (pTargetCreature->isSlayer()) {
        Slayer* pTargetSlayer = dynamic_cast<Slayer*>(pTargetCreature);
        Assert(pTargetSlayer != NULL);

        Protection_t Protection = pTargetSlayer->getProtection();

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else if (pTargetCreature->isVampire()) {
        Vampire* pTargetVampire = dynamic_cast<Vampire*>(pTargetCreature);
        Assert(pTargetVampire != NULL);

        Protection_t Protection = pTargetVampire->getProtection();
        Protection = (Protection_t)getPercentValue(Protection, VampireTimebandFactor[timeband]);

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else if (pTargetCreature->isOusters()) {
        Ousters* pTargetOusters = dynamic_cast<Ousters*>(pTargetCreature);
        Assert(pTargetOusters != NULL);

        Protection_t Protection = pTargetOusters->getProtection();

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else if (pTargetCreature->isMonster()) {
        Monster* pTargetMonster = dynamic_cast<Monster*>(pTargetCreature);
        Assert(pTargetMonster != NULL);

        Protection_t Protection = pTargetMonster->getProtection();
        Protection = (Protection_t)getPercentValue(Protection, MonsterTimebandFactor[timeband]);

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else {
        // Anything else is an NPC, so no damage.
        return 0;
    }

    // The balance adjustment is done in AbilityBalance.cpp.

    return (Damage_t)FinalDamage;
}

//////////////////////////////////////////////////////////////////////////////
// Computes the damage between an Ousters attacker and the target.
//////////////////////////////////////////////////////////////////////////////
Damage_t computeOustersDamage(Ousters* pOusters, Creature* pTargetCreature, bool bCritical) {
    Assert(pOusters != NULL);
    Assert(pTargetCreature != NULL);

    double FinalDamage = 0;
    Damage_t MinDamage = pOusters->getDamage(ATTR_CURRENT);
    Damage_t MaxDamage = pOusters->getDamage(ATTR_MAX);
    uint timeband = getZoneTimeband(pOusters->getZone());

    // Damage from the Ousters weapon
    Item* pItem = pOusters->getWearItem(Ousters::WEAR_RIGHTHAND);

    // If a weapon is held, add the weapon's min and max to min and max.
    if (pItem != NULL && pOusters->isRealWearingEx(Ousters::WEAR_RIGHTHAND)) {
        MinDamage += pItem->getMinDamage();
        MaxDamage += pItem->getMaxDamage();
    }

    // Computes the actual random damage.
    Damage_t RealDamage = max(1, Random(MinDamage, MaxDamage));

    if (pTargetCreature->isSlayer()) {
        Slayer* pTargetSlayer = dynamic_cast<Slayer*>(pTargetCreature);
        Assert(pTargetSlayer != NULL);

        Protection_t Protection = pTargetSlayer->getProtection();

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else if (pTargetCreature->isVampire()) {
        Vampire* pTargetVampire = dynamic_cast<Vampire*>(pTargetCreature);
        Assert(pTargetVampire != NULL);

        Protection_t Protection = pTargetVampire->getProtection();
        Protection = (Protection_t)getPercentValue(Protection, VampireTimebandFactor[timeband]);

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else if (pTargetCreature->isOusters()) {
        Ousters* pTargetOusters = dynamic_cast<Ousters*>(pTargetCreature);
        Assert(pTargetOusters != NULL);

        Protection_t Protection = pTargetOusters->getProtection();

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else if (pTargetCreature->isMonster()) {
        Monster* pTargetMonster = dynamic_cast<Monster*>(pTargetCreature);
        Assert(pTargetMonster != NULL);

        Protection_t Protection = pTargetMonster->getProtection();
        Protection = (Protection_t)getPercentValue(Protection, MonsterTimebandFactor[timeband]);

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else {
        // Anything else is an NPC, so no damage.
        return 0;
    }

    // The balance adjustment is done in AbilityBalance.cpp.

    return (Damage_t)FinalDamage;
}

//////////////////////////////////////////////////////////////////////////////
// Computes the damage between a monster attacker and the target.
//////////////////////////////////////////////////////////////////////////////
Damage_t computeMonsterDamage(Monster* pMonster, Creature* pTargetCreature, bool bCritical) {
    Assert(pMonster != NULL);
    Assert(pTargetCreature != NULL);

    double FinalDamage = 0;
    Damage_t MinDamage = pMonster->getDamage(ATTR_CURRENT);
    Damage_t MaxDamage = pMonster->getDamage(ATTR_MAX);
    Damage_t RealDamage = max(1, Random(MinDamage, MaxDamage));
    uint timeband = getZoneTimeband(pMonster->getZone());

    RealDamage = (Damage_t)getPercentValue(RealDamage, MonsterTimebandFactor[timeband]);

    if (pTargetCreature->isSlayer()) {
        Slayer* pTargetSlayer = dynamic_cast<Slayer*>(pTargetCreature);
        Assert(pTargetSlayer != NULL);

        Protection_t Protection = pTargetSlayer->getProtection();

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else if (pTargetCreature->isVampire()) {
        Vampire* pTargetVampire = dynamic_cast<Vampire*>(pTargetCreature);
        Assert(pTargetVampire != NULL);

        Protection_t Protection = pTargetVampire->getProtection();
        Protection = (Protection_t)getPercentValue(Protection, VampireTimebandFactor[timeband]);

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else if (pTargetCreature->isOusters()) {
        Ousters* pTargetOusters = dynamic_cast<Ousters*>(pTargetCreature);
        Assert(pTargetOusters != NULL);

        Protection_t Protection = pTargetOusters->getProtection();

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else if (pTargetCreature->isMonster()) {
        Monster* pTargetMonster = dynamic_cast<Monster*>(pTargetCreature);
        Assert(pTargetMonster != NULL);

        Protection_t Protection = pTargetMonster->getProtection();
        Protection = (Protection_t)getPercentValue(Protection, MonsterTimebandFactor[timeband]);

        FinalDamage = computeFinalDamage(MinDamage, MaxDamage, RealDamage, Protection, bCritical);
    } else {
        // Anything else is an NPC, so no damage.
        return 0;
    }

    return (Damage_t)FinalDamage;
}

//////////////////////////////////////////////////////////////////////////////
// Computes magic damage with resistance taken into account.
//////////////////////////////////////////////////////////////////////////////
Damage_t computeMagicDamage(Creature* pTargetCreature, int Damage, SkillType_t SkillType, bool bVampire,
                            Creature* pAttacker) {
    Assert(pTargetCreature != NULL);

    SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SkillType);
    Assert(pSkillInfo != NULL);

    int MagicDomain = pSkillInfo->getMagicDomain();
    int MagicLevel = pSkillInfo->getLevel();

    int Resist = pTargetCreature->getResist(MagicDomain);

    if (pAttacker != NULL && pAttacker->isVampire()) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pAttacker);
        if (pVampire != NULL) {
            if (pSkillInfo->getMagicDomain() >= MAGIC_DOMAIN_POISON &&
                pSkillInfo->getMagicDomain() <= MAGIC_DOMAIN_BLOOD) {
                int mastery = pVampire->getMastery(pSkillInfo->getMagicDomain());
                Resist = getPercentValue(Resist, 100 - mastery);
            }
        }
    }

    float penalty = 1.5 * (Resist - (MagicLevel / 5.0)) / (Resist - (MagicLevel / 5.0) + 100.0);
    penalty = max(penalty, -0.2f);
    Damage = (int)(Damage * (1.0 - penalty));

    return (Damage_t)max(0, (int)Damage);
}

//////////////////////////////////////////////////////////////////////////////
// Computes Ousters magic damage with the wristlet taken into account.
//////////////////////////////////////////////////////////////////////////////
Damage_t computeOustersMagicDamage(Ousters* pOusters, Creature* pTargetCreature, int Damage, SkillType_t SkillType) {
    Assert(pOusters != NULL);

    Item* pWeapon = pOusters->getWearItem(Ousters::WEAR_RIGHTHAND);
    if (pWeapon == NULL || pWeapon->getItemClass() != Item::ITEM_CLASS_OUSTERS_WRISTLET)
        return 0;

    Damage_t MinDamage = pWeapon->getMinDamage();
    Damage_t MaxDamage = pWeapon->getMaxDamage();

    Damage_t RealDamage = max(1, Random(MinDamage, MaxDamage));
    return RealDamage + Damage;
}

//////////////////////////////////////////////////////////////////////////////
// Computes the silver damage dealt to the target.
//////////////////////////////////////////////////////////////////////////////
Damage_t computeSlayerSilverDamage(Creature* pCreature, int Damage, ModifyInfo* pMI) {
    Assert(pCreature != NULL);

    // Only a Slayer has any reason to deal silver damage.
    if (pCreature->isSlayer() == false)
        return 0;

    Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
    Assert(pSlayer != NULL);

    // Checks for a weapon and returns 0 when there is none.
    Item* pWeapon = pSlayer->getWearItem(Slayer::WEAR_RIGHTHAND);
    if (pWeapon == NULL)
        return 0;

    Damage_t silverDamage = 0;

    if (isMeleeWeapon(pWeapon)) {
        // A cleric weapon deals 10% silver damage by default.
        if (isClericWeapon(pWeapon)) {
            silverDamage = max(1, (int)(Damage * 0.1));
            silverDamage = min((int)silverDamage, (int)pWeapon->getSilver());

            if (silverDamage > 0) {
                pWeapon->setSilver(pWeapon->getSilver() - silverDamage);

                if (pMI != NULL)
                    pMI->addShortData(MODIFY_SILVER_DURABILITY, pWeapon->getSilver());
            }

            // The base 10% silver damage
            silverDamage += max(1, (int)(Damage * 0.1));
        }
        // A weapon that is not a cleric weapon deals silver damage only when it is silver-plated.
        else {
            silverDamage = max(1, (int)(Damage * 0.1));
            silverDamage = min((int)silverDamage, (int)pWeapon->getSilver());

            if (silverDamage > 0) {
                pWeapon->setSilver(pWeapon->getSilver() - silverDamage);

                if (pMI != NULL)
                    pMI->addShortData(MODIFY_SILVER_DURABILITY, pWeapon->getSilver());
            }
        }
    } else if (isArmsWeapon(pWeapon) && pWeapon->getSilver() > 0) {
        // A gun fires silver bullets, so the weapon's own silver must
        // not be reduced. That is handled outside, in the code that
        // consumes the bullets.
        silverDamage = max(1, (int)(Damage * 0.1));
    }

    return silverDamage;
}

void computeCriticalBonus(Ousters* pOusters, SkillType_t skillType, Damage_t& Damage, bool& bCriticalHit) {
    switch (skillType) {
    case SKILL_KASAS_ARROW:
    case SKILL_BLAZE_BOLT:
    case SKILL_EARTHS_TEETH:
    case SKILL_HANDS_OF_NIZIE:
    case SKILL_STONE_AUGER:
    case SKILL_EMISSION_WATER:
    case SKILL_BEAT_HEAD:
    case SKILL_MAGNUM_SPEAR:
    case SKILL_FIRE_PIERCING:
    case SKILL_SUMMON_FIRE_ELEMENTAL:
    case SKILL_ICE_LANCE:
    case SKILL_EXPLOSION_WATER:
    case SKILL_METEOR_STORM: {
        OustersSkillSlot* pSkillSlot = pOusters->hasSkill(SKILL_CRITICAL_MAGIC);
        if (pSkillSlot == NULL)
            return;

        SkillLevel_t level = pSkillSlot->getExpLevel();

        int Ratio = 5 + (level / 3);
        if ((rand() % 100) < Ratio) {
            bCriticalHit = true;
            if (level <= 15) {
                Damage += (Damage_t)(Damage * level / 19.5);
            } else {
                int bonus = (Damage_t)(Damage * (0.1 + (level / 75.0)));
                if (level == 30)
                    bonus = (Damage_t)(bonus * 1.1);

                Damage += bonus;
            }
        }
    }

    default:
        return;
    }
}

HP_t setCounterDamage(Creature* pAttacker, Creature* pTarget, Damage_t counterDamage, bool& bBroadcastAttackerHP,
                      bool& bSendAttackerHP) {
    HP_t Result2 = 0;
    // Safe zone check
    // 2003.1.10 by bezz, Sequoia
    if (pAttacker != NULL && checkZoneLevelToHitTarget(pAttacker)) {
        if (pAttacker->isSlayer()) {
            Slayer* pSlayerAttacker = dynamic_cast<Slayer*>(pAttacker);
            Result2 = max(0, (int)pSlayerAttacker->getHP() - (int)counterDamage);
            pSlayerAttacker->setHP(Result2, ATTR_CURRENT);

            bBroadcastAttackerHP = true;
            bSendAttackerHP = true;
        } else if (pAttacker->isVampire()) {
            Vampire* pVampireAttacker = dynamic_cast<Vampire*>(pAttacker);
            Result2 = max(0, (int)pVampireAttacker->getHP() - (int)counterDamage);
            pVampireAttacker->setHP(Result2, ATTR_CURRENT);

            bBroadcastAttackerHP = true;
            bSendAttackerHP = true;

            // The Mephisto effect is cleared once HP drops low.
            if (pVampireAttacker->isFlag(Effect::EFFECT_CLASS_MEPHISTO)) {
                HP_t maxHP = pVampireAttacker->getHP(ATTR_MAX);

                // A third, not 30%.
                if (Result2 * 3 < maxHP) {
                    Effect* pEffect = pVampireAttacker->findEffect(Effect::EFFECT_CLASS_MEPHISTO);
                    if (pEffect != NULL) {
                        pEffect->setDeadline(0);
                    } else {
                        pVampireAttacker->removeFlag(Effect::EFFECT_CLASS_MEPHISTO);
                    }
                }
            }
        } else if (pAttacker->isOusters()) {
            Ousters* pOustersAttacker = dynamic_cast<Ousters*>(pAttacker);
            Result2 = max(0, (int)pOustersAttacker->getHP() - (int)counterDamage);
            pOustersAttacker->setHP(Result2, ATTR_CURRENT);

            bBroadcastAttackerHP = true;
            bSendAttackerHP = true;
        } else if (pAttacker->isMonster()) {
            Monster* pMonsterAttacker = dynamic_cast<Monster*>(pAttacker);
            Result2 = max(0, (int)pMonsterAttacker->getHP() - (int)counterDamage);
            pMonsterAttacker->setHP(Result2, ATTR_CURRENT);
            pMonsterAttacker->setDamaged(true);

            // Counter damage on a monster gives precedence to the Slayer whose Sharp Shield reflected it.
            pMonsterAttacker->addPrecedence(pTarget->getName(), pTarget->getPartyID(), counterDamage);
            pMonsterAttacker->setLastHitCreatureClass(pTarget->getCreatureClass());

            bBroadcastAttackerHP = true;
            if (pMonsterAttacker->getHP(ATTR_CURRENT) * 3 < pMonsterAttacker->getHP(ATTR_MAX)) {
                PrecedenceTable* pTable = pMonsterAttacker->getPrecedenceTable();

                // Computing unconditionally whenever HP is below a third
                // would repeat meaningless work every turn, so once the
                // computation has run a flag is set and it does not run
                // again before the monster dies, sparing the useless work.
                if (pTable->getComputeFlag() == false) {
                    // Computes the table.
                    pTable->compute();

                    // Applies the effect using the host's name and party ID.
                    EffectPrecedence* pEffectPrecedence = new EffectPrecedence(pMonsterAttacker);
                    pEffectPrecedence->setDeadline(100);
                    pEffectPrecedence->setHostName(pTable->getHostName());
                    pEffectPrecedence->setHostPartyID(pTable->getHostPartyID());
                    pMonsterAttacker->setFlag(Effect::EFFECT_CLASS_PRECEDENCE);
                    pMonsterAttacker->addEffect(pEffectPrecedence);
                }
            }
        }
    }

    return Result2;
}

bool canBlockByWaterBarrier(SkillType_t SkillType) {
    switch (SkillType) {
    case SKILL_METEOR_STRIKE:
    case SKILL_POISON_STORM:
    case SKILL_GREEN_POISON:
    case SKILL_GREEN_STALKER:
    case SKILL_ACID_STORM:
    case SKILL_ACID_BOLT:
    case SKILL_ACID_STRIKE:
    case SKILL_ACID_SWAMP:
    case SKILL_BLOODY_KNIFE:
    case SKILL_BLOODY_WAVE:
    case SKILL_BLOODY_STRIKE:
    case SKILL_BLOODY_BALL:
    case SKILL_BLOODY_WALL:
    case SKILL_BLOODY_SPEAR:
    case SKILL_WIND_DIVIDER:
    case SKILL_SWORD_RAY:
    case SKILL_WIDE_LIGHTNING:
    case SKILL_EARTHQUAKE:
    case SKILL_POWER_OF_LAND:
    case SKILL_MULTI_AMPUTATE:
    case SKILL_WILD_TYPHOON:
    case SKILL_ATTACK_ARMS:
    case SKILL_DOUBLE_SHOT:
    case SKILL_TRIPLE_SHOT:
    case SKILL_MULTI_SHOT:
    case SKILL_HEAD_SHOT:
    case SKILL_QUICK_FIRE:
    case SKILL_ULTIMATE_BLOW:
    case SKILL_PIERCING:
    case SKILL_THROW_BOMB:
    case SKILL_BULLET_OF_LIGHT:
    case SKILL_GUN_SHOT_GUIDANCE:
    case SKILL_HOLY_ARROW:
    case SKILL_CAUSE_LIGHT_WOUNDS:
    case SKILL_CAUSE_SERIOUS_WOUNDS:
    case SKILL_CAUSE_CRITICAL_WOUNDS:
    case SKILL_VIGOR_DROP:
    case SKILL_TURN_UNDEAD:
    case SKILL_ILLENDUE:
    case SKILL_THROW_HOLY_WATER:
    case SKILL_LIGHT_BALL:
    case SKILL_AURA_BALL:
    case SKILL_AURA_RING:
    case SKILL_ENERGY_DROP:
    case SKILL_REBUKE:
    case SKILL_SPIRIT_GUARD:
    case SKILL_KASAS_ARROW:
    case SKILL_PROMINENCE:
    case SKILL_RING_OF_FLARE:
    case SKILL_BLAZE_BOLT:
    case SKILL_HANDS_OF_NIZIE:
    case SKILL_STONE_AUGER:
    case SKILL_EARTHS_TEETH:

        return true;
    default:
        return false;
    }
}

bool canBlockByGrayDarkness(SkillType_t skillType) {
    switch (skillType) {
    case SKILL_SWORD_WAVE:
    case SKILL_SWORD_RAY:
    case SKILL_WIND_DIVIDER:
    case SKILL_THUNDER_BOLT:
    case SKILL_THUNDER_STORM:
    case SKILL_WIDE_LIGHTNING:
    case SKILL_TORNADO_SEVER:
    case SKILL_EARTHQUAKE:
    case SKILL_POWER_OF_LAND:
    case SKILL_WILD_TYPHOON:
    case SKILL_MULTI_SHOT:
    case SKILL_MOLE_SHOT:
    case SKILL_GUN_SHOT_GUIDANCE:
    case SKILL_VIGOR_DROP:
    case SKILL_TURN_UNDEAD:
    case SKILL_ENERGY_DROP:
        return true;
    default:
        return false;
    }
}

//////////////////////////////////////////////////////////////////////////////
// Applies damage directly.
//////////////////////////////////////////////////////////////////////////////
HP_t setDamage(Creature* pTargetCreature, Damage_t Damage, Creature* pAttacker, SkillType_t SkillType, ModifyInfo* pMI,
               ModifyInfo* pAttackerMI, bool canKillTarget, bool canSteal) {
    Assert(pTargetCreature != NULL);

    if (pTargetCreature->isFlag(Effect::EFFECT_CLASS_NO_DAMAGE) ||
        pTargetCreature->isFlag(Effect::EFFECT_CLASS_TENDRIL) || pTargetCreature->isDead()) {
        //  Returning the current HP would be the correct thing to do,
        //  but nothing uses the return value, so it is ignored.
        //  by sigi. 2002.9.5
        return 0;
    }


    Zone* pZone = pTargetCreature->getZone();
    Assert(pZone != NULL);

    // In the Gilles de Rais lair, one PC cannot damage another.
    if (pZone->getZoneID() == 1412 || pZone->getZoneID() == 1413) {
        if (pTargetCreature->isPC() && pAttacker != NULL && pAttacker->isPC())
            return 0;
    }

    HP_t Result = 0;
    HP_t Result2 = 0;
    HP_t hp = 0;
    MP_t mp = 0;
    ZoneCoord_t TX = 0;
    ZoneCoord_t TY = 0;
    ObjectID_t TOID = 0;
    ZoneCoord_t AX = 0;
    ZoneCoord_t AY = 0;
    ObjectID_t AOID = 0;

    Damage_t OriginalDamage = Damage;

    bool bBroadcastTargetHP = false;   // Broadcast the target's HP?
    bool bSendTargetHP = false;        // Send the target's HP?
    bool bSendTargetMP = false;        // Send the target's MP?
    bool bBroadcastAttackerHP = false; // Broadcast the attacker's HP?
    bool bSendAttackerHP = false;      // Send the attacker's HP?
    bool bSendAttackerMP = false;      // Send the attacker's MP?

    GCStatusCurrentHP gcTargetHP;
    GCStatusCurrentHP gcAttackerHP;

    SkillProperty* pSkillProperty = de::gameContext().skillProps().getSkillProperty(SkillType);
    bool bPhysicDamage = pSkillProperty->isPhysic();
    bool bMagicDamage = pSkillProperty->isMagic();

    SkillInfo* pSkillInfo = NULL;

    if (SkillType >= SKILL_DOUBLE_IMPACT) {
        pSkillInfo = g_pSkillInfoManager->getSkillInfo(SkillType);
        Assert(pSkillInfo != NULL);

        BYTE domain = pSkillInfo->getDomainType();
        switch (domain) {
        case SKILL_DOMAIN_BLADE:
        case SKILL_DOMAIN_SWORD:
        case SKILL_DOMAIN_GUN:
            bPhysicDamage = true;
            bMagicDamage = false;
            break;
        case SKILL_DOMAIN_HEAL:
        case SKILL_DOMAIN_ENCHANT:
            bPhysicDamage = false;
            bMagicDamage = true;
            break;
        default:
            break;
        }
    }

    if (pTargetCreature != NULL && pTargetCreature->isMonster()) {
        Monster* pMonster = dynamic_cast<Monster*>(pTargetCreature);
        if (pMonster != NULL && pMonster->getMonsterType() == GROUND_ELEMENTAL_TYPE)
            if (pAttacker != NULL && pAttacker->isOusters())
                return 0;
    }

    // Striking inflates magic damage.
    if (pAttacker != NULL && pSkillProperty->isMagic() && pAttacker->isFlag(Effect::EFFECT_CLASS_STRIKING)) {
        EffectStriking* pEffect = dynamic_cast<EffectStriking*>(pAttacker->findEffect(Effect::EFFECT_CLASS_STRIKING));
        if (pEffect != NULL) {
            Damage += pEffect->getDamageBonus();
        }
    }

    if (pTargetCreature != NULL && pTargetCreature->isFlag(Effect::EFFECT_CLASS_INSTALL_TURRET)) {
        EffectInstallTurret* pEffect =
            dynamic_cast<EffectInstallTurret*>(pTargetCreature->findEffect(Effect::EFFECT_CLASS_INSTALL_TURRET));
        if (pEffect != NULL) {
            Damage -= getPercentValue(Damage, pEffect->getDefense());
        }
    }

    // Denial Magic checks whether the damage is magic and refuses it.
    if (pTargetCreature != NULL && pTargetCreature->isFlag(Effect::EFFECT_CLASS_DENIAL_MAGIC) &&
        pSkillProperty->isMagic()) {
        // Basic skills have no SkillInfo.
        if (SkillType >= SKILL_DOUBLE_IMPACT) {
            // Blocks Vampire and Ousters magic damage.
            if (pSkillInfo->getDomainType() == SKILL_DOMAIN_VAMPIRE ||
                pSkillInfo->getDomainType() == SKILL_DOMAIN_OUSTERS) {
                // On a successful block, sends the effect.
                GCAddEffect gcAddEffect;
                gcAddEffect.setObjectID(pTargetCreature->getObjectID());
                gcAddEffect.setEffectID(Effect::EFFECT_CLASS_DENIAL_MAGIC_DAMAGED);
                gcAddEffect.setDuration(0);

                pTargetCreature->getZone()->broadcastPacket(pTargetCreature->getX(), pTargetCreature->getY(),
                                                            &gcAddEffect);

                // Not a full block: the damage is cut by 60%.
                Damage = max(1, (int)(Damage * 0.4));
            }
        }
    }

    // Water Barrier reduces the damage of the non-melee skills
    // canBlockByWaterBarrier lists.
    if (pTargetCreature != NULL && pTargetCreature->isFlag(Effect::EFFECT_CLASS_WATER_BARRIER) &&
        !pSkillProperty->isMelee()) {
        // Basic skills have no SkillInfo.
        if (canBlockByWaterBarrier(SkillType)) {
            EffectWaterBarrier* pEWB =
                dynamic_cast<EffectWaterBarrier*>(pTargetCreature->findEffect(Effect::EFFECT_CLASS_WATER_BARRIER));
            if (pEWB != NULL) {
                int bonus = pEWB->getBonus();
                if (bonus > 100)
                    bonus = 100;

                Damage = (Damage_t)(Damage * ((float)(100 - bonus) / 100.0));
                Damage = max(1, (int)Damage);
            }
        }
    }

    // When Water Shield can be up, physical attacks deal no damage.
    if (pTargetCreature != NULL && pSkillProperty->isPhysic()) {
        if (pTargetCreature->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pTargetCreature);

            if (pOusters->isPassiveAvailable(SKILL_WATER_SHIELD)) {
                if ((rand() % 100) < min(20, (int)(pOusters->getPassiveRatio() * 3 / 2))) {
                    GCAddEffect gcAddEffect;
                    gcAddEffect.setObjectID(pOusters->getObjectID());
                    gcAddEffect.setEffectID(Effect::EFFECT_CLASS_WATER_SHIELD);
                    gcAddEffect.setDuration(0);

                    pZone->broadcastPacket(pOusters->getX(), pOusters->getY(), &gcAddEffect);
                    return 0;
                }
            }
        }
    }

    if (pTargetCreature != NULL && pTargetCreature->isFlag(Effect::EFFECT_CLASS_STONE_SKIN) &&
        (SkillType >= SKILL_DOUBLE_IMPACT || SkillType == SKILL_ATTACK_ARMS)) {
        if (SkillType == SKILL_ATTACK_ARMS ||
            g_pSkillInfoManager->getSkillInfo(SkillType)->getDomainType() == SKILL_DOMAIN_GUN) {
            EffectStoneSkin* pStoneSkin =
                dynamic_cast<EffectStoneSkin*>(pTargetCreature->findEffect(Effect::EFFECT_CLASS_STONE_SKIN));
            if (pStoneSkin != NULL) {
                if ((rand() % 100) < pStoneSkin->getBonus()) {
                    Damage = 0;
                    GCAddEffect gcAddEffect;
                    gcAddEffect.setObjectID(pTargetCreature->getObjectID());
                    gcAddEffect.setEffectID(Effect::EFFECT_CLASS_STONE_SKIN_DAMAGED);
                    gcAddEffect.setDuration(0);

                    pTargetCreature->getZone()->broadcastPacket(pTargetCreature->getX(), pTargetCreature->getY(),
                                                                &gcAddEffect);
                }
            }
        }
    }


    if (pAttacker != NULL && pAttacker->isOusters() && SkillType >= SKILL_DOUBLE_IMPACT) {
        Ousters* pOusters = dynamic_cast<Ousters*>(pAttacker);
        Assert(pOusters != NULL);
        SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SkillType);
        Assert(pSkillInfo != NULL);

        if (pSkillProperty->isMagic()) {
            switch (pSkillInfo->getElementalDomain()) {
            case ELEMENTAL_DOMAIN_FIRE:
                Damage += pOusters->getFireDamage();

                if (pOusters->isFlag(Effect::EFFECT_CLASS_HANDS_OF_FIRE)) {
                    EffectHandsOfFire* pEffect =
                        dynamic_cast<EffectHandsOfFire*>(pOusters->findEffect(Effect::EFFECT_CLASS_HANDS_OF_FIRE));
                    if (pEffect != NULL) {
                        Damage = (Damage_t)(Damage * (1.0 + (pEffect->getBonus() / 100.0)));
                    }
                }
                break;
            case ELEMENTAL_DOMAIN_WATER:
                Damage += pOusters->getWaterDamage();
                break;
            case ELEMENTAL_DOMAIN_EARTH:
                Damage += pOusters->getEarthDamage();
                break;
            default:
                break;
            }
        }
    }

    if (pTargetCreature != NULL) {
        TX = pTargetCreature->getX();
        TY = pTargetCreature->getY();
        TOID = pTargetCreature->getObjectID();

        gcTargetHP.setObjectID(TOID);
    }

    ////////////////////////////////////////////////////////////////////
    // Handles the effects on the target creature.
    ////////////////////////////////////////////////////////////////////
    if (pTargetCreature != NULL) {
        // Removes the SLEEP effect when it is applied.
        if (pTargetCreature->isFlag(Effect::EFFECT_CLASS_SLEEP) && SkillType != SKILL_REBUKE) {
            EffectSleep* pEffect = dynamic_cast<EffectSleep*>(pTargetCreature->findEffect(Effect::EFFECT_CLASS_SLEEP));
            Assert(pEffect != NULL);

            pEffect->setDeadline(0);
        }
    }

    if (pAttacker != NULL) {
        AX = pAttacker->getX();
        AY = pAttacker->getY();
        AOID = pAttacker->getObjectID();

        gcAttackerHP.setObjectID(AOID);

        // If this is ever optimized, the penalty-related members would
        // be better placed on Creature.
        // Hymn applies a damage penalty percentage.
        if (pAttacker->isFlag(Effect::EFFECT_CLASS_HYMN)) {
            EffectHymn* pHymn =
                dynamic_cast<EffectHymn*>(pAttacker->getEffectManager()->findEffect(Effect::EFFECT_CLASS_HYMN));

            Damage = Damage * (100 - pHymn->getDamagePenalty()) / 100;
        }

        if (pAttacker->isSlayer() && pAttackerMI != NULL && !pTargetCreature->isSlayer()) {
            Slayer* pAttackSlayer = dynamic_cast<Slayer*>(pAttacker);
            Assert(pAttackSlayer != NULL);

            // Gives skill experience for the self skills.
            if (canGiveSkillExp(pAttackSlayer, SKILL_DOMAIN_SWORD, SkillType)) {
                if (pAttackSlayer->isFlag(Effect::EFFECT_CLASS_DANCING_SWORD) && (rand() % 2) != 0)
                    giveSkillExp(pAttackSlayer, SKILL_DANCING_SWORD, *pAttackerMI);
                if (pAttackSlayer->isFlag(Effect::EFFECT_CLASS_REDIANCE) && (rand() % 2) != 0)
                    giveSkillExp(pAttackSlayer, SKILL_REDIANCE, *pAttackerMI);
                if (pAttackSlayer->isFlag(Effect::EFFECT_CLASS_EXPANSION) && (rand() % 2) != 0)
                    giveSkillExp(pAttackSlayer, SKILL_EXPANSION, *pAttackerMI);
            } else if (canGiveSkillExp(pAttackSlayer, SKILL_DOMAIN_BLADE, SkillType)) {
                if (pAttackSlayer->isFlag(Effect::EFFECT_CLASS_GHOST_BLADE) && (rand() % 2) != 0)
                    giveSkillExp(pAttackSlayer, SKILL_GHOST_BLADE, *pAttackerMI);
                if (pAttackSlayer->isFlag(Effect::EFFECT_CLASS_POTENTIAL_EXPLOSION) && (rand() % 2) != 0)
                    giveSkillExp(pAttackSlayer, SKILL_POTENTIAL_EXPLOSION, *pAttackerMI);
                if (pAttackSlayer->isFlag(Effect::EFFECT_CLASS_CHARGING_POWER) && (rand() % 2) != 0)
                    giveSkillExp(pAttackSlayer, SKILL_CHARGING_POWER, *pAttackerMI);
                if (pAttackSlayer->isFlag(Effect::EFFECT_CLASS_BERSERKER) && (rand() % 2) != 0)
                    giveSkillExp(pAttackSlayer, SKILL_BERSERKER, *pAttackerMI);
                if (pAttackSlayer->isFlag(Effect::EFFECT_CLASS_AIR_SHIELD_1) && (rand() % 2) != 0)
                    giveSkillExp(pAttackSlayer, SKILL_AIR_SHIELD, *pAttackerMI);
            } else if (canGiveSkillExp(pAttackSlayer, SKILL_DOMAIN_GUN, SkillType)) {
                if (pAttackSlayer->isFlag(Effect::EFFECT_CLASS_CONCEALMENT) && (rand() % 2) != 0)
                    giveSkillExp(pAttackSlayer, SKILL_CONCEALMENT, *pAttackerMI);
                if (pAttackSlayer->isFlag(Effect::EFFECT_CLASS_REVEALER) && (rand() % 2) != 0)
                    giveSkillExp(pAttackSlayer, SKILL_REVEALER, *pAttackerMI);
                if (pAttackSlayer->isFlag(Effect::EFFECT_CLASS_OBSERVING_EYE) && (rand() % 2) != 0)
                    giveSkillExp(pAttackSlayer, SKILL_OBSERVING_EYE, *pAttackerMI);
                if (pAttackSlayer->isFlag(Effect::EFFECT_CLASS_HEART_CATALYST) && (rand() % 2) != 0)
                    giveSkillExp(pAttackSlayer, SKILL_HEART_CATALYST, *pAttackerMI);
            } else if (canGiveSkillExp(pAttackSlayer, SKILL_DOMAIN_ENCHANT, SkillType)) {
                if (pAttackSlayer->isFlag(Effect::EFFECT_CLASS_AURA_SHIELD) && (rand() % 2) != 0)
                    giveSkillExp(pAttackSlayer, SKILL_AURA_SHIELD, *pAttackerMI);
            }
        }

        // Applies the Blood Bible bonus.
        if (pAttacker->isPC()) {
            PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pAttacker);
            Damage_t MagicBonusDamage = pPC->getMagicBonusDamage();
            Damage_t PhysicBonusDamage = pPC->getPhysicBonusDamage();

            if (MagicBonusDamage != 0 && bMagicDamage) {
                Damage += MagicBonusDamage;
            }
            if (PhysicBonusDamage != 0 && bPhysicDamage) {
                Damage += PhysicBonusDamage;
            }
        }
    }

    ////////////////////////////////////////////////////////////
    // Handles HP and MP steal first.
    ////////////////////////////////////////////////////////////
    if (pAttacker != NULL && canSteal) //(SkillType != SKILL_PROMINENCE && SkillType != SKILL_HELLFIRE)
    {
        Steal_t HPStealAmount = pAttacker->getHPStealAmount();
        Steal_t MPStealAmount = pAttacker->getMPStealAmount();

        // Checks the HP steal.
        if (HPStealAmount != 0 && (rand() % 100) < pAttacker->getHPStealRatio()) {
            if (pAttacker->isSlayer()) {
                if (pAttacker->isAlive()) {
                    Slayer* pSlayer = dynamic_cast<Slayer*>(pAttacker);

                    // Adds the stolen amount to the current HP and
                    // checks that it does not exceed the max.
                    hp = pSlayer->getHP(ATTR_CURRENT) + (int)HPStealAmount;
                    hp = min(hp, pSlayer->getHP(ATTR_MAX));

                    // Sets the HP and turns the flags on.
                    pSlayer->setHP(hp, ATTR_CURRENT);
                    bBroadcastAttackerHP = true;
                    bSendAttackerHP = true;
                }
            } else if (pAttacker->isVampire()) {
                Vampire* pVampire = dynamic_cast<Vampire*>(pAttacker);

                // Adds the stolen amount to the current HP and
                // checks that it does not exceed the max.
                hp = pVampire->getHP(ATTR_CURRENT) + (int)HPStealAmount;
                hp = min(hp, pVampire->getHP(ATTR_MAX));

                // Sets the HP and turns the flags on.
                pVampire->setHP(hp, ATTR_CURRENT);
                bBroadcastAttackerHP = true;
                bSendAttackerHP = true;
            } else if (pAttacker->isOusters()) {
                // Does not raise the HP of a dead attacker.
                if (pAttacker->isAlive()) {
                    Ousters* pOusters = dynamic_cast<Ousters*>(pAttacker);

                    // Adds the stolen amount to the current HP and
                    // checks that it does not exceed the max.
                    hp = pOusters->getHP(ATTR_CURRENT) + (int)HPStealAmount;
                    hp = min(hp, pOusters->getHP(ATTR_MAX));

                    // Sets the HP and turns the flags on.
                    pOusters->setHP(hp, ATTR_CURRENT);
                    bBroadcastAttackerHP = true;
                    bSendAttackerHP = true;
                }
            } else
                Assert(false);
        }

        // Checks the MP steal.
        if (MPStealAmount != 0 && (rand() % 100) < pAttacker->getMPStealRatio()) {
            // Handles MP steal for a Slayer or an Ousters.
            if (pAttacker->isSlayer()) {
                Slayer* pSlayer = dynamic_cast<Slayer*>(pAttacker);

                // Adds the stolen amount to the current MP and
                // checks that it does not exceed the max.
                mp = pSlayer->getMP(ATTR_CURRENT) + (int)MPStealAmount;
                mp = min(mp, pSlayer->getMP(ATTR_MAX));

                pSlayer->setMP(mp, ATTR_CURRENT);

                bSendAttackerMP = true;
            } else if (pAttacker->isOusters()) {
                Ousters* pOusters = dynamic_cast<Ousters*>(pAttacker);

                if (pOusters->getMP(ATTR_CURRENT) < pOusters->getMP(ATTR_MAX)) {
                    // Adds the stolen amount to the current MP and
                    // checks that it does not exceed the max.
                    mp = pOusters->getMP(ATTR_CURRENT) + (int)MPStealAmount;
                    mp = min(mp, pOusters->getMP(ATTR_MAX));

                    pOusters->setMP(mp, ATTR_CURRENT);

                    bSendAttackerMP = true;
                }
            }
        }
    }

    if (pTargetCreature != NULL && pTargetCreature->isFlag(Effect::EFFECT_CLASS_EXPLOSION_WATER)) {
        EffectExplosionWater* pEffect =
            dynamic_cast<EffectExplosionWater*>(pTargetCreature->findEffect(Effect::EFFECT_CLASS_EXPLOSION_WATER));

        if (pEffect != NULL) {
            int ratio = 100;
            ratio -= pEffect->getDamageReduce();
            if (ratio < 0)
                ratio = 0;

            Damage = max(1, getPercentValue(Damage, ratio));
        }
    }

    if (pAttacker != NULL && pAttacker->isFlag(Effect::EFFECT_CLASS_REPUTO_FACTUM)) {
        Damage_t counterDamage = getPercentValue(Damage, 30);

        GCAddEffect gcAddEffect;
        gcAddEffect.setObjectID(pAttacker->getObjectID());
        gcAddEffect.setEffectID(Effect::EFFECT_CLASS_REPUTO_FACTUM);
        gcAddEffect.setDuration(0);
        pAttacker->getZone()->broadcastPacket(pAttacker->getX(), pAttacker->getY(), &gcAddEffect);
        setCounterDamage(pAttacker, pTargetCreature, counterDamage, bBroadcastAttackerHP, bSendAttackerHP);
    }

    if (pTargetCreature != NULL && !pTargetCreature->isSlayer()) {
        Effect* pEffect = pTargetCreature->getZone()
                              ->getTile(pTargetCreature->getX(), pTargetCreature->getY())
                              .getEffect(Effect::EFFECT_CLASS_SWORD_OF_THOR);
        if (pEffect != NULL) {
            EffectSwordOfThor* pThor = dynamic_cast<EffectSwordOfThor*>(pEffect);
            if (pThor != NULL) {
                cout << "before thor : " << Damage << endl;
                Damage = getPercentValue(Damage, 140 + (pThor->getLevel() / 10));
                cout << "after thor : " << Damage << endl;
            }
        }
    }

    ////////////////////////////////////////////////////////////
    // The target is a Slayer.
    ////////////////////////////////////////////////////////////
    if (pTargetCreature->isSlayer()) {
        Slayer* pSlayer = dynamic_cast<Slayer*>(pTargetCreature);
        bool bSetDamage = false;

        if (pSlayer->isFlag(Effect::EFFECT_CLASS_REQUITAL) && pAttacker != NULL && pSkillProperty != NULL &&
            pSkillProperty->isMelee()) {
            EffectRequital* pEffectRequital = (EffectRequital*)(pSlayer->findEffect(Effect::EFFECT_CLASS_REQUITAL));
            int refl = pEffectRequital->getReflection();
            Damage_t counterDamage = max(1, getPercentValue(Damage, refl));

            Result2 = setCounterDamage(pAttacker, pSlayer, counterDamage, bBroadcastAttackerHP, bSendAttackerHP);
        }

        // The Aura Shield effect can consume MP instead of HP.
        // The ground attack in the master lair (a pillar of fire) ignores Aura Shield and burns HP.
        // 2003. 1.16. Sequoia
        if (pSlayer->isFlag(Effect::EFFECT_CLASS_AURA_SHIELD) && SkillType != SKILL_GROUND_ATTACK) {
            // Damage has to be returned to the attacker.
            if (pAttacker != NULL) {
                Damage_t counterDamage = 0;

                EffectAuraShield* pEffectAuraShield =
                    (EffectAuraShield*)(pSlayer->findEffect(Effect::EFFECT_CLASS_AURA_SHIELD));
                Assert(pEffectAuraShield != NULL);

                // The counter damage is a tenth of the original damage.
                counterDamage = max(1, getPercentValue(Damage, 10));

                if (pAttacker->isVampire()) {
                    Vampire* pVampireAttacker = dynamic_cast<Vampire*>(pAttacker);
                    Result2 = max(0, (int)pVampireAttacker->getHP() - (int)counterDamage);
                    pVampireAttacker->setHP(Result2, ATTR_CURRENT);

                    bBroadcastAttackerHP = true;
                    bSendAttackerHP = true;
                } else if (pAttacker->isOusters()) {
                    Ousters* pOustersAttacker = dynamic_cast<Ousters*>(pAttacker);
                    Result2 = max(0, (int)pOustersAttacker->getHP() - (int)counterDamage);
                    pOustersAttacker->setHP(Result2, ATTR_CURRENT);

                    bBroadcastAttackerHP = true;
                    bSendAttackerHP = true;
                } else if (pAttacker->isMonster()) {
                    Monster* pMonsterAttacker = dynamic_cast<Monster*>(pAttacker);
                    Result2 = max(0, (int)pMonsterAttacker->getHP() - (int)counterDamage);
                    pMonsterAttacker->setHP(Result2, ATTR_CURRENT);
                    pMonsterAttacker->setDamaged(true);

                    bBroadcastAttackerHP = true;
                }
            }

            Result = max(0, (int)pSlayer->getMP(ATTR_CURRENT) - (int)(Damage * 2));

            pSlayer->setMP(Result, ATTR_CURRENT);
            bSendTargetMP = true;

            // A Result of 0 means the mana has run out, so the
            // effect is removed.
            if (Result == 0) {
                Effect* pEffect = pSlayer->findEffect(Effect::EFFECT_CLASS_AURA_SHIELD);
                if (pEffect != NULL)
                    pEffect->setDeadline(0);

                // Clears the skill delay so the skill can be used again.
                SkillSlot* pSkillSlot = pSlayer->hasSkill(SKILL_AURA_SHIELD);
                if (pSkillSlot != NULL) {
                    pSkillSlot->setRunTime(0, false);
                }
            }
            bSetDamage = true;
        }

        // by Sequoia 2002.12.26
        // A melee attack deals reduced damage and damages the attacker back.
        // Melee is decided by the skill property rather than by a switch.
        if (pSlayer->isFlag(Effect::EFFECT_CLASS_SHARP_SHIELD_1) && pSkillProperty != NULL &&
            pSkillProperty->isMelee() && pAttacker != NULL) {
            // With Sharp Shield the melee attack deals half damage.
            Damage = max(0, (int)Damage - ((int)OriginalDamage >> 1));

            EffectSharpShield* pEffect =
                dynamic_cast<EffectSharpShield*>(pSlayer->findEffect(Effect::EFFECT_CLASS_SHARP_SHIELD_1));
            Assert(pEffect != NULL);

            Damage_t counterDamage = pEffect->getDamage();

            Result2 = setCounterDamage(pAttacker, pSlayer, counterDamage, bBroadcastAttackerHP, bSendAttackerHP);

            if (pAttacker != NULL && !pAttacker->isSlayer())
                if (pMI != NULL && (rand() % 2) != 0)
                    giveSkillExp(pSlayer, SKILL_SHARP_SHIELD, *pMI);
        }

        if (pSlayer->isFlag(Effect::EFFECT_CLASS_AIR_SHIELD_1) && pSkillProperty != NULL && pSkillProperty->isMelee()) {
            bool isUnderworld = false;

            if (!isUnderworld) {
                EffectAirShield* pEffect =
                    dynamic_cast<EffectAirShield*>(pSlayer->findEffect(Effect::EFFECT_CLASS_AIR_SHIELD_1));
                Assert(pEffect != NULL);

                Damage = max(1, (int)Damage - getPercentValue(OriginalDamage, pEffect->getDamage()));
            }
        }

        Tile& rTile = pZone->getTile(TX, TY);

        if (rTile.getEffect(Effect::EFFECT_CLASS_MAGIC_ELUSION) != NULL
            // Magic Elusion halves the damage of a ranged magic attack by a Vampire.
            && (!pSkillProperty->isMelee() && pSkillProperty->isMagic()) && pAttacker != NULL &&
            pAttacker->isVampire()) {
            Damage /= 2;
        }

        if (!bSetDamage) {
            if (pSlayer->isFlag(Effect::EFFECT_CLASS_HAS_BLOOD_BIBLE))
                Damage *= 1.5;

            if (bMagicDamage) {
                Damage -= min(Damage - 1, (int)pSlayer->getMagicDamageReduce());
            } else if (bPhysicDamage) {
                Damage -= min(Damage - 1, (int)pSlayer->getPhysicDamageReduce());
            }

            // There is no Aura Shield, so the hit lands on HP.
            if (canKillTarget)
                Result = max(0, (int)pSlayer->getHP(ATTR_CURRENT) - (int)Damage);
            else
                Result = max(1, (int)pSlayer->getHP(ATTR_CURRENT) - (int)Damage);

            pSlayer->setHP(Result, ATTR_CURRENT);

            bBroadcastTargetHP = true;
            bSendTargetHP = true;
        }
    }
    ////////////////////////////////////////////////////////////
    // The target is a Vampire.
    ////////////////////////////////////////////////////////////
    else if (pTargetCreature->isVampire()) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pTargetCreature);
        Silver_t silverDamage = 0;

        Tile& rTile = pZone->getTile(TX, TY);

        if (rTile.getEffect(Effect::EFFECT_CLASS_GRAY_DARKNESS) != NULL && canBlockByGrayDarkness(SkillType)) {
            // Inside Gray Darkness the damage is reduced by 30%.
            Damage = (Damage_t)(Damage * 0.7);
        }

        if (pAttacker != NULL && pAttacker->isSlayer()) {
            // A Slayer attacker can add silver damage to the damage.
            silverDamage = computeSlayerSilverDamage(pAttacker, Damage, pAttackerMI);
        }

        // Silver damage is extra damage.
        Damage += silverDamage;
        // Shine Sword counts the whole damage as silver damage.
        if (SkillType == SKILL_SHINE_SWORD && silverDamage != 0) {
            silverDamage = Damage;
        }
        // end

        if (pVampire->isFlag(Effect::EFFECT_CLASS_HAS_BLOOD_BIBLE))
            Damage *= 1.5;

        if (bMagicDamage) {
            Damage -= min(Damage - 1, (int)pVampire->getMagicDamageReduce());
        } else if (bPhysicDamage) {
            Damage -= min(Damage - 1, (int)pVampire->getPhysicDamageReduce());
        }

        HP_t currentHP = pVampire->getHP(ATTR_CURRENT);

        if (canKillTarget)
            Result = max(0, (int)currentHP - (int)Damage);
        else
            Result = max(1, (int)currentHP - (int)Damage);

        pVampire->setHP(Result, ATTR_CURRENT);

        // The Mephisto effect is cleared once HP drops low.
        if (pVampire->isFlag(Effect::EFFECT_CLASS_MEPHISTO)) {
            HP_t maxHP = pVampire->getHP(ATTR_MAX);

            // A third, not 30%.
            if (currentHP * 3 < maxHP) {
                Effect* pEffect = pVampire->findEffect(Effect::EFFECT_CLASS_MEPHISTO);
                if (pEffect != NULL) {
                    pEffect->setDeadline(0);
                } else {
                    pVampire->removeFlag(Effect::EFFECT_CLASS_MEPHISTO);
                }
            }
        }


        bBroadcastTargetHP = true;
        bSendTargetHP = true;

        if (silverDamage != 0 && pVampire->getSilverDamage() < (pVampire->getHP(ATTR_MAX) / 2)) {
            Silver_t newSilverDamage = pVampire->getSilverDamage() + silverDamage;
            pVampire->saveSilverDamage(newSilverDamage);
            if (pMI)
                pMI->addShortData(MODIFY_SILVER_DAMAGE, newSilverDamage);
        }
    }
    ////////////////////////////////////////////////////////////
    // The target is an Ousters.
    ////////////////////////////////////////////////////////////
    else if (pTargetCreature->isOusters()) {
        Ousters* pOusters = dynamic_cast<Ousters*>(pTargetCreature);
        Silver_t silverDamage = 0;

        if (pOusters->isFlag(Effect::EFFECT_CLASS_REACTIVE_ARMOR)) {
            EffectReactiveArmor* pEffect =
                dynamic_cast<EffectReactiveArmor*>(pOusters->findEffect(Effect::EFFECT_CLASS_REACTIVE_ARMOR));
            if (pEffect != NULL && pEffect->getDamageReduce() != 0) {
                cout << "before reactive : " << Damage << endl;
                Damage -= getPercentValue(Damage, pEffect->getDamageReduce());
                cout << "after  reactive : " << Damage << endl;
            }
        }

        // Divine Spirits absorbs part of the magic damage into MP.
        if (pOusters->isFlag(Effect::EFFECT_CLASS_DIVINE_SPIRITS) && pSkillProperty->isMagic()) {
            EffectDivineSpirits* pEffect =
                dynamic_cast<EffectDivineSpirits*>(pOusters->findEffect(Effect::EFFECT_CLASS_DIVINE_SPIRITS));
            if (pEffect != NULL) {
                int Ratio = pEffect->getBonus();
                Damage_t mpDamage = (Damage_t)(((DWORD)Damage) * Ratio / 100);
                mpDamage = min(Damage, mpDamage);
                mpDamage = min(pOusters->getMP(), mpDamage);

                Damage -= mpDamage;

                MP_t resultMP = pOusters->getMP() - mpDamage;
                pOusters->setMP(resultMP, ATTR_CURRENT);
                bSendTargetMP = true;
            }
        }

        if (pOusters->isFlag(Effect::EFFECT_CLASS_FROZEN_ARMOR) && pSkillProperty->isMelee()) {
            EffectFrozenArmor* pFrozenArmor =
                dynamic_cast<EffectFrozenArmor*>(pOusters->findEffect(Effect::EFFECT_CLASS_FROZEN_ARMOR));
            if (pFrozenArmor != NULL) {
                Damage -= getPercentValue(Damage, pFrozenArmor->getBonus());
                if (pAttacker != NULL) {
                    // Creates the effect class and attaches it.
                    EffectIceFieldToCreature* pEffect = new EffectIceFieldToCreature(pAttacker, true);
                    pEffect->setDeadline(pFrozenArmor->getTargetDuration());
                    pAttacker->addEffect(pEffect);
                    pAttacker->setFlag(Effect::EFFECT_CLASS_ICE_FIELD_TO_CREATURE);

                    GCAddEffect gcAddEffect;
                    gcAddEffect.setObjectID(pAttacker->getObjectID());
                    gcAddEffect.setEffectID(pEffect->getSendEffectClass());
                    gcAddEffect.setDuration(pFrozenArmor->getTargetDuration());

                    pAttacker->getZone()->broadcastPacket(pAttacker->getX(), pAttacker->getY(), &gcAddEffect);
                }
            }
        }

        if (pAttacker != NULL && pAttacker->isSlayer()) {
            // A Slayer attacker can add silver damage to the damage.
            // An Ousters takes 1.5 times the silver damage.
            silverDamage = (Silver_t)(computeSlayerSilverDamage(pAttacker, Damage, pAttackerMI) * 1.5);
            silverDamage = max(0, getPercentValue(silverDamage, 100 - pOusters->getSilverResist()));
        }

        // Silver damage is extra damage.
        Damage += silverDamage;
        // Shine Sword counts the whole damage as silver damage.
        if (SkillType == SKILL_SHINE_SWORD && silverDamage != 0) {
            silverDamage = Damage;
        }
        // end

        HP_t currentHP = pOusters->getHP(ATTR_CURRENT);

        if (bMagicDamage) {
            Damage -= min(Damage - 1, (int)pOusters->getMagicDamageReduce());
        } else if (bPhysicDamage) {
            Damage -= min(Damage - 1, (int)pOusters->getPhysicDamageReduce());
        }

        if (canKillTarget)
            Result = max(0, (int)currentHP - (int)Damage);
        else
            Result = max(1, (int)currentHP - (int)Damage);

        pOusters->setHP(Result, ATTR_CURRENT);

        bBroadcastTargetHP = true;
        bSendTargetHP = true;

        if (silverDamage != 0 && pOusters->getSilverDamage() < (pOusters->getHP(ATTR_MAX) / 2)) {
            Silver_t newSilverDamage = pOusters->getSilverDamage() + silverDamage;
            pOusters->saveSilverDamage(newSilverDamage);
            if (pMI)
                pMI->addShortData(MODIFY_SILVER_DAMAGE, newSilverDamage);
        }
    }
    ////////////////////////////////////////////////////////////
    // The target is a monster.
    ////////////////////////////////////////////////////////////
    else if (pTargetCreature->isMonster()) {
        Monster* pMonster = dynamic_cast<Monster*>(pTargetCreature);
        Silver_t silverDamage = 0;

        if (pAttacker != NULL && pAttacker->isSlayer()) {
            // A Slayer attacker can add silver damage to the damage.
            silverDamage = computeSlayerSilverDamage(pAttacker, Damage, pAttackerMI);
        }

        // Silver damage is extra damage.
        Damage += silverDamage;

        if (canKillTarget)
            Result = max(0, (int)pMonster->getHP(ATTR_CURRENT) - (int)Damage);
        else
            Result = max(1, (int)pMonster->getHP(ATTR_CURRENT) - (int)Damage);

        pMonster->setHP(Result, ATTR_CURRENT);

        if (pMonster->isFlag(Effect::EFFECT_CLASS_SHARE_HP)) {
            EffectShareHP* pEffect = dynamic_cast<EffectShareHP*>(pMonster->findEffect(Effect::EFFECT_CLASS_SHARE_HP));
            if (pEffect != NULL) {
                list<ObjectID_t>::iterator itr = pEffect->getSharingCreatures().begin();

                for (; itr != pEffect->getSharingCreatures().end(); ++itr) {
                    Monster* pMonster = dynamic_cast<Monster*>(pZone->getCreature(*itr));
                    if (pMonster != NULL) {
                        GCStatusCurrentHP gcHP;
                        pMonster->setHP(Result, ATTR_CURRENT);
                        gcHP.setObjectID(pMonster->getObjectID());
                        gcHP.setCurrentHP(Result);
                        pZone->broadcastPacket(pMonster->getX(), pMonster->getY(), &gcHP);
                    }
                }
            }
        }

        if (Result == 0 && pAttacker != NULL && pAttacker->isPC()) {
            increaseFame(pAttacker, Damage);

            PlayerCreature* pAttackerPC = dynamic_cast<PlayerCreature*>(pAttacker);
            Assert(pAttackerPC != NULL);

            GCModifyInformation gcFameMI;
            gcFameMI.addLongData(MODIFY_FAME, pAttackerPC->getFame());

            pAttackerPC->getPlayer()->sendPacket(&gcFameMI);
        }

        bBroadcastTargetHP = true;

        if (silverDamage != 0 && pMonster->getSilverDamage() < (pMonster->getHP(ATTR_MAX) / 2)) {
            pMonster->setSilverDamage(pMonster->getSilverDamage() + silverDamage);
        }

        pMonster->setDamaged(true);

        if (pAttacker != NULL && pAttacker->isPC()) {
            // When the target is a monster and the attacker is a PC,
            // the precedence table, which follows the damage, is updated.
            pMonster->addPrecedence(pAttacker->getName(), pAttacker->getPartyID(), Damage);
            pMonster->setLastHitCreatureClass(pAttacker->getCreatureClass());
        }

        // While the monster is not dead yet but can be blood drained,
        // the precedence is computed if it has not been computed yet.
        if (pMonster->getHP(ATTR_CURRENT) * 3 < pMonster->getHP(ATTR_MAX)) {
            PrecedenceTable* pTable = pMonster->getPrecedenceTable();

            // Computing unconditionally whenever HP is below a third
            // would repeat meaningless work every turn, so once the
            // computation has run a flag is set and it does not run
            // again before the monster dies, sparing the useless work.
            if (pTable->getComputeFlag() == false) {
                // Computes the table.
                pTable->compute();

                // Applies the effect using the host's name and party ID.
                EffectPrecedence* pEffectPrecedence = new EffectPrecedence(pMonster);
                pEffectPrecedence->setDeadline(100);
                pEffectPrecedence->setHostName(pTable->getHostName());
                pEffectPrecedence->setHostPartyID(pTable->getHostPartyID());
                pMonster->setFlag(Effect::EFFECT_CLASS_PRECEDENCE);
                pMonster->addEffect(pEffectPrecedence);
            }
        }

        if (pMonster->getMonsterType() == 722 && pAttacker != NULL &&
            !pAttacker->isFlag(Effect::EFFECT_CLASS_BLINDNESS)) {
            if ((rand() % 100) < 30) {
                // The Gilles de Rais statue blinds its attacker.
                EffectBlindness* pEffect = new EffectBlindness(pAttacker);
                pEffect->setDamage(50);
                pEffect->setNextTime(0);
                pEffect->setDeadline(100);
                pAttacker->setFlag(Effect::EFFECT_CLASS_BLINDNESS);
                pAttacker->addEffect(pEffect);

                GCAddEffect gcAddEffect;
                gcAddEffect.setObjectID(pAttacker->getObjectID());
                gcAddEffect.setEffectID(pEffect->getSendEffectClass());
                gcAddEffect.setDuration(100);

                pZone->broadcastPacket(pAttacker->getX(), pAttacker->getY(), &gcAddEffect);
            }
        }
    }


    ////////////////////////////////////////////////////////////
    // Sends the changes according to the flags.
    ////////////////////////////////////////////////////////////
    if (bBroadcastTargetHP && pTargetCreature != NULL) // The target's HP dropped, so broadcast it.
    {
        if (pTargetCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pTargetCreature);
            gcTargetHP.setCurrentHP(pSlayer->getHP(ATTR_CURRENT));
        } else if (pTargetCreature->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pTargetCreature);
            gcTargetHP.setCurrentHP(pVampire->getHP(ATTR_CURRENT));
        } else if (pTargetCreature->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pTargetCreature);
            gcTargetHP.setCurrentHP(pOusters->getHP(ATTR_CURRENT));
        } else if (pTargetCreature->isMonster()) {
            Monster* pMonster = dynamic_cast<Monster*>(pTargetCreature);
            gcTargetHP.setCurrentHP(pMonster->getHP(ATTR_CURRENT));
        } else
            Assert(false);

        pZone->broadcastPacket(TX, TY, &gcTargetHP, pTargetCreature);
    }

    if (bSendTargetHP && pTargetCreature != NULL) // Tells the target itself that its HP dropped.
    {
        if (pTargetCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pTargetCreature);
            if (pMI != NULL)
                pMI->addShortData(MODIFY_CURRENT_HP, pSlayer->getHP(ATTR_CURRENT));
        } else if (pTargetCreature->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pTargetCreature);
            if (pMI != NULL)
                pMI->addShortData(MODIFY_CURRENT_HP, pVampire->getHP(ATTR_CURRENT));
        } else if (pTargetCreature->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pTargetCreature);
            if (pMI != NULL)
                pMI->addShortData(MODIFY_CURRENT_HP, pOusters->getHP(ATTR_CURRENT));
        } else
            Assert(false);
    }

    if (bSendTargetMP && pTargetCreature != NULL) // Tells the target itself that its MP dropped.
    {
        if (pTargetCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pTargetCreature);
            if (pMI != NULL)
                pMI->addShortData(MODIFY_CURRENT_MP, pSlayer->getMP(ATTR_CURRENT));
            else {
                GCModifyInformation gcMI;
                gcMI.addShortData(MODIFY_CURRENT_MP, pSlayer->getMP(ATTR_CURRENT));

                pSlayer->getPlayer()->sendPacket(&gcMI);
            }
        } else if (pTargetCreature->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pTargetCreature);
            if (pMI != NULL)
                pMI->addShortData(MODIFY_CURRENT_MP, pOusters->getMP(ATTR_CURRENT));
            else {
                GCModifyInformation gcMI;
                gcMI.addShortData(MODIFY_CURRENT_MP, pOusters->getMP(ATTR_CURRENT));

                pOusters->getPlayer()->sendPacket(&gcMI);
            }
        } else
            Assert(false);
    }

    if (bBroadcastAttackerHP && pAttacker != NULL) // The attacker's HP dropped, so broadcast it.
    {
        if (pAttacker->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pAttacker);
            gcAttackerHP.setCurrentHP(pSlayer->getHP(ATTR_CURRENT));
        } else if (pAttacker->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pAttacker);
            gcAttackerHP.setCurrentHP(pVampire->getHP(ATTR_CURRENT));
        } else if (pAttacker->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pAttacker);
            gcAttackerHP.setCurrentHP(pOusters->getHP(ATTR_CURRENT));
        } else if (pAttacker->isMonster()) {
            Monster* pMonster = dynamic_cast<Monster*>(pAttacker);
            gcAttackerHP.setCurrentHP(pMonster->getHP(ATTR_CURRENT));
        } else
            Assert(false);

        pZone->broadcastPacket(AX, AY, &gcAttackerHP, pAttacker);
    }

    if (bSendAttackerHP && pAttacker != NULL) // Tells the attacker that its HP dropped.
    {
        if (pAttacker->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pAttacker);
            if (pAttackerMI != NULL)
                pAttackerMI->addShortData(MODIFY_CURRENT_HP, pSlayer->getHP(ATTR_CURRENT));
        } else if (pAttacker->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pAttacker);
            if (pAttackerMI != NULL)
                pAttackerMI->addShortData(MODIFY_CURRENT_HP, pVampire->getHP(ATTR_CURRENT));
        } else if (pAttacker->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pAttacker);
            if (pAttackerMI != NULL)
                pAttackerMI->addShortData(MODIFY_CURRENT_HP, pOusters->getHP(ATTR_CURRENT));
        } else
            Assert(false);
    }

    if (bSendAttackerMP && pAttacker != NULL) // Tells the attacker that its MP dropped.
    {
        if (pAttacker->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pAttacker);
            if (pAttackerMI != NULL)
                pAttackerMI->addShortData(MODIFY_CURRENT_MP, pSlayer->getMP(ATTR_CURRENT));
        } else if (pAttacker->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pAttacker);
            if (pAttackerMI != NULL)
                pAttackerMI->addShortData(MODIFY_CURRENT_MP, pOusters->getMP(ATTR_CURRENT));
        } else
            Assert(false);
    }

    // Increases the kill count when the target dies.
    if (pTargetCreature->isDead()) {
        affectKillCount(pAttacker, pTargetCreature);
    }

    return Result;
}

//////////////////////////////////////////////////////////////////////////////
// Reduces item durability.
//////////////////////////////////////////////////////////////////////////////
void decreaseDurability(Creature* pCreature, Creature* pTargetCreature, SkillInfo* pSkillInfo, ModifyInfo* pMI1,
                        ModifyInfo* pMI2) {
    WORD Point = (pSkillInfo) ? (pSkillInfo->getConsumeMP() / 3) : 1;

    // Nothing to do when the durability to take off is 0.
    if (Point == 0)
        return;

    Item* pWeapon = NULL;
    Item* pGear = NULL;
    int slot = 0;
    int durDiff = 0;
    int CurDur = 0;
    int Result = 0;
    ulong value = 0;

    ////////////////////////////////////////////////////////////////
    // Reduces the durability of the right-hand weapon of a Slayer,
    // a Vampire or an Ousters attacker.
    ////////////////////////////////////////////////////////////////
    if (pCreature != NULL) {
        if (pCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);

            slot = Slayer::WEAR_RIGHTHAND;
            pWeapon = pSlayer->getWearItem((Slayer::WearPart)slot);

            // Reduces it when a weapon is held.
            if (pWeapon != NULL && canDecreaseDurability(pWeapon))
            //				&& !pWeapon->isUnique())
            {
                CurDur = pWeapon->getDurability();
                durDiff = Point;
                Result = max(0, CurDur - durDiff);

                if (Result == 0) // Destroys the weapon when its durability reaches 0.
                {
                    GCRemoveFromGear gcRemoveFromGear;
                    gcRemoveFromGear.setSlotID(slot);
                    pSlayer->takeOffItem((Slayer::WearPart)slot, false, true);

                    Player* pPlayer = pSlayer->getPlayer();
                    pPlayer->sendPacket(&gcRemoveFromGear);

                    // Log
                    log(LOG_DESTROY_ITEM, pCreature->getName(), "", pWeapon->toString());

                    // Saves the reduced durability.
                    pWeapon->setDurability(Result);
                    pWeapon->save(pCreature->getName(), STORAGE_GEAR, 0, slot, 0);

                    // Deletes it from the database.
                    pWeapon->destroy();
                    SAFE_DELETE(pWeapon);
                } else {
                    if (pMI1 == NULL) {
                        return;
                    }

                    pWeapon->setDurability(Result);

                    value = (DWORD)(slot) << 24 | (DWORD)(Result);
                    pMI1->addLongData(MODIFY_DURABILITY, value);

                    // Saves the reduced durability.
                }
            } // if (pWeapon != NULL)
        } // if (pCreature->isSlayer())
        else if (pCreature->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);

            slot = Vampire::WEAR_RIGHTHAND;
            pWeapon = pVampire->getWearItem((Vampire::WearPart)slot);

            // Reduces it when a weapon is held.
            if (pWeapon != NULL && canDecreaseDurability(pWeapon)) {
                CurDur = pWeapon->getDurability();
                durDiff = Point;
                Result = max(0, CurDur - durDiff);

                if (Result == 0) // Destroys the weapon when its durability reaches 0.
                {
                    GCRemoveFromGear gcRemoveFromGear;
                    gcRemoveFromGear.setSlotID(slot);
                    pVampire->takeOffItem((Vampire::WearPart)slot, false, true);

                    Player* pPlayer = pVampire->getPlayer();
                    pPlayer->sendPacket(&gcRemoveFromGear);

                    // Log
                    log(LOG_DESTROY_ITEM, pCreature->getName(), "", pWeapon->toString());

                    // Saves the reduced durability.
                    pWeapon->setDurability(Result);
                    pWeapon->save(pCreature->getName(), STORAGE_GEAR, 0, slot, 0);

                    // Deletes it from the database.
                    pWeapon->destroy();
                    SAFE_DELETE(pWeapon);
                } else {
                    if (pMI1 == NULL) {
                        return;
                    }

                    pWeapon->setDurability(Result);

                    value = (DWORD)(slot) << 24 | (DWORD)(Result);
                    pMI1->addLongData(MODIFY_DURABILITY, value);
                }
            } // if (pWeapon != NULL)
        } // if (pCreature->isVampire())
        else if (pCreature->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);

            slot = Ousters::WEAR_RIGHTHAND;
            pWeapon = pOusters->getWearItem((Ousters::WearPart)slot);

            // Reduces it when a weapon is held.
            if (pWeapon != NULL && canDecreaseDurability(pWeapon)) {
                CurDur = pWeapon->getDurability();
                durDiff = Point;
                Result = max(0, CurDur - durDiff);

                if (Result == 0) // Destroys the weapon when its durability reaches 0.
                {
                    GCRemoveFromGear gcRemoveFromGear;
                    gcRemoveFromGear.setSlotID(slot);
                    pOusters->takeOffItem((Ousters::WearPart)slot, false, true);

                    Player* pPlayer = pOusters->getPlayer();
                    pPlayer->sendPacket(&gcRemoveFromGear);

                    // Log
                    log(LOG_DESTROY_ITEM, pCreature->getName(), "", pWeapon->toString());

                    // Saves the reduced durability.
                    pWeapon->setDurability(Result);
                    pWeapon->save(pCreature->getName(), STORAGE_GEAR, 0, slot, 0);

                    // Deletes it from the database.
                    pWeapon->destroy();
                    SAFE_DELETE(pWeapon);
                } else {
                    if (pMI1 == NULL) {
                        return;
                    }

                    pWeapon->setDurability(Result);

                    value = (DWORD)(slot) << 24 | (DWORD)(Result);
                    pMI1->addLongData(MODIFY_DURABILITY, value);
                }
            } // if (pWeapon != NULL)
        } // if (pCreature->isOusters())
    } // if (pCreature != NULL)

    ////////////////////////////////////////////////////////////////
    // Randomly reduces the durability of the armor of the creature under attack.
    ////////////////////////////////////////////////////////////////
    if (pTargetCreature != NULL) {
        // Decides which slot's gear has its durability reduced.
        if (pTargetCreature->isSlayer()) {
            slot = Random(0, Slayer::WEAR_MAX - 1);
            pGear = dynamic_cast<Slayer*>(pTargetCreature)->getWearItem((Slayer::WearPart)slot);
        } else if (pTargetCreature->isVampire()) {
            slot = Random(0, Vampire::VAMPIRE_WEAR_MAX - 1);
            pGear = dynamic_cast<Vampire*>(pTargetCreature)->getWearItem((Vampire::WearPart)slot);
        } else if (pTargetCreature->isOusters()) {
            slot = Random(0, Ousters::OUSTERS_WEAR_MAX - 1);
            pGear = dynamic_cast<Ousters*>(pTargetCreature)->getWearItem((Ousters::WearPart)slot);
        }

        // When an item is worn in the chosen slot.
        // A vampire amulet does not wear down.
        if (pGear != NULL && canDecreaseDurability(pGear))
        //			&& !pGear->isUnique()
        //			&& pGear->getItemClass()!=Item::ITEM_CLASS_VAMPIRE_AMULET)
        {
            // When the item in the chosen slot is a two-handed weapon,
            // the slot is forced to the right hand.
            if (isTwohandWeapon(pGear)) {
                if (pTargetCreature->isSlayer())
                    slot = Slayer::WEAR_RIGHTHAND;
                else if (pTargetCreature->isVampire())
                    slot = Vampire::WEAR_RIGHTHAND;
                else if (pTargetCreature->isOusters())
                    slot = Ousters::WEAR_RIGHTHAND;
            }

            CurDur = pGear->getDurability();
            durDiff = Point * Random(1, 3);
            Result = max(0, CurDur - durDiff);

            if (Result == 0) {
                GCRemoveFromGear gcRemoveFromGear;
                gcRemoveFromGear.setSlotID(slot);
                if (pTargetCreature->isSlayer()) {
                    Slayer* pTargetSlayer = dynamic_cast<Slayer*>(pTargetCreature);
                    Player* pPlayer = pTargetSlayer->getPlayer();

                    Assert(pTargetSlayer != NULL);
                    Assert(pPlayer != NULL);

                    pTargetSlayer->takeOffItem((Slayer::WearPart)slot, false, true);
                    pPlayer->sendPacket(&gcRemoveFromGear);
                } else if (pTargetCreature->isVampire()) {
                    Vampire* pTargetVampire = dynamic_cast<Vampire*>(pTargetCreature);
                    Player* pPlayer = pTargetVampire->getPlayer();

                    Assert(pTargetVampire != NULL);
                    Assert(pPlayer != NULL);

                    pTargetVampire->takeOffItem((Vampire::WearPart)slot, false, true);
                    pPlayer->sendPacket(&gcRemoveFromGear);
                } else if (pTargetCreature->isOusters()) {
                    Ousters* pTargetOusters = dynamic_cast<Ousters*>(pTargetCreature);
                    Player* pPlayer = pTargetOusters->getPlayer();

                    Assert(pTargetOusters != NULL);
                    Assert(pPlayer != NULL);

                    pTargetOusters->takeOffItem((Ousters::WearPart)slot, false, true);
                    pPlayer->sendPacket(&gcRemoveFromGear);
                }

                // Log
                log(LOG_DESTROY_ITEM, pTargetCreature->getName(), "", pGear->toString());

                // Destroy
                pGear->save(pTargetCreature->getName(), STORAGE_GEAR, 0, slot, 0);
                pGear->destroy();
                SAFE_DELETE(pGear);
            } else {
                pGear->setDurability(Result);

                value = (DWORD)(slot) << 24 | (DWORD)(Result);

                if (pMI2 == NULL) {
                    return;
                }

                pMI2->addLongData(MODIFY_DURABILITY, value);

                // Saves the reduced durability.
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// Is there any chance of hitting the target?
//////////////////////////////////////////////////////////////////////////////
bool canHit(Creature* pAttacker, Creature* pDefender, SkillType_t SkillType, SkillLevel_t SkillLevel) {
    // Invulnerable state

    // Checks the states that prevent a hit, regardless of the skill.
    if (pAttacker->isSlayer()) {
        // A race check could be done here, but it is left out to keep
        // the number of checks down for speed.

        // Fetches the attacker's Revealer effect.
        EffectRevealer* pEffectRevealer = NULL;
        if (pAttacker->isFlag(Effect::EFFECT_CLASS_REVEALER)) {
            pEffectRevealer = dynamic_cast<EffectRevealer*>(pAttacker->findEffect(Effect::EFFECT_CLASS_REVEALER));
            Assert(pEffectRevealer);
        }

        // A hidden defender is visible only with the Detect Hidden magic.
        if (pDefender->isFlag(Effect::EFFECT_CLASS_HIDE)) {
            if (!pAttacker->isFlag(Effect::EFFECT_CLASS_DETECT_HIDDEN) &&
                !(pEffectRevealer != NULL && pEffectRevealer->canSeeHide(pDefender)))
                return false;
        }
        // An invisible defender is visible only with the Detect Invisibility magic.
        if (pDefender->isFlag(Effect::EFFECT_CLASS_INVISIBILITY)) {
            if (!pAttacker->isFlag(Effect::EFFECT_CLASS_DETECT_INVISIBILITY) &&
                !(pEffectRevealer != NULL && pEffectRevealer->canSeeInvisibility(pDefender)))
                return false;
        }
    }

    // Checks whether the skill type can hit.
    // Basic attacks have no skill info, so they are checked here.
    switch (SkillType) {
    // A normal melee attack or a blood drain cannot reach a flying target.
    case SKILL_ATTACK_MELEE:
    case SKILL_BLOOD_DRAIN:
        if (pDefender != NULL) {
            if (pDefender->isFlag(Effect::EFFECT_CLASS_TRANSFORM_TO_BAT))
                return false;
        }
        return true;

    // A gun attack can reach a flying target as well.
    case SKILL_ATTACK_ARMS:
        return true;

    default:
        break;
    }

    // Returns whether the attack is possible for the skill type and the target's move mode.
    SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SkillType);
    Assert(pSkillInfo != NULL);

    uint TType = pSkillInfo->getTarget();

    if ((TType & TARGET_GROUND) && (pDefender->getMoveMode() == Creature::MOVE_MODE_WALKING)) {
        return true;
    } else if ((TType & TARGET_UNDERGROUND) && (pDefender->getMoveMode() == Creature::MOVE_MODE_BURROWING)) {
        return true;
    } else if ((TType & TARGET_AIR) && (pDefender->getMoveMode() == Creature::MOVE_MODE_FLYING)) {
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Computes the splash size an SG shot covers at the firing distance.
//////////////////////////////////////////////////////////////////////////////
int computeArmsWeaponSplashSize(Item* pWeapon, int ox, int oy, int tx, int ty) {
    Assert(pWeapon != NULL);
    Item::ItemClass IClass = pWeapon->getItemClass();
    int Splash = 0;

    // Only an SG has a splash effect.
    if (IClass == Item::ITEM_CLASS_SG) {
        switch (getDistance(ox, oy, tx, ty)) {
        case 1:
            Splash = 6;
            break;
        case 2:
            Splash = 5;
            break;
        case 3:
            Splash = 4;
            break;
        case 4:
            Splash = 3;
            break;
        case 5:
            Splash = 2;
            break;
        default:
            break;
        }
    }

    return Splash;
}

int computeArmsWeaponDamageBonus(Item* pWeapon, int ox, int oy, int tx, int ty) {
    Assert(pWeapon != NULL);

    Item::ItemClass IClass = pWeapon->getItemClass();
    int DamageBonus = 0;

    int range = getDistance(ox, oy, tx, ty);

    if (IClass == Item::ITEM_CLASS_SR) {
        DamageBonus = range + 3;
    }
    // by sigi. 2002.12.3
    else if (IClass == Item::ITEM_CLASS_AR || IClass == Item::ITEM_CLASS_SMG) {
        switch (range) {
        case 6:
            DamageBonus = 5;
            break;
        case 5:
            DamageBonus = 3;
            break;
        case 4:
            DamageBonus = 1;
            break;
        case 3:
            DamageBonus = 1;
            break;
        case 2:
            DamageBonus = 3;
            break;
        case 1:
            DamageBonus = 5;
            break;

        default:
            break;
        }
    } else if (IClass == Item::ITEM_CLASS_SG) {
        // by sigi. 2002.12.3
        DamageBonus = max(0, 6 - range);
    }

    return DamageBonus;
}

int computeArmsWeaponToHitBonus(Item* pWeapon, int ox, int oy, int tx, int ty) {
    Assert(pWeapon != NULL);

    Item::ItemClass IClass = pWeapon->getItemClass();
    int ToHitBonus = 0;

    if (IClass == Item::ITEM_CLASS_SR) {
        int range = getDistance(ox, oy, tx, ty);
        ToHitBonus = max(0, range - 1);
    } else if (IClass == Item::ITEM_CLASS_SG) {
        switch (getDistance(ox, oy, tx, ty)) {
        case 1:
            ToHitBonus = 20;
            break;
        case 2:
            ToHitBonus = 15;
            break;
        case 3:
            ToHitBonus = 10;
            break;
        case 4:
            ToHitBonus = 0;
            break;
        case 5:
            ToHitBonus = 0;
            break;
        default:
            break;
        }
    }

    return ToHitBonus;
}

// Reduces HP.
// by sigi. 2002.9.10
void decreaseHP(Zone* pZone, Creature* pCreature, int Damage, ObjectID_t attackerObjectID) {
    if (!(pZone->getZoneLevel() & COMPLETE_SAFE_ZONE)
        // Invincibility check.
        && !pCreature->isFlag(Effect::EFFECT_CLASS_NO_DAMAGE)) {
        if (pCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);

            HP_t CurrentHP = pSlayer->getHP(ATTR_CURRENT);

            if (CurrentHP > 0) {
                HP_t RemainHP = max(0, CurrentHP - (int)Damage);

                pSlayer->setHP(RemainHP, ATTR_CURRENT);

                GCModifyInformation gcMI;
                gcMI.addShortData(MODIFY_CURRENT_HP, RemainHP);
                pSlayer->getPlayer()->sendPacket(&gcMI);

                // Broadcasts the changed HP.
                GCStatusCurrentHP pkt;
                pkt.setObjectID(pSlayer->getObjectID());
                pkt.setCurrentHP(RemainHP);
                pZone->broadcastPacket(pSlayer->getX(), pSlayer->getY(), &pkt);
            }
        } else if (pCreature->isVampire()) {
            Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);

            HP_t CurrentHP = pVampire->getHP(ATTR_CURRENT);

            if (CurrentHP > 0) {
                HP_t RemainHP = max(0, CurrentHP - (int)Damage);

                pVampire->setHP(RemainHP, ATTR_CURRENT);

                GCModifyInformation gcMI;
                gcMI.addShortData(MODIFY_CURRENT_HP, RemainHP);
                pVampire->getPlayer()->sendPacket(&gcMI);

                // Broadcasts the changed HP.
                GCStatusCurrentHP pkt;
                pkt.setObjectID(pVampire->getObjectID());
                pkt.setCurrentHP(RemainHP);
                pZone->broadcastPacket(pVampire->getX(), pVampire->getY(), &pkt);
            }
        } else if (pCreature->isOusters()) {
            Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);

            HP_t CurrentHP = pOusters->getHP(ATTR_CURRENT);

            if (CurrentHP > 0) {
                HP_t RemainHP = max(0, CurrentHP - (int)Damage);

                pOusters->setHP(RemainHP, ATTR_CURRENT);

                GCModifyInformation gcMI;
                gcMI.addShortData(MODIFY_CURRENT_HP, RemainHP);
                pOusters->getPlayer()->sendPacket(&gcMI);

                // Broadcasts the changed HP.
                GCStatusCurrentHP pkt;
                pkt.setObjectID(pOusters->getObjectID());
                pkt.setCurrentHP(RemainHP);
                pZone->broadcastPacket(pOusters->getX(), pOusters->getY(), &pkt);
            }
        } else if (pCreature->isMonster()) {
            Monster* pMonster = dynamic_cast<Monster*>(pCreature);

            HP_t CurrentHP = pMonster->getHP(ATTR_CURRENT);

            if (CurrentHP > 0) {
                HP_t RemainHP = max(0, CurrentHP - (int)Damage);

                pMonster->setHP(RemainHP, ATTR_CURRENT);

                // Broadcasts the changed HP.
                GCStatusCurrentHP pkt;
                pkt.setObjectID(pMonster->getObjectID());
                pkt.setCurrentHP(RemainHP);
                pZone->broadcastPacket(pMonster->getX(), pMonster->getY(), &pkt);
            }
        }

        // Handles the kill count when attackerObjectID killed pCreature.
        // by sigi. 2002.9.9
        if (attackerObjectID != 0 && pCreature->isDead()) {
            Creature* pAttacker = pZone->getCreature(attackerObjectID);

            if (pAttacker != NULL) {
                affectKillCount(pAttacker, pCreature);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// Computes the pure damage of a Slayer attacker.
//////////////////////////////////////////////////////////////////////////////
Damage_t computePureSlayerDamage(Slayer* pSlayer) {
    Assert(pSlayer != NULL);

    Item* pItem = pSlayer->getWearItem(Slayer::WEAR_RIGHTHAND);

    // Start from the bare-handed damage.
    Damage_t MinDamage = pSlayer->getDamage(ATTR_CURRENT);
    Damage_t MaxDamage = pSlayer->getDamage(ATTR_MAX);

    // If a weapon is held, add the weapon's min and max to min and max.
    if (pItem != NULL && pSlayer->isRealWearingEx(Slayer::WEAR_RIGHTHAND)) {
        MinDamage += pItem->getMinDamage();
        MaxDamage += pItem->getMaxDamage();
    }

    // Computes the actual random damage.
    Damage_t RealDamage = max(1, Random(MinDamage, MaxDamage));

    return RealDamage;
}

//////////////////////////////////////////////////////////////////////////////
// Computes the pure damage of a Vampire attacker.
//////////////////////////////////////////////////////////////////////////////
Damage_t computePureVampireDamage(Vampire* pVampire) {
    Assert(pVampire != NULL);

    Damage_t MinDamage = pVampire->getDamage(ATTR_CURRENT);
    Damage_t MaxDamage = pVampire->getDamage(ATTR_MAX);
    uint timeband = getZoneTimeband(pVampire->getZone());

    // Damage from the Vampire's weapon
    Item* pItem = pVampire->getWearItem(Vampire::WEAR_RIGHTHAND);

    // If a weapon is held, add the weapon's min and max to min and max.
    if (pItem != NULL && pVampire->isRealWearingEx(Vampire::WEAR_RIGHTHAND)) {
        MinDamage += pItem->getMinDamage();
        MaxDamage += pItem->getMaxDamage();
    }

    // Computes the actual random damage.
    Damage_t RealDamage = max(1, Random(MinDamage, MaxDamage));

    RealDamage = (Damage_t)getPercentValue(RealDamage, VampireTimebandFactor[timeband]);

    return RealDamage;
}

//////////////////////////////////////////////////////////////////////////////
// Computes the pure damage of an Ousters attacker.
//////////////////////////////////////////////////////////////////////////////
Damage_t computePureOustersDamage(Ousters* pOusters) {
    Assert(pOusters != NULL);

    Damage_t MinDamage = pOusters->getDamage(ATTR_CURRENT);
    Damage_t MaxDamage = pOusters->getDamage(ATTR_MAX);

    // Damage from the Vampire's weapon
    Item* pItem = pOusters->getWearItem(Ousters::WEAR_RIGHTHAND);

    // If a weapon is held, add the weapon's min and max to min and max.
    if (pItem != NULL && pOusters->isRealWearingEx(Ousters::WEAR_RIGHTHAND)) {
        MinDamage += pItem->getMinDamage();
        MaxDamage += pItem->getMaxDamage();
    }

    // Computes the actual random damage.
    Damage_t RealDamage = max(1, Random(MinDamage, MaxDamage));

    return RealDamage;
}

//////////////////////////////////////////////////////////////////////////////
// Computes the pure damage of a monster attacker.
//////////////////////////////////////////////////////////////////////////////
Damage_t computePureMonsterDamage(Monster* pMonster) {
    Assert(pMonster != NULL);

    Damage_t MinDamage = pMonster->getDamage(ATTR_CURRENT);
    Damage_t MaxDamage = pMonster->getDamage(ATTR_MAX);
    Damage_t RealDamage = max(1, Random(MinDamage, MaxDamage));
    uint timeband = getZoneTimeband(pMonster->getZone());

    RealDamage = (Damage_t)getPercentValue(RealDamage, MonsterTimebandFactor[timeband]);

    return RealDamage;
}

Damage_t computeElementalCombatSkill(Ousters* pOusters, Creature* pTargetCreature, ModifyInfo& AttackerMI) {
    Assert(pOusters != NULL);
    Assert(pTargetCreature != NULL);

    Zone* pZone = pTargetCreature->getZone();
    Assert(pZone != NULL);

    Damage_t ret = 0;
    int ratio = pOusters->getPassiveRatio();
    bool bMaster = false;

    if (pTargetCreature->isMonster()) {
        Monster* pMonster = dynamic_cast<Monster*>(pTargetCreature);
        Assert(pTargetCreature != NULL);

        if (pMonster->isMaster())
            bMaster = true;
    }

    if (pOusters->isPassiveAvailable(SKILL_FIRE_OF_SOUL_STONE)) {
        if ((rand() % 100) < min(30, ratio)) {
            ret += pOusters->getPassiveBonus(SKILL_FIRE_OF_SOUL_STONE);

            GCAddEffect gcAddEffect;
            gcAddEffect.setObjectID(pTargetCreature->getObjectID());
            gcAddEffect.setEffectID(Effect::EFFECT_CLASS_FIRE_OF_SOUL_STONE);
            gcAddEffect.setDuration(0);

            pZone->broadcastPacket(pTargetCreature->getX(), pTargetCreature->getY(), &gcAddEffect);
        }
    }

    if (pOusters->isPassiveAvailable(SKILL_ICE_OF_SOUL_STONE)) {
        if (!bMaster && !pTargetCreature->isFlag(Effect::EFFECT_CLASS_ICE_OF_SOUL_STONE) &&
            (rand() % 100) < min(23, ratio * 2 / 3)) {
            Turn_t duration = pOusters->getPassiveBonus(SKILL_ICE_OF_SOUL_STONE);
            // Creates the effect class and attaches it.
            EffectIceOfSoulStone* pEffect = new EffectIceOfSoulStone(pTargetCreature);
            pEffect->setDeadline(duration);
            pTargetCreature->addEffect(pEffect);
            pTargetCreature->setFlag(Effect::EFFECT_CLASS_ICE_OF_SOUL_STONE);

            GCAddEffect gcAddEffect;
            gcAddEffect.setObjectID(pTargetCreature->getObjectID());
            gcAddEffect.setEffectID(Effect::EFFECT_CLASS_ICE_OF_SOUL_STONE);
            gcAddEffect.setDuration(duration);

            pZone->broadcastPacket(pTargetCreature->getX(), pTargetCreature->getY(), &gcAddEffect);
        }
    }

    if (pOusters->isPassiveAvailable(SKILL_SAND_OF_SOUL_STONE)) {
        if ((rand() % 100) < min(30, ratio)) {
            GCAddEffectToTile gcAddEffect;
            gcAddEffect.setObjectID(pTargetCreature->getObjectID());
            gcAddEffect.setEffectID(Effect::EFFECT_CLASS_SAND_OF_SOUL_STONE);
            gcAddEffect.setDuration(7);
            gcAddEffect.setXY(pTargetCreature->getX(), pTargetCreature->getY());

            pZone->broadcastPacket(pTargetCreature->getX(), pTargetCreature->getY(), &gcAddEffect);

            Damage_t bonusDamage = pOusters->getPassiveBonus(SKILL_SAND_OF_SOUL_STONE);
            VSRect rect(0, 0, pZone->getWidth() - 1, pZone->getHeight() - 1);

            GCSkillToObjectOK6 gcOK6;
            gcOK6.setXY(pOusters->getX(), pOusters->getY());
            gcOK6.setSkillType(SKILL_ATTACK_MELEE);
            gcOK6.setDuration(0);

            for (int i = pTargetCreature->getX() - 1; i <= pTargetCreature->getX() + 1; ++i)
                for (int j = pTargetCreature->getY() - 1; j <= pTargetCreature->getY() + 1; ++j) {
                    if (!rect.ptInRect(i, j))
                        continue;

                    Tile& rTile = pZone->getTile(i, j);
                    if (!rTile.hasCreature(Creature::MOVE_MODE_WALKING))
                        continue;
                    Creature* pTileCreature = rTile.getCreature(Creature::MOVE_MODE_WALKING);

                    if (pTileCreature == NULL || pTileCreature->getObjectID() == pTargetCreature->getObjectID() ||
                        pTileCreature->isOusters() || pTileCreature->isNPC())
                        continue;

                    GCSkillToObjectOK4 gcOK4;
                    gcOK4.setTargetObjectID(pTileCreature->getObjectID());
                    gcOK4.setSkillType(SKILL_ATTACK_MELEE);
                    gcOK4.setDuration(0);

                    if (pTileCreature->isPC()) {
                        gcOK6.clearList();
                        setDamage(pTileCreature, bonusDamage, pOusters, SKILL_SAND_OF_SOUL_STONE, &gcOK6, &AttackerMI);
                        pTileCreature->getPlayer()->sendPacket(&gcOK6);
                    } else if (pTileCreature->isMonster()) {
                        setDamage(pTileCreature, bonusDamage, pOusters, SKILL_SAND_OF_SOUL_STONE, NULL, &AttackerMI);
                    }

                    pZone->broadcastPacket(pTileCreature->getX(), pTileCreature->getY(), &gcOK4);

                    if (pTileCreature->isDead()) {
                        int exp = computeCreatureExp(pTileCreature, 100, pOusters);
                        shareOustersExp(pOusters, exp, AttackerMI);
                        increaseAlignment(pOusters, pTileCreature, AttackerMI);
                    }
                }

            ret += bonusDamage;
        }
    }

    if (pOusters->isPassiveAvailable(SKILL_BLOCK_HEAD)) {
        if (!bMaster && !pTargetCreature->isFlag(Effect::EFFECT_CLASS_BLOCK_HEAD) &&
            (rand() % 100) < min(15, ratio / 2)) {
            Turn_t duration = pOusters->getPassiveBonus(SKILL_BLOCK_HEAD);
            // Creates the effect class and attaches it.
            EffectBlockHead* pEffect = new EffectBlockHead(pTargetCreature);
            pEffect->setDeadline(duration);
            pTargetCreature->addEffect(pEffect);
            pTargetCreature->setFlag(Effect::EFFECT_CLASS_BLOCK_HEAD);

            GCAddEffect gcAddEffect;
            gcAddEffect.setObjectID(pTargetCreature->getObjectID());
            gcAddEffect.setEffectID(Effect::EFFECT_CLASS_BLOCK_HEAD);
            gcAddEffect.setDuration(duration);

            pZone->broadcastPacket(pTargetCreature->getX(), pTargetCreature->getY(), &gcAddEffect);
        }
    }

    if (pOusters->isPassiveAvailable(SKILL_BLESS_FIRE)) {
        if ((rand() % 100) < min(30, ratio)) {
            ret += pOusters->getPassiveBonus(SKILL_BLESS_FIRE);

            GCAddEffectToTile gcAddEffect;
            gcAddEffect.setObjectID(pTargetCreature->getObjectID());
            gcAddEffect.setXY(pTargetCreature->getX(), pTargetCreature->getY());
            gcAddEffect.setEffectID(Effect::EFFECT_CLASS_BLESS_FIRE);
            gcAddEffect.setDuration(0);

            pZone->broadcastPacket(pTargetCreature->getX(), pTargetCreature->getY(), &gcAddEffect);
        }
    }

    if (pOusters->isPassiveAvailable(SKILL_SAND_CROSS)) {
        if ((rand() % 100) < min(30, ratio)) {
            GCAddEffectToTile gcAddEffect;
            gcAddEffect.setObjectID(pTargetCreature->getObjectID());
            gcAddEffect.setEffectID(Effect::EFFECT_CLASS_SAND_CROSS);
            gcAddEffect.setDuration(0);
            gcAddEffect.setXY(pTargetCreature->getX(), pTargetCreature->getY());

            pZone->broadcastPacket(pTargetCreature->getX(), pTargetCreature->getY(), &gcAddEffect);

            Damage_t bonusDamage = pOusters->getPassiveBonus(SKILL_SAND_CROSS);
            VSRect rect(0, 0, pZone->getWidth() - 1, pZone->getHeight() - 1);

            GCSkillToObjectOK6 gcOK6;
            gcOK6.setXY(pOusters->getX(), pOusters->getY());
            gcOK6.setSkillType(SKILL_ATTACK_MELEE);
            gcOK6.setDuration(0);

            for (int i = pTargetCreature->getX() - 3; i <= pTargetCreature->getX() + 3; ++i) {
                int j = pTargetCreature->getY();

                if (!rect.ptInRect(i, j))
                    continue;

                Tile& rTile = pZone->getTile(i, j);
                if (!rTile.hasCreature(Creature::MOVE_MODE_WALKING))
                    continue;
                Creature* pTileCreature = rTile.getCreature(Creature::MOVE_MODE_WALKING);

                if (pTileCreature == NULL || pTileCreature->getObjectID() == pTargetCreature->getObjectID() ||
                    pTileCreature->isOusters() || pTileCreature->isNPC())
                    continue;

                GCSkillToObjectOK4 gcOK4;
                gcOK4.setTargetObjectID(pTileCreature->getObjectID());
                gcOK4.setSkillType(SKILL_ATTACK_MELEE);
                gcOK4.setDuration(0);

                if (pTileCreature->isPC()) {
                    gcOK6.clearList();
                    setDamage(pTileCreature, bonusDamage, pOusters, SKILL_SAND_CROSS, &gcOK6, &AttackerMI);
                    pTileCreature->getPlayer()->sendPacket(&gcOK6);
                } else if (pTileCreature->isMonster()) {
                    setDamage(pTileCreature, bonusDamage, pOusters, SKILL_SAND_CROSS, NULL, &AttackerMI);
                }

                pZone->broadcastPacket(pTileCreature->getX(), pTileCreature->getY(), &gcOK4);

                if (pTileCreature->isDead()) {
                    int exp = computeCreatureExp(pTileCreature, 100, pOusters);
                    shareOustersExp(pOusters, exp, AttackerMI);
                    increaseAlignment(pOusters, pTileCreature, AttackerMI);
                }
            }

            for (int j = pTargetCreature->getY() - 3; j <= pTargetCreature->getY() + 3; ++j) {
                int i = pTargetCreature->getX();

                if (!rect.ptInRect(i, j))
                    continue;

                Tile& rTile = pZone->getTile(i, j);
                if (!rTile.hasCreature(Creature::MOVE_MODE_WALKING))
                    continue;
                Creature* pTileCreature = rTile.getCreature(Creature::MOVE_MODE_WALKING);

                if (pTileCreature == NULL || pTileCreature->getObjectID() == pTargetCreature->getObjectID() ||
                    pTileCreature->isOusters() || pTileCreature->isNPC())
                    continue;

                GCSkillToObjectOK4 gcOK4;
                gcOK4.setTargetObjectID(pTileCreature->getObjectID());
                gcOK4.setSkillType(SKILL_ATTACK_MELEE);
                gcOK4.setDuration(0);

                if (pTileCreature->isPC()) {
                    gcOK6.clearList();
                    setDamage(pTileCreature, bonusDamage, pOusters, SKILL_SAND_CROSS, &gcOK6, &AttackerMI);
                    pTileCreature->getPlayer()->sendPacket(&gcOK6);
                } else if (pTileCreature->isMonster()) {
                    setDamage(pTileCreature, bonusDamage, pOusters, SKILL_SAND_CROSS, NULL, &AttackerMI);
                }

                pZone->broadcastPacket(pTileCreature->getX(), pTileCreature->getY(), &gcOK4);

                if (pTileCreature->isDead()) {
                    int exp = computeCreatureExp(pTileCreature, 100, pOusters);
                    shareOustersExp(pOusters, exp, AttackerMI);
                    increaseAlignment(pOusters, pTileCreature, AttackerMI);
                }
            }

            ret += bonusDamage;
        }
    }

    return ret;
}
