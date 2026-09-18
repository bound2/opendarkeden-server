//////////////////////////////////////////////////////////////////////////////
// Filename    : OustersSkillSlot.h
// Written by  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __OUSTERS_SKILL_SLOT_H__
#define __OUSTERS_SKILL_SLOT_H__

#include "Exception.h"
#include "Skill.h"
#include "Timeval.h"
#include "Types.h"


class OustersSkillSlot {
public:
    OustersSkillSlot();
    OustersSkillSlot(SkillType_t SkillType, ulong Interval, ulong CastingTime);
    virtual ~OustersSkillSlot();

public:
    virtual void save(const string& ownerID);
    virtual void save();
    virtual void create(const string& ownerID);
    virtual void destroy(const string& ownerID);

    SkillType_t getSkillType() {
        return m_SkillType;
    }
    void setSkillType(SkillType_t Type) {
        m_SkillType = Type;
    }

    ExpLevel_t getExpLevel() const {
        return m_ExpLevel;
    }
    void setExpLevel(ExpLevel_t ExpLevel) {
        m_ExpLevel = ExpLevel;
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

private:
    string m_Name;
    SkillType_t m_SkillType; // Kind of magic or skill
    ExpLevel_t m_ExpLevel;   // Skill level
    Turn_t m_Interval;       // Magic or skill delay, in seconds
    Turn_t m_CastingTime;    // Casting time, in tenths of a second
    Timeval m_runTime;       // Time when it can be used next
};

#endif // __OUSTERS_SKILL_SLOT_H__
