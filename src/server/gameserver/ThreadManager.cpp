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

    // Create the zone thread pool.
    m_pZoneGroupThreadPool = new ThreadPool();

    __END_CATCH
}


//--------------------------------------------------------------------------------
//
// destructor
//
// Must be run when Stop() has not been called.
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
// Initialize the thread manager.
//
// Create and register threads in the sub thread pools.
//
// *CAUTION*
//
// The zone group manager must be initialized before the thread manager.
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
// Start the sub thread pools.
//
//--------------------------------------------------------------------------------
void ThreadManager::start()

{
    __BEGIN_TRY

    // Start the Zone Thread Pool.
    m_pZoneGroupThreadPool->start();

    __END_CATCH
}


//--------------------------------------------------------------------------------
//
// deactivate sub thread pools
//
// Stop the sub thread pools.
//
//--------------------------------------------------------------------------------
void ThreadManager::stop()

{
    __BEGIN_TRY

    m_pZoneGroupThreadPool->stop();

    __END_CATCH
}
