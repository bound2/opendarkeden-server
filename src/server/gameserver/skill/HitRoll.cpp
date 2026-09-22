//////////////////////////////////////////////////////////////////////////////
// Filename    : HitRoll.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "HitRoll.h"

#include "Monster.h"
#include "Ousters.h"
#include "RankBonus.h"
#include "Skill.h"
#include "SkillInfo.h"
#include "Slayer.h"
#include "Vampire.h"
#include "Zone.h"
#include "domain/Formulas.h"
// #include <math.h>

#include "EffectHymn.h"
#include "EffectPrecedence.h"

//////////////////////////////////////////////////////////////////////////////
// Hit roll for a normal attack
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccess(Creature* pAttacker, Creature* pDefender, int ToHitBonus) {
    Assert(pAttacker != NULL);
    Assert(pDefender != NULL);

    Zone* pZone = pAttacker->getZone();
    Assert(pZone != NULL);

    // Checks for the invulnerable state.
    if (pDefender->isFlag(Effect::EFFECT_CLASS_NO_DAMAGE)) {
        return false;
    }


    ToHit_t ToHit = 0;
    Defense_t Defense = 0;
    uint timeband = pZone->getTimeband();

    bool isMonster = false;

    // Compute the attacker's to-hit.
    if (pAttacker->isSlayer()) {
        Slayer* pSlayerAttacker = dynamic_cast<Slayer*>(pAttacker);
        Assert(pSlayerAttacker != NULL);

        ToHit = pSlayerAttacker->getToHit();

    } else if (pAttacker->isVampire()) {
        Vampire* pVampireAttacker = dynamic_cast<Vampire*>(pAttacker);
        Assert(pVampireAttacker != NULL);

        ToHit = pVampireAttacker->getToHit();
        ToHit = (ToHit_t)getPercentValue(ToHit, VampireTimebandFactor[timeband]);

        // If this is optimized some day,
        // it would be better to put the penalty-related members into Creature.
        if (pAttacker->isFlag(Effect::EFFECT_CLASS_HYMN)) {
            EffectHymn* pHymn =
                dynamic_cast<EffectHymn*>(pAttacker->getEffectManager()->findEffect(Effect::EFFECT_CLASS_HYMN));

            ToHit = ToHit * (100 - pHymn->getToHitPenalty()) / 100;
        }
    } else if (pAttacker->isOusters()) {
        Ousters* pOustersAttacker = dynamic_cast<Ousters*>(pAttacker);
        Assert(pOustersAttacker != NULL);

        ToHit = pOustersAttacker->getToHit();
    } else if (pAttacker->isMonster()) {
        Monster* pMonsterAttacker = dynamic_cast<Monster*>(pAttacker);
        Assert(pMonsterAttacker != NULL);

        ToHit = pMonsterAttacker->getToHit();
        ToHit = (ToHit_t)getPercentValue(ToHit, MonsterTimebandFactor[timeband]);
        isMonster = true;

        // If this is optimized some day,
        // it would be better to put the penalty-related members into Creature.
        if (pAttacker->isFlag(Effect::EFFECT_CLASS_HYMN)) {
            EffectHymn* pHymn =
                dynamic_cast<EffectHymn*>(pAttacker->getEffectManager()->findEffect(Effect::EFFECT_CLASS_HYMN));

            ToHit = ToHit * (100 - pHymn->getToHitPenalty()) / 100;
        }

    } else {
        // The creature classes are Slayer, Vampire, Ousters, Monster and NPC,
        // so reaching here means the attacker is an NPC.
        // NPC AI is not implemented, so this always returns.
        return false;
    }

    // Compute the defender's defense.
    if (pDefender->isSlayer()) {
        Slayer* pSlayerDefender = dynamic_cast<Slayer*>(pDefender);
        Defense = pSlayerDefender->getDefense();

    } else if (pDefender->isVampire()) {
        Vampire* pVampireDefender = dynamic_cast<Vampire*>(pDefender);
        Defense = pVampireDefender->getDefense();
        Defense = (Defense_t)getPercentValue(Defense, VampireTimebandFactor[timeband]);
    } else if (pDefender->isOusters()) {
        Ousters* pOustersDefender = dynamic_cast<Ousters*>(pDefender);
        Defense = pOustersDefender->getDefense();
    } else if (pDefender->isMonster()) {
        Monster* pMonsterDefender = dynamic_cast<Monster*>(pDefender);
        Defense = pMonsterDefender->getDefense();
        Defense = (Defense_t)getPercentValue(Defense, MonsterTimebandFactor[timeband]);
        isMonster = true;
    } else {
        // The creature classes are Slayer, Vampire, Ousters, Monster and NPC,
        // so reaching here means the defender is an NPC.
        // NPC AI is not implemented, so this always returns.
        return false;
    }

    int RandValue = Random(0, 100);
    int Result = 0;

    Result = decore::meleeHitRatio(ToHit, Defense, ToHitBonus, isMonster);

    if (RandValue <= Result)
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Hit roll for a normal attack
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Hit roll for Slayer magic
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessMagic(Slayer* pSlayer, SkillInfo* pSkillInfo, SkillSlot* pSkillSlot) {
    Assert(pSlayer != NULL);
    Assert(pSkillInfo != NULL);
    Assert(pSkillSlot != NULL);

    int RandValue = Random(1, 100);
    int SuccessRatio = decore::slayerMagicRatio(pSkillInfo->getLevel(), pSlayer->getINT(), pSkillSlot->getExpLevel(),
                                                isSlayerSelfSkill(pSkillSlot->getSkillType()));

    if (RandValue < SuccessRatio)
        return true;

    return false;
}


//////////////////////////////////////////////////////////////////////////////
// Hit roll for Vampire magic
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessMagic(Vampire* pVampire, SkillInfo* pSkillInfo, VampireSkillSlot* pVampireSkillSlot,
                             int BonusPoint) {
    Assert(pVampire != NULL);
    Assert(pSkillInfo != NULL);
    Assert(pVampireSkillSlot != NULL);

    int RandValue = Random(1, 100);
    int Success =
        decore::vampireMagicRatio(pSkillInfo->getLevel(), pVampire->getINT(), pVampire->getLevel(), BonusPoint);

    if (RandValue < Success)
        return true;


    return false;
}


//////////////////////////////////////////////////////////////////////////////
// Hit roll for Ousters magic
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessMagic(Ousters* pOusters, SkillInfo* pSkillInfo, OustersSkillSlot* pOustersSkillSlot,
                             int BonusPoint) {
    Assert(pOusters != NULL);
    Assert(pSkillInfo != NULL);
    Assert(pOustersSkillSlot != NULL);

    int RandValue = Random(1, 100);
    int Success = decore::oustersMagicRatio(pOusters->getINT(), pOusters->getLevel(), pOustersSkillSlot->getExpLevel(),
                                            isOustersSelfSkill(pOustersSkillSlot->getSkillType()), BonusPoint);

    if (RandValue < Success)
        return true;

    return false;
}


//////////////////////////////////////////////////////////////////////////////
// Hit roll for normal monster magic
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessMagic(Monster* pMonster, SkillInfo* pSkillInfo) {
    Assert(pMonster != NULL);
    Assert(pSkillInfo != NULL);

    int ratio = decore::monsterMagicRatio(pSkillInfo->getLevel(), pMonster->getINT(), pMonster->getLevel());
    if (rand() % 100 < ratio)
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Blood drain hit roll for Vampires and monsters
//////////////////////////////////////////////////////////////////////////////
// multiplier is normally 3, so a target at 33% HP or below can be drained.
// For a master it is 2, so 50% or below is the drainable threshold.
// by sigi. 2002.9.16
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessBloodDrain(Creature* pAttacker, Creature* pDefender, int multiplier) {
    Assert(pAttacker != NULL);
    Assert(pDefender != NULL);

    Zone* pZone = pAttacker->getZone();
    Assert(pZone != NULL);

    // Checks for the invulnerable state.
    if (pDefender->isFlag(Effect::EFFECT_CLASS_NO_DAMAGE)) {
        return false;
    }


    const int normalMultiplier = 3; // Drop this once a master flag is passed in as a parameter.
    bool bHPCheck = false;
    bool bEffected = false;
    uint timeband = pZone->getTimeband();

    int ToHit = 0;
    int Defense = 0;
    int OtherLevel = 0;
    int ratio = 0;

    // Works out whether the target can be drained,
    // what its defense is and what its level is.
    if (pDefender->isSlayer()) {
        Slayer* pTargetSlayer = dynamic_cast<Slayer*>(pDefender);
        int MaxHP = pTargetSlayer->getHP(ATTR_MAX);
        int CurHP = pTargetSlayer->getHP(ATTR_CURRENT);

        bHPCheck = decore::bloodDrainHPGate(CurHP, MaxHP, multiplier);
        bEffected = (normalMultiplier == multiplier) && pTargetSlayer->isFlag(Effect::EFFECT_CLASS_BLOOD_DRAIN);
        Defense = pTargetSlayer->getDefense();

        OtherLevel = pTargetSlayer->getSTR(ATTR_CURRENT) + pTargetSlayer->getDEX(ATTR_CURRENT) +
                     pTargetSlayer->getINT(ATTR_CURRENT) + pTargetSlayer->getSkillDomainLevel(SKILL_DOMAIN_BLADE) +
                     pTargetSlayer->getSkillDomainLevel(SKILL_DOMAIN_SWORD) +
                     pTargetSlayer->getSkillDomainLevel(SKILL_DOMAIN_GUN) +
                     pTargetSlayer->getSkillDomainLevel(SKILL_DOMAIN_HEAL) +
                     pTargetSlayer->getSkillDomainLevel(SKILL_DOMAIN_ENCHANT);

        OtherLevel = (int)(OtherLevel / 350);

        // Blood cannot be drained if HP is still above 1/3
        // or the target has already been drained.
        if (!bHPCheck || bEffected)
            return false;
    } else if (pDefender->isVampire()) {
        Vampire* pTargetVampire = dynamic_cast<Vampire*>(pDefender);
        int MaxHP = pTargetVampire->getHP(ATTR_MAX);
        int CurHP = pTargetVampire->getHP(ATTR_CURRENT);

        bHPCheck = decore::bloodDrainHPGate(CurHP, MaxHP, multiplier);
        bEffected = (normalMultiplier == multiplier) && pTargetVampire->isFlag(Effect::EFFECT_CLASS_BLOOD_DRAIN);
        Defense = pTargetVampire->getDefense() + pTargetVampire->getLevel() / 5;
        OtherLevel = pTargetVampire->getLevel();

        Defense = (Defense_t)getPercentValue(Defense, VampireTimebandFactor[timeband]);

        // Blood cannot be drained if HP is still above 1/3
        // or the target has already been drained.
        if (!bHPCheck || bEffected)
            return false;
    } else if (pDefender->isOusters()) {
        Ousters* pTargetOusters = dynamic_cast<Ousters*>(pDefender);
        int MaxHP = pTargetOusters->getHP(ATTR_MAX);
        int CurHP = pTargetOusters->getHP(ATTR_CURRENT);

        bHPCheck = decore::bloodDrainHPGate(CurHP, MaxHP, multiplier);
        bEffected = (normalMultiplier == multiplier) && pTargetOusters->isFlag(Effect::EFFECT_CLASS_BLOOD_DRAIN);
        Defense = pTargetOusters->getDefense() + pTargetOusters->getLevel() / 5;
        OtherLevel = pTargetOusters->getLevel();

        // Blood cannot be drained if HP is still above 1/3
        // or the target has already been drained.
        if (!bHPCheck || bEffected)
            return false;
    } else if (pDefender->isMonster()) {
        Monster* pTargetMonster = dynamic_cast<Monster*>(pDefender);
        int MaxHP = pTargetMonster->getHP(ATTR_MAX);
        int CurHP = pTargetMonster->getHP(ATTR_CURRENT);

        bHPCheck = decore::bloodDrainHPGate(CurHP, MaxHP, multiplier);
        bEffected = (normalMultiplier == multiplier) && pTargetMonster->isFlag(Effect::EFFECT_CLASS_BLOOD_DRAIN);
        Defense = pTargetMonster->getDefense() + pTargetMonster->getLevel() / 5;
        OtherLevel = pTargetMonster->getLevel();

        Defense = (Defense_t)getPercentValue(Defense, MonsterTimebandFactor[timeband]);

        // Blood cannot be drained if HP is still above 1/3
        // or the target has already been drained.
        if (!bHPCheck || bEffected)
            return false;

        // If the monster already has precedence set,
        // draining requires holding precedence or belonging to the party that holds it.
        if (pTargetMonster->isFlag(Effect::EFFECT_CLASS_PRECEDENCE)) {
            EffectPrecedence* pEffectPrecedence =
                dynamic_cast<EffectPrecedence*>(pTargetMonster->findEffect(Effect::EFFECT_CLASS_PRECEDENCE));
            Assert(pEffectPrecedence != NULL);

            // If this is not the holder of precedence,
            if (pAttacker->getName() != pEffectPrecedence->getHostName()) {
                // draining fails when the attacker is in no party at all,
                // or is in a party that is not the host party.
                if (pAttacker->getPartyID() == 0 || pAttacker->getPartyID() != pEffectPrecedence->getHostPartyID()) {
                    return false;
                }
            }
        }
    } else
        Assert(false);

    // Compute the attacker's to-hit and level.
    if (pAttacker->isVampire()) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pAttacker);

        ToHit = pVampire->getToHit();

        ToHit = (ToHit_t)getPercentValue(ToHit, VampireTimebandFactor[timeband]);
    } else if (pAttacker->isMonster()) {
        Monster* pMonster = dynamic_cast<Monster*>(pAttacker);

        ToHit = pMonster->getToHit();

        ToHit = (ToHit_t)getPercentValue(ToHit, MonsterTimebandFactor[timeband]);
    }


    ratio = decore::bloodDrainHitRatio(ToHit, Defense);

    if ((rand() % 100) < ratio) {
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Hit roll for curse magic
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessCurse(int MagicLevel, Resist_t resist) {
    // MagicLevel is the level a Vampire needs to learn that magic.
    // So the higher the level of the curse magic, the higher MagicLevel is.
    //
    // With MagicLevel 30 and resistance 20,
    // curse_prob = 95 and the curse lands 95 times in 100.
    // With MagicLevel 30 and resistance 100,
    // curse_prob = 15 and the magic fails 85% of the time.
    int curse_prob = decore::curseRatio(MagicLevel, resist);
    int randomValue = rand() % 100;

    // The curse lands.
    if (randomValue < curse_prob)
        return true;

    // Resistance kept it from landing.
    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Hit roll for curse magic - when a Vampire casts the curse
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessVampireCurse(int MagicLevel, Resist_t resist) {
    // MagicLevel is half the level a Vampire needs to learn that magic.
    // So the higher the level of the curse magic, the higher MagicLevel is.
    //
    // With MagicLevel 30 and resistance 20,
    // curse_prob = 75 and the curse lands three times in four.
    // With MagicLevel 30 and resistance 100,
    // curse_prob is floored at 5 and the magic fails 95% of the time.
    int curse_prob = decore::vampireCurseRatio(MagicLevel, resist);

    int randomValue = rand() % 100;

    // The curse lands.
    if (randomValue < curse_prob)
        return true;

    // Resistance kept it from landing.
    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Hit roll for CurePoison
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessCurePoison(int Base, int SkillLevel, int Difficulty, int MagicLevel, int MinRatio) {
    // Minimum ratio added by Sequoia 2003. 3. 20
    int ratio = decore::dispelRatio(Base, SkillLevel, Difficulty, MagicLevel, MinRatio);

    if (rand() % 100 < ratio)
        return true;
    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Hit roll for Flare
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessFlare(Creature* pTargetCreature, int SkillLevel) {
    Assert(pTargetCreature != NULL);

    // Checks for the invulnerable state.
    if (pTargetCreature->isFlag(Effect::EFFECT_CLASS_NO_DAMAGE))
        return false;

    int ratio = 0;

    if (pTargetCreature->isPC()) {
        ratio = decore::flareRatio(SkillLevel, pTargetCreature->getLevel());
    } else if (pTargetCreature->isMonster()) {
        Monster* pMonster = dynamic_cast<Monster*>(pTargetCreature);

        //
        // by sigi. 2002.10.30
        if (pMonster->isMaster()) {
            return false;
        }

        ratio = decore::flareRatio(SkillLevel, pMonster->getLevel());
    }

    if (rand() % 100 < ratio)
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Hit roll for RemoveCurse
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessRemoveCurse(int Base, int SkillLevel, int Difficulty, int MagicLevel, int MinRatio /* = 0 */) {
    // Minimum ratio added by Sequoia 2003. 3. 20
    int ratio = decore::dispelRatio(Base, SkillLevel, Difficulty, MagicLevel, MinRatio);

    if (rand() % 100 < ratio)
        return true;
    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Hit roll for Rebuke
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessRebuke(Slayer* pSlayer, SkillSlot* pSkillSlot, Creature* pDefender) {
    if (pDefender->isSlayer())
        return false;

    // A Vampire or an Ousters of level 80 or above does not fall asleep.
    if (pDefender->isVampire()) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pDefender);
        if (pVampire->getLevel() >= 80)
            return false;
    } else if (pDefender->isOusters()) {
        Ousters* pOusters = dynamic_cast<Ousters*>(pDefender);
        if (pOusters->getLevel() >= 80)
            return false;
    }

    // Masters (Bathory, Tepes) do not fall asleep.
    if (pDefender->isMonster()) {
        Monster* pMonster = dynamic_cast<Monster*>(pDefender);
        if (pMonster->isMaster())
            return false;
    }

    Attr_t INTE = pSlayer->getINT(ATTR_CURRENT);
    SkillLevel_t SkillLevel = pSkillSlot->getExpLevel();

    int ratio = decore::rebukeRatio(INTE, SkillLevel);
    if (rand() % 100 < ratio)
        return true;
    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Hit roll for Magic Elusion
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessMagicElusion(Slayer* pSlayer) {
    Attr_t SUM = pSlayer->getTotalAttr(ATTR_CURRENT);
    int Ratio = decore::totalAttrDefenseRatio(SUM);

    return (rand() % 100) < Ratio;
}

//////////////////////////////////////////////////////////////////////////////
// Hit roll for Poison Mesh
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessPoisonMesh(Vampire* pVampire) {
    int Ratio = decore::poisonMeshRatio(pVampire->getLevel());

    return (rand() % 100) < Ratio;
}

//////////////////////////////////////////////////////////////////////////////
// Hit roll for Illusion Of Avenge
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessIllusionOfAvenge(Slayer* pSlayer) {
    Attr_t SUM = pSlayer->getTotalAttr(ATTR_CURRENT);
    int Ratio = decore::totalAttrDefenseRatio(SUM);

    return (rand() % 100) < Ratio;
}

//////////////////////////////////////////////////////////////////////////////
// Hit roll for Will Of Life
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSuccessWillOfLife(Vampire* pVampire) {
    int Ratio = decore::willOfLifeRatio(pVampire->getLevel());

    return (rand() % 100) < Ratio;
}


//////////////////////////////////////////////////////////////////////////////
// Performs the critical hit roll.
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isCriticalHit(Creature* pCreature, int CriticalBonus) {
    Assert(pCreature != NULL);

    int CriticalRatio = pCreature->getCriticalRatio() + CriticalBonus;

    if (rand() % 1000 < CriticalRatio)
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Checks whether the skill is a Slayer self skill
//////////////////////////////////////////////////////////////////////////////
bool HitRoll::isSlayerSelfSkill(SkillType_t skillType) {
    switch (skillType) {
    case SKILL_DANCING_SWORD:
    case SKILL_CROSS_COUNTER:
    case SKILL_EXPANSION:
    case SKILL_SHARP_SHIELD:

    case SKILL_GHOST_BLADE:
    case SKILL_POTENTIAL_EXPLOSION:
    case SKILL_CHARGING_POWER:
    case SKILL_BERSERKER:
    case SKILL_AIR_SHIELD:

    case SKILL_SNIPING:
    case SKILL_OBSERVING_EYE:

    case SKILL_CREATE_HOLY_WATER:
    case SKILL_LIGHT:
    case SKILL_DETECT_HIDDEN:
    case SKILL_BLESS:
    case SKILL_STRIKING:
    case SKILL_DETECT_INVISIBILITY:
    case SKILL_AURA_SHIELD:
    case SKILL_REFLECTION:
    case SKILL_REBUKE:
    case SKILL_SPIRIT_GUARD:

    case SKILL_CURE_LIGHT_WOUNDS:
    case SKILL_CURE_POISON:
    case SKILL_PROTECTION_FROM_POISON:
    case SKILL_CURE_SERIOUS_WOUNDS:
    case SKILL_REMOVE_CURSE:
    case SKILL_PROTECTION_FROM_CURSE:
    case SKILL_CURE_CRITICAL_WOUNDS:
    case SKILL_PROTECTION_FROM_ACID:
    case SKILL_CURE_ALL:
    case SKILL_PEACE:
    case SKILL_ACTIVATION:
    case SKILL_REGENERATION:
    case SKILL_TURN_UNDEAD:
        return true;

    default:
        return false;
    }
}

bool HitRoll::isOustersSelfSkill(SkillType_t skillType) {
    switch (skillType) {
    case SKILL_EVADE:
    case SKILL_BLUNTING:
    case SKILL_CROSS_GUARD:
    case SKILL_SHARP_CHAKRAM:
        return true;

    default:
        return false;
    }
}
bool HitRoll::isSuccessHallucination(Vampire* pAttacker, Creature* pTarget) {
    if (pTarget->isMonster())
        return true;

    Attr_t attackTotalAttr = pAttacker->getSTR() + pAttacker->getDEX() + pAttacker->getINT();
    Attr_t targetTotalAttr = 0;
    int minRatio = 0, maxRatio = 100;

    if (pTarget->isSlayer()) {
        Slayer* pTargetSlayer = dynamic_cast<Slayer*>(pTarget);
        targetTotalAttr = pTargetSlayer->getTotalAttr();
        minRatio = 30;
        maxRatio = 60;
    } else if (pTarget->isVampire()) {
        Vampire* pTargetVampire = dynamic_cast<Vampire*>(pTarget);
        targetTotalAttr = pTargetVampire->getSTR() + pTargetVampire->getDEX() + pTargetVampire->getINT();
        minRatio = 10;
        maxRatio = 40;
    } else if (pTarget->isOusters()) {
        Ousters* pTargetOusters = dynamic_cast<Ousters*>(pTarget);
        targetTotalAttr = pTargetOusters->getSTR() + pTargetOusters->getDEX() + pTargetOusters->getINT();
        minRatio = 10;
        maxRatio = 50;

    } else
        return false;

    int Ratio = decore::hallucinationRatio(attackTotalAttr, targetTotalAttr, minRatio, maxRatio);

    return (rand() % 100) < Ratio;
}

bool HitRoll::isSuccessBackStab(Ousters* pAttacker) {
    int Ratio = decore::backStabRatio(pAttacker->getINT(), pAttacker->getDEX());
    if (rand() % 100 < Ratio)
        return true;
    return false;
}
