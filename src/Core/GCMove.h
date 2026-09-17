//////////////////////////////////////////////////////////////////////
//
// Filename    : GCMove.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_MOVE_H__
#define __GC_MOVE_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCMove;
//
// Packet used when the game server tells the client that a particular user
// has moved. It carries (ObjectID, X, Y, DIR).
//
//////////////////////////////////////////////////////////////////////

class GCMove : public Packet {
public:
    // constructor
    GCMove() {}
    GCMove(ObjectID_t objectID, Coord_t x, Coord_t y, Dir_t dir) : m_ObjectID(objectID), m_X(x), m_Y(y), m_Dir(dir) {}
    ~GCMove(){};


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_MOVE;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCMovePacketSize.
    PacketSize_t getPacketSize() const {
        return szObjectID + szCoord + szCoord + szDir;
    }

    // get packet's name
    string getPacketName() const {
        return "GCMove";
    }

    // get packet's debug string
    string toString() const;


public:
    // get/set Creature ID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t objectID) {
        m_ObjectID = objectID;
    }

    // get/set X
    Coord_t getX() const {
        return m_X;
    }
    void setX(Coord_t x) {
        m_X = x;
    }

    // get/set Y
    Coord_t getY() const {
        return m_Y;
    }
    void setY(Coord_t y) {
        m_Y = y;
    }

    // get/set Dir
    Dir_t getDir() const {
        return m_Dir;
    }
    void setDir(Dir_t dir) {
        m_Dir = dir;
    }


private:
    ObjectID_t m_ObjectID = 0; // Creature id
    Coord_t m_X = 0;           // X coordinate
    Coord_t m_Y = 0;           // Y coordinate
    Dir_t m_Dir = 0;           // Direction
};


//////////////////////////////////////////////////////////////////////
//
// class GCMoveFactory;
//
// Factory for GCMove
//
//////////////////////////////////////////////////////////////////////

class GCMoveFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MOVE;
    static constexpr std::string_view kName = "GCMove";
    static constexpr PacketSize_t kMaxSize{szObjectID + szCoord + szCoord + szDir};

    // create packet
    Packet* createPacket() override {
        return new GCMove();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's max body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCMovePacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
