//--------------------------------------------------------------------------------
//
// Filename    : ThreadManager.h
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __THREAD_MANAGER_H__
#define __THREAD_MANAGER_H__

// include files
#include "Exception.h"
#include "Types.h"

// forward declaration
class ThreadPool;


//////////////////////////////////////////////////////////////////////
//
// class ThreadManager
//
// Manages every thread pool in the game server. When the game server issues init,
// start or stop, the same method is called on each sub thread pool.
//
// init  : creates and registers the configured number of threads in each pool.
// start : starts the threads registered in each pool.
// stop  : stops the threads registered in each pool.
//
//////////////////////////////////////////////////////////////////////

class ThreadManager {
public:
    // constructor
    ThreadManager();

    // destructor
    ~ThreadManager();


public:
    // Initialize the thread manager.
    void init();

    // activate sub thread pools
    // Start the sub thread pools.
    void start();

    // deactivate sub thread pools
    // Stop the sub thread pools.
    void stop();

    // #ifdef __NO_COMBAT__
    ThreadPool* getThreadPool() {
        return m_pZoneGroupThreadPool;
    }
    // #endif

private:
    // Thread pool
    ThreadPool* m_pZoneGroupThreadPool;
};

#endif
