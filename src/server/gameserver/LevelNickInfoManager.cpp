#include "LevelNickInfoManager.h"

#include "Creature.h"
#include "repository/GameInfoRepository.h"

void LevelNickInfoManager::clear() {
    unordered_map<Level_t, vector<LevelNickInfo*>>::iterator itr = m_LevelNickInfoMap.begin();
    unordered_map<Level_t, vector<LevelNickInfo*>>::iterator endItr = m_LevelNickInfoMap.end();

    for (; itr != endItr; ++itr) {
        vector<LevelNickInfo*>::iterator vitr = itr->second.begin();
        vector<LevelNickInfo*>::iterator eitr = itr->second.end();

        for (; vitr != eitr; ++vitr) {
            SAFE_DELETE((*vitr));
        }
    }

    m_LevelNickInfoMap.clear();
}

void LevelNickInfoManager::load() {
    clear();
    vector<LevelNickRow> rows = defaultGameInfoRepository().loadLevelNicks();

    for (size_t r = 0; r < rows.size(); r++) {
        DWORD index = rows[r].nickIndex;
        Race_t race = rows[r].race;
        Level_t level10 = rows[r].level10;

        LevelNickInfo* pInfo = new LevelNickInfo(race, level10, index);

        m_LevelNickInfoMap[level10].push_back(pInfo);
    }
}

bool LevelNickInfo::isFitRace(Creature* pCreature) const {
    return m_Race == 3 || pCreature->getRace() == m_Race;
}
