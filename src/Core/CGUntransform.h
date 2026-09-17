//////////////////////////////////////////////////////////////////////
//
// Filename    : CGUntransform.h
// Written By  : crazydog
// Description : When a bat or a wolf wants to return to its original shape..
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_UNTRANSFORM_H__
#define __CG_UNTRANSFORM_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class CGUntransform;
//
//////////////////////////////////////////////////////////////////////

class CGUntransform : public Packet {
public:
    CGUntransform(){};
    virtual ~CGUntransform(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_UNTRANSFORM;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static CGUntransformPacketSize.
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet name
    string getPacketName() const {
        return "CGUntransform";
    }

    // get packet's debug string
    string toString() const;
};


//////////////////////////////////////////////////////////////////////
//
// class CGUntransformFactory;
//
// Factory for CGUntransform
//
//////////////////////////////////////////////////////////////////////

class CGUntransformFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_UNTRANSFORM;
    static constexpr std::string_view kName = "CGUntransform";
    static constexpr PacketSize_t kMaxSize{0};

    // create packet
    Packet* createPacket() override {
        return new CGUntransform();
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
    // Define and return const static CGUntransformPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGUntransformHandler;
//
//////////////////////////////////////////////////////////////////////

class CGUntransformHandler {
public:
    // execute packet's handler
    static void execute(CGUntransform* pPacket, Player* player);
};

#endif
