#include "NewbieFlagWar.h"

#include "FlagManager.h"
#include "MonsterSummonInfo.h"
#include "Zone.h"
#include "ZoneInfoManager.h"
#include "ZoneUtil.h"
#include "war/WarZoneWork.h"

// The winning race's reward, a monster summoned at its end of the newbie
// zone, is posted to the zone's own thread like the rest of the end.
void NewbieFlagWar::executeEnd() {
    FlagWar::executeEnd();
    ZoneCoord_t ZoneX, ZoneY;

    switch (m_FlagManager.getWinnerRace()) {
    case RACE_SLAYER:
        ZoneX = 90;
        ZoneY = 50;
        break;

    case RACE_VAMPIRE:
        ZoneX = 24;
        ZoneY = 52;
        break;

    case RACE_OUSTERS:
        ZoneX = 30;
        ZoneY = 86;
        break;

    default:
        return;
    }

    de::war::postToZone(1122, [ZoneX, ZoneY](Zone& zone) {
        SUMMON_INFO summonInfo;
        summonInfo.canScanEnemy = false;
        summonInfo.clanType = SUMMON_INFO::CLAN_TYPE_DEFAULT;

        addMonstersToZone(&zone, ZoneX, ZoneY, 0, 599, 1, summonInfo);
    });
}

VSDateTime NewbieFlagWar::getNextFlagWarTime() {
    static const int NextFlagWarDay[8] = {0, 1, 0, 1, 0, 4, 3, 2};

    VSDateTime dt = VSDateTime::currentDateTime();

    VSDateTime nextWarDateTime;
    nextWarDateTime = dt.addDays(NextFlagWarDay[dt.date().dayOfWeek()]);

    nextWarDateTime.setTime(VSTime(18, 55, 0));

    if (nextWarDateTime < VSDateTime::currentDateTime()) {
        nextWarDateTime = nextWarDateTime.addDays(1);
        nextWarDateTime = nextWarDateTime.addDays(NextFlagWarDay[nextWarDateTime.date().dayOfWeek()]);
    }

    filelog("FlagWar.log", "Newbie-zone capture-the-flag event starts at %s", nextWarDateTime.toString().c_str());
    return nextWarDateTime;
}

std::vector<de::ctf::FlagDrop> NewbieFlagWar::flagDrops() const {
    return {{1122, 20}};
}
