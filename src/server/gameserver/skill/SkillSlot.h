//////////////////////////////////////////////////////////////////////////////
// Filename    : SkillSlot.h
// Written by  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_SLOT_H__
#define __SKILL_SLOT_H__

#include "Exception.h"
#include "Skill.h"
#include "Timeval.h"
#include "Types.h"


class SkillSlot {
public:
    SkillSlot();
    SkillSlot(SkillType_t SkillType, DWORD Exp, ulong Interval);
    virtual ~SkillSlot();

public:
    virtual void save(const string& ownerID);
    virtual void save();

    virtual void create(const string& ownerID);

    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t Type) {
        m_SkillType = Type;
    }

    void setExp(Exp_t Exp) {
        m_Exp = Exp;
    }
    Exp_t getExp() {
        return m_Exp;
    }

    void setExpLevel(ExpLevel_t ExpLevel) {
        m_ExpLevel = ExpLevel;
    }
    ExpLevel_t getExpLevel() {
        return m_ExpLevel;
    }

    Turn_t getInterval() {
        return m_Interval;
    }
    void setInterval(Turn_t Interval) {
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
    void setRunTime(Turn_t delay, bool bSave = true);

    string getName() const {
        return m_Name;
    }
    void setName(const string& Name) {
        m_Name = Name;
    }

    // true when the slot can be used
    // false when it cannot be used
    void setDisable() {
        m_Enable = false;
    }
    void setEnable() {
        m_Enable = true;
    }
    bool canUse() const {
        return m_Enable;
    }

    // Time remaining until the next cast is possible
    Turn_t getRemainTurn(Timeval currentTime) const;

protected:
    string m_Name;
    SkillType_t m_SkillType; // Kind of magic or skill
    Exp_t m_Exp;             // Proficiency
    ExpLevel_t m_ExpLevel;   // Proficiency level
    Turn_t m_Interval;       // Delay of the magic or skill, in units of 0.1 second
    Turn_t m_CastingTime;    // Casting time, in tenths of a second
    Timeval m_runTime;       // Time when it can be used next
    bool m_Enable;           // Whether the slot can be used
};

#endif // __SKILL_SLOT_H__
