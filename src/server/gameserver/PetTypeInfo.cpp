
#include "PetTypeInfo.h"

#include "repository/GameInfoRepository.h"

MonsterType_t PetTypeInfo::getPetCreatureType(PetLevel_t petLevel) const {
    if (petLevel < 10)
        return getPetCreatureTypeByIndex(0);
    return getPetCreatureTypeByIndex(petLevel / 10 - 1);
}

void PetTypeInfoManager::clear() {
    vector<PetTypeInfo*>::iterator itr = m_PetTypeInfos.begin();
    vector<PetTypeInfo*>::iterator endItr = m_PetTypeInfos.end();

    for (; itr != endItr; ++itr) {
        SAFE_DELETE((*itr));
    }

    m_PetTypeInfos.clear();
}

void PetTypeInfoManager::load() {
    clear();

    int maxPetType = 0;
    if (!defaultGameInfoRepository().loadMaxPetType(maxPetType))
        throw Error("PetTypeInfo has no rows.");

    PetType_t MaxPetType = maxPetType;

    m_PetTypeInfos.reserve(MaxPetType + 1);

    vector<PetTypeRow> rows = defaultGameInfoRepository().loadPetTypes();

    for (size_t r = 0; r < rows.size(); r++) {
        PetTypeInfo* pPetTypeInfo = new PetTypeInfo(rows[r].petType);
        pPetTypeInfo->m_OriginalMonsterType = rows[r].originalMonsterType;
        pPetTypeInfo->m_PetCreatureType[0] = rows[r].creatureType[0];
        pPetTypeInfo->m_PetCreatureType[1] = rows[r].creatureType[1];
        pPetTypeInfo->m_PetCreatureType[2] = rows[r].creatureType[2];
        pPetTypeInfo->m_PetCreatureType[3] = rows[r].creatureType[3];
        pPetTypeInfo->m_PetCreatureType[4] = rows[r].creatureType[4];
        pPetTypeInfo->m_FoodType = rows[r].foodType;

        addPetTypeInfo(pPetTypeInfo);
    }
}

void PetTypeInfoManager::addPetTypeInfo(PetTypeInfo* pPetTypeInfo) {
    if (pPetTypeInfo->m_PetType >= m_PetTypeInfos.capacity())
        throw Error("Pet Type이 최대값을 초과했네용");

    m_PetTypeInfos[pPetTypeInfo->m_PetType] = pPetTypeInfo;
}

PetTypeInfo* PetTypeInfoManager::getPetTypeInfo(PetType_t PetType) {
    if (PetType >= m_PetTypeInfos.capacity())
        return NULL;

    return m_PetTypeInfos[PetType];
}
