//////////////////////////////////////////////////////////////////////
//
// Filename    : CGVisible.h
// Written By  : crazydog
// Description : When a bat or a wolf wants to return to its original shape..
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_VISIBLE_H__
#define __CG_VISIBLE_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class CGVisible;
//
//////////////////////////////////////////////////////////////////////

class CGVisible : public Packet {
public:
    CGVisible(){};
    virtual ~CGVisible(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_VISIBLE;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static CGVisiblePacketSize.
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet name
    string getPacketName() const {
        return "CGVisible";
    }

    // get packet's debug string
    string toString() const;
};


//////////////////////////////////////////////////////////////////////
//
// class CGVisibleFactory;
//
// Factory for CGVisible
//
//////////////////////////////////////////////////////////////////////

class CGVisibleFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_VISIBLE;
    static constexpr std::string_view kName = "CGVisible";
    static constexpr PacketSize_t kMaxSize{0};

    // create packet
    Packet* createPacket() override {
        return new CGVisible();
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
    // Define and return const static CGVisiblePacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGVisibleHandler;
//
//////////////////////////////////////////////////////////////////////

class CGVisibleHandler {
public:
    // execute packet's handler
    static void execute(CGVisible* pPacket, Player* player);
};

#endif
