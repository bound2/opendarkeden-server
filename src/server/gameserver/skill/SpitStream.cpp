//////////////////////////////////////////////////////////////////////////////
// Filename    : SpitStream.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "SpitStream.h"

#include "SimpleLinearMeleeSkill.h"

//////////////////////////////////////////////////////////////////////////////
// Slayer object handler
//////////////////////////////////////////////////////////////////////////////
void SpitStream::execute(Slayer* pSlayer, ObjectID_t TargetObjectID, SkillSlot* pSkillSlot, CEffectID_t CEffectID)

{
    __BEGIN_TRY


    Assert(pSlayer != NULL);
    Assert(pSkillSlot != NULL);

    try {
        Zone* pZone = pSlayer->getZone();
        Creature* pTargetCreature = pZone->getCreature(TargetObjectID);

        // A missing target fails the skill instead of throwing.
        if (pTargetCreature == NULL) {
            executeSkillFailException(pSlayer, getSkillType());
            return;
        }

        execute(pSlayer, pTargetCreature->getX(), pTargetCreature->getY(), pSkillSlot, CEffectID);
    } catch (Throwable& t) {
        executeSkillFailException(pSlayer, getSkillType());
    }


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Slayer self handler
//////////////////////////////////////////////////////////////////////////////
void SpitStream::execute(Slayer* pSlayer, ZoneCoord_t X, ZoneCoord_t Y, SkillSlot* pSkillSlot, CEffectID_t CEffectID)

{
    __BEGIN_TRY

    cout << "TID[" << Thread::self() << "]" << getSkillHandlerName() << " Begin" << endl;

    SkillInput input(pSlayer, pSkillSlot);
    SkillOutput output;
    computeOutput(input, output);

    SIMPLE_SKILL_INPUT param;
    param.SkillType = getSkillType();
    param.SkillDamage = output.Damage;
    param.Delay = output.Delay;
    param.ItemClass = Item::ITEM_CLASS_BLADE;
    param.STRMultiplier = 8;
    param.DEXMultiplier = 1;
    param.INTMultiplier = 1;
    param.bMagicHitRoll = false;
    param.bMagicDamage = false;
    param.bAdd = true;

    SIMPLE_SKILL_OUTPUT result;

    g_SimpleLinearMeleeSkill.execute(pSlayer, X, Y, pSkillSlot, param, result);

    cout << "TID[" << Thread::self() << "]" << getSkillHandlerName() << " End" << endl;

    __END_CATCH
}

SpitStream g_SpitStream;
