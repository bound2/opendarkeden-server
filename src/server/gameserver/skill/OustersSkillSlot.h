//////////////////////////////////////////////////////////////////////////////
// Filename    : OustersSkillSlot.h
// Written by  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __OUSTERS_SKILL_SLOT_H__
#define __OUSTERS_SKILL_SLOT_H__

#include "RaceSkillSlot.h"


// An ousters' skill slot: the shared slot state plus the per-skill level,
// persisted to the ousters skill table.
class OustersSkillSlot : public RaceSkillSlot {
public:
    OustersSkillSlot() : RaceSkillSlot() {}
    OustersSkillSlot(SkillType_t SkillType, ulong Interval, ulong CastingTime)
        : RaceSkillSlot(SkillType, Interval, CastingTime) {}

public:
    virtual void save(const string& ownerID);
    virtual void save();
    virtual void create(const string& ownerID);
    virtual void destroy(const string& ownerID);

    ExpLevel_t getExpLevel() const {
        return m_ExpLevel;
    }
    void setExpLevel(ExpLevel_t ExpLevel) {
        m_ExpLevel = ExpLevel;
    }

private:
    ExpLevel_t m_ExpLevel; // Skill level
};

#endif // __OUSTERS_SKILL_SLOT_H__
