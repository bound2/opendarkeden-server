//////////////////////////////////////////////////////////////////////
//
// Filename    : CGMove.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_MOVE_H__
#define __CG_MOVE_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class CGMove;
//
//////////////////////////////////////////////////////////////////////

class CGMove : public Packet {
public:
    CGMove(){};
    ~CGMove(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_MOVE;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static CGMovePacketSize.
    PacketSize_t getPacketSize() const {
        return szCoord + szCoord + szDir;
    }

    // get packet name
    string getPacketName() const {
        return "CGMove";
    }

    // get packet's debug string
    string toString() const;

public:
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
// class CGMoveFactory;
//
// Factory for CGMove
//
//////////////////////////////////////////////////////////////////////

class CGMoveFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_MOVE;
    static constexpr std::string_view kName = "CGMove";
    static constexpr PacketSize_t kMaxSize{szCoord + szCoord + szDir};

    // create packet
    Packet* createPacket() override {
        return new CGMove();
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
    // Define and return const static CGMovePacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGMoveHandler;
//
//////////////////////////////////////////////////////////////////////

class CGMoveHandler {
public:
    // execute packet's handler
    static void execute(CGMove* pPacket, Player* player);
};

#endif
