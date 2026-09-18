//////////////////////////////////////////////////////////////////////////////
// Filename    : ObjectRegistry.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __OBJECT_REGISTERY_H__
#define __OBJECT_REGISTERY_H__

#include "Exception.h"
#include "Mutex.h"
#include "Object.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ObjectRegistry;
// Assigns an ObjectID to an object.
//////////////////////////////////////////////////////////////////////////////

class ObjectRegistry {
public:
    ObjectRegistry();

public:
    void registerObject(Object* pObject);
    void registerObject_NOLOCKED(Object* pObject);
    void lock();
    void unlock();

private:
    ObjectID_t m_ObjectIDKey; // Key value used to generate Object IDs
    mutable Mutex m_Mutex;    // mutex
};

#endif
