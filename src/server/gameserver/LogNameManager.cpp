#include "LogNameManager.h"

#include "Exception.h"
#include "repository/GameInfoRepository.h"

void LogNameManager::init() {
    try {
        release();

        vector<string> names = defaultGameInfoRepository().loadLogUserNames();

        for (size_t i = 0; i < names.size(); i++) {
            // the 0 is meaningless
            add(names[i], 0);
        }

    } catch (Throwable& t) {
        filelog("LogNameManagerBUG.txt", "%s", t.toString().c_str());
    }
}
