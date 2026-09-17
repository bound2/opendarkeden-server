//////////////////////////////////////////////////////////////////////
//
// Filename    : GCTakeOutFail.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_TAKE_OUT_FAIL_H__
#define __GC_TAKE_OUT_FAIL_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCTakeOutFail;
//
//////////////////////////////////////////////////////////////////////

class GCTakeOutFail : public Packet {
public:
    GCTakeOutFail(){};
    ~GCTakeOutFail(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_TAKE_OUT_FAIL;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCTakeOutFailPacketSize.
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }

    // get packet name
    string getPacketName() const {
        return "GCTakeOutFail";
    }

    // get packet's debug string
    string toString() const;

public:
    // get / set ObjectID
    ObjectID_t getObjectID() {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

private:
    // ObjectID
    ObjectID_t m_ObjectID = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCTakeOutFailFactory;
//
// Factory for GCTakeOutFail
//
//////////////////////////////////////////////////////////////////////

class GCTakeOutFailFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_TAKE_OUT_FAIL;
    static constexpr std::string_view kName = "GCTakeOutFail";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    // create packet
    Packet* createPacket() override {
        return new GCTakeOutFail();
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
    // Define and return const static GCTakeOutFailPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
