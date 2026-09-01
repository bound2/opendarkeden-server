#include "PetExpInfo.h"

#include "repository/BalanceInfoRepository.h"

void PetExpInfoManager::clear() {
    vector<PetExpInfo*>::iterator itr = m_PetExpInfos.begin();
    vector<PetExpInfo*>::iterator endItr = m_PetExpInfos.end();

    for (; itr != endItr; ++itr) {
        SAFE_DELETE(*itr);
    }

    m_PetExpInfos.clear();
}

void PetExpInfoManager::load() {
    vector<PetExpRow> rows = defaultBalanceInfoRepository().loadPetExp();

    for (size_t r = 0; r < rows.size(); r++) {
        PetLevel_t PetLevel = rows[r].petLevel;
        PetExp_t PetExp = rows[r].petAccumExp;

        m_PetExpInfos[PetLevel] = new PetExpInfo(PetLevel, PetExp);
    }
}

bool PetExpInfoManager::canLevelUp(PetLevel_t level, PetExp_t exp) {
    if (level >= PetMaxLevel)
        return false;
    PetExpInfo* pPetExpInfo = m_PetExpInfos[level];
    if (pPetExpInfo == NULL)
        return false;

    return pPetExpInfo->getPetGoalExp() <= exp;
}
