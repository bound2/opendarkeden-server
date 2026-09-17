//----------------------------------------------------------------------
//
// Filename    : GCSetPosition.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __GC_SET_POSITION_H__
#define __GC_SET_POSITION_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class GCSetPosition;
//
// Packet with which the game server sets the player's position.
// It is expected to be folded into the GCPatchPCInfo (working name) packet later.
//
//----------------------------------------------------------------------

class GCSetPosition : public Packet {
public:
    GCSetPosition(){};
    ~GCSetPosition(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SET_POSITION;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCSetPositionPacketSize.
    PacketSize_t getPacketSize() const {
        return szCoord + szCoord + szDir;
    }

    // get packet name
    string getPacketName() const {
        return "GCSetPosition";
    }

    // get packet's debug string
    string toString() const;

    // get/set X Coordicate
    Coord_t getX() const {
        return m_X;
    }
    void setX(Coord_t x) {
        m_X = x;
    }

    // get/set Y Coordicate
    Coord_t getY() const {
        return m_Y;
    }
    void setY(Coord_t y) {
        m_Y = y;
    }

    // get/set Direction
    Dir_t getDir() const {
        return m_Dir;
    }
    void setDir(Dir_t dir) {
        m_Dir = dir;
    }

private:
    Coord_t m_X; // X coordinate
    Coord_t m_Y; // Y coordinate
    Dir_t m_Dir; // Direction
};


//////////////////////////////////////////////////////////////////////
//
// class GCSetPositionFactory;
//
// Factory for GCSetPosition
//
//////////////////////////////////////////////////////////////////////

class GCSetPositionFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SET_POSITION;
    static constexpr std::string_view kName = "GCSetPosition";
    static constexpr PacketSize_t kMaxSize{szCoord + szCoord + szDir};

    // create packet
    Packet* createPacket() override {
        return new GCSetPosition();
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
    // Define and return const static GCSetPositionPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
