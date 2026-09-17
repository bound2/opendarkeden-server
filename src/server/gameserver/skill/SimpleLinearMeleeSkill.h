//////////////////////////////////////////////////////////////////////////////
// Filename    : SimpleLinearMeleeSkill.h
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SIMPLELINEARMELEESKILL__
#define __SIMPLELINEARMELEESKILL__

#include "SimpleSkill.h"

class SimpleLinearMeleeSkill {
public:
    void execute(Slayer* pSlayer, int X, int Y, SkillSlot* pSkillSlot, const SIMPLE_SKILL_INPUT& param,
                 SIMPLE_SKILL_OUTPUT& result, CEffectID_t CEffectID = 0);
};

extern SimpleLinearMeleeSkill g_SimpleLinearMeleeSkill;

#endif
