//////////////////////////////////////////////////////////////////////////////
// Filename    : MonsterNameManager.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "MonsterNameManager.h"

#include "Monster.h"
#include "MonsterInfo.h"
#include "repository/GameInfoRepository.h"

//////////////////////////////////////////////////////////////////////////////
// class MonsterNameManager member methods
//////////////////////////////////////////////////////////////////////////////

MonsterNameManager::MonsterNameManager()

{
    __BEGIN_TRY

    m_pFirstName = NULL;
    m_pMiddleName = NULL;
    m_pLastName = NULL;
    m_pEventLastName = NULL;
    m_nFirstNameCount = 0;
    m_nMiddleNameCount = 0;
    m_nLastNameCount = 0;
    m_nEventLastNameCount = 0;

    __END_CATCH
}

MonsterNameManager::~MonsterNameManager()

{
    __BEGIN_TRY

    SAFE_DELETE_ARRAY(m_pFirstName);
    SAFE_DELETE_ARRAY(m_pMiddleName);
    SAFE_DELETE_ARRAY(m_pLastName);
    SAFE_DELETE_ARRAY(m_pEventLastName);

    m_UsedName.clear();

    __END_CATCH_NO_RETHROW
}

void MonsterNameManager::init()

{
    __BEGIN_TRY

    int nCount = 0;

    // Load the first names.
    vector<string> firstNames = defaultGameInfoRepository().loadMonsterNames(MONSTER_NAMES_FIRST_BASIC);
    nCount = firstNames.size();
    if (nCount == 0) {
        cerr << "MonsterNameManager::init() : No data exist on FirstNameInfo" << endl;
        throw Error("MonsterNameManager::init() : No data exist on FirstNameInfo");
    }

    m_nFirstNameCount = nCount;
    m_pFirstName = new string[m_nFirstNameCount];

    for (nCount = 0; nCount < (int)firstNames.size(); nCount++) {
        m_pFirstName[nCount] = firstNames[nCount];
    }

    // Load the middle names.
    vector<string> middleNames = defaultGameInfoRepository().loadMonsterNames(MONSTER_NAMES_MIDDLE_BASIC);
    nCount = middleNames.size();
    if (nCount == 0) {
        cerr << "MonsterNameManager::init() : No data exist on MiddleNameInfo" << endl;
        throw Error("MonsterNameManager::init() : No data exist on MiddleNameInfo");
    }

    m_nMiddleNameCount = nCount;
    m_pMiddleName = new string[m_nMiddleNameCount];

    for (nCount = 0; nCount < (int)middleNames.size(); nCount++) {
        m_pMiddleName[nCount] = middleNames[nCount];
    }

    // Load the last names.
    vector<string> lastNames = defaultGameInfoRepository().loadMonsterNames(MONSTER_NAMES_LAST_BASIC);
    nCount = lastNames.size();
    if (nCount == 0) {
        cerr << "MonsterNameManager::init() : No data exist on LastNameInfo" << endl;
        throw Error("MonsterNameManager::init() : No data exist on LastNameInfo");
    }

    m_nLastNameCount = nCount;
    m_pLastName = new string[m_nLastNameCount];

    for (nCount = 0; nCount < (int)lastNames.size(); nCount++) {
        m_pLastName[nCount] = lastNames[nCount];
    }


    /////////////////////////////////////////////////////////////////////////////////////////
    // Event monsters get their own array of last names.
    ////////////////////////////////////////////////////////////////////////////////////////
    vector<string> eventLastNames = defaultGameInfoRepository().loadMonsterNames(MONSTER_NAMES_LAST_EVENT);
    nCount = eventLastNames.size();
    if (nCount == 0) {
        cerr << "MonsterNameManager::init() : no data exist on EventMiddleNameInfo" << endl;
        throw Error("MonsterNameManager::init() : no data exist on EventMiddleNameInfo");
    }

    m_nEventLastNameCount = nCount;
    m_pEventLastName = new string[m_nEventLastNameCount];

    for (nCount = 0; nCount < (int)eventLastNames.size(); nCount++) {
        m_pEventLastName[nCount] = eventLastNames[nCount];
    }

    __END_CATCH
}


string MonsterNameManager::getRandomName(Monster* pMonster, bool event)

{
    __BEGIN_TRY

    if (pMonster == NULL)
        return "";

    string Name = "";
    int trial = 0;

    // The name is the event part alone, drawn again while the row that
    // came up is empty.
    while (Name == "" && trial++ < 300) {
        short nLastNameIndex = rand() % m_nEventLastNameCount;

        Name = m_pEventLastName[nLastNameIndex];
    }

    // If trial goes over 300 no proper name was found,
    // so give it an arbitrary name.
    if (Name == "")
        Name = "무명씨";

    return Name;

    __END_CATCH
}


string MonsterNameManager::getRandomName(Monster* pMonster)

{
    __BEGIN_TRY

    if (pMonster == NULL)
        return "";


    // Level_t MonsterLevel = pInfo->getLevel();
    string Name = "";
    int trial = 0;

    // The name is the middle part alone, drawn again while the row that
    // came up is empty.
    while (Name == "" && trial++ < 300) {
        short nMiddleNameIndex = rand() % m_nMiddleNameCount;

        Name = m_pMiddleName[nMiddleNameIndex];
    }

    // If trial goes over 300 no proper name was found,
    // so give it an arbitrary name.
    if (Name == "")
        Name = "무명씨";

    return Name;

    __END_CATCH
}
