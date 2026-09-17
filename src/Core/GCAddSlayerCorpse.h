//----------------------------------------------------------------------
//
// Filename    : GCAddSlayerCorpse.h
// Written By  : Reiot
//
//----------------------------------------------------------------------

#ifndef __GC_ADD_SLAYER_CORPSE_H__
#define __GC_ADD_SLAYER_CORPSE_H__

// include files
#include "PCSlayerInfo3.h"
#include "Packet.h"
#include "PacketFactory.h"

//----------------------------------------------------------------------
//
// class GCAddSlayerCorpse;
//
// Carries the slayer's corpse information and sends it to the client.
//
//----------------------------------------------------------------------

class GCAddSlayerCorpse : public Packet {
public:
    // constructor
    GCAddSlayerCorpse() {
        m_TreasureCount = 0;
    }
    GCAddSlayerCorpse(const PCSlayerInfo3& slayerInfo) : m_SlayerInfo(slayerInfo), m_TreasureCount(0) {}
    ~GCAddSlayerCorpse(){};


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_SLAYER_CORPSE;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return m_SlayerInfo.getSize() + szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "GCAddSlayerCorpse";
    }

    // get packet's debug string
    string toString() const;


public:
    // get slayer info
    PCSlayerInfo3& getSlayerInfo() {
        return m_SlayerInfo;
    }
    const PCSlayerInfo3& getSlayerInfo() const {
        return m_SlayerInfo;
    }
    void setSlayerInfo(const PCSlayerInfo3& slayerInfo) {
        m_SlayerInfo = slayerInfo;
    }

    // get/set Treasure Count
    BYTE getTreasureCount() const {
        return m_TreasureCount;
    }
    void setTreasureCount(BYTE Count) {
        m_TreasureCount = Count;
    }

private:
    // Slayer's appearance information
    PCSlayerInfo3 m_SlayerInfo;

    // Number of treasures
    BYTE m_TreasureCount;
};


//--------------------------------------------------------------------------------
//
// class GCAddSlayerCorpseFactory;
//
// Factory for GCAddSlayerCorpse
//
//--------------------------------------------------------------------------------

class GCAddSlayerCorpseFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_SLAYER_CORPSE;
    static constexpr std::string_view kName = "GCAddSlayerCorpse";
    static constexpr PacketSize_t kMaxSize{PCSlayerInfo3::getMaxSize() + szBYTE};

    // create packet
    Packet* createPacket() override {
        return new GCAddSlayerCorpse();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCAddSlayerCorpsePacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
//
//--------------------------------------------------------------------------------

#endif
