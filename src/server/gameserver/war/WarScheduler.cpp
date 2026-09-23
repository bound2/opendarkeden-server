#include "WarScheduler.h"

#include <stdio.h>

#include "Assert.h"
#include "DB.h"
#include "GCWarScheduleList.h"
#include "GameContext.h"
#include "GuildWar.h"
#include "Properties.h"
#include "SiegeWar.h"
#include "VariableManager.h"
#include "WarSchedule.h"
#include "WarSystem.h"
#include "Zone.h"
#include "repository/WarInfoRepository.h"

// Days from each weekday to the next war of each kind, then the start hours;
// the tables below are the schedule, not a fixed evening.
const int NextWarDay[2][8] = {
    {0, 1, 7, 6, 5, 4, 3, 2}, // guild war
    //{ 0, 2, 1, 0, 3, 2, 1, 0 }	// RaceWar Sun,Mon,Tue,Wed,Thu,Fri,Sat,Sun
    {0, 6, 5, 4, 3, 2, 1, 0} // RaceWar Sun,Mon,Tue,Wed,Thu,Fri,Sat,Sun
};

// On the test server..
const int NextWarHour[2][24] = {
    //                               *     *              *     *
    // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18  19  20  21  22  23
    {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 2, 1, 5, 4, 3, 2, 1, 2, 1, 15, 14, 13, 12, 11}, // guild war

    //                                           *                          *
    // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18  19  20  21  22  23
    {14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 8, 7, 6, 5, 4, 3, 2, 1, 16, 15} // race war
};


WarScheduler::WarScheduler(Zone* pZone)

    : m_pZone(pZone) {
    // It runs attached to a Zone.
    // But cancelGuildSchedules() is called from outside.
    m_Mutex.setName("WarSheduler");
}

WarScheduler::~WarScheduler()

{}

bool WarScheduler::makeGCWarScheduleList(GCWarScheduleList* pGCWarScheduleList) const

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    const RecentSchedules::container_type& Schedules = m_RecentSchedules.getSchedules();
    RecentSchedules::const_iterator itr = Schedules.begin();

    for (; itr != Schedules.end(); itr++) {
        const WarSchedule* pWarSchedule = dynamic_cast<WarSchedule*>(*itr);
        Assert(pWarSchedule != NULL);

        WarScheduleInfo* pWSI = new WarScheduleInfo;
        pWarSchedule->makeWarScheduleInfo(pWSI);

        pGCWarScheduleList->addWarScheduleInfo(pWSI);
    }

    // When the automatic start is configured, the race war information always goes in.
    if (de::gameContext().variables().isAutoStartRaceWar()) {
        WarScheduleInfo* pWSI = new WarScheduleInfo;
        if (de::gameContext().warSystem().addRaceWarScheduleInfo(pWSI)) {
            pGCWarScheduleList->addWarScheduleInfo(pWSI);
        } else {
            SAFE_DELETE(pWSI);
        }
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH

    return true;
}

Work* WarScheduler::heartbeat()

{
    __BEGIN_TRY

    Work* pWork = NULL;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    pWork = Scheduler::heartbeat();

    __LEAVE_CRITICAL_SECTION(m_Mutex)


    // For a race war the schedule for a week later goes back in.

    return pWork;

    __END_CATCH
}

