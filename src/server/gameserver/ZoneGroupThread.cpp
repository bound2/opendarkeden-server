//////////////////////////////////////////////////////////////////////////////
// Filename    : ZoneGroupThread.cc
// Written by  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "ZoneGroupThread.h"

#include "DB.h"
#include "GMServerInfo.h"
#include "LogClient.h"
#include "Profile.h"
#include "Properties.h"
#include "Timeval.h"
#include "VSDateTime.h"
#include "ZonePlayerManager.h"

// #define __FULL_PROFILE__

#ifndef __FULL_PROFILE__
#undef beginProfileEx
#define beginProfileEx(name) ((void)0)
#undef endProfileEx
#define endProfileEx(name) ((void)0)
#endif

//////////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////////
ZoneGroupThread::ZoneGroupThread(ZoneGroup* pZoneGroup)

    : m_pZoneGroup(pZoneGroup){__BEGIN_TRY __END_CATCH}

      //////////////////////////////////////////////////////////////////////////////
      // destructor
      //////////////////////////////////////////////////////////////////////////////
      ZoneGroupThread::~ZoneGroupThread() noexcept

{
    __BEGIN_TRY

    stop();
    join();

    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////////////
// 쓰레드 메쏘드들은 최상위로 사용되므로 __BEGIN_TRY와 __END_CATCH
// 를 할 필요가 없다. 즉 모든 예외를 잡아서 처리해야 한다는 소리.
//////////////////////////////////////////////////////////////////////////////
void ZoneGroupThread::run()

{
    __BEGIN_DEBUG

    // From here on, this thread owns the zone group's state: arm the
    // debug ownership check (see CLAUDE.md, "Thread ownership").
    m_pZoneGroup->armOwnershipAssert();

    string host = g_pConfig->getProperty("DB_HOST");
    string db = g_pConfig->getProperty("DB_DB");
    string user = g_pConfig->getProperty("DB_USER");
    string password = g_pConfig->getProperty("DB_PASSWORD");
    uint port = 0;
    if (g_pConfig->hasKey("DB_PORT"))
        port = g_pConfig->getPropertyInt("DB_PORT");

    if (stopRequested())
        return;
    Connection* pConnection = new Connection(host, db, user, password, port);
    g_pDatabaseManager->addConnection((int)(long)Thread::self(), pConnection);
    cout << "******************************************************" << endl;
    cout << " THREAD CONNECT DB " << endl;
    cout << "******************************************************" << endl;

    string dist_host = g_pConfig->getProperty("UI_DB_HOST");
    string dist_db = "DARKEDEN";
    string dist_user = g_pConfig->getProperty("UI_DB_USER");
    string dist_password = g_pConfig->getProperty("UI_DB_PASSWORD");
    uint dist_port = 0;
    if (g_pConfig->hasKey("UI_DB_PORT"))
        dist_port = g_pConfig->getPropertyInt("UI_DB_PORT");

    if (stopRequested())
        return;
    Connection* pDistConnection = new Connection(dist_host, dist_db, dist_user, dist_password, dist_port);
    g_pDatabaseManager->addDistConnection(((int)(long)Thread::self()), pDistConnection);
    cout << "******************************************************" << endl;
    cout << " THREAD CONNECT UIIRIBUTION DB " << endl;
    cout << " TID Number = " << (int)(long)Thread::self() << endl;
    cout << "******************************************************" << endl;

    Timeval NextTime;
    getCurrentTime(NextTime);
    Timeval currentTime;

    NextTime.tv_sec += 2;

    Timeval dummyQueryTime;
    getCurrentTime(dummyQueryTime);

    try {
        while (!stopRequested()) {
            //		beginProfileEx("ZGT_MAIN");
            try {
                beginProfileExNoTry("ZGT_MAIN");

                if (!pauseFor(std::chrono::milliseconds(1))) {
                    endProfileExNoCatch("ZGT_MAIN");
                    break;
                }

                __ENTER_CRITICAL_SECTION((*m_pZoneGroup))

                // Group-level commands other threads posted run first, under
                // the group mutex we now hold -- see ZoneGroup::post(). (Work
                // aimed at one player rides the player's own box, drained in
                // ZonePlayerManager::processCommands below.)
                beginProfileEx("ZG_MAILBOX");
                m_pZoneGroup->drainMailbox();
                endProfileEx("ZG_MAILBOX");

                beginProfileEx("ZG_PP");
                m_pZoneGroup->processPlayers(); // process all players in ZonePlayerManager;
                endProfileEx("ZG_PP");

                beginProfileEx("ZG_HEARTBEAT");
                m_pZoneGroup->heartbeat(); // process all npc, monster, ... in Zones
                endProfileEx("ZG_HEARTBEAT");

                __LEAVE_CRITICAL_SECTION((*m_pZoneGroup))

                getCurrentTime(currentTime);

                //		endProfileEx("ZGT_MAIN");
                endProfileExNoCatch("ZGT_MAIN");
            } catch (Throwable&) {
                endProfileExNoCatch("ZGT_MAIN");
                throw;
            }

            if (dummyQueryTime < currentTime) {
                g_pDatabaseManager->executeDummyQuery(pConnection);
                g_pDatabaseManager->executeDummyQuery(pDistConnection);

                // 1시간 ~ 1시간 30분 사이에서 dummy query 시간을 설정한다.
                // timeout이 되지 않게 하기 위해서이다.
                dummyQueryTime.tv_sec += (60 + rand() % 30) * 60;
            }


            if (NextTime < currentTime) {
                GMServerInfo gmServerInfo;
                m_pZoneGroup->makeZoneUserInfo(gmServerInfo);

                // outputProfileEx(false, false);
                (g_ProfileSampleManager.getProfileSampleSet())
                    ->outputProfileToFile("Profile", false, false, &gmServerInfo);


                NextTime.tv_sec = currentTime.tv_sec + 10;
                NextTime.tv_usec = currentTime.tv_usec;

                // 매턴마다 프로파일 데이터를 초기화해준다.
                // 누적 데이터보다는 시간대에 따른 시간을 측정하기 위해서...
                initProfileEx();
            }
        }

    } catch (Throwable& t) {
        filelog("zoneGroupThreadError.log", "%s", t.toString().c_str());
        throw;
    }

    __END_DEBUG
}

//////////////////////////////////////////////////////////////////////////////
// get debug string
//////////////////////////////////////////////////////////////////////////////
string ZoneGroupThread::toString() const

{
    StringStream msg;
    msg << "ZoneGroupThread(" << m_pZoneGroup->toString() << ")";
    return msg.toString();
}
