//////////////////////////////////////////////////////////////////////
//
// Filename    : GCFakeMove.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_FAKE_MOVE_H__
#define __GC_FAKE_MOVE_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCFakeMove;
//
// Packet object used when the game server tells the client that a particular
// user has moved. It holds (ObjectID,X,Y,DIR).
//
//////////////////////////////////////////////////////////////////////

class GCFakeMove : public Packet {
public:
    // constructor
    GCFakeMove() {}
    GCFakeMove(ObjectID_t objectID, Coord_t x, Coord_t y, Coord_t x2, Coord_t y2)
        : m_ObjectID(objectID), m_ToX(x2), m_ToY(y2) {}
    ~GCFakeMove(){};


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_FAKE_MOVE;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCFakeMovePacketSize.
    PacketSize_t getPacketSize() const {
        return szObjectID + 2 * szCoord;
    }

    // get packet's name
    string getPacketName() const {
        return "GCFakeMove";
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

    void setXY(Coord_t x2, Coord_t y2) {
        m_ToX = x2;
        m_ToY = y2;
    }

    // get
    Coord_t getToX() const {
        return m_ToX;
    }
    Coord_t getToY() const {
        return m_ToY;
    }


private:
    ObjectID_t m_ObjectID = 0;    // Creature id
    Coord_t m_ToX = 0, m_ToY = 0; // Destination coordinates
};


//////////////////////////////////////////////////////////////////////
//
// class GCFakeMoveFactory;
//
// Factory for GCFakeMove
//
//////////////////////////////////////////////////////////////////////

class GCFakeMoveFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_FAKE_MOVE;
    static constexpr std::string_view kName = "GCFakeMove";
    static constexpr PacketSize_t kMaxSize{szObjectID + 2 * szCoord};

    // create packet
    Packet* createPacket() override {
        return new GCFakeMove();
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
    // Define and return const static GCFakeMovePacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
