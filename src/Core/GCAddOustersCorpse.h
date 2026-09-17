//----------------------------------------------------------------------
//
// Filename    : GCAddOustersCorpse.h
// Written By  : Reiot
//
//----------------------------------------------------------------------

#ifndef __GC_ADD_OUSTERS_CORPSE_H__
#define __GC_ADD_OUSTERS_CORPSE_H__

// include files
#include "PCOustersInfo3.h"
#include "Packet.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class GCAddOustersCorpse;
//
//----------------------------------------------------------------------

class GCAddOustersCorpse : public Packet {
public:
    // constructor
    GCAddOustersCorpse() {
        m_TreasureCount = 0;
    }
    GCAddOustersCorpse(const PCOustersInfo3& oustersInfo) : m_OustersInfo(oustersInfo), m_TreasureCount(0) {}
    ~GCAddOustersCorpse(){};


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_OUSTERS_CORPSE;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return m_OustersInfo.getSize() + szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "GCAddOustersCorpse";
    }

    // get packet's debug string
    string toString() const;


public:
    // get ousters info
    PCOustersInfo3& getOustersInfo() {
        return m_OustersInfo;
    }
    const PCOustersInfo3& getOustersInfo() const {
        return m_OustersInfo;
    }
    void setOustersInfo(const PCOustersInfo3& oustersInfo) {
        m_OustersInfo = oustersInfo;
    }

    // get/set Treasure Count
    BYTE getTreasureCount() const {
        return m_TreasureCount;
    }
    void setTreasureCount(BYTE Count) {
        m_TreasureCount = Count;
    }

private:
    PCOustersInfo3 m_OustersInfo;

    BYTE m_TreasureCount;
};


//--------------------------------------------------------------------------------
//
// class GCAddOustersCorpseFactory;
//
// Factory for GCAddOustersCorpse
//
//--------------------------------------------------------------------------------

class GCAddOustersCorpseFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_OUSTERS_CORPSE;
    static constexpr std::string_view kName = "GCAddOustersCorpse";
    static constexpr PacketSize_t kMaxSize{PCOustersInfo3::getMaxSize() + szBYTE};

    // create packet
    Packet* createPacket() override {
        return new GCAddOustersCorpse();
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
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
//
//--------------------------------------------------------------------------------

#endif
