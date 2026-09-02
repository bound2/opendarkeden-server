//////////////////////////////////////////////////////////////////////////////
// Filename    : BloodBibleBonusManager.cpp
// Written By  : beowulf
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "BloodBibleBonusManager.h"

#include "BloodBibleBonus.h"
#include "GCHolyLandBonusInfo.h"
#include "repository/GameInfoRepository.h"

//////////////////////////////////////////////////////////////////////////////
// class BloodBibleBonusManager member methods
//////////////////////////////////////////////////////////////////////////////

BloodBibleBonusManager::BloodBibleBonusManager()

{
    __BEGIN_TRY

    m_Count = 0;

    __END_CATCH
}

BloodBibleBonusManager::~BloodBibleBonusManager()

{
    __BEGIN_TRY

    clear();

    __END_CATCH_NO_RETHROW
}

void BloodBibleBonusManager::init()

{
    __BEGIN_TRY

    load();

    __END_CATCH
}

void BloodBibleBonusManager::clear()

{
    __BEGIN_TRY

    BloodBibleBonusHashMapItor itr = m_BloodBibleBonuses.begin();
    for (; itr != m_BloodBibleBonuses.end(); itr++) {
        SAFE_DELETE(itr->second);
    }

    m_BloodBibleBonuses.clear();

    __END_CATCH
}

void BloodBibleBonusManager::load()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    clear();

    int maxType = 0;
    if (!defaultGameInfoRepository().loadMaxBloodBibleBonusType(maxType)) {
        throw Error("There is no data in BloodBibleBonusInfo Table");
    }

    m_Count = maxType + 1;

    Assert(m_Count > 0);

    vector<BloodBibleBonusRow> rows = defaultGameInfoRepository().loadBloodBibleBonuses();

    for (size_t r = 0; r < rows.size(); r++) {
        BloodBibleBonus* pBloodBibleBonus = new BloodBibleBonus();

        pBloodBibleBonus->setType(rows[r].type);
        pBloodBibleBonus->setName(rows[r].name);
        pBloodBibleBonus->setOptionTypeList(rows[r].optionList);
        pBloodBibleBonus->setRace(0);

        addBloodBibleBonus(pBloodBibleBonus);
    }

    __END_DEBUG
    __END_CATCH
}

void BloodBibleBonusManager::save()

{
    __BEGIN_TRY

    throw UnsupportedError(__PRETTY_FUNCTION__);

    __END_CATCH
}

BloodBibleBonus* BloodBibleBonusManager::getBloodBibleBonus(BloodBibleBonusType_t bloodBibleBonusType) const {
    __BEGIN_TRY

    BloodBibleBonusHashMapConstItor itr = m_BloodBibleBonuses.find(bloodBibleBonusType);

    if (itr == m_BloodBibleBonuses.end()) {
        cerr << "BloodBibleBonusManager::getBloodBibleBonus() : no such element" << endl;
        throw NoSuchElementException();
    }

    return itr->second;

    __END_CATCH
}

void BloodBibleBonusManager::addBloodBibleBonus(BloodBibleBonus* pBloodBibleBonus)

{
    __BEGIN_TRY

    Assert(pBloodBibleBonus != NULL);

    BloodBibleBonusHashMapConstItor itr = m_BloodBibleBonuses.find(pBloodBibleBonus->getType());
    if (itr != m_BloodBibleBonuses.end()) {
        throw DuplicatedException();
    }

    m_BloodBibleBonuses[pBloodBibleBonus->getType()] = pBloodBibleBonus;

    __END_CATCH
}

void BloodBibleBonusManager::setBloodBibleBonusRace(BloodBibleBonusType_t bloodBibleBonusType, Race_t race)

{
    __BEGIN_TRY

    getBloodBibleBonus(bloodBibleBonusType)->setRace(race);

    __END_CATCH
}

void BloodBibleBonusManager::makeHolyLandBonusInfo(GCHolyLandBonusInfo& gcHolyLandBonusInfo)

    {__BEGIN_TRY

         /*	BloodBibleBonusHashMapConstItor itr = m_BloodBibleBonuses.begin();
              for ( ; itr != m_BloodBibleBonuses.end(); itr++ )
              {
                  BloodBibleBonusInfo* pInfo = new BloodBibleBonusInfo();
                  BloodBibleBonus* pBonus = itr->second;

                  pInfo->setType( pBonus->getType() );
                  pInfo->setRace( pBonus->getRace() );
                  pInfo->setOptionType( pBonus->getOptionTypeList() );

                  gcHolyLandBonusInfo.addBloodBibleBonusInfo( pInfo );
              }*/

         __END_CATCH}

string BloodBibleBonusManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "BloodBibleBonusManager(\n";

    BloodBibleBonusHashMapConstItor itr = m_BloodBibleBonuses.begin();
    for (; itr != m_BloodBibleBonuses.end(); itr++) {
        msg << itr->second->toString() << ",";
    }

    return msg.toString();

    __END_CATCH
}

// Global Variable definition
BloodBibleBonusManager* g_pBloodBibleBonusManager = NULL;
