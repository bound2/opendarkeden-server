//////////////////////////////////////////////////////////////////////////////
// Filename    : VampireSkillSlot.h
// Written by  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __VAMPIRE_SKILL_SLOT_H__
#define __VAMPIRE_SKILL_SLOT_H__

#include "RaceSkillSlot.h"


// A vampire's skill slot: the shared slot state, persisted to the vampire
// skill table.
class VampireSkillSlot : public RaceSkillSlot {
public:
    VampireSkillSlot() : RaceSkillSlot() {}
    VampireSkillSlot(SkillType_t SkillType, ulong Interval, ulong CastingTime)
        : RaceSkillSlot(SkillType, Interval, CastingTime) {}

public:
    virtual void save(const string& ownerID);
    virtual void save();
    virtual void create(const string& ownerID);
};

#endif // __VAMPIRE_SKILL_SLOT_H__
