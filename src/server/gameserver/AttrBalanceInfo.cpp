////////////////////////////////////////////////////////////////////////////////
// Filename    : AttrBalanceInfo.cpp
// Written By  : beowulf
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "AttrBalanceInfo.h"

#include <algo.h>

#include "Assert.h"
#include "repository/BalanceInfoRepository.h"

////////////////////////////////////////////////////////////////////////////////
// Global Variable initialization
////////////////////////////////////////////////////////////////////////////////
STRBalanceInfoManager* g_pSTRBalanceInfoManager = NULL;
DEXBalanceInfoManager* g_pDEXBalanceInfoManager = NULL;
INTBalanceInfoManager* g_pINTBalanceInfoManager = NULL;


////////////////////////////////////////////////////////////////////////////////
// class STRBalanceInfo member methods
////////////////////////////////////////////////////////////////////////////////

STRBalanceInfo::STRBalanceInfo()

    {__BEGIN_TRY __END_CATCH}

STRBalanceInfo::~STRBalanceInfo()

    {__BEGIN_TRY __END_CATCH}

string STRBalanceInfo::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "STRBalanceInfo (" << " Level : " << (int)m_Level << " GoalExp : " << (int)m_GoalExp
        << " AccumExp : " << (int)m_AccumExp << ")";

    return msg.toString();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// class STRBalanceInfoManager member methods
////////////////////////////////////////////////////////////////////////////////

STRBalanceInfoManager::STRBalanceInfoManager()

{
    __BEGIN_TRY

    m_STRBalanceCount = 0;
    m_STRBalanceInfoList = NULL;

    __END_CATCH
}

STRBalanceInfoManager::~STRBalanceInfoManager()

{
    __BEGIN_TRY

    if (m_STRBalanceInfoList != NULL) {
        for (uint i = 0; i < m_STRBalanceCount; i++)
            SAFE_DELETE(m_STRBalanceInfoList[i]);

        SAFE_DELETE_ARRAY(m_STRBalanceInfoList);
    }

    __END_CATCH
}

void STRBalanceInfoManager::init()

{
    __BEGIN_TRY

    load();

    __END_CATCH
}

void STRBalanceInfoManager::load()

{
    __BEGIN_TRY

    int maxLevel = 0;
    if (!defaultBalanceInfoRepository().loadMaxLevel(LEVEL_EXP_TABLE_STR, maxLevel)) {
        throw Error("There is no data in STRBalanceInfo Table");
    }

    // Size the table from the highest level.
    m_STRBalanceCount = maxLevel + 1;

    Assert(m_STRBalanceCount > 0);

    m_STRBalanceInfoList = new STRBalanceInfo*[m_STRBalanceCount];
    Assert(m_STRBalanceInfoList != NULL);

    // Clear the array.
    for (uint i = 0; i < m_STRBalanceCount; i++)
        m_STRBalanceInfoList[i] = NULL;

    // Fill in the rows.
    vector<LevelExpRow> rows = defaultBalanceInfoRepository().loadLevels(LEVEL_EXP_TABLE_STR);
    for (size_t r = 0; r < rows.size(); r++) {
        STRBalanceInfo* pSTRBalanceInfo = new STRBalanceInfo();
        Assert(pSTRBalanceInfo != NULL);

        pSTRBalanceInfo->setLevel(rows[r].level);
        pSTRBalanceInfo->setGoalExp(rows[r].goalExp);
        pSTRBalanceInfo->setAccumExp(rows[r].accumExp);

        addSTRBalanceInfo(pSTRBalanceInfo);
    }

    __END_CATCH
}

void STRBalanceInfoManager::addSTRBalanceInfo(STRBalanceInfo* pSTRBalanceInfo)

{
    __BEGIN_TRY

    Assert(pSTRBalanceInfo != NULL);
    Assert(m_STRBalanceInfoList[pSTRBalanceInfo->getLevel()] == NULL);

    m_STRBalanceInfoList[pSTRBalanceInfo->getLevel()] = pSTRBalanceInfo;

    __END_CATCH
}

STRBalanceInfo* STRBalanceInfoManager::getSTRBalanceInfo(uint value) const

