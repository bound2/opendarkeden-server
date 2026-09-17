//////////////////////////////////////////////////////////////////////
//
// Filename    : LCCreatePCOK.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_CREATE_PC_OK_H__
#define __LC_CREATE_PC_OK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class LCCreatePCOK;
//
// Packet with which the login server tells the client that login succeeded.
//
//////////////////////////////////////////////////////////////////////

class LCCreatePCOK : public Packet {
public:
    LCCreatePCOK(){};
    ~LCCreatePCOK(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream) {}

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const {}


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_CREATE_PC_OK;
    }

    // get packet body size
    // *OPTIMIZATION HINT*
    // Define and return const static LCCreatePCOKPacketSize.
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet's name
    string getPacketName() const {
        return "LCCreatePCOK";
    }

    // get packet's debug string
    string toString() const {
        return "LCCreatePCOK";
    }
};


//////////////////////////////////////////////////////////////////////
//
// class LCCreatePCOKFactory;
//
// Factory for LCCreatePCOK
//
//////////////////////////////////////////////////////////////////////

class LCCreatePCOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_CREATE_PC_OK;
    static constexpr std::string_view kName = "LCCreatePCOK";
    static constexpr PacketSize_t kMaxSize{0};

    // create packet
    Packet* createPacket() override {
        return new LCCreatePCOK();
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
//
//////////////////////////////////////////////////////////////////////

#endif
