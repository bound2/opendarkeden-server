//////////////////////////////////////////////////////////////////////////////
// Filename    : VampireSkillSlot.cpp
// Written By  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "VampireSkillSlot.h"

#include "repository/SkillSaveRepository.h"

VampireSkillSlot::VampireSkillSlot() throw() {
    __BEGIN_TRY
    m_SkillType = 0;
    m_Interval = 0;
    m_CastingTime = 0;
    __END_CATCH
}

VampireSkillSlot::VampireSkillSlot(SkillType_t SkillType, ulong Interval, ulong CastingTime) throw() {
    __BEGIN_TRY

    m_SkillType = SkillType;
    m_Interval = Interval;
    m_CastingTime = CastingTime;

    __END_CATCH
}

VampireSkillSlot::~VampireSkillSlot() throw() {
    __BEGIN_TRY

    m_SkillType = 0;
    m_Interval = 0;
    m_CastingTime = 0;

    __END_CATCH
}

void VampireSkillSlot::create(const string& OwnerID)

{
    __BEGIN_TRY

    VampireSkillRecord record;
    record.skillType = m_SkillType;
    record.delay = m_Interval;
    record.castingTime = m_CastingTime;
    record.nextTime = m_runTime.tv_sec;
    defaultSkillSaveRepository().insertVampireSkill(OwnerID, record);

    __END_CATCH
}

void VampireSkillSlot::save(const string& OwnerID)

{
    __BEGIN_TRY

    defaultSkillSaveRepository().updateVampireSkill(OwnerID, m_SkillType, m_Interval);

    __END_CATCH
}

void VampireSkillSlot::save()

{
    __BEGIN_TRY

    defaultSkillSaveRepository().updateVampireSkill(m_Name, m_SkillType, m_Interval);

    __END_CATCH
}

Turn_t VampireSkillSlot::getRemainTurn(Timeval currentTime) const throw() {
    Turn_t remainTurn =
        (m_runTime.tv_sec - currentTime.tv_sec) * 10 + (m_runTime.tv_usec - currentTime.tv_usec) / 100000;

    return remainTurn;
}

void VampireSkillSlot::setRunTime() throw() {
    // 현재 시간을 받아온다.
    getCurrentTime(m_runTime);

    // 다음 쓸 수 있는 시간을 세팅한다.
    m_runTime.tv_sec += m_Interval / 10;
    m_runTime.tv_usec += (m_Interval % 10) * 100000;
}

void VampireSkillSlot::setRunTime(Turn_t delay) throw() {
    // 현재 시간을 받아온다.
    getCurrentTime(m_runTime);

    // 다음 쓸 수 있는 시간을 세팅한다.
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
            save(m_Name); // 달리진 딜레이를 세이브한다.
            break;
        }
    }
}
