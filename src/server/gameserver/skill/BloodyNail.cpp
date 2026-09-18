//////////////////////////////////////////////////////////////////////////////
// Filename    : BloodyNail.cpp
// Written by  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "BloodyNail.h"

#include "RankBonus.h"
#include "SimpleMeleeSkill.h"

//////////////////////////////////////////////////////////////////////////////
// Vampire object handler
//////////////////////////////////////////////////////////////////////////////
void BloodyNail::execute(Vampire* pVampire, ObjectID_t TargetObjectID, VampireSkillSlot* pVampireSkillSlot,
                         CEffectID_t CEffectID)

{
    __BEGIN_TRY


    SkillInput input(pVampire);
    SkillOutput output;
    computeOutput(input, output);

    SIMPLE_SKILL_INPUT param;
    param.SkillType = getSkillType();
    param.SkillDamage = output.Damage;
    param.Delay = output.Delay;
    param.ItemClass = Item::ITEM_CLASS_MAX;
    param.STRMultiplier = 0;
    param.DEXMultiplier = 0;
    param.INTMultiplier = 0;
    param.bMagicHitRoll = false;
    param.bMagicDamage = true;
    param.bAdd = true;

    SIMPLE_SKILL_OUTPUT result;

    // Tiger Nail raises the damage by its rank bonus percentage.
    if (pVampire->hasRankBonus(RankBonus::RANK_BONUS_TIGER_NAIL)) {
        RankBonus* pRankBonus = pVampire->getRankBonus(RankBonus::RANK_BONUS_TIGER_NAIL);
        Assert(pRankBonus != NULL);

        param.SkillDamage += getPercentValue(param.SkillDamage, pRankBonus->getPoint());
    }

    // Knowledge of Blood gives a hit bonus of 10.
    int HitBonus = 0;
    if (pVampire->hasRankBonus(RankBonus::RANK_BONUS_KNOWLEDGE_OF_BLOOD)) {
        RankBonus* pRankBonus = pVampire->getRankBonus(RankBonus::RANK_BONUS_KNOWLEDGE_OF_BLOOD);
        Assert(pRankBonus != NULL);

        HitBonus = pRankBonus->getPoint();
    }

    g_SimpleMeleeSkill.execute(pVampire, TargetObjectID, pVampireSkillSlot, param, result, CEffectID, HitBonus);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Monster object handler
//////////////////////////////////////////////////////////////////////////////
void BloodyNail::execute(Monster* pMonster, Creature* pEnemy)

{
    __BEGIN_TRY


    SkillInput input(pMonster);
    SkillOutput output;
    computeOutput(input, output);

    SIMPLE_SKILL_INPUT param;
    param.SkillType = getSkillType();
    param.SkillDamage = output.Damage;
    param.Delay = output.Delay;
    param.ItemClass = Item::ITEM_CLASS_MAX;
    param.STRMultiplier = 0;
    param.DEXMultiplier = 0;
    param.INTMultiplier = 0;
    param.bMagicHitRoll = false;
    param.bMagicDamage = true;
    param.bAdd = true;

    SIMPLE_SKILL_OUTPUT result;

    g_SimpleMeleeSkill.execute(pMonster, pEnemy, param, result);


    __END_CATCH
}

BloodyNail g_BloodyNail;
