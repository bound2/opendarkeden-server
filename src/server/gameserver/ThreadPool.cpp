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
// 컨테이너속의 쓰레드 객체가 특정 TID를 갖고 있을 경우 true를 리턴한다.
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
// 포함하고 있는 모든 쓰레드 객체를 삭제해야 한다.
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

        // 쓰레드는 종료한 상태여야 한다.
        Assert(temp != NULL && temp->getStatus() == Thread::EXIT);

        SAFE_DELETE(temp);
    }

    m_Threads.erase(m_Threads.begin() , m_Threads.end());

    g_pLogManager->Log5("after erase(begin , end) , list's size == %d\n" , m_Threads.size());

    */

    list<Thread*>::iterator itr;

    while ((itr = m_Threads.begin()) != m_Threads.end()) {
        // 아직도 리스트에 노드가 남아있다는 뜻이다.

        // 쓰레드는 종료한 상태여야 한다.
        Assert(*itr != NULL && (*itr)->getStatus() == Thread::EXIT);

        SAFE_DELETE(*itr);

        m_Threads.pop_front();
    }

    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// 쓰레드풀안에 등록된 쓰레드들을 RUNNING 상태로 만든다.
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
// 쓰레드풀안에 등록된 모든 쓰레드의 실행을 중단시킨다.
// (이는 singal 혹은 cancellation 으로 구현해야 하겠다.)
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
// 쓰레드풀에 쓰레드 객체를 등록한다.
//////////////////////////////////////////////////////////////////////
void ThreadPool::addThread(Thread* thread)

{
    __BEGIN_TRY

    //////////////////////////////////////////////////
    // enter critical section
    //////////////////////////////////////////////////
    std::lock_guard lock(m_Mutex);

    // 쓰레드는 널이 아니어야 한다.
    Assert(thread != NULL);

    // 리스트의 맨 마지막에 쓰레드 객체를 삽입한다.
    m_Threads.push_back(thread);

    string msg = "== " + thread->getName() + " added to thread pool";
    log(LOG_DEBUG_MSG, "", "", msg);

    //////////////////////////////////////////////////
    // leave critical section
    //////////////////////////////////////////////////

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 쓰레드풀에서 특정 쓰레드 객체를 삭제한다.
//////////////////////////////////////////////////////////////////////
void ThreadPool::deleteThread(TID tid) {
    __BEGIN_TRY

    //////////////////////////////////////////////////
    // enter critical section
    //////////////////////////////////////////////////
    std::lock_guard lock(m_Mutex);

    // function object로 특정 TID를 가진 쓰레드 객체가 담긴 노드를 담은
    // iterator를 찾아낸다.
    list<Thread*>::iterator itr = find_if(m_Threads.begin(), m_Threads.end(), isSameTID(tid));

    if (itr != m_Threads.end()) // found!
    {
        // 쓰레드 객체를 임시로 저장해둔다.
        Thread* temp = *itr;

        // 쓰레드는 종료한 상태여야 한다.
        // 하위 클래스에 Mutex가 존재할 경우, getStatus(), setStatus()는 Mutex로 보호되어야 한다.
        Assert(temp != NULL && temp->getStatus() == Thread::EXIT);

        StringStream msg;
        msg << "== Thread[" << temp->getTID() << "] has been removed from ThreadPool ==";
        log(LOG_DEBUG_MSG, "", "", msg.toString());

        // 쓰레드 객체를 삭제한다.
        SAFE_DELETE(temp);

        // 노드를 삭제한다.
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
// 쓰레드풀에서 특정 쓰레드 객체를 찾아서 리턴한다.
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
