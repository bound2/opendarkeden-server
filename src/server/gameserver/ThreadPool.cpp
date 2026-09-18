//////////////////////////////////////////////////////////////////////
//
// ThreadPool.cpp
//
// by Reiot
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////
#include "ThreadPool.h"

#include <algorithm>
#include <mutex>

#include "Assert.h"
#include "LogClient.h"
#include "ManagedThread.h"
#include "Thread.h"

//////////////////////////////////////////////////
// function object for find_if ()
//////////////////////////////////////////////////

//
// Returns true if the thread object in the container has the given TID.
//
class isSameTID {
public:
    // constructor
    isSameTID(TID tid) : m_TID(tid) {}

    //
    bool operator()(Thread* pThread) {
        return pThread->getTID() == m_TID;
    }

private:
    // thread identifier
    TID m_TID;
};

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
ThreadPool::ThreadPool()

    {__BEGIN_TRY


         __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
// Must delete every thread object it holds.
//////////////////////////////////////////////////////////////////////
ThreadPool::~ThreadPool()

{
    __BEGIN_TRY

    stop();

    std::lock_guard lock(m_Mutex);

    /*
    list<Thread*>::iterator itr = m_Threads.begin();
    for (; itr != m_Threads.end() ; itr ++)
    {
        Thread* temp = *itr;

        // The thread must already have exited.
        Assert(temp != NULL && temp->getStatus() == Thread::EXIT);

        SAFE_DELETE(temp);
    }

    m_Threads.erase(m_Threads.begin() , m_Threads.end());

    g_pLogManager->Log5("after erase(begin , end) , list's size == %d\n" , m_Threads.size());

    */

    list<Thread*>::iterator itr;

    while ((itr = m_Threads.begin()) != m_Threads.end()) {
        // Means nodes are still left in the list.

        // The thread must already have exited.
        Assert(*itr != NULL && (*itr)->getStatus() == Thread::EXIT);

        SAFE_DELETE(*itr);

        m_Threads.pop_front();
    }

    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// Puts the threads registered in the thread pool into the RUNNING state.
//////////////////////////////////////////////////////////////////////
void ThreadPool::start()

{
    __BEGIN_TRY

    log(LOG_DEBUG_MSG, "", "", "== ThreadPool has started ==");

    //////////////////////////////////////////////////
    // enter critical section
    //////////////////////////////////////////////////
    std::lock_guard lock(m_Mutex);

    try {
        for (list<Thread*>::iterator itr = m_Threads.begin(); itr != m_Threads.end(); itr++) {
            // start threads
            Assert(*itr != NULL);
            (*itr)->start();

            string msg = "== " + (*itr)->getName() + " has been started == ";
            log(LOG_DEBUG_MSG, "", "", msg);
        }

        //////////////////////////////////////////////////
        // leave critical section
        //////////////////////////////////////////////////
    } catch (...) {
        // Arm the process deadline before rollback can wait on blocked work.
        ServerShutdown::fail();
        // Also stop unstarted members: the pool is terminal after a failed
        // startup, and no successfully started worker may escape rollback.
        for (Thread* thread : m_Threads)
            thread->stop();
        for (Thread* thread : m_Threads)
            thread->join();
        throw;
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Stops every thread registered in the thread pool.
// (This should be implemented with a signal or cancellation.)
//////////////////////////////////////////////////////////////////////
void ThreadPool::stop()

{
    __BEGIN_TRY

    std::lock_guard lock(m_Mutex);

    // Request every stop before joining any worker so zone groups wind down
    // concurrently instead of serially extending shutdown.
    for (Thread* thread : m_Threads) {
        Assert(thread != NULL);
        thread->stop();
    }

    for (Thread* thread : m_Threads)
        thread->join();

    for (Thread* thread : m_Threads) {
        auto* managed = dynamic_cast<ManagedThread*>(thread);
        if (managed == nullptr)
            continue;
        try {
            managed->rethrowFailure();
        } catch (Throwable& error) {
            cerr << thread->getName() << ": " << error.toString() << endl;
        } catch (const std::exception& error) {
            cerr << thread->getName() << ": " << error.what() << endl;
        } catch (...) {
            cerr << thread->getName() << ": unknown worker failure" << endl;
        }
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Registers a thread object in the thread pool.
//////////////////////////////////////////////////////////////////////
void ThreadPool::addThread(Thread* thread)

{
    __BEGIN_TRY

    //////////////////////////////////////////////////
    // enter critical section
    //////////////////////////////////////////////////
    std::lock_guard lock(m_Mutex);

    // The thread must not be null.
    Assert(thread != NULL);

    // Insert the thread object at the end of the list.
    m_Threads.push_back(thread);

    string msg = "== " + thread->getName() + " added to thread pool";
    log(LOG_DEBUG_MSG, "", "", msg);

    //////////////////////////////////////////////////
    // leave critical section
    //////////////////////////////////////////////////

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Deletes a specific thread object from the thread pool.
//////////////////////////////////////////////////////////////////////
void ThreadPool::deleteThread(TID tid) {
    __BEGIN_TRY

    //////////////////////////////////////////////////
    // enter critical section
    //////////////////////////////////////////////////
    std::lock_guard lock(m_Mutex);

    // Use a function object to find the iterator for the node holding
    // the thread object with the given TID.
    list<Thread*>::iterator itr = find_if(m_Threads.begin(), m_Threads.end(), isSameTID(tid));

    if (itr != m_Threads.end()) // found!
    {
        // Keep the thread object temporarily.
        Thread* temp = *itr;

        // The thread must already have exited.
        // If a subclass has a Mutex, getStatus() and setStatus() must be protected by it.
        Assert(temp != NULL && temp->getStatus() == Thread::EXIT);

        StringStream msg;
        msg << "== Thread[" << temp->getTID() << "] has been removed from ThreadPool ==";
        log(LOG_DEBUG_MSG, "", "", msg.toString());

        // Delete the thread object.
        SAFE_DELETE(temp);

        // Delete the node.
        m_Threads.erase(itr);
    } else // not found
    {
        StringStream buf;
        buf << "TID(" << tid << ")";

        //////////////////////////////////////////////////
        // leave critical section
        //////////////////////////////////////////////////

        throw NoSuchElementException(buf.toString());
    }

    //////////////////////////////////////////////////
    // leave critical section
    //////////////////////////////////////////////////

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Finds and returns a specific thread object in the thread pool.
//////////////////////////////////////////////////////////////////////
Thread* ThreadPool::getThread(TID tid) {
    __BEGIN_TRY

    Thread* thread = NULL;

    //////////////////////////////////////////////////
    // enter critical section
    //////////////////////////////////////////////////
    std::lock_guard lock(m_Mutex);

    list<Thread*>::iterator itr = find_if(m_Threads.begin(), m_Threads.end(), isSameTID(tid));

    if (itr != m_Threads.end()) { // found

        Assert(*itr != NULL);

        thread = *itr;

    } else { // not found

        StringStream buf;
        buf << "TID(" << tid << ")";

        //////////////////////////////////////////////////
        // leave critical section
        //////////////////////////////////////////////////

        throw NoSuchElementException(buf.toString());
    }

    //////////////////////////////////////////////////
    // leave critical section
    //////////////////////////////////////////////////

    return thread;

    __END_CATCH
}
