//////////////////////////////////////////////////////////////////////
//
// Filename    : GCUseOK.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_USE_OK_H__
#define __GC_USE_OK_H__

// include files
#include "ModifyInfo.h"
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCUseOK;
//
//////////////////////////////////////////////////////////////////////

class GCUseOK : public ModifyInfo {
public:
    // Constructor
    GCUseOK();

    // Desctructor
    ~GCUseOK();

    // Initialize from the incoming stream.
    void read(SocketInputStream& iStream);

    // Write the packet payload to the outgoing stream.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_USE_OK;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Use the const static GCUseOKPacketSize for efficiency.
    PacketSize_t getPacketSize() const {
        return ModifyInfo::getPacketSize();
    }

    // get packet name
    string getPacketName() const {
        return "GCUseOK";
    }

    // get packet's debug string
    string toString() const;
};


//////////////////////////////////////////////////////////////////////
//
// class GCUseOKFactory;
//
// Factory for GCUseOK
//
//////////////////////////////////////////////////////////////////////

class GCUseOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_USE_OK;
    static constexpr std::string_view kName = "GCUseOK";
    static constexpr PacketSize_t kMaxSize{ModifyInfo::getPacketMaxSize()};

    // create packet
    Packet* createPacket() override {
        return new GCUseOK();
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
    // Use the const static GCUseOKPacketSize for efficiency.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
