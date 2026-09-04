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
// global varible
//////////////////////////////////////////////////////////////////////////////
MonsterNameManager* g_pMonsterNameManager = NULL;

//////////////////////////////////////////////////////////////////////////////
// class MonsterNameManager member methods
//////////////////////////////////////////////////////////////////////////////

MonsterNameManager::MonsterNameManager()

{
    __BEGIN_TRY

    m_pFirstName = NULL;
    m_pMiddleName = NULL;
    m_pLastName = NULL;
    m_nFirstNameCount = 0;
    m_nMiddleNameCount = 0;
    m_nLastNameCount = 0;

    __END_CATCH
}

MonsterNameManager::~MonsterNameManager()

{
    __BEGIN_TRY

    SAFE_DELETE_ARRAY(m_pFirstName);
    SAFE_DELETE_ARRAY(m_pMiddleName);
    SAFE_DELETE_ARRAY(m_pLastName);

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
        throw "MonsterNameManager::init() : No data exist on FirstNameInfo";
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
        throw "MonsterNameManager::init() : No data exist on MiddleNameInfo";
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
        throw "MonsterNameManager::init() : No data exist on LastNameInfo";
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
        throw "MonsterNameManager::init() : no data exist on EventMiddleNameInfo";
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
    bool bContinue = true;
    int trial = 0;

    while (bContinue && trial++ < 300) {
        short nLastNameIndex = -1;

        nLastNameIndex = rand() % m_nEventLastNameCount;

        if (nLastNameIndex != -1)
            Name += m_pEventLastName[nLastNameIndex];

        return Name;
    }

    // trial이 300번을 초과하면, 제대로 된 이름을 찾지 못했다는
    // 말이니까, 아무 이름이나 붙여준다.
    if (Name == "")
        Name == "무명씨";

    return Name;

    __END_CATCH
}


string MonsterNameManager::getRandomName(Monster* pMonster)

{
    __BEGIN_TRY

    if (pMonster == NULL)
        return "";

    // const MonsterInfo* pInfo = g_pMonsterInfoManager->getMonsterInfo(pMonster->getMonsterType());

    // Level_t MonsterLevel = pInfo->getLevel();
    string Name = "";
    bool bContinue = true;
    int trial = 0;

    while (bContinue && trial++ < 300) {
        short nFirstNameIndex = -1;
        short nMiddleNameIndex = -1;
        short nLastNameIndex = -1;

        nMiddleNameIndex = rand() % m_nMiddleNameCount;

        /*
        if (0 < MonsterLevel && MonsterLevel <= 33)
        {
            // 하급 뱀파는 라스트 네임만 붙는다.
            nFirstNameIndex  = -1;
            nMiddleNameIndex = -1;
            nLastNameIndex   = rand()%m_nLastNameCount;
        }
        else if (33 < MonsterLevel && MonsterLevel <= 66)
        {
            // 중급 뱀파는 퍼스트와 라스트 네임만 붙는다.
            nFirstNameIndex  = rand()%m_nFirstNameCount;
            nMiddleNameIndex = -1;
            nLastNameIndex   = rand()%m_nLastNameCount;
        }
        else
        {
            // 고급 뱀파는 모든 이름이 다 붙는다.
            nFirstNameIndex  = rand()%m_nFirstNameCount;
            nMiddleNameIndex = rand()%m_nMiddleNameCount;
            nLastNameIndex   = rand()%m_nLastNameCount;
        }
        */

        /*
         * 존마다 유니크한 이름은 나중에다 붙는다네...
         *
        ulonglong NameKey = 0;

        NameKey |= nFirstNameIndex  < 32;
        NameKey |= nMiddleNameIndex < 16;
        NameKey |= nLastNameIndex       ;

        unordered_map<ulonglong, string>::iterator itr = m_UsedName.find(NameKey);

        if (itr == m_UsedName.end())
        {
            if (nFirstNameIndex != -1)  Name += m_pFirstName[nFirstNameIndex] + " ";
            if (nMiddleNameIndex != -1) Name += m_pMiddleName[nMiddleNameIndex] + " ";
            if (nLastNameIndex != -1)   Name += m_pLastName[nLastNameIndex];

            bContinue = false;
        }
        */

        if (nFirstNameIndex != -1)
            Name += m_pFirstName[nFirstNameIndex] + " ";
        if (nMiddleNameIndex != -1)
            Name += m_pMiddleName[nMiddleNameIndex] + " ";
        if (nLastNameIndex != -1)
            Name += m_pLastName[nLastNameIndex];

        return Name;
    }

    // trial이 300번을 초과하면, 제대로 된 이름을 찾지 못했다는
    // 말이니까, 아무 이름이나 붙여준다.
    if (Name == "")
        Name == "무명씨";

    return Name;

    __END_CATCH
}
