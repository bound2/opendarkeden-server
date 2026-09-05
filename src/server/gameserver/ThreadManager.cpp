//--------------------------------------------------------------------------------
//
// Filename    : ThreadManager.cc
// Written By  : Reiot
//
//--------------------------------------------------------------------------------

// include files
#include "ThreadManager.h"

#include "Assert.h"
#include "LogClient.h"
#include "Properties.h"
#include "ThreadPool.h"
#include "ZoneGroupManager.h"
#include "ZoneGroupThread.h"
#include "repository/ZoneInfoRepository.h"


//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
ThreadManager::ThreadManager()

    : m_pZoneGroupThreadPool(NULL) {
    __BEGIN_TRY

    // 존쓰레드풀을 생성한다.
    m_pZoneGroupThreadPool = new ThreadPool();

    __END_CATCH
}


//--------------------------------------------------------------------------------
//
// destructor
//
// Stop()이 되지 않았을 경우 실행시켜야 한다. State 개념을 도입할까?
//
//--------------------------------------------------------------------------------
ThreadManager::~ThreadManager()

{
    __BEGIN_TRY

    SAFE_DELETE(m_pZoneGroupThreadPool);

    __END_CATCH_NO_RETHROW
}


//--------------------------------------------------------------------------------
//
// 쓰레드 매니저를 초기화한다.
//
// 하위 쓰레드풀에 쓰레드들을 생성, 등록시킨다.
//
// *CAUTION*
//
// 당연히, 쓰레드 매니저를 초기화하기 전에, 존그룹매니저를 초기화해야 한다.
//
//--------------------------------------------------------------------------------
void ThreadManager::init()

{
    __BEGIN_TRY

    // Register one thread per zone group.
    vector<int> zoneGroupIDs = defaultZoneInfoRepository().loadZoneGroupIDs(false);

    for (size_t i = 0; i < zoneGroupIDs.size(); i++) {
        ZoneGroupID_t zoneGroupID = zoneGroupIDs[i];
        ZoneGroupThread* pZoneGroupThread = new ZoneGroupThread(g_pZoneGroupManager->getZoneGroup(zoneGroupID));
        m_pZoneGroupThreadPool->addThread(pZoneGroupThread);
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
//
// activate sub thread pools
//
// 하위 쓰레드 풀을 활성화시킨다.
//
//--------------------------------------------------------------------------------
void ThreadManager::start()

{
    __BEGIN_TRY

    // Zone Thread Pool 을 활성화시킨다.
    m_pZoneGroupThreadPool->start();

    __END_CATCH
}


//--------------------------------------------------------------------------------
//
// deactivate sub thread pools
//
// 하위 쓰레드 풀을 종료시킨다.
//
//--------------------------------------------------------------------------------
void ThreadManager::stop()

{
    __BEGIN_TRY

    m_pZoneGroupThreadPool->stop();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// global variable definition
//--------------------------------------------------------------------------------
ThreadManager* g_pThreadManager = NULL;
