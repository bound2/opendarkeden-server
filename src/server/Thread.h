//////////////////////////////////////////////////////////////////////
//
// Thread.h
//
// by Reiot
//
//////////////////////////////////////////////////////////////////////
//
// POSIX Thread Class
//
// To run some routine on a thread, inherit the Thread class, design a
// derived class that reimplements only the run() member function, then
// create an object of the derived class and call Start().
//
// It is used like this.
//
// MDerivedThread * dt = new DerivedThread (...);
// dt->Start();
// delete dt;
//
// The virtual destructor is defined so that the following works properly.
// ( without it the derived class's
// destructor is not called. )
//
// Thread * t = new DerivedThread (...);
// t->Start();
// delete t;
//
// Deleting the thread object has nothing to do with ending the thread.
//
//////////////////////////////////////////////////////////////////////


#ifndef __THREAD_H__
#define __THREAD_H__


#include <atomic>


//////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////

#include "Exception.h"
#include "Types.h"
#include "pthreadAPI.h"


//////////////////////////////////////////////////
// forward declaration
//////////////////////////////////////////////////
class ThreadAttr;
class ThreadException;


//////////////////////////////////////////////////////////////////////
//
// class Thread
//
// POSIX Thread Class
//
//////////////////////////////////////////////////////////////////////

class Thread {
    //////////////////////////////////////////////////
    // constants
    //////////////////////////////////////////////////
public:
    enum ThreadStatus {
        READY,   // ready to run
        RUNNING, // actually running
        EXITING, // just before exit (still doing work)
        EXIT     // completely finished
    };


    //////////////////////////////////////////////////
    // constructor and destructor
    //////////////////////////////////////////////////

public:
    // constructor
    Thread(ThreadAttr* attr = NULL);

    // destructor
    virtual ~Thread() noexcept(false);


    //////////////////////////////////////////////////
    // public methods
    //////////////////////////////////////////////////

public:
    // The trigger function that first sets the thread going. Calling it after
    // creating the thread object calls the derived class's run() member
    // function internally.
    virtual void start();

    // Stops a running thread.
    // Only possible in a derived thread class that uses a mutex internally.
    virtual void stop();

    // Wait for this thread to finish. Cooperative implementations override
    // this so callers do not need to know which threading backend owns it.
    virtual void join();

    // Waits until the thread finishes. Used between threads too. Usually it
    // creates a thread, gives it a task, and sleeps the creating thread until
    // that work is done. It plays the same role as join in a multiprocessing
    // environment.
    //
    // Note that pthread_join() makes the thread running this code wait for the
    // thread given as the parameter. That is, one thread cannot be made to wait
    // for another given thread.
    //
    // ex> Thread t;
    //     Thread::Join ( t );
    static void join(const Thread& t);

    // Ends the current thread. That is, it does not end a given thread but the
    // thread running this method. A given object can be passed as the parameter
    // so that the JOINing thread can take that
    // value.
    //
    // ex> Thread::Exit();
    //     or
    //     Thread::Exit(retval);
    static void exit(void* retval = NULL);

    // Where the code that runs independently on the thread goes. A subclass of
    // the Thread class always has to reimplement this function.
    virtual void run() {};


    //////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////
public:
    // get current thread's tid
    static TID self();

    // For debugging. Returns the thread's information as a string.
    virtual string toString() const;

    // get thread identifier
    TID getTID() const {
        return m_TID;
    }

    // get/set thread's status
    ThreadStatus getStatus() const {
        return m_Status.load(std::memory_order_acquire);
    }
    void setStatus(ThreadStatus status) {
        m_Status.store(status, std::memory_order_release);
    }

    // get thread name
    virtual string getName() const {
        return "Thread";
    }


    //////////////////////////////////////////////////
    // data members
    //////////////////////////////////////////////////

protected:
    void setTID(TID tid) {
        m_TID = tid;
    }

private:
    // thread identifier variable
    TID m_TID;

    // thread-attribute object
    ThreadAttr* m_ThreadAttr;

    // thread status
    std::atomic<ThreadStatus> m_Status;
};


//////////////////////////////////////////////////
// thread function used at pthread_create()
//////////////////////////////////////////////////
void* start_routine(void* derivedThread);

#endif
