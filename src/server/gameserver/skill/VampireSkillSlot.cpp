//////////////////////////////////////////////////////////////////////////////
// Filename    : VampireSkillSlot.cpp
// Written By  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "VampireSkillSlot.h"

#include "repository/SkillSaveRepository.h"

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
