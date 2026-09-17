//--------------------------------------------------------------------------------
//
// Filename    : GCLightning.h
// Written By  : reiot
//
//--------------------------------------------------------------------------------

#ifndef __GC_LIGHTNING_H__
#define __GC_LIGHTNING_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCLightning;
//
// Packet the server sends to every client in the zone when lightning strikes.
//
//
//////////////////////////////////////////////////////////////////////

class GCLightning : public Packet {
public:
    GCLightning(){};
    ~GCLightning(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_LIGHTNING;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCLightningPacketSize.
    PacketSize_t getPacketSize() const {
        return szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "GCLightning";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set delay
    BYTE getDelay() const {
        return m_Delay;
    }
    void setDelay(BYTE delay) {
        m_Delay = delay;
    }


private:
    // Delay from the lightning until the thunder
    // 1 -> 0.1 second
    BYTE m_Delay = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCLightningFactory;
//
// Factory for GCLightning
//
//////////////////////////////////////////////////////////////////////

class GCLightningFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_LIGHTNING;
    static constexpr std::string_view kName = "GCLightning";
    static constexpr PacketSize_t kMaxSize{szBYTE};

    // create packet
    Packet* createPacket() override {
        return new GCLightning();
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
    // Define and return const static GCLightningPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