void WarScheduler::load()

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    clear();


    VSDateTime currentDateTime(VSDateTime::currentDateTime());

    WarInfoRepository& repository = defaultWarInfoRepository();

    vector<WarScheduleRow> schedules =
        repository.loadWarSchedules(g_pConfig->getPropertyInt("ServerID"), (int)m_pZone->getZoneID());

    if (!schedules.empty()) {
        WarID_t warID;
        WarType_t warType;
        uint challengerNum;
        GuildID_t challengerGuildID[5];
        Gold_t warRegistrationFee;
        string dateTemp;
        VSDateTime warStartTime;

        for (size_t r = 0; r < schedules.size(); r++) {
            warID = (WarID_t)schedules[r].warID;
            string warTypeStr = schedules[r].warType;

            if (warTypeStr == "GUILD")
                warType = WAR_GUILD;
            else if (warTypeStr == "RACE")
                continue; // warType = WAR_RACE;
            else
                Assert(false);
            challengerNum = schedules[r].attackerCount;

            for (int j = 0; j < 5; ++j) {
                challengerGuildID[j] = (GuildID_t)schedules[r].attackGuildID[j];
            }

            warRegistrationFee = (Gold_t)schedules[r].warFee;
            dateTemp = schedules[r].startTime;
            warStartTime = VSDateTime(dateTemp);

            // For a war that should already have started, the start time is changed.
            if (warStartTime < currentDateTime) {
                warStartTime = currentDateTime;
            }

            SiegeWar* pWar = new SiegeWar(m_pZone->getZoneID(), War::WAR_STATE_WAIT, warID);
            pWar->setWarStartTime(warStartTime);
            pWar->setRegistrationFee(warRegistrationFee);

            int reinforceGuildID = 0;

            if (repository.loadAcceptedReinforceGuild(warID, reinforceGuildID)) {
                pWar->setReinforceGuildID(reinforceGuildID);
            }

            for (int j = 0; j < challengerNum; ++j) {
                pWar->addChallengerGuild(challengerGuildID[j]);
            }

            WarSchedule* pWarSchedule = new WarSchedule(pWar, warStartTime, Schedule::SCHEDULE_TYPE_ONCE);
            addSchedule(pWarSchedule);

            filelog("WarLog.txt", "[LOAD] %s", pWar->toString().c_str());
        }
    }

    // If no race war is set, set one.

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

int WarScheduler::getWarTypeCount(WarType_t warType)

