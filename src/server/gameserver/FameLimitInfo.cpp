////////////////////////////////////////////////////////////////////////////////
// Filename    : FameLimitInfo.cpp
// Written By  : beowulf
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "FameLimitInfo.h"

#include <algo.h>

#include "Assert.h"
#include "repository/BalanceInfoRepository.h"

////////////////////////////////////////////////////////////////////////////////
// class FameLimitInfo member methods
////////////////////////////////////////////////////////////////////////////////

FameLimitInfo::FameLimitInfo()

    {__BEGIN_TRY __END_CATCH}

FameLimitInfo::~FameLimitInfo()

    {__BEGIN_TRY __END_CATCH}

string FameLimitInfo::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "FameLimitInfo (" << " Level : " << (int)m_Level << " Fame : " << (int)m_Fame << ")";

    return msg.toString();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// class FameLimitInfoManager member methods
////////////////////////////////////////////////////////////////////////////////

FameLimitInfoManager::FameLimitInfoManager()

    {__BEGIN_TRY

         __END_CATCH}

FameLimitInfoManager::~FameLimitInfoManager()

{
    __BEGIN_TRY

    clear();

    __END_CATCH
}

void FameLimitInfoManager::clear()

{
    __BEGIN_TRY

    if (m_FameLimitInfoList != NULL) {
        for (int i = 0; i < SKILL_DOMAIN_MAX; i++) {
            for (int j = 0; j <= 150; j++)
                SAFE_DELETE(m_FameLimitInfoList[i][j]);
            SAFE_DELETE_ARRAY(m_FameLimitInfoList[i]);
        }
    }

    __END_CATCH
}

void FameLimitInfoManager::init()

{
    __BEGIN_TRY

    load();

    __END_CATCH
}

void FameLimitInfoManager::load()

{
    __BEGIN_TRY

    for (int i = 0; i < SKILL_DOMAIN_MAX; i++) {
        int maxLevel = 0;
        if (!defaultBalanceInfoRepository().loadMaxFameLevel(i, maxLevel)) {
            throw Error("There is no data in FameLimitInfo Table");
        }

        // Size the table from the highest level.
        int count = maxLevel + 1;

        Assert(count > 0);
        Assert(count <= 151);

        m_FameLimitInfoList[i] = new FameLimitInfo*[count];

        for (int j = 0; j < count; j++)
            m_FameLimitInfoList[i][j] = NULL;

        vector<FameLimitRow> rows = defaultBalanceInfoRepository().loadFameLimits(i);
        for (size_t r = 0; r < rows.size(); r++) {
            FameLimitInfo* pFameLimitInfo = new FameLimitInfo();

            pFameLimitInfo->setDomainType(rows[r].domainType);
            pFameLimitInfo->setLevel(rows[r].level);
            pFameLimitInfo->setFame(rows[r].fame);

            addFameLimitInfo(pFameLimitInfo);
        }
    }

    __END_CATCH
}

void FameLimitInfoManager::addFameLimitInfo(FameLimitInfo* pFameLimitInfo)

{
    __BEGIN_TRY

    Assert(pFameLimitInfo != NULL);

    SkillDomainType_t DomainType = pFameLimitInfo->getDomainType();
    uint Level = pFameLimitInfo->getLevel();

    Assert(DomainType < SKILL_DOMAIN_MAX);
    Assert(Level < 151);
    Assert(m_FameLimitInfoList[DomainType][Level] == NULL);

    m_FameLimitInfoList[DomainType][Level] = pFameLimitInfo;

    __END_CATCH
}

FameLimitInfo* FameLimitInfoManager::getFameLimitInfo(SkillDomainType_t DomainType, uint Level) const

{
    __BEGIN_TRY

    Assert(DomainType < SKILL_DOMAIN_MAX);
    Assert(Level < 151);
    Assert(m_FameLimitInfoList[DomainType] != NULL);
    Assert(m_FameLimitInfoList[DomainType][Level] != NULL);

    return m_FameLimitInfoList[DomainType][Level];

    __END_CATCH
}

string FameLimitInfoManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "FameLimitInfoManager(";

    for (int i = 0; i < SKILL_DOMAIN_MAX; i++) {
        for (int j = 0; j <= 150; j++) {
            if (m_FameLimitInfoList[i][j] == NULL) {
                msg << "NULL";
            } else {
                msg << "FameLimitInfo[" << i << "][" << j << "](" << m_FameLimitInfoList[i][j]->toString();
            }
        }
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
// Global Variable initialization
////////////////////////////////////////////////////////////////////////////////
FameLimitInfoManager* g_pFameLimitInfoManager = NULL;
