//////////////////////////////////////////////////////////////////////
//
// pthreadAPI.cpp
//
// by Reiot, the Fallen Lord of MUDMANIA(TM)
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////
#include "pthreadAPI.h"

#include <errno.h>
#include <pthread.h>

//////////////////////////////////////////////////
//////////////////////////////////////////////////
extern int errno;


//////////////////////////////////////////////////////////////////////
//
// exception version of pthread_create()
//
//////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_create_ex(pthread_t* thread, pthread_attr_t* attr, void* (*start_routine)(void*), void* arg) {
    __BEGIN_TRY

    if (pthread_create(thread, attr, start_routine, arg) < 0) {
        switch (errno) {
        case EAGAIN:
            throw ThreadException("Out of system resources, or too many threads are active.");
        default:
            throw UnknownError(strerror(errno), errno);
        }
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// exception version of pthread_join()
//
//////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_join_ex(pthread_t th, void** thread_return) {
    __BEGIN_TRY

    if (pthread_join(th, thread_return) < 0) {
        switch (errno) {
        case ESRCH:
            throw Error("The specified thread was not found.");
        case EINVAL:
            throw ThreadException("The specified thread is already detached, or another thread is already joining it.");
        case EDEADLK:
            throw Error("A thread cannot join itself.");
        default:
            throw UnknownError(strerror(errno), errno);
        }
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// exception version of pthread_detach()
//
//////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_detach_ex(pthread_t th) {
    __BEGIN_TRY

    if (pthread_detach(th) < 0) {
        switch (errno) {
        case ESRCH:
            throw Error("The specified thread was not found.");
        case EINVAL:
            throw ThreadException("The specified thread is already detached.");
        default:
            throw UnknownError(strerror(errno), errno);
        }
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// exception version of pthread_exit()
//
//////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_exit_ex(void* retval) {
    pthread_exit(retval);
}


//////////////////////////////////////////////////////////////////////
//
// exception version of pthread_self()
//
//////////////////////////////////////////////////////////////////////
pthread_t pthreadAPI::pthread_self_ex() {
    return pthread_self();
}


//////////////////////////////////////////////////////////////////////
//
// exception version of pthread_attr_init()
//
//////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_attr_init_ex(pthread_attr_t* attr) {
    __BEGIN_TRY

    if (pthread_attr_init(attr) != 0)
        throw UnknownError();

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// exception version of pthread_attr_destroy()
//
//////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_attr_destroy_ex(pthread_attr_t* attr) {
    __BEGIN_TRY

    if (pthread_attr_destroy(attr) != 0)
        throw UnknownError();

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// exception version of pthread_getdetachstate()
//
//////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_attr_getdetachstate_ex(const pthread_attr_t* attr, int* detachstate) {
    __BEGIN_TRY

    if (pthread_attr_getdetachstate(attr, detachstate) != 0)
        throw UnknownError();

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// exception version of pthread_setdetachstate()
//
//////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_attr_setdetachstate_ex(pthread_attr_t* attr, int detachstate) {
    __BEGIN_TRY

    if (pthread_attr_setdetachstate(attr, detachstate) < 0) {
        switch (errno) {
        case EINVAL:
            throw Error("invalid thread attribute state");
        default:
            throw UnknownError(strerror(errno), errno);
        }
    }

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_mutex_init()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_mutex_init_ex(pthread_mutex_t* mutex, const pthread_mutexattr_t* mutexattr) {
    __BEGIN_TRY

    if (pthread_mutex_init(mutex, mutexattr) != 0)
        throw UnknownError();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_mutex_destroy()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_mutex_destroy_ex(pthread_mutex_t* mutex) {
    __BEGIN_TRY

    if (pthread_mutex_destroy(mutex) < 0) {
        switch (errno) {
        case EBUSY:
            throw MutexException("The mutex is already locked.");
        default:
            throw UnknownError(strerror(errno), errno);
        }
    }

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_mutex_lock()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_mutex_lock_ex(pthread_mutex_t* mutex) {
    __BEGIN_TRY

    if (pthread_mutex_lock(mutex) < 0) {
        switch (errno) {
        case EINVAL:
            throw Error("The mutex is not properly initialized.");
        case EDEADLK:
            throw MutexException("DEADLOCK - the mutex is already locked by the current thread.");
        default:
            throw UnknownError(strerror(errno), errno);
        }
    }

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_mutex_unlock()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_mutex_unlock_ex(pthread_mutex_t* mutex) {
    __BEGIN_TRY

    if (pthread_mutex_unlock(mutex) < 0) {
        switch (errno) {
        case EINVAL:
            throw Error("The mutex is not properly initialized.");
        case EPERM:
            throw MutexException("The current thread does not hold the mutex.");
        default:
            throw UnknownError(strerror(errno), errno);
        }
    }

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_mutex_trylock()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_mutex_trylock_ex(pthread_mutex_t* mutex) {
    __BEGIN_TRY

    if (pthread_mutex_trylock(mutex) < 0) {
        switch (errno) {
        case EINVAL:
            throw Error("The mutex is not properly initialized.");
        case EBUSY:
            throw MutexException("mutex already locked...");
        default:
            throw UnknownError(strerror(errno), errno);
        }
    }

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_mutexattr_init()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_mutexattr_init_ex(pthread_mutexattr_t* attr) {
    __BEGIN_TRY

    if (pthread_mutexattr_init(attr) != 0)
        throw UnknownError();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_mutexattr_destroy()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_mutexattr_destroy_ex(pthread_mutexattr_t* attr) {
    __BEGIN_TRY

    if (pthread_mutexattr_destroy(attr) != 0)
        throw UnknownError();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_cond_init()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_cond_init_ex(pthread_cond_t* cond, pthread_condattr_t* cond_attr) {
    __BEGIN_TRY

    if (pthread_cond_init(cond, cond_attr) != 0)
        throw UnknownError();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_cond_destroy()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_cond_destroy_ex(pthread_cond_t* cond) {
    __BEGIN_TRY

    switch (pthread_cond_destroy(cond)) {
    case EBUSY:
        throw CondVarException("conditional variable is busy now.");
    default:
        throw UnknownError(strerror(errno), errno);
    }

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_cond_signal()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_cond_signal_ex(pthread_cond_t* cond) {
    __BEGIN_TRY

    if (pthread_cond_signal(cond) != 0)
        throw UnknownError();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_cond_wait()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_cond_wait_ex(pthread_cond_t* cond, pthread_mutex_t* mutex) {
    __BEGIN_TRY

    if (pthread_cond_wait(cond, mutex) != 0)
        throw UnknownError();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_cond_timedwait()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_cond_timedwait_ex(pthread_cond_t* cond, pthread_mutex_t* mutex,
                                           const struct timespec* abstime) {
    __BEGIN_TRY

    switch (pthread_cond_timedwait(cond, mutex, abstime)) {
    case 0:
        break;
    case ETIMEDOUT:
        throw CondVarException("timeout");
    case EINTR:
        throw InterruptedException();
    default:
        throw UnknownError(strerror(errno), errno);
    }

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_cond_broadcast()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_cond_broadcast_ex(pthread_cond_t* cond) {
    __BEGIN_TRY

    if (pthread_cond_broadcast(cond) != 0)
        throw UnknownError();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_condattr_init_ex ()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_condattr_init_ex(pthread_condattr_t* attr) {
    __BEGIN_TRY

    pthread_condattr_init(attr);

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
//
// exception version of pthread_condattr_destroy_ex()
//
////////////////////////////////////////////////////////////////////////////////
void pthreadAPI::pthread_condattr_destroy_ex(pthread_condattr_t* attr) {
    __BEGIN_TRY

    pthread_condattr_destroy(attr);

    __END_CATCH
}
