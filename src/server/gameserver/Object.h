//////////////////////////////////////////////////////////////////////////////
// Filename    : Object.h
// Written By  : Elca
// Description : the root of every class
//////////////////////////////////////////////////////////////////////////////

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "Assert.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// Object Priority
//
// Priority between Object subclass instances in a tile's Object List. A
// smaller value means a higher priority, and the object has to sit nearer
// the front of the tile's Object List.
//
// The most active objects, that is the ones that move the most, should be
// given the highest priority, because the tile's Object List is a slist and
// a slist has the shortest insert/delete times at its front.
//////////////////////////////////////////////////////////////////////////////
enum ObjectPriority {
    OBJECT_PRIORITY_WALKING_CREATURE,
    OBJECT_PRIORITY_FLYING_CREATURE,
    OBJECT_PRIORITY_BURROWING_CREATURE,
    OBJECT_PRIORITY_EFFECT,
    OBJECT_PRIORITY_ITEM,
    OBJECT_PRIORITY_PORTAL,
    OBJECT_PRIORITY_OBSTACLE,
    OBJECT_PRIORITY_NONE // when the object is not a tile object
};

class Packet;

//////////////////////////////////////////////////////////////////////////////
// class Object
// The root of every game class.
//////////////////////////////////////////////////////////////////////////////

class Object {
public:
    // Object Class
    // Classification of Object's subclasses. Only classes that derive
    // directly from Object should be added to ObjectClass.
    enum ObjectClass {
        OBJECT_CLASS_CREATURE,
        OBJECT_CLASS_ITEM,
        OBJECT_CLASS_OBSTACLE,
        OBJECT_CLASS_EFFECT,
        OBJECT_CLASS_PORTAL
    };

public:
    Object(ObjectID_t objectID = 0) : m_ObjectID(objectID) {}
    virtual ~Object() {}

public:
    // get/set object id
    //
    // Used as an identifier that is unique at the zone level. It could have
    // been unique across the game server, but if the game server runs a long
    // time without a reboot, even 4G of ids could start repeating, so the
    // range was narrowed to the zone level. That way, even at 1000 new
    // objects a second, it is safe for 4M seconds, which means 40 to 50
    // days.
    ObjectID_t getObjectID() const {
        Assert(m_ObjectID != 0);
        return m_ObjectID;
    };
    void setObjectID(ObjectID_t objectID) {
        Assert(objectID != 0);
        m_ObjectID = objectID;
    }

    // get object class(virtual)
    // Used to tell whether an Object* pObject is a creature, an item or an
    // obstacle. Subclasses have to override this method.
    //
    // *CAUTION*
    // An m_ObjectClass data member on Object would work too, but compiler
    // alignment would then cost every Object subclass instance extra
    // bytes, so a virtual method is used instead.
    virtual ObjectClass getObjectClass() const = 0;

    // get object priority(virtual)
    virtual ObjectPriority getObjectPriority() const = 0;

    // get debug string
    virtual string toString() const = 0;

    virtual Packet* getAddPacket() const {
        return NULL;
    }

protected:
    ObjectID_t m_ObjectID; // Object ID
};

//////////////////////////////////////////////////////////////////////////////
// function object
//////////////////////////////////////////////////////////////////////////////
class isSameObjectID {
public:
    isSameObjectID(ObjectID_t objectID) : m_ObjectID(objectID) {}

    bool operator()(Object* pObject) {
        return pObject->getObjectID() == m_ObjectID;
    }

private:
    ObjectID_t m_ObjectID;
};

#endif
