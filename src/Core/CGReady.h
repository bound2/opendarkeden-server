//////////////////////////////////////////////////////////////////////
//
// Filename    : CGReady.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_READY_H__
#define __CG_READY_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class CGReady;
//
//////////////////////////////////////////////////////////////////////

class CGReady : public Packet {
public:
    CGReady(){};
    ~CGReady(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_READY;
    }

    // get packet body size
    // *OPTIMIZATION HINT*
    // Define and return const static CGReadyPacketSize.
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet's name
    string getPacketName() const {
        return "CGReady";
    }

    // get packet's debug string
    string toString() const {
        return "CGReady";
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGReadyFactory;
//
// Factory for CGReady
//
//////////////////////////////////////////////////////////////////////

class CGReadyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_READY;
    static constexpr std::string_view kName = "CGReady";
    static constexpr PacketSize_t kMaxSize{0};

    // create packet
    Packet* createPacket() override {
        return new CGReady();
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
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGReadyHandler;
//
//////////////////////////////////////////////////////////////////////

class CGReadyHandler {
public:
    // execute packet's handler
    static void execute(CGReady* pPacket, Player* pPlayer);
};

#endif
