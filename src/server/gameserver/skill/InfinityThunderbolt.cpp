//////////////////////////////////////////////////////////////////////////////
// Filename    : InfinityThunderbolt.cpp
// Written by  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "InfinityThunderbolt.h"

#include "SimpleMeleeSkill.h"
#include "SimpleTileMissileSkill.h"
// #include "EffectInfinityThunderbolt.h"
#include <list>

#include "GCAddEffectToTile.h"
#include "ZoneUtil.h"

void InfinityThunderbolt::execute(Slayer* pSlayer, ObjectID_t targetObjectID, SkillSlot* pSkillSlot,
                                  CEffectID_t CEffectID) {
    __BEGIN_TRY

    Zone* pZone = pSlayer->getZone();
    Assert(pZone != NULL);

    Creature* pTargetCreature = pZone->getCreature(targetObjectID);

    // NoSuch제거. by sigi. 2002.5.2
    if (pTargetCreature == NULL) {
        executeSkillFailException(pSlayer, getSkillType());
        return;
    }

    execute(pSlayer, pTargetCreature->getX(), pTargetCreature->getY(), pSkillSlot, CEffectID);

    __END_CATCH
}

void InfinityThunderbolt::execute(Slayer* pSlayer, ZoneCoord_t X, ZoneCoord_t Y, SkillSlot* pSkillSlot,
                                  CEffectID_t CEffectID)

{
    __BEGIN_TRY


    Zone* pZone = pSlayer->getZone();
    Assert(pZone != NULL);

    SkillInput input(pSlayer, pSkillSlot);
    SkillOutput output;
    computeOutput(input, output);

    SIMPLE_SKILL_INPUT param;
    param.SkillType = getSkillType();
    param.SkillDamage = output.Damage;
    param.Delay = output.Delay;
    param.ItemClass = Item::ITEM_CLASS_MAX;
    param.STRMultiplier = 8;
    param.DEXMultiplier = 1;
    param.INTMultiplier = 1;
    param.bMagicHitRoll = false;
    param.bMagicDamage = false;
    param.bAdd = true;
    param.bExpForTotalDamage = false;

    for (int i = -1; i <= 1; ++i)
        for (int j = -1; j <= 1; ++j) {
            param.addMask(i, j, 100);
        }

    SIMPLE_SKILL_OUTPUT result;

    // 목표위치+4방향

    g_SimpleTileMissileSkill.execute(pSlayer, X, Y, pSkillSlot, param, result);

    __END_CATCH
}

InfinityThunderbolt g_InfinityThunderbolt;
