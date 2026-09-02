//////////////////////////////////////////////////////////////////////////////
// Filename    : RankBonusInfo.cpp
// Written By  : beowulf
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "RankBonusInfo.h"

#include "Assert.h"
#include "repository/GameInfoRepository.h"
// #include <algo.h>

//////////////////////////////////////////////////////////////////////////////
// class RankBonusInfo member methods
//////////////////////////////////////////////////////////////////////////////

RankBonusInfo::RankBonusInfo(){__BEGIN_TRY __END_CATCH}

RankBonusInfo::~RankBonusInfo(){__BEGIN_TRY __END_CATCH_NO_RETHROW}

string RankBonusInfo::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "RankBonusInfo (" << "Type:" << (int)m_Type << ",Name:" << m_Name << ",Rank:" << m_Rank
        << ",Point:" << m_Point << ")";
    return msg.toString();

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// class RankBonusInfoManager member methods
//////////////////////////////////////////////////////////////////////////////

RankBonusInfoManager::RankBonusInfoManager()

{
    __BEGIN_TRY

    m_Count = 0;
    m_RankBonusInfoList = NULL;

    __END_CATCH
}

RankBonusInfoManager::~RankBonusInfoManager()

{
    __BEGIN_TRY

    SAFE_DELETE_ARRAY(m_RankBonusInfoList);

    __END_CATCH_NO_RETHROW
}

void RankBonusInfoManager::init()

{
    __BEGIN_TRY

    load();

    __END_CATCH
}

void RankBonusInfoManager::clear()

{
    __BEGIN_TRY

    if (m_RankBonusInfoList != NULL) {
        for (uint i = 0; i < m_Count; i++) {
            if (m_RankBonusInfoList[i] != NULL)
                SAFE_DELETE(m_RankBonusInfoList[i]);
        }
    }
    SAFE_DELETE_ARRAY(m_RankBonusInfoList);

    __END_CATCH
}

void RankBonusInfoManager::load()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    clear();

    int maxType = 0;
    if (!defaultGameInfoRepository().loadMaxRankBonusType(maxType)) {
        throw Error("There is no data in RankBonusInfo Table");
    }

    m_Count = maxType + 1;

    Assert(m_Count > 0);

    m_RankBonusInfoList = new RankBonusInfo*[m_Count];

    for (uint i = 0; i < m_Count; i++)
        m_RankBonusInfoList[i] = NULL;

    vector<RankBonusInfoRow> rows = defaultGameInfoRepository().loadRankBonusInfos();

    for (size_t r = 0; r < rows.size(); r++) {
        RankBonusInfo* pRankBonusInfo = new RankBonusInfo();

        pRankBonusInfo->setType(rows[r].type);
        pRankBonusInfo->setName(rows[r].name);
        pRankBonusInfo->setRank(rows[r].rank);
        pRankBonusInfo->setPoint(rows[r].point);
        pRankBonusInfo->setRace(rows[r].race);

        addRankBonusInfo(pRankBonusInfo);
    }

    __END_DEBUG
    __END_CATCH
}

void RankBonusInfoManager::save()

{
    __BEGIN_TRY

    throw UnsupportedError(__PRETTY_FUNCTION__);

    __END_CATCH
}

RankBonusInfo* RankBonusInfoManager::getRankBonusInfo(DWORD rankBonusType) const {
    __BEGIN_TRY

    if (rankBonusType >= m_Count) {
        cerr << "RankBonusInfoManager::getRankBonusInfo() : out of bound" << endl;
        throw OutOfBoundException();
    }

    if (m_RankBonusInfoList[rankBonusType] == NULL) {
        cerr << "RankBonusInfoManager::getRankBonusInfo() : no such element" << endl;
        throw NoSuchElementException();
    }

    return m_RankBonusInfoList[rankBonusType];

    __END_CATCH
}

void RankBonusInfoManager::addRankBonusInfo(RankBonusInfo* pRankBonusInfo)

{
    __BEGIN_TRY

    Assert(pRankBonusInfo != NULL);

    if (m_RankBonusInfoList[pRankBonusInfo->getType()] != NULL)
        throw DuplicatedException();

    m_RankBonusInfoList[pRankBonusInfo->getType()] = pRankBonusInfo;

    __END_CATCH
}

string RankBonusInfoManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "RankBonusInfoManager(\n";

    for (int i = 0; i < (int)m_Count; i++) {
        msg << "RankBonusInfos[" << i << "] == ";
        msg << (m_RankBonusInfoList[i] == NULL ? "NULL" : m_RankBonusInfoList[i]->getName());
        msg << "\n";
        if (m_RankBonusInfoList[i] != NULL) {
            msg << m_RankBonusInfoList[i]->toString() << "\n";
        }
    }

    return msg.toString();

    __END_CATCH
}

// Global Variable definition
RankBonusInfoManager* g_pRankBonusInfoManager = NULL;
