//////////////////////////////////////////////////////////////////////////////
// Filename    : Skill.h
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_H__
#define __SKILL_H__

#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// ½ºÅ³ °ü·Ã »ó¼öµé...
//////////////////////////////////////////////////////////////////////////////

const int ATTR_SAVE_DIVIDER = 100;
const int SKILL_EXP_SAVE_DIVIDER = 100;
const int DOMAIN_EXP_SAVE_DIVIDER = 100;
const int VAMPIRE_EXP_SAVE_DIVIDER = 100;
const int FAME_SAVE_DIVIDER = 100;
const int ALIGNMENT_SAVE_DIVIDER = 100;

const int VAMPIRE_DAY_FACTOR = 100;
const int VAMPIRE_DAWN_FACTOR = 125;
const int VAMPIRE_DUSK_FACTOR = 125;
const int VAMPIRE_NIGHT_FACTOR = 150;

const int MONSTER_DAY_FACTOR = 50;
const int MONSTER_DAWN_FACTOR = 75;
const int MONSTER_DUSK_FACTOR = 75;
const int MONSTER_NIGHT_FACTOR = 100;

const int VampireTimebandFactor[4] = {
    125, // »õº®
    100, // ³·
    125, // Àú³á
    150  // ¹ã
};

const int MonsterTimebandFactor[4] = {
    75, // »õº®
    50, // ³·
    75, // Àú³á
    100 // ¹ã
};

const int AttrExpTimebandFactor[4] = {
    100, // »õº®
    100, // ³·
    100, // Àú³á
    150  // ¹ã
};

const int DomainExpTimebandFactor[4] = {
    100, // »õº®
    100, // ³·
    100, // Àú³á
    150  // ¹ã
};

// The SkillTypes enum and its name table are wire vocabulary shared
// with the packet layer; they moved to types/SkillTypes.h (de-kernel).
#include "types/SkillTypes.h"


#endif // __SKILL_H__
