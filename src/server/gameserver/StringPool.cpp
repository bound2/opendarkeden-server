///////////////////////////////////////////////////////////
// Filename : StringPool.cpp
///////////////////////////////////////////////////////////

#include "StringPool.h"

#include "repository/GameInfoRepository.h"

StringPool::StringPool()

    {__BEGIN_TRY

         __END_CATCH}

StringPool::~StringPool()

{
    __BEGIN_TRY

    clear();

    __END_CATCH_NO_RETHROW
}

void StringPool::clear()

{
    __BEGIN_TRY

    m_Strings.clear();

    __END_CATCH
}

void StringPool::load()

{
    __BEGIN_TRY

    clear();

    vector<StringPoolRow> rows = defaultGameInfoRepository().loadStrings();

    for (size_t r = 0; r < rows.size(); r++) {
        uint strID = rows[r].id;
        string str = rows[r].text;

        addString(strID, str);
    }

    __END_CATCH
}

void StringPool::addString(uint strID, string sString) {
    __BEGIN_TRY

    StringHashMapItor itr = m_Strings.find(strID);

    if (itr != m_Strings.end()) {
        throw DuplicatedException("StringPool::addString()");
    }

    m_Strings[strID] = sString;


    __END_CATCH
}

string StringPool::getString(uint strID) {
    __BEGIN_TRY

    StringHashMapItor itr = m_Strings.find(strID);

    if (itr == m_Strings.end()) {
        throw NoSuchElementException("StringPool::getString()");
    }

    return itr->second;

    __END_CATCH
}

const char* StringPool::c_str(uint strID) {
    __BEGIN_TRY

    StringHashMapItor itr = m_Strings.find(strID);

    if (itr == m_Strings.end()) {
        throw NoSuchElementException("StringPool::getString()");
    }

    return itr->second.c_str();

    __END_CATCH
}

StringPool* g_pStringPool = NULL;
