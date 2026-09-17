////////////////////////////////////////////////////////////////////////////////
//
// Thread.cpp
//
// by Reiot, the Lord of MUDMANIA(TM)
//
// Last Updated : 1999. 07. 02.
//
////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////
#include "Thread.h"

#include "ThreadAttr.h"
#include "pthreadAPI.h"

using namespace pthreadAPI;


////////////////////////////////////////////////////////////////////////////////
//
// constructor
//
////////////////////////////////////////////////////////////////////////////////
//
// A DerivedThread inheriting the Thread class takes the ThreadAttr given here.
// If several DerivedThreads need the same attribute, build one ThreadAttr,
// hand its pointer to every constructor, and delete the Thread-Attribute
// object once every thread is gone.
//
// Note that creating the thread and creating the object do not happen at the
// same time: the ThreadAttr object has to stay alive until start() is called,
// or the creation fails.
//
// The object itself has to be created with new, because pthread_create() is
// handed the object's address as a parameter; an object on the function's
// stack would be gone by the Scope Rule. So a thread object must live on
// the (Heap)!!!
//
////////////////////////////////////////////////////////////////////////////////
Thread::Thread(ThreadAttr* attr) : m_TID(0), m_ThreadAttr(attr), m_Status(Thread::READY) {}


////////////////////////////////////////////////////////////////////////////////
//
// destructor (virtual)
//
////////////////////////////////////////////////////////////////////////////////
//
// It is declared virtual so that an attribute a DerivedThread class added can
// be released. It does nothing itself.
//
////////////////////////////////////////////////////////////////////////////////
Thread::~Thread() noexcept(false) {}


////////////////////////////////////////////////////////////////////////////////
//
// Starts the thread.
//
////////////////////////////////////////////////////////////////////////////////
//
// A look at pthread_create().
//
// int pthread_create ( pthread_t * tid ,
//                      pthread_attr_t * atttr ,
//                      void * (*start_routine)(void*) ,
//                      void * arg
//                    );
//
// It returns 0 on success and an error code on failure.
// An error code of EAGAIN means the system is out of resources or there are
// too many threads.
//
// Passing a pointer to tid as the first parameter stores the thread identifier
// in it.
//
// With attr NULL the thread is created with the Default Attribute. To set a
// particular attribute, create a pthread_attr_t and set it
// there.
//
// start_routine is a friend method of the Thread class, and arg is passed
// this - the Thread Object. Inside start_routine, polymorphism then calls the
// derived class's run() method automatically,
// which is what makes it work.
//
////////////////////////////////////////////////////////////////////////////////
void Thread::start() {
    __BEGIN_TRY

    if (getStatus() != Thread::READY)
        throw ThreadException("invalid thread's status");

    pthread_create_ex(&m_TID, (m_ThreadAttr == NULL ? NULL : m_ThreadAttr->getAttr()), start_routine, this);

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// A derived class has to reimplement this with a mutex of its own.
//
////////////////////////////////////////////////////////////////////////////////
void Thread::stop() {
    __BEGIN_TRY

    throw UnsupportedError();

    __END_CATCH
}

void Thread::join() {
    __BEGIN_TRY

    pthread_join_ex(m_TID, NULL);

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
//
// Waits for a given thread to finish before the current thread goes on.
//
// Note that it *waits*. Thread A does not make thread B wait,
// so this method has to be static.
//
// status here may be any data type. (structure, class...)
//
////////////////////////////////////////////////////////////////////////////////
void Thread::join(const Thread& t) {
    const_cast<Thread&>(t).join();
}


////////////////////////////////////////////////////////////////////////////////
//
// Ends the current thread.
//
// void pthread_exit ( void * retval );
//
// If needed, any data type may be passed back to the thread that joins.
// (then again, most threads run detached, so returning NULL
// is usually good enough.)
//
////////////////////////////////////////////////////////////////////////////////
void Thread::exit(void* retval) {
    pthread_exit_ex(retval);
}


////////////////////////////////////////////////////////////////////////////////
//
// thread's start routine
//
// It is a friend method of the Thread class.
//
// It is a REENTRANT function, so it should be fine here.. every thread
// keeps its own thread specific data. Running inside one object should make no difference.
//
// Note that it sets the thread's status to RUNNING before calling the derived
// class's virtual run(), and to EXIT after. Once it is EXIT the thread no longer runs.
//
////////////////////////////////////////////////////////////////////////////////
void* start_routine(void* derivedThread) {
    Thread* thread = (Thread*)derivedThread;

    // set thread's status to "RUNNING"
    thread->setStatus(Thread::RUNNING);

    // here - polymorphism used. (derived::run() called.)
    thread->run();

    // set thread's status to "EXIT"
    thread->setStatus(Thread::EXIT);

    Thread::exit(NULL);

    return NULL; // avoid compiler's warning
}


////////////////////////////////////////////////////////////////////////////////
//
// Finds the current thread's TID. It is a static member function.
//
////////////////////////////////////////////////////////////////////////////////
TID Thread::self() {
    return pthread_self_ex();
}


////////////////////////////////////////////////////////////////////////////////
//
// return thread information string
//
////////////////////////////////////////////////////////////////////////////////
string Thread::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "Thread[" << (ulong)(uintptr_t)m_TID << "]";
    return msg.toString();

    __END_CATCH
}
