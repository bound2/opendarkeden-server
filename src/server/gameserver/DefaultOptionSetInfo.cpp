//////////////////////////////////////////////////////////////////////////////
// Filename    : DefaultOptionSetInfo.cpp
// Written By  : 배재형
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "DefaultOptionSetInfo.h"

#include "ItemUtil.h"
#include "repository/GameInfoRepository.h"


//////////////////////////////////////////////////////////////////////////////
// DefalutOptionSetInfo class
//////////////////////////////////////////////////////////////////////////////
DefaultOptionSetInfo::DefaultOptionSetInfo() {}

DefaultOptionSetInfo::~DefaultOptionSetInfo() {}


//////////////////////////////////////////////////////////////////////////////
// global varible initialization
//////////////////////////////////////////////////////////////////////////////
DefaultOptionSetInfoManager* g_pDefaultOptionSetInfoManager = NULL;

//////////////////////////////////////////////////////////////////////////////
// DefalutOptionSetInfoManager class
//////////////////////////////////////////////////////////////////////////////
DefaultOptionSetInfoManager::DefaultOptionSetInfoManager() {}

DefaultOptionSetInfoManager::~DefaultOptionSetInfoManager() {}

void DefaultOptionSetInfoManager::load()

{
    vector<DefaultOptionSetRow> rows = defaultGameInfoRepository().loadDefaultOptionSets();

    for (size_t r = 0; r < rows.size(); r++) {
        DefaultOptionSetInfo* pDefaultOptionSetInfo = new DefaultOptionSetInfo();

        pDefaultOptionSetInfo->setType((DefaultOptionSetType_t)rows[r].type);
        string optionField = rows[r].optionList;
        list<OptionType_t> optionList;
        makeOptionList(optionField, optionList);
        pDefaultOptionSetInfo->setOptionTypeList(optionList);

        addDefaultOptionSetInfo(pDefaultOptionSetInfo);
    }
}

DefaultOptionSetInfo* DefaultOptionSetInfoManager::getDefaultOptionSetInfo(DefaultOptionSetType_t type) {
    HashMapDefaultOptionSetInfoItor itr = m_DefaultOptionSetInfos.find(type);

    if (itr == m_DefaultOptionSetInfos.end())
        return NULL;

    return itr->second;
}

void DefaultOptionSetInfoManager::addDefaultOptionSetInfo(DefaultOptionSetInfo* pDefaultOptionSetInfo) {
    HashMapDefaultOptionSetInfoItor itr = m_DefaultOptionSetInfos.find(pDefaultOptionSetInfo->getType());

    if (itr != m_DefaultOptionSetInfos.end()) {
        throw DuplicatedException();
        return;
    }

    m_DefaultOptionSetInfos[pDefaultOptionSetInfo->getType()] = pDefaultOptionSetInfo;
}
