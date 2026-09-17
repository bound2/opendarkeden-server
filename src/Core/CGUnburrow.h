//////////////////////////////////////////////////////////////////////
//
// Filename    : CGUnburrow.h
// Written By  : crazydog
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_UNBURROW_H__
#define __CG_UNBURROW_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class CGUnburrow;
//
//////////////////////////////////////////////////////////////////////

class CGUnburrow : public Packet {
public:
    CGUnburrow(){};
    virtual ~CGUnburrow(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_UNBURROW;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static CGUnburrowPacketSize.
    PacketSize_t getPacketSize() const {
        return szCoord + szCoord + szDir;
    }

    // get packet name
    string getPacketName() const {
        return "CGUnburrow";
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
    Coord_t m_X = 0; // X coordinate
    Coord_t m_Y = 0; // Y coordinate
    Dir_t m_Dir = 0; // Direction
};


//////////////////////////////////////////////////////////////////////
//
// class CGUnburrowFactory;
//
// Factory for CGUnburrow
//
//////////////////////////////////////////////////////////////////////

class CGUnburrowFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_UNBURROW;
    static constexpr std::string_view kName = "CGUnburrow";
    static constexpr PacketSize_t kMaxSize{szCoord + szCoord + szDir};

    // create packet
    Packet* createPacket() override {
        return new CGUnburrow();
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
    // Define and return const static CGUnburrowPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGUnburrowHandler;
//
//////////////////////////////////////////////////////////////////////

class CGUnburrowHandler {
public:
    // execute packet's handler
    static void execute(CGUnburrow* pPacket, Player* player);
};

#endif
