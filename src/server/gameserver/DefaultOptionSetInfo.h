//////////////////////////////////////////////////////////////////////////////
// Filename    : DefaultOptionSetInfo.h
// Description :
//////////////////////////////////////////////////////////////////////////////


#ifndef __DEFAULT_OPTION_SET_INFO_H__
#define __DEFAULT_OPTION_SET_INFO_H__

#include <list>

#include <unordered_map>

#include "Exception.h"
#include "OptionInfo.h"
#include "Types.h"


enum DefaultOptionSetTypes {
    DEFAULT_OPTION_SET_FAMILY_PAY, // 0. For the family billing plan. All resistances +7

    DEFAULT_OPTION_SET_MAX
};

class DefaultOptionSetInfo {
public:
    DefaultOptionSetInfo();
    ~DefaultOptionSetInfo();

public:
    void setType(DefaultOptionSetType_t type) {
        m_Type = type;
    }
    DefaultOptionSetType_t getType() const {
        return m_Type;
    }

    void setOptionTypeList(const list<OptionType_t>& optionTypes) {
        m_OptionTypes = optionTypes;
    }
    const list<OptionType_t>& getOptionTypeList() const {
        return m_OptionTypes;
    }

private:
    DefaultOptionSetType_t m_Type;
    list<OptionType_t> m_OptionTypes;
};

typedef unordered_map<DefaultOptionSetType_t, DefaultOptionSetInfo*> HashMapDefaultOptionSetInfo;
typedef HashMapDefaultOptionSetInfo::iterator HashMapDefaultOptionSetInfoItor;
typedef HashMapDefaultOptionSetInfo::const_iterator HashMapDefaultOptionSetInfoConstItor;


class DefaultOptionSetInfoManager {
public:
    DefaultOptionSetInfoManager();
    ~DefaultOptionSetInfoManager();

public:
    void load();

    DefaultOptionSetInfo* getDefaultOptionSetInfo(DefaultOptionSetType_t type);
    void addDefaultOptionSetInfo(DefaultOptionSetInfo* pDefaultOptionSetInfo);

private:
    HashMapDefaultOptionSetInfo m_DefaultOptionSetInfos;
};

#endif
