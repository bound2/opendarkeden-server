//////////////////////////////////////////////////////////////////////////////
// Filename    : Obstacle.h
// Written by  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __OBSTACLE_H__
#define __OBSTACLE_H__

#include "Object.h"

//////////////////////////////////////////////////////////////////////////////
// Obstacle Type
//
// Up to 256 kinds exist for each obstacle subtype.
//(Could doors, switches or traps ever go past 256?)
//////////////////////////////////////////////////////////////////////////////
typedef BYTE ObstacleType_t;
const uint szObstacleType = sizeof(ObstacleType_t);

//////////////////////////////////////////////////////////////////////////////
// class Obstacle;
//
// An Object subclass that belongs to a tile, blocks creature movement
// and carries state. Doors, traps and switches are the sort of thing
// that belongs here.
//////////////////////////////////////////////////////////////////////////////

class Obstacle : public Object {
public:
    enum ObstacleClass { OBSTACLE_CLASS_DOOR, OBSTACLE_CLASS_SWITCH, OBSTACLE_CLASS_TRAP };

public:
    Obstacle(ObjectID_t objectID) : Object(objectID) {}
    virtual ~Obstacle() {}

    // methods from Object
public:
    virtual ObjectClass getObjectClass() const {
        return OBJECT_CLASS_OBSTACLE;
    }
    virtual ObjectPriority getObjectPriority() const {
        return OBJECT_PRIORITY_OBSTACLE;
    }
    virtual string toString() const = 0;

    // own methods
public:
    virtual ObstacleClass getObstacleClass() const = 0;
    virtual ObstacleType_t getObstacleType() const = 0;
};

#endif
