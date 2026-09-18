//////////////////////////////////////////////////////////////////////////////
// Filename    : PoisonStrike.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "PoisonStrike.h"

#include "RankBonus.h"
#include "SimpleMissileSkill.h"

//////////////////////////////////////////////////////////////////////////////
// Vampire object handler
//////////////////////////////////////////////////////////////////////////////
void PoisonStrike::execute(Vampire* pVampire, ObjectID_t TargetObjectID, VampireSkillSlot* pVampireSkillSlot,
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
    param.bMagicHitRoll = true;
    param.bMagicDamage = true;
    param.bAdd = false;

    SIMPLE_SKILL_OUTPUT result;

    // Knowledge of Poison grants a to-hit bonus.
    int HitBonus = 0;
    if (pVampire->hasRankBonus(RankBonus::RANK_BONUS_KNOWLEDGE_OF_POISON)) {
        RankBonus* pRankBonus = pVampire->getRankBonus(RankBonus::RANK_BONUS_KNOWLEDGE_OF_POISON);
        Assert(pRankBonus != NULL);

        HitBonus = pRankBonus->getPoint();
    }

    g_SimpleMissileSkill.execute(pVampire, TargetObjectID, pVampireSkillSlot, param, result, CEffectID, HitBonus);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Monster object handler
//////////////////////////////////////////////////////////////////////////////
void PoisonStrike::execute(Monster* pMonster, Creature* pEnemy)

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
    param.bMagicHitRoll = true;
    param.bMagicDamage = true;
    param.bAdd = false;

    SIMPLE_SKILL_OUTPUT result;

    g_SimpleMissileSkill.execute(pMonster, pEnemy, param, result);


    __END_CATCH
}

PoisonStrike g_PoisonStrike;
