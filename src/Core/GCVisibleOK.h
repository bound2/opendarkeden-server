//////////////////////////////////////////////////////////////////////
//
// Filename    :  GCVisibleOK.h
// Written By  :  Elca
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_VISIBLE_OK_H__
#define __GC_VISIBLE_OK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class  GCVisibleOK;
//
// Packet used when the game server tells the client that a particular user
// has moved. It carries (CreatureID, X, Y, DIR).
//
//////////////////////////////////////////////////////////////////////

class GCVisibleOK : public Packet {
public:
    // constructor
    GCVisibleOK() {}
    ~GCVisibleOK(){};


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_VISIBLE_OK;
    }

    // get packet body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCVisibleOKPacketSize.
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet's name
    string getPacketName() const {
        return "GCVisibleOK";
    }

    // get packet's debug string
    string toString() const;


public:
};


//////////////////////////////////////////////////////////////////////
//
// class GCVisibleOKFactory;
//
// Factory for GCVisibleOK
//
//////////////////////////////////////////////////////////////////////

class GCVisibleOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_VISIBLE_OK;
    static constexpr std::string_view kName = "GCVisibleOK";
    static constexpr PacketSize_t kMaxSize{0};

    // create packet
    Packet* createPacket() override {
        return new GCVisibleOK();
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
    // Define and return const static GCVisibleOKPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


#endif
