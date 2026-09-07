///////////////////////////////////////////////////////////
// Filename : StringPool.cpp
///////////////////////////////////////////////////////////

#include "StringPool.h"

#include "repository/SharedConfigRepository.h"

StringPool::StringPool() noexcept(false){__BEGIN_TRY __END_CATCH}

StringPool::~StringPool() noexcept {
    try {
        clear();
    } catch (...) {
        // destructor must not throw
    }
}

void StringPool::clear() noexcept(false) {
    __BEGIN_TRY

    m_Strings.clear();

    __END_CATCH
}

void StringPool::load() noexcept(false) {
    __BEGIN_TRY

    clear();

    vector<SharedStringRow> rows = defaultSharedConfigRepository().loadStrings();

    for (size_t i = 0; i < rows.size(); i++) {
        uint strID = rows[i].id;
        string str = rows[i].text;

        addString(strID, str);
    }

    __END_CATCH
}

void StringPool::addString(uint strID, string sString) noexcept(false) {
    __BEGIN_TRY

    StringHashMapItor itr = m_Strings.find(strID);

    if (itr != m_Strings.end()) {
        throw DuplicatedException("StringPool::addString()");
    }

    m_Strings[strID] = sString;

    __END_CATCH
}

string StringPool::getString(uint strID) noexcept(false) {
    __BEGIN_TRY

    StringHashMapItor itr = m_Strings.find(strID);

    if (itr == m_Strings.end()) {
        throw NoSuchElementException("StringPool::getStrind()");
    }

    return itr->second;

    __END_CATCH
}

const char* StringPool::c_str(uint strID) noexcept(false) {
    __BEGIN_TRY

    StringHashMapItor itr = m_Strings.find(strID);

    if (itr == m_Strings.end()) {
        throw NoSuchElementException("StringPool::getStrind()");
    }

    return itr->second.c_str();

    __END_CATCH
}

StringPool* g_pStringPool = NULL;
