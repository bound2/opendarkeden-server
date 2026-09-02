//--------------------------------------------------------------------
//
// Filename    : SkillDomainInfoManager.cpp
// Written By  : Elca
//
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// include files
//--------------------------------------------------------------------
#include "SkillDomainInfoManager.h"

#include "Assert.h"
#include "Exception.h"
#include "StringStream.h"
#include "repository/BalanceInfoRepository.h"

DomainInfo::DomainInfo()

    {__BEGIN_TRY __END_CATCH}

DomainInfo::~DomainInfo()

    {__BEGIN_TRY __END_CATCH_NO_RETHROW}

//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string DomainInfo::toString() const

{
    __BEGIN_TRY
    StringStream msg;

    msg << "DomainInfo(" << "DomainType : " << (int)m_Type << "Level : " << (int)m_Level
        << "GoalExp: " << (int)m_GoalExp << "AccumExp: " << (int)m_AccumExp << "BestItemType: " << (int)m_BestItemType
        << ")";

    return msg.toString();

    __END_CATCH
}

//--------------------------------------------------------------------
//
// Constructor
//
//--------------------------------------------------------------------
SkillDomainInfoManager::SkillDomainInfoManager()

    {__BEGIN_TRY

         /*
          for (int i = 0 ; i < SKILL_DOMAIN_MAX; i ++) {
              for(int j = 0; j <= 100; i++) {
                  m_DomainInfoLists[i][j] = NULL;
              }
          }
          */

         __END_CATCH}

//--------------------------------------------------------------------
//
// Destructor
//
//--------------------------------------------------------------------
SkillDomainInfoManager::~SkillDomainInfoManager()

{
    __BEGIN_TRY

    for (int i = 0; i < SKILL_DOMAIN_MAX; i++)
        for (int j = 0; j <= 150; j++)
            SAFE_DELETE(m_DomainInfoLists[i][j]);

    __END_CATCH_NO_RETHROW
}

//--------------------------------------------------------------------
//
// SkillDomainInfoManager::init()
//
//--------------------------------------------------------------------
void SkillDomainInfoManager::init()

{
    __BEGIN_TRY

    for (int i = 0; i < SKILL_DOMAIN_MAX; i++) {
        int maxLevel = 0;
        if (!defaultBalanceInfoRepository().loadMaxDomainLevel(i, maxLevel)) {
            cerr << "There is no data in DomainInfo Table" << endl;
            throw Error("There is no data in DomainInfo Table");
        }

        int Count = maxLevel + 1;

        Assert(Count > 0);
        Assert(Count <= 151);

        m_DomainInfoLists[i] = new DomainInfo*[Count];

        for (int j = 0; j < Count; j++)
            m_DomainInfoLists[i][j] = NULL;

        vector<DomainLevelRow> rows = defaultBalanceInfoRepository().loadDomainLevels(i);

        for (size_t r = 0; r < rows.size(); r++) {
            DomainInfo* pDomainInfo = new DomainInfo();

            pDomainInfo->setType(rows[r].domainType);
            pDomainInfo->setLevel(rows[r].level);
            pDomainInfo->setGoalExp(rows[r].goalExp);
            pDomainInfo->setAccumExp(rows[r].accumExp);
            pDomainInfo->setBestItemType(rows[r].bestItemType);
            addDomainInfo(pDomainInfo);
        }
    }

    __END_CATCH
}

//--------------------------------------------------------------------------------
// get item info
//--------------------------------------------------------------------------------
DomainInfo* SkillDomainInfoManager::getDomainInfo(SkillDomain DomainType, Level_t Level) const {
    __BEGIN_TRY

    Assert(DomainType < SKILL_DOMAIN_MAX);
    Assert(Level < 151);
    Assert(m_DomainInfoLists[DomainType] != NULL);
    Assert(m_DomainInfoLists[DomainType][Level] != NULL);

    return m_DomainInfoLists[DomainType][Level];

    __END_CATCH
}

//--------------------------------------------------------------------------------
// get item info
//--------------------------------------------------------------------------------
void SkillDomainInfoManager::addDomainInfo(DomainInfo* pDomainInfo) const

{
    __BEGIN_TRY

    SkillDomainType_t DomainType = pDomainInfo->getType();
    Level_t Level = pDomainInfo->getLevel();

    Assert(DomainType < SKILL_DOMAIN_MAX);
    Assert(Level < 151);
    Assert(m_DomainInfoLists[DomainType][Level] == NULL);

    m_DomainInfoLists[DomainType][Level] = pDomainInfo;

    __END_CATCH
}

//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string SkillDomainInfoManager::toString() const

{
    __BEGIN_TRY
    StringStream msg;

    msg << "SkillDomainInfoManager(";

    for (uint i = 0; i < SKILL_DOMAIN_MAX; i++) {
        for (int j = 0; j <= 150; j++) {
            if (m_DomainInfoLists[i][j] == NULL) {
                msg << "NULL";
            } else {
                msg << "DomainInfo[" << (int)i << "][" << (int)j << "](" << m_DomainInfoLists[i][j]->toString();
            }
            msg << "\n";
        }
    }

    msg << ")";

    return msg.toString();
    __END_CATCH
}

// global variable declaration
SkillDomainInfoManager* g_pSkillDomainInfoManager = NULL;
