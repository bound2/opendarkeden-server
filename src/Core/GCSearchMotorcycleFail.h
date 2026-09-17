//--------------------------------------------------------------------------------
//
// Filename    : GCSearchMotorcycleFail.h
// Description : Packet that tells the player the server-side shop version.
//
//--------------------------------------------------------------------------------

#ifndef __GC_SEARCH_MOTORCYCLE_FAIL_H__
#define __GC_SEARCH_MOTORCYCLE_FAIL_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//--------------------------------------------------------------------------------
//
// class GCSearchMotorcycleFail;
//
//--------------------------------------------------------------------------------

class GCSearchMotorcycleFail : public Packet {
public:
    GCSearchMotorcycleFail(){};
    ~GCSearchMotorcycleFail(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_SEARCH_MOTORCYCLE_FAIL;
    }
    PacketSize_t getPacketSize() const {
        return 0;
    }
    string getPacketName() const {
        return "GCSearchMotorcycleFail";
    }
    string toString() const;
};


//////////////////////////////////////////////////////////////////////
//
// class GCSearchMotorcycleFailFactory;
//
// Factory for GCSearchMotorcycleFail
//
//////////////////////////////////////////////////////////////////////

class GCSearchMotorcycleFailFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SEARCH_MOTORCYCLE_FAIL;
    static constexpr std::string_view kName = "GCSearchMotorcycleFail";
    static constexpr PacketSize_t kMaxSize{0};

    Packet* createPacket() override {
        return new GCSearchMotorcycleFail();
    }
    string getPacketName() const override {
        return string(kName);
    }
    PacketID_t getPacketID() const override {
        return kPacketID;
    }
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
