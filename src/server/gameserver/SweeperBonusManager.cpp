//////////////////////////////////////////////////////////////////////////////
// Filename    : SweeperBonusManager.cpp
// Written By  : beowulf
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "SweeperBonusManager.h"

#include "GCSweeperBonusInfo.h"
#include "LevelWarManager.h"
#include "LevelWarZoneInfoManager.h"
#include "SweeperBonus.h"
#include "Zone.h"
#include "ZoneUtil.h"
#include "repository/WarInfoRepository.h"

//////////////////////////////////////////////////////////////////////////////
// class SweeperBonusManager member methods
//////////////////////////////////////////////////////////////////////////////

SweeperBonusManager::SweeperBonusManager()

{
    __BEGIN_TRY

    m_Count = 0;

    __END_CATCH
}

SweeperBonusManager::~SweeperBonusManager()

{
    __BEGIN_TRY

    clear();

    __END_CATCH_NO_RETHROW
}

void SweeperBonusManager::init()

{
    __BEGIN_TRY

    load();

    __END_CATCH
}

void SweeperBonusManager::clear()

{
    __BEGIN_TRY

    SweeperBonusHashMapItor itr = m_SweeperBonuses.begin();
    for (; itr != m_SweeperBonuses.end(); itr++) {
        SAFE_DELETE(itr->second);
    }

    m_SweeperBonuses.clear();

    __END_CATCH
}

void SweeperBonusManager::load()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    clear();

    int maxType = 0;
    if (!defaultWarInfoRepository().loadMaxSweeperBonusType(maxType)) {
        throw Error("There is no data in SweeperBonusInfo Table");
    }

    m_Count = maxType + 1;

    Assert(m_Count > 0);

    vector<SweeperBonusRow> rows = defaultWarInfoRepository().loadSweeperBonuses();

    for (size_t r = 0; r < rows.size(); r++) {
        const SweeperBonusRow& row = rows[r];
        SweeperBonus* pSweeperBonus = new SweeperBonus();

        pSweeperBonus->setType(row.type);
        pSweeperBonus->setName(row.name);
        pSweeperBonus->setOptionTypeList(row.optionList);
        pSweeperBonus->setRace(row.ownerRace);
        pSweeperBonus->setLevel(row.level);

        addSweeperBonus(pSweeperBonus);
    }

    __END_DEBUG
    __END_CATCH
}

void SweeperBonusManager::reloadOwner(int level)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    int maxType = 0;
    if (!defaultWarInfoRepository().loadMaxSweeperBonusType(maxType)) {
        throw Error("There is no data in SweeperBonusInfo Table");
    }

    m_Count = maxType + 1;

    Assert(m_Count > 0);

    vector<SweeperBonusOwnerRow> owners = defaultWarInfoRepository().loadSweeperBonusOwners(level);

    for (size_t r = 0; r < owners.size(); r++) {
        SweeperBonusType_t type = owners[r].type;

        SweeperBonusHashMapItor itr = m_SweeperBonuses.find(type);

        if (itr != m_SweeperBonuses.end()) {
            itr->second->setRace(owners[r].ownerRace);
        }
    }

    __END_DEBUG
    __END_CATCH
}

void SweeperBonusManager::save()

{
    __BEGIN_TRY

    throw UnsupportedError(__PRETTY_FUNCTION__);

    __END_CATCH
}

SweeperBonus* SweeperBonusManager::getSweeperBonus(SweeperBonusType_t sweeperBonusType) const {
    __BEGIN_TRY

    SweeperBonusHashMapConstItor itr = m_SweeperBonuses.find(sweeperBonusType);

    if (itr == m_SweeperBonuses.end()) {
        cerr << "SweeperBonusManager::getSweeperBonus() : no such element" << endl;
        throw NoSuchElementException();
    }

    return itr->second;

    __END_CATCH
}

void SweeperBonusManager::addSweeperBonus(SweeperBonus* pSweeperBonus)

{
    __BEGIN_TRY

    Assert(pSweeperBonus != NULL);

    SweeperBonusHashMapConstItor itr = m_SweeperBonuses.find(pSweeperBonus->getType());
    if (itr != m_SweeperBonuses.end()) {
        throw DuplicatedException();
    }

    m_SweeperBonuses[pSweeperBonus->getType()] = pSweeperBonus;

    __END_CATCH
}


bool SweeperBonusManager::isAble(ZoneID_t zoneID) const {
    __BEGIN_TRY

    ZoneID_t levelWarZoneID;
    if (g_pLevelWarZoneInfoManager->getLevelWarZoneID(zoneID, levelWarZoneID)) {
        Zone* pZone = getZoneByZoneID(levelWarZoneID);
        if (pZone == NULL)
            return false;

        LevelWarManager* pLevelWarManager = pZone->getLevelWarManager();
        if (pLevelWarManager == NULL)
            return false;

        return !pLevelWarManager->hasWar();
    }

    return false;

    __END_CATCH
}

void SweeperBonusManager::setSweeperBonusRace(SweeperBonusType_t sweeperBonusType, Race_t race)

{
    __BEGIN_TRY

    getSweeperBonus(sweeperBonusType)->setRace(race);

    __END_CATCH
}

void SweeperBonusManager::makeSweeperBonusInfo(GCSweeperBonusInfo& gcSweeperBonusInfo)

{
    __BEGIN_TRY

    SweeperBonusHashMapConstItor itr = m_SweeperBonuses.begin();
    for (; itr != m_SweeperBonuses.end(); itr++) {
        SweeperBonusInfo* pInfo = new SweeperBonusInfo();
        SweeperBonus* pBonus = itr->second;

        pInfo->setType(pBonus->getType());
        pInfo->setRace(pBonus->getRace());
        pInfo->setOptionType(pBonus->getOptionTypeList());

        gcSweeperBonusInfo.addSweeperBonusInfo(pInfo);
    }

    __END_CATCH
}

void SweeperBonusManager::makeVoidSweeperBonusInfo(GCSweeperBonusInfo& gcSweeperBonusInfo)

{
    __BEGIN_TRY

    SweeperBonusHashMapConstItor itr = m_SweeperBonuses.begin();
    for (; itr != m_SweeperBonuses.end(); itr++) {
        SweeperBonusInfo* pInfo = new SweeperBonusInfo();
        SweeperBonus* pBonus = itr->second;

        pInfo->setType(pBonus->getType());
        pInfo->setRace(3);
        pInfo->setOptionType(pBonus->getOptionTypeList());

        gcSweeperBonusInfo.addSweeperBonusInfo(pInfo);
    }

    __END_CATCH
}

string SweeperBonusManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "SweeperBonusManager(\n";

    SweeperBonusHashMapConstItor itr = m_SweeperBonuses.begin();
    for (; itr != m_SweeperBonuses.end(); itr++) {
        msg << itr->second->toString() << ",";
    }

    return msg.toString();

    __END_CATCH
}

// Global Variable definition
SweeperBonusManager* g_pSweeperBonusManager = NULL;
