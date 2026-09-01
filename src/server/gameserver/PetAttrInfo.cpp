#include "PetAttrInfo.h"

#include "PetInfo.h"
#include "repository/BalanceInfoRepository.h"

void PetAttrInfoManager::clear() {
    unordered_map<PetAttr_t, PetAttrInfo*>::iterator itr = m_PetAttrInfoMap.begin();
    unordered_map<PetAttr_t, PetAttrInfo*>::iterator endItr = m_PetAttrInfoMap.end();

    for (; itr != endItr; ++itr) {
        SAFE_DELETE(itr->second);
    }

    m_PetAttrInfoMap.clear();
}

void PetAttrInfoManager::load() {
    vector<PetAttrBalanceRow> balances = defaultBalanceInfoRepository().loadPetAttrBalance();

    for (size_t r = 0; r < balances.size(); r++) {
        PetAttr_t PetAttr = balances[r].petAttr;

        if (m_PetAttrInfoMap[PetAttr] == NULL)
            m_PetAttrInfoMap[PetAttr] = new PetAttrInfo(PetAttr);
        PetLevel_t PetLevel = balances[r].level;

        m_PetAttrInfoMap[PetAttr]->setPetAttrLevel(PetLevel, (PetAttrLevel_t)balances[r].accumAttr);
    }

    vector<PetAttrRatioRow> ratios = defaultBalanceInfoRepository().loadPetAttrRatios();

    for (size_t r = 0; r < ratios.size(); r++) {
        PetAttr_t PetAttr = ratios[r].petAttr;
        if (m_PetAttrInfoMap[PetAttr] != NULL)
            m_PetAttrInfoMap[PetAttr]->setEnchantRatio(ratios[r].enchantRatio);
        else
            cout << "PetAttrInfo names a PetAttr that has no balance rows." << endl;
    }
}

bool PetAttrInfoManager::enchantRandomAttr(PetInfo* pPetInfo, int ratio) {
    int value = rand() % 100;

    cout << "ratio : " << ratio << endl;
    cout << "value : " << value << endl;

    unordered_map<PetAttr_t, PetAttrInfo*>::iterator itr = m_PetAttrInfoMap.begin();
    unordered_map<PetAttr_t, PetAttrInfo*>::iterator endItr = m_PetAttrInfoMap.end();

    if (pPetInfo->getPetLevel() < 10)
        return false;
    if (value < ratio)
        return false;

    value = rand() % 100;

    cout << "옵션선택 : " << value << endl;

    for (; itr != endItr; ++itr) {
        PetAttrInfo* pPetAttrInfo = itr->second;

        if (pPetAttrInfo == NULL)
            continue;

        cout << (int)pPetAttrInfo->getPetAttr() << " : " << pPetAttrInfo->getEnchantRatio() << endl;

        if (pPetAttrInfo->getEnchantRatio() > value) {
            cout << "selected" << endl;

            pPetInfo->setPetAttr(pPetAttrInfo->getPetAttr());
            pPetInfo->setPetAttrLevel(pPetAttrInfo->getPetAttrLevel(pPetInfo->getPetLevel()));
            return true;
        }

        value -= pPetAttrInfo->getEnchantRatio();
    }

    return false;
}

bool PetAttrInfoManager::enchantSpecAttr(PetInfo* pPetInfo, PetAttr_t PetAttr) {
    int value = rand() % 100;
    if (value > 70)
        return false;

    PetAttrInfo* pPetAttrInfo = m_PetAttrInfoMap[PetAttr];
    if (pPetAttrInfo == NULL) {
        filelog("PetBug.log", "속성 지정 펫 인챈트에서 이상한 값이 들어있다. : %u", PetAttr);
        return false;
    }

    pPetInfo->setPetAttr(pPetAttrInfo->getPetAttr());
    pPetInfo->setPetAttrLevel(pPetAttrInfo->getPetAttrLevel(pPetInfo->getPetLevel()));

    return true;
}

PetAttrInfo* PetAttrInfoManager::getPetAttrInfo(PetAttr_t PetAttr) const {
    unordered_map<PetAttr_t, PetAttrInfo*>::const_iterator itr = m_PetAttrInfoMap.find(PetAttr);

    if (itr == m_PetAttrInfoMap.end())
        return NULL;

    return itr->second;
}
