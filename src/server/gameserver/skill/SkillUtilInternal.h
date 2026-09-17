//////////////////////////////////////////////////////////////////////////////
// FileName 	: SkillUtilInternal.h
// Description	: Skill helpers shared between the SkillUtil translation units.
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_UTIL_INTERNAL_H__
#define __SKILL_UTIL_INTERNAL_H__

#include "SkillUtil.h"

// Whether a use of UseSkillType earns the slayer experience in SkillDomainType,
// which it does only while the weapon in his right hand belongs to that domain.
bool canGiveSkillExp(Slayer* pSlayer, SkillDomainType_t SkillDomainType, SkillType_t UseSkillType);

// Gives the slayer the experience one successful use of SkillType is worth.
void giveSkillExp(Slayer* pSlayer, SkillType_t SkillType, ModifyInfo& AttackerMI);

#endif
