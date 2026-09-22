//////////////////////////////////////////////////////////////////////////////
// Filename    : RaceSkillSlot.cpp
// Description : Skill slot state shared by the vampire and ousters slots
//////////////////////////////////////////////////////////////////////////////

#include "RaceSkillSlot.h"

RaceSkillSlot::RaceSkillSlot() {
    __BEGIN_TRY
    m_SkillType = 0;
    m_Interval = 0;
    m_CastingTime = 0;
    __END_CATCH
}

RaceSkillSlot::RaceSkillSlot(SkillType_t SkillType, ulong Interval, ulong CastingTime) {
    __BEGIN_TRY

    m_SkillType = SkillType;
    m_Interval = Interval;
    m_CastingTime = CastingTime;

    __END_CATCH
}

RaceSkillSlot::~RaceSkillSlot() {
    __BEGIN_TRY

    m_SkillType = 0;
    m_Interval = 0;
    m_CastingTime = 0;

    __END_CATCH
}

Turn_t RaceSkillSlot::getRemainTurn(Timeval currentTime) const {
    Turn_t remainTurn =
        (m_runTime.tv_sec - currentTime.tv_sec) * 10 + (m_runTime.tv_usec - currentTime.tv_usec) / 100000;

    return remainTurn;
}

void RaceSkillSlot::setRunTime() {
    // Get the current time.
    getCurrentTime(m_runTime);

    // Set the time at which it can be used again.
    m_runTime.tv_sec += m_Interval / 10;
    m_runTime.tv_usec += (m_Interval % 10) * 100000;
}

void RaceSkillSlot::setRunTime(Turn_t delay) {
    // Get the current time.
    getCurrentTime(m_runTime);

    // Set the time at which it can be used again.
    m_runTime.tv_sec += delay / 10;
    m_runTime.tv_usec += (delay % 10) * 100000;

    if (m_Interval != delay) {
        m_Interval = delay;

        switch (m_SkillType) {
        case SKILL_ATTACK_MELEE:
        case SKILL_ATTACK_ARMS:
        case SKILL_SELF:
        case SKILL_TILE:
        case SKILL_OBJECT:
        case SKILL_BLOOD_DRAIN:
        case SKILL_UN_BURROW:
        case SKILL_UN_TRANSFORM:
        case SKILL_UN_INVISIBILITY:
        case SKILL_THROW_HOLY_WATER:
        case SKILL_EAT_CORPSE:
        case SKILL_HOWL:
            break;
        default:
            save(m_Name); // Save the changed delay.
            break;
        }
    }
}
