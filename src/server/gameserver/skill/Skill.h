//////////////////////////////////////////////////////////////////////////////
// Filename    : Skill.h
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_H__
#define __SKILL_H__

#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// Skill-related constants
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
    125, // Dawn
    100, // Day
    125, // Dusk
    150  // Night
};

const int MonsterTimebandFactor[4] = {
    75, // Dawn
    50, // Day
    75, // Dusk
    100 // Night
};

const int AttrExpTimebandFactor[4] = {
    100, // Dawn
    100, // Day
    100, // Dusk
    150  // Night
};

const int DomainExpTimebandFactor[4] = {
    100, // Dawn
    100, // Day
    100, // Dusk
    150  // Night
};

// The SkillTypes enum and its name table are wire vocabulary shared
// with the packet layer; they moved to types/SkillTypes.h (de-kernel).
#include "types/SkillTypes.h"


#endif // __SKILL_H__
