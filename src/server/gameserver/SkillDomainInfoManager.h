//////////////////////////////////////////////////////////////////////////////
// Filename    : SkillDomainInfoManager.h
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_DOMAIN_INFO_MANAGER_H__
#define __SKILL_DOMAIN_INFO_MANAGER_H__

#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class DomainInfo
//////////////////////////////////////////////////////////////////////////////

class DomainInfo {
public:
    DomainInfo();
    ~DomainInfo();

public:
    SkillDomainType_t getType() const {
        return m_Type;
    }
    void setType(SkillDomainType_t type) {
        m_Type = type;
    }

    Level_t getLevel() const {
        return m_Level;
    }
    void setLevel(Level_t level) {
        m_Level = level;
    }

    uint getGoalExp() const {
        return m_GoalExp;
    }
    void setGoalExp(Exp_t exp) {
        m_GoalExp = exp;
    }

    uint getAccumExp() const {
        return m_AccumExp;
    }
    void setAccumExp(Exp_t exp) {
        m_AccumExp = exp;
    }

    ItemType_t getBestItemType() const {
        return m_BestItemType;
    }
    void setBestItemType(ItemType_t it) {
        m_BestItemType = it;
    }

    string toString() const;

private:
    // Type of the skill domain
    SkillDomainType_t m_Type;

    // Stage (this skill's depth in the skill tree)
    Level_t m_Level;

    // Target experience
    Exp_t m_GoalExp;

    // Accumulated experience
    Exp_t m_AccumExp;

    // Suitable weapon tier
    ItemType_t m_BestItemType;
};

//--------------------------------------------------------------------
//
// Class DomainInfoManager
//
//--------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Class SkillDomainInfoManager
//////////////////////////////////////////////////////////////////////////////

class SkillDomainInfoManager {
public:
    SkillDomainInfoManager();
    ~SkillDomainInfoManager();

public:
    // initialize manager
    void init();

    // get item info
    DomainInfo* getDomainInfo(SkillDomain DomainType, Level_t Level) const;

    // addDomainInfo
    void addDomainInfo(DomainInfo* pDomainInfo) const;

    // toString for debug
    string toString() const;

private:
    DomainInfo** m_DomainInfoLists[SKILL_DOMAIN_MAX];
};

#endif
