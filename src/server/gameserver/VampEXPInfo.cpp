////////////////////////////////////////////////////////////////////////////////
// Filename    : VampEXPInfo.cpp
// Written By  : beowulf
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "VampEXPInfo.h"

#include "Assert.h"
#include "repository/BalanceInfoRepository.h"
// #include <algo.h>

////////////////////////////////////////////////////////////////////////////////
// Global Variable definition
////////////////////////////////////////////////////////////////////////////////
VampEXPInfoManager* g_pVampEXPInfoManager = NULL;


////////////////////////////////////////////////////////////////////////////////
// class VampEXPInfo member methods
////////////////////////////////////////////////////////////////////////////////

VampEXPInfo::VampEXPInfo()

    {__BEGIN_TRY __END_CATCH}

VampEXPInfo::~VampEXPInfo()

    {__BEGIN_TRY __END_CATCH_NO_RETHROW}

string VampEXPInfo::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "VampEXPInfo (" << " Level : " << (int)m_Level << " GoalExp : " << (int)m_GoalExp
        << " AccumExp : " << (int)m_AccumExp << ")";
    return msg.toString();

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
// class VampEXPInfoManager member methods
////////////////////////////////////////////////////////////////////////////////

VampEXPInfoManager::VampEXPInfoManager()

{
    __BEGIN_TRY

    m_VampEXPCount = 0;
    m_VampEXPInfoList = NULL;

    __END_CATCH
}

VampEXPInfoManager::~VampEXPInfoManager()

{
    __BEGIN_TRY

    if (m_VampEXPInfoList != NULL) {
        for (uint i = 0; i < m_VampEXPCount; i++)
            SAFE_DELETE(m_VampEXPInfoList[i]);

        SAFE_DELETE_ARRAY(m_VampEXPInfoList);
    }

    __END_CATCH_NO_RETHROW
}

void VampEXPInfoManager::init()

{
    __BEGIN_TRY

    load();

    __END_CATCH
}

void VampEXPInfoManager::load()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    int maxLevel = 0;
    if (!defaultBalanceInfoRepository().loadMaxLevel(LEVEL_EXP_TABLE_VAMP_EXP, maxLevel)) {
        throw Error("There is no data in VampEXPInfo Table");
    }

    // Size the table from the highest level.
    m_VampEXPCount = maxLevel + 1;

    Assert(m_VampEXPCount > 0);

    m_VampEXPInfoList = new VampEXPInfo*[m_VampEXPCount];

    Assert(m_VampEXPInfoList != NULL);

    // Clear the array.
    for (uint i = 0; i < m_VampEXPCount; i++)
        m_VampEXPInfoList[i] = NULL;

    vector<LevelExpRow> rows = defaultBalanceInfoRepository().loadLevels(LEVEL_EXP_TABLE_VAMP_EXP);

    for (size_t r = 0; r < rows.size(); r++) {
        VampEXPInfo* pVampEXPInfo = new VampEXPInfo();

        pVampEXPInfo->setLevel(rows[r].level);
        pVampEXPInfo->setGoalExp(rows[r].goalExp);
        pVampEXPInfo->setAccumExp(rows[r].accumExp);

        addVampEXPInfo(pVampEXPInfo);
    }

    __END_DEBUG
    __END_CATCH
}

void VampEXPInfoManager::addVampEXPInfo(VampEXPInfo* pVampEXPInfo)

{
    __BEGIN_TRY

    Assert(pVampEXPInfo != NULL);
    Assert(m_VampEXPInfoList[pVampEXPInfo->getLevel()] == NULL);

    m_VampEXPInfoList[pVampEXPInfo->getLevel()] = pVampEXPInfo;

    __END_CATCH
}

VampEXPInfo* VampEXPInfoManager::getVampEXPInfo(uint VampEXPType) const {
    __BEGIN_TRY

    Assert(VampEXPType < m_VampEXPCount);
    Assert(m_VampEXPInfoList[VampEXPType] != NULL);

    return m_VampEXPInfoList[VampEXPType];

    __END_CATCH
}

string VampEXPInfoManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "VampEXPInfoManager(";

    for (uint i = 0; i < m_VampEXPCount; i++) {
        if (m_VampEXPInfoList[i] != NULL) {
            msg << m_VampEXPInfoList[i]->toString();
        } else {
            msg << "NULL";
        }
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}
