//////////////////////////////////////////////////////////////////////
//
// ThreadAttr.h
//
// by Reiot
//
//////////////////////////////////////////////////////////////////////

#ifndef __THREAD_ATTR_H__
#define __THREAD_ATTR_H__

//////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////
#include <pthread.h>

#include "Exception.h"
#include "Types.h"


//////////////////////////////////////////////////
// forward declaration
//////////////////////////////////////////////////
class ThreadAttrException;


//////////////////////////////////////////////////////////////////////
//
// class ThreadAttr;
//
// The Thread-Attribute class is used as the parameter to pthread_create()
// when several Thread objects with the same attribute have to be created.
// That is, only one Thread-Attribute object has to be created.
// (remember specifying the thread-flag on every creation with Solaris
// threads.)
//
// In practice it is hardly used, because the most useful
// setstacksize() is not provided and detach is covered by
// pthread_detach().
//
//////////////////////////////////////////////////////////////////////

class ThreadAttr {
    //////////////////////////////////////////////////
    // constructor / destructor
    //////////////////////////////////////////////////
public:
    // constructor
    ThreadAttr();

    // destructor
    ~ThreadAttr();


    //////////////////////////////////////////////////
    // public methods
    //////////////////////////////////////////////////
public:
    //
    // return thread-attribute object
    //
    // *CAUTION*
    //
    // do not return pthread_attr_t value !!
    // use pthread_attr_t pointer instead.
    // (assignment may not be supported for pthread_attr_t)
    //
    pthread_attr_t* getAttr() {
        return &attr;
    }

    // is thread attribute detached state?
    bool isDetached() const {
        return getDetachState() == PTHREAD_CREATE_DETACHED;
    }

    // set thread attribute detached state
    void setDetached() {
        setDetachState(PTHREAD_CREATE_DETACHED);
    }

    // is thread attribute joinable state?
    bool isJoinable() const {
        return getDetachState() == PTHREAD_CREATE_JOINABLE;
    }

    // set thread attribute joinable state
    void setJoinable() {
        setDetachState(PTHREAD_CREATE_JOINABLE);
    }


    //////////////////////////////////////////////////
    // protected methods
    //////////////////////////////////////////////////

protected:
    // get thread attribute's state
    int getDetachState() const;

    // set thread attribute's state
    void setDetachState(int state);


    //////////////////////////////////////////////////
    // attributes
    //////////////////////////////////////////////////

private:
    // thread attribute
    pthread_attr_t attr;
};

#endif
