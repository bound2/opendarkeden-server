//////////////////////////////////////////////////////////////////////
//
// ThreadPool.h
//
// by Reiot
//
//////////////////////////////////////////////////////////////////////

#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

//////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////
#include <list>
#include <mutex>

#include "Exception.h"
#include "Thread.h"
#include "Types.h"


//////////////////////////////////////////////////
// forward declaration
//////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
// class ThreadPool
//
// A collection class of thread objects. To stop every running thread when
// the server shuts down, use the thread pool's stop method. Internally it
// is expected to use signals or cancellation, but there is no material on
// that yet, so it is not implemented.
//
//////////////////////////////////////////////////////////////////////

class ThreadPool {
    //////////////////////////////////////////////////
    // constructor/destructor
    //////////////////////////////////////////////////
public:
    // constructor
    ThreadPool();

    // destructor
    // Must delete every thread object it holds.
    virtual ~ThreadPool();


    //////////////////////////////////////////////////
    // methods
    //////////////////////////////////////////////////
public:
    // Puts the threads registered in the thread pool into the RUNNING state.
    void start();

    // Stops every thread registered in the thread pool.
    //(This should be implemented with a signal or cancellation.)
    void stop();

    // Registers a thread object in the thread pool.
    void addThread(Thread* thread);

    // Deletes a specific thread object from the thread pool.
    void deleteThread(TID tid);

    // Finds and returns a specific thread object in the thread pool.
    Thread* getThread(TID tid);

    // #ifdef __NO_COMBAT__
    list<Thread*> getThreads() {
        std::lock_guard lock(m_Mutex);
        return m_Threads;
    }
    // #endif

    //////////////////////////////////////////////////
    // attributes
    //////////////////////////////////////////////////
private:
    //
    // List of pointers to thread objects.
    // In practice subclasses of the Thread class are stored.
    // Normally threads of the same kind are registered.
    //
    // ex> PlayerThreadPool - PlayerThread
    //     NPCThreadPool    - NPCThread
    //     MobThreadPool    - MobThread
    //
    list<Thread*> m_Threads;

    // mutex for list operation(add, delete, get ...)
    mutable std::mutex m_Mutex;
};

#endif