{
    __BEGIN_TRY

    int raceWarCount = 0;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    RecentSchedules::const_iterator itr = m_RecentSchedules.getSchedules().begin();

    for (; itr != m_RecentSchedules.getSchedules().end(); itr++) {
        WarSchedule* pWarSchedule = dynamic_cast<WarSchedule*>((*itr));
        Assert(pWarSchedule != NULL);

        War* pWar = dynamic_cast<War*>(pWarSchedule->getWork());
        Assert(pWar != NULL);

        if (pWar->getWarType() == warType) {
            raceWarCount++;
        }
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return raceWarCount;

    __END_CATCH
}

void WarScheduler::tinysave(WarID_t warID, const string& query)

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    RecentSchedules::const_iterator itr = m_RecentSchedules.getSchedules().begin();

    for (; itr != m_RecentSchedules.getSchedules().end(); itr++) {
        WarSchedule* pWarSchedule = dynamic_cast<WarSchedule*>((*itr));

        if (pWarSchedule->getWarID() == warID) {
            pWarSchedule->tinysave(query);
            return;
        }
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    filelog("WarError.log", "WarScheduler::tinySave() DB에 WarID:%d 인 WarSchedule이 없습니다.", warID);

    __END_CATCH
}

VSDateTime WarScheduler::getLastWarDateTime(WarType_t warType) const {
    const RecentSchedules::container_type& schedules = m_RecentSchedules.getSchedules();
    RecentSchedules::const_iterator itr = schedules.begin();

    bool bFound = false;
    VSDateTime dt = VSDateTime::currentDateTime();

    for (; itr != schedules.end(); itr++) {
        WarSchedule* pSchedule = dynamic_cast<WarSchedule*>(*itr);
        if (pSchedule->getWar()->getWarType() == warType) {
            if (bFound) {
                if (dt < pSchedule->getScheduledTime())
                    dt = pSchedule->getScheduledTime();
            } else {
                dt = pSchedule->getScheduledTime();
                bFound = true;
            }
        }
    }

    return dt;
}

// Get the war time after dt.
VSDateTime WarScheduler::getNextWarDateTime(WarType_t warType, const VSDateTime& dt) {
    int startHour = 0;

    VSDateTime nextWarDateTime;
    VSTime nextWarTime;

    if (de::gameContext().variables().isWarPeriodWeek()) // that is a bit much
    {
        switch (warType) {
        case WAR_GUILD:
            // Monday, Wednesday and Friday after dt, 8 pm (~9 pm)
            startHour = 20;
            break;

        case WAR_RACE:
            // Sunday after dt, 7 pm (~9 pm)
            startHour = 19;
            break;
        }

        nextWarDateTime = dt.addDays(NextWarDay[warType][dt.date().dayOfWeek()]);
        nextWarTime = VSTime(startHour, 0, 0);
        nextWarDateTime.setTime(nextWarTime);

        if (nextWarDateTime < VSDateTime::currentDateTime()) {
            nextWarDateTime = nextWarDateTime.addDays(1);
            nextWarDateTime = nextWarDateTime.addDays(NextWarDay[warType][nextWarDateTime.date().dayOfWeek()]);
        }
    } else {
        nextWarDateTime = dt.addSecs(NextWarHour[warType][dt.time().hour()] * 60 * 60);
        nextWarTime = VSTime(nextWarDateTime.time().hour(), 0, 0);
        nextWarDateTime.setTime(nextWarTime);
    }

    return nextWarDateTime;
}

VSDateTime WarScheduler::getNextWarDateTime(WarType_t warType) const {
    return getNextWarDateTime(warType, getLastWarDateTime(warType));
}

bool WarScheduler::addWar(War* pWar)

{
    __BEGIN_TRY

    WarType_t warType = pWar->getWarType();
    VSDateTime warStartTime = getNextWarDateTime(warType);
    pWar->setWarStartTime(warStartTime);

    Schedule::ScheduleType scheduleType;

    if (warType == WAR_GUILD) {
        scheduleType = Schedule::SCHEDULE_TYPE_ONCE;
    } else // if (warType==WAR_RACE)
    {
        scheduleType = Schedule::SCHEDULE_TYPE_PERIODIC;
    }

    WarSchedule* pWarSchedule = new WarSchedule(pWar, warStartTime, scheduleType);

    __ENTER_CRITICAL_SECTION(m_Mutex)

    addSchedule(pWarSchedule);

    filelog("WarLog.txt", "[%d][WarID=%d] %s 전쟁을 신청했으므로 스케쥴에 추가합니다.", (int)m_pZone->getZoneID(),
            (int)pWar->getWarID(), (pWar->getWarType() == WAR_GUILD ? "길드" : "종족"));

    pWarSchedule->create();

    __LEAVE_CRITICAL_SECTION(m_Mutex)


    return true;

    __END_CATCH
}

bool WarScheduler::canAddWar(WarType_t warType)

{
    __BEGIN_TRY

    return getSize() < MaxWarSchedule;

    __END_CATCH
}

void WarScheduler::cancelGuildSchedules()

{
    __BEGIN_TRY

    defaultWarInfoRepository().cancelGuildWarSchedules(g_pConfig->getPropertyInt("ServerID"), m_pZone->getZoneID());

    // Load it again.
    load();

    __END_CATCH
}

bool WarScheduler::hasSchedule(GuildID_t gID) {
    __BEGIN_TRY

    const RecentSchedules::container_type& schedules = m_RecentSchedules.getSchedules();
    RecentSchedules::const_iterator itr = schedules.begin();

    for (; itr != schedules.end(); itr++) {
        WarSchedule* pSchedule = dynamic_cast<WarSchedule*>(*itr);
        if (pSchedule == NULL)
            continue;

        // Both castle war classes report WAR_GUILD and answer for their own
        // participants, so no cast to one of them is made here.
        War* pWar = dynamic_cast<War*>(pSchedule->getWork());
        if (pWar != NULL && pWar->getWarType() == WAR_GUILD && pWar->isWarParticipant(gID) &&
            pWar->getState() == War::WAR_STATE_WAIT) {
            return true;
        }
    }

    return false;

    __END_CATCH
}
