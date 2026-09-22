//////////////////////////////////////////////////////////////////////////////
// Filename    : RaceSkillSlot.h
// Description : Skill slot state shared by the vampire and ousters slots
//////////////////////////////////////////////////////////////////////////////

#ifndef __RACE_SKILL_SLOT_H__
#define __RACE_SKILL_SLOT_H__

#include "Exception.h"
#include "Skill.h"
#include "Timeval.h"
#include "Types.h"


// One learned skill: its name, kind, delay and casting time, plus the time
// at which it may next be cast. Persistence belongs to the derived class --
// each race writes its own table -- so the run-time save this class triggers
// when a cast changes the delay goes out through save().
class RaceSkillSlot {
public:
    RaceSkillSlot();
    RaceSkillSlot(SkillType_t SkillType, ulong Interval, ulong CastingTime);
    virtual ~RaceSkillSlot();

public:
    virtual void save(const string& ownerID) = 0;
    virtual void save() = 0;
    virtual void create(const string& ownerID) = 0;

    SkillType_t getSkillType() {
        return m_SkillType;
    }
    void setSkillType(SkillType_t Type) {
        m_SkillType = Type;
    }

    ulong getInterval() {
        return m_Interval;
    }
    void setInterval(ulong Interval) {
        m_Interval = Interval;
    }

    Turn_t getCastingTime() {
        return m_CastingTime;
    }
    void setCastingTime(Turn_t CastingTime) {
        m_CastingTime = CastingTime;
    }

    Timeval getRunTime() {
        return m_runTime;
    }
    void setRunTime();
    void setRunTime(Turn_t delay);

    string getName() const {
        return m_Name;
    }
    void setName(const string& Name) {
        m_Name = Name;
    }

    // Time remaining until the next cast is possible
    Turn_t getRemainTurn(Timeval currentTime) const;

protected:
    string m_Name;
    SkillType_t m_SkillType; // Kind of magic or skill
    Turn_t m_Interval;       // Magic or skill delay, in seconds
    Turn_t m_CastingTime;    // Casting time, in tenths of a second
    Timeval m_runTime;       // Time when it can be used next
};

#endif // __RACE_SKILL_SLOT_H__