{
    __BEGIN_TRY

    if (value >= m_STRBalanceCount || m_STRBalanceInfoList[value] == NULL) {
        filelog("AttrError.log", "STR 능력치 초과 또는 미만");
        throw InvalidProtocolException();
    }

    return m_STRBalanceInfoList[value];

    __END_CATCH
}

string STRBalanceInfoManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "STRBalanceInfoManager(";

    for (uint i = 0; i < m_STRBalanceCount; i++) {
        if (m_STRBalanceInfoList[i] != NULL) {
            msg << m_STRBalanceInfoList[i]->toString();
        } else {
            msg << "NULL";
        }
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// class DEXBalanceInfo member methods
////////////////////////////////////////////////////////////////////////////////

DEXBalanceInfo::DEXBalanceInfo()

    {__BEGIN_TRY __END_CATCH}

DEXBalanceInfo::~DEXBalanceInfo()

    {__BEGIN_TRY __END_CATCH}

string DEXBalanceInfo::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "DEXBalanceInfo (" << " Level : " << (int)m_Level << " GoalExp : " << (int)m_GoalExp
        << " AccumExp : " << (int)m_AccumExp << ")";

    return msg.toString();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// class DEXBalanceInfoManager member methods
////////////////////////////////////////////////////////////////////////////////

DEXBalanceInfoManager::DEXBalanceInfoManager()

{
    __BEGIN_TRY

    m_DEXBalanceCount = 0;
    m_DEXBalanceInfoList = NULL;

    __END_CATCH
}

DEXBalanceInfoManager::~DEXBalanceInfoManager()

{
    __BEGIN_TRY

    if (m_DEXBalanceInfoList != NULL) {
        for (uint i = 0; i < m_DEXBalanceCount; i++)
            SAFE_DELETE(m_DEXBalanceInfoList[i]);

        SAFE_DELETE_ARRAY(m_DEXBalanceInfoList);
    }

    __END_CATCH
}

void DEXBalanceInfoManager::init()

{
    __BEGIN_TRY

    load();

    __END_CATCH
}

void DEXBalanceInfoManager::load()

{
    __BEGIN_TRY

    int maxLevel = 0;
    if (!defaultBalanceInfoRepository().loadMaxLevel(LEVEL_EXP_TABLE_DEX, maxLevel)) {
        throw Error("There is no data in DEXBalanceInfo Table");
    }

    // Size the table from the highest level.
    m_DEXBalanceCount = maxLevel + 1;

    Assert(m_DEXBalanceCount > 0);

    m_DEXBalanceInfoList = new DEXBalanceInfo*[m_DEXBalanceCount];
    Assert(m_DEXBalanceInfoList != NULL);

    // Clear the array.
    for (uint i = 0; i < m_DEXBalanceCount; i++)
        m_DEXBalanceInfoList[i] = NULL;

    // Fill in the rows.
    vector<LevelExpRow> rows = defaultBalanceInfoRepository().loadLevels(LEVEL_EXP_TABLE_DEX);
    for (size_t r = 0; r < rows.size(); r++) {
        DEXBalanceInfo* pDEXBalanceInfo = new DEXBalanceInfo();
        Assert(pDEXBalanceInfo != NULL);

        pDEXBalanceInfo->setLevel(rows[r].level);
        pDEXBalanceInfo->setGoalExp(rows[r].goalExp);
        pDEXBalanceInfo->setAccumExp(rows[r].accumExp);

        addDEXBalanceInfo(pDEXBalanceInfo);
    }

    __END_CATCH
}

void DEXBalanceInfoManager::addDEXBalanceInfo(DEXBalanceInfo* pDEXBalanceInfo)

{
    __BEGIN_TRY

    Assert(pDEXBalanceInfo != NULL);
    Assert(m_DEXBalanceInfoList[pDEXBalanceInfo->getLevel()] == NULL);

    m_DEXBalanceInfoList[pDEXBalanceInfo->getLevel()] = pDEXBalanceInfo;

    __END_CATCH
}

DEXBalanceInfo* DEXBalanceInfoManager::getDEXBalanceInfo(uint value) const

{
    __BEGIN_TRY

    if (value >= m_DEXBalanceCount || m_DEXBalanceInfoList[value] == NULL) {
        filelog("AttrError.log", "DEX 능력치 초과 또는 미만");
        throw InvalidProtocolException();
    }

    return m_DEXBalanceInfoList[value];

    __END_CATCH
}

string DEXBalanceInfoManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "DEXBalanceInfoManager(";

    for (uint i = 0; i < m_DEXBalanceCount; i++) {
        if (m_DEXBalanceInfoList[i] != NULL) {
            msg << m_DEXBalanceInfoList[i]->toString();
        } else {
            msg << "NULL";
        }
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// class INTBalanceInfo member methods
////////////////////////////////////////////////////////////////////////////////

INTBalanceInfo::INTBalanceInfo()

    {__BEGIN_TRY __END_CATCH}

INTBalanceInfo::~INTBalanceInfo()

    {__BEGIN_TRY __END_CATCH}

string INTBalanceInfo::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "INTBalanceInfo (" << " Level : " << (int)m_Level << " GoalExp : " << (int)m_GoalExp
        << " AccumExp : " << (int)m_AccumExp << ")";

    return msg.toString();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// class INTBalanceInfoManager member methods
////////////////////////////////////////////////////////////////////////////////

INTBalanceInfoManager::INTBalanceInfoManager()

{
    __BEGIN_TRY

    m_INTBalanceCount = 0;
    m_INTBalanceInfoList = NULL;

    __END_CATCH
}

INTBalanceInfoManager::~INTBalanceInfoManager()

{
    __BEGIN_TRY

    if (m_INTBalanceInfoList != NULL) {
        for (uint i = 0; i < m_INTBalanceCount; i++)
            SAFE_DELETE(m_INTBalanceInfoList[i]);

        SAFE_DELETE_ARRAY(m_INTBalanceInfoList);
    }

    __END_CATCH
}

void INTBalanceInfoManager::init()

{
    __BEGIN_TRY

    load();

    __END_CATCH
}

void INTBalanceInfoManager::load()

{
    __BEGIN_TRY

    int maxLevel = 0;
    if (!defaultBalanceInfoRepository().loadMaxLevel(LEVEL_EXP_TABLE_INT, maxLevel)) {
        throw Error("There is no data in INTBalanceInfo Table");
    }

    // Size the table from the highest level.
    m_INTBalanceCount = maxLevel + 1;

    Assert(m_INTBalanceCount > 0);

    m_INTBalanceInfoList = new INTBalanceInfo*[m_INTBalanceCount];

    Assert(m_INTBalanceInfoList != NULL);

    // Clear the array.
    for (uint i = 0; i < m_INTBalanceCount; i++)
        m_INTBalanceInfoList[i] = NULL;

    // Fill in the rows.
    vector<LevelExpRow> rows = defaultBalanceInfoRepository().loadLevels(LEVEL_EXP_TABLE_INT);
    for (size_t r = 0; r < rows.size(); r++) {
        INTBalanceInfo* pINTBalanceInfo = new INTBalanceInfo();
        Assert(pINTBalanceInfo != NULL);

        pINTBalanceInfo->setLevel(rows[r].level);
        pINTBalanceInfo->setGoalExp(rows[r].goalExp);
        pINTBalanceInfo->setAccumExp(rows[r].accumExp);

        addINTBalanceInfo(pINTBalanceInfo);
    }

    __END_CATCH
}

void INTBalanceInfoManager::addINTBalanceInfo(INTBalanceInfo* pINTBalanceInfo)

{
    __BEGIN_TRY

    Assert(pINTBalanceInfo != NULL);

    if (m_INTBalanceInfoList[pINTBalanceInfo->getLevel()] != NULL)
        throw DuplicatedException();

    m_INTBalanceInfoList[pINTBalanceInfo->getLevel()] = pINTBalanceInfo;

    __END_CATCH
}

INTBalanceInfo* INTBalanceInfoManager::getINTBalanceInfo(uint value) const

{
    __BEGIN_TRY

    if (value >= m_INTBalanceCount || m_INTBalanceInfoList[value] == NULL) {
        filelog("AttrError.log", "INT 능력치 초과 또는 미만");
        throw InvalidProtocolException();
    }


    return m_INTBalanceInfoList[value];

    __END_CATCH
}

string INTBalanceInfoManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "INTBalanceInfoManager(";

    for (uint i = 0; i < m_INTBalanceCount; i++) {
        if (m_INTBalanceInfoList[i] != NULL) {
            msg << m_INTBalanceInfoList[i]->toString();
        } else {
            msg << "NULL";
        }
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}
