//////////////////////////////////////////////////////////////////////
//
// Filename    :  GCUntransformOK.h
// Written By  :  Elca
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_UNTRANSFORM_OK_H__
#define __GC_UNTRANSFORM_OK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class  GCUntransformOK;
//
// Packet used when the game server tells the client that a particular user
// has moved. It carries (CreatureID, X, Y, DIR).
//
//////////////////////////////////////////////////////////////////////

class GCUntransformOK : public Packet {
public:
    // constructor
    GCUntransformOK() {}
    GCUntransformOK(Coord_t x, Coord_t y, Dir_t dir) : m_X(x), m_Y(y), m_Dir(dir) {}
    ~GCUntransformOK(){};


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_UNTRANSFORM_OK;
    }

    // get packet body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCUntransformOKPacketSize.
    PacketSize_t getPacketSize() const {
        return szCoord + szCoord + szDir;
    }

    // get packet's name
    string getPacketName() const {
        return "GCUntransformOK";
    }

    // get packet's debug string
    string toString() const;


public:
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
    Coord_t m_X = 0; // Target X coordinate
    Coord_t m_Y = 0; // Target Y coordinate
    Dir_t m_Dir = 0; // Target direction
};


//////////////////////////////////////////////////////////////////////
//
// class GCUntransformOKFactory;
//
// Factory for GCUntransformOK
//
//////////////////////////////////////////////////////////////////////

class GCUntransformOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_UNTRANSFORM_OK;
    static constexpr std::string_view kName = "GCUntransformOK";
    static constexpr PacketSize_t kMaxSize{szCoord + szCoord + szDir};

    // create packet
    Packet* createPacket() override {
        return new GCUntransformOK();
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
    // Define and return const static GCUntransformOKPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


#endif
