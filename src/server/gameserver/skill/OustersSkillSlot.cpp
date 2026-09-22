//////////////////////////////////////////////////////////////////////////////
// Filename    : OustersSkillSlot.cpp
// Written By  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "OustersSkillSlot.h"

#include "repository/SkillSaveRepository.h"

void OustersSkillSlot::create(const string& OwnerID)

{
    __BEGIN_TRY

    OustersSkillRecord record;
    record.skillType = m_SkillType;
    record.skillLevel = m_ExpLevel;
    record.delay = m_Interval;
    record.castingTime = m_CastingTime;
    record.nextTime = m_runTime.tv_sec;
    defaultSkillSaveRepository().insertOustersSkill(OwnerID, record);

    __END_CATCH
}

void OustersSkillSlot::save(const string& OwnerID)

{
    __BEGIN_TRY

    defaultSkillSaveRepository().updateOustersSkill(OwnerID, m_SkillType, m_ExpLevel, m_Interval);

    __END_CATCH
}

void OustersSkillSlot::destroy(const string& OwnerID)

{
    __BEGIN_TRY

    defaultSkillSaveRepository().deleteOustersSkill(OwnerID, m_SkillType);

    __END_CATCH
}

void OustersSkillSlot::save()

{
    __BEGIN_TRY

    save(m_Name);

    __END_CATCH
}
