////////////////////////////////////////////////////////////////////////////////
// Filename    : RankEXPInfo.cpp
// Written By  : beowulf
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "RankEXPInfo.h"

#include <algo.h>

#include "Assert.h"
#include "repository/BalanceInfoRepository.h"

////////////////////////////////////////////////////////////////////////////////
// Global Variable initialization
////////////////////////////////////////////////////////////////////////////////
RankEXPInfoManager* g_pRankEXPInfoManager[RANK_TYPE_MAX] = {
    NULL,
};


////////////////////////////////////////////////////////////////////////////////
// class RankEXPInfo member methods
////////////////////////////////////////////////////////////////////////////////

RankEXPInfo::RankEXPInfo()

    {__BEGIN_TRY __END_CATCH}

RankEXPInfo::~RankEXPInfo()

    {__BEGIN_TRY __END_CATCH}

string RankEXPInfo::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "RankEXPInfo (" << " Level : " << (int)m_Level << " GoalExp : " << (int)m_GoalExp
        << " AccumExp : " << (int)m_AccumExp << ")";

    return msg.toString();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// class RankEXPInfoManager member methods
////////////////////////////////////////////////////////////////////////////////

RankEXPInfoManager::RankEXPInfoManager()

{
    __BEGIN_TRY

    m_RankEXPCount = 0;
    m_RankEXPInfoList = NULL;

    __END_CATCH
}

RankEXPInfoManager::~RankEXPInfoManager()

{
    __BEGIN_TRY

    if (m_RankEXPInfoList != NULL) {
        for (uint i = 0; i < m_RankEXPCount; i++)
            SAFE_DELETE(m_RankEXPInfoList[i]);

        SAFE_DELETE_ARRAY(m_RankEXPInfoList);
    }

    __END_CATCH
}

void RankEXPInfoManager::init(RankType rankType)

{
    __BEGIN_TRY

    load(rankType);

    __END_CATCH
}

void RankEXPInfoManager::load(RankType rankType)

{
    __BEGIN_TRY

    int maxLevel = 0;
    if (!defaultBalanceInfoRepository().loadMaxRankLevel((int)rankType, maxLevel)) {
        throw Error("There is no data in RankEXPInfo Table");
    }

    // Size the table from the highest level.
    m_RankEXPCount = maxLevel + 1;

    Assert(m_RankEXPCount > 0);

    m_RankEXPInfoList = new RankEXPInfo*[m_RankEXPCount];
    Assert(m_RankEXPInfoList != NULL);

    // Clear the array.
    for (uint i = 0; i < m_RankEXPCount; i++)
        m_RankEXPInfoList[i] = NULL;

    // Fill in the rows.
    vector<LevelExpRow> rows = defaultBalanceInfoRepository().loadRankLevels((int)rankType);
    for (size_t r = 0; r < rows.size(); r++) {
        RankEXPInfo* pRankEXPInfo = new RankEXPInfo();
        Assert(pRankEXPInfo != NULL);

        pRankEXPInfo->setLevel(rows[r].level);
        pRankEXPInfo->setGoalExp(rows[r].goalExp);
        pRankEXPInfo->setAccumExp(rows[r].accumExp);

        addRankEXPInfo(pRankEXPInfo);
    }

    __END_CATCH
}

void RankEXPInfoManager::addRankEXPInfo(RankEXPInfo* pRankEXPInfo)

{
    __BEGIN_TRY

    Assert(pRankEXPInfo != NULL);
    Assert(m_RankEXPInfoList[pRankEXPInfo->getLevel()] == NULL);

    m_RankEXPInfoList[pRankEXPInfo->getLevel()] = pRankEXPInfo;

    __END_CATCH
}

RankEXPInfo* RankEXPInfoManager::getRankEXPInfo(uint value) const

{
    __BEGIN_TRY

    if (value >= m_RankEXPCount || m_RankEXPInfoList[value] == NULL) {
        filelog("RankEXPError.log", "RankEXP 능력치 초과 또는 미만");
        throw InvalidProtocolException();
    }

    return m_RankEXPInfoList[value];

    __END_CATCH
}

string RankEXPInfoManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "RankEXPInfoManager(";

    for (uint i = 0; i < m_RankEXPCount; i++) {
        if (m_RankEXPInfoList[i] != NULL) {
            msg << m_RankEXPInfoList[i]->toString();
        } else {
            msg << "NULL";
        }
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}
