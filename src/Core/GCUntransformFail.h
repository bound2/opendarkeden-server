//////////////////////////////////////////////////////////////////////
//
// Filename    : GCUntransformFail.h
// Written By  : crazydog
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_UNTRANSFORM_FAIL_H__
#define __GC_UNTRANSFORM_FAIL_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class GCUntransformFail;
//
//////////////////////////////////////////////////////////////////////

class GCUntransformFail : public Packet {
public:
    // constructor
    GCUntransformFail() {}
    ~GCUntransformFail(){};


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_UNTRANSFORM_FAIL;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet's name
    string getPacketName() const {
        return "GCUntransformFail";
    }

    // get packet's debug string
    string toString() const;


public:
private:
};


//////////////////////////////////////////////////////////////////////
//
// class  GCUntransformFailFactory;
//
// Factory for  GCUntransformFail
//
//////////////////////////////////////////////////////////////////////

class GCUntransformFailFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_UNTRANSFORM_FAIL;
    static constexpr std::string_view kName = "GCUntransformFail";
    static constexpr PacketSize_t kMaxSize{0};

    // create packet
    Packet* createPacket() override {
        return new GCUntransformFail();
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


#endif
