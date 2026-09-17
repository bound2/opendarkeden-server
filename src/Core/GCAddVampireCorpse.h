//----------------------------------------------------------------------
//
// Filename    : GCAddVampireCorpse.h
// Written By  : Reiot
//
//----------------------------------------------------------------------

#ifndef __GC_ADD_VAMPIRE_CORPSE_H__
#define __GC_ADD_VAMPIRE_CORPSE_H__

// include files
#include "PCVampireInfo3.h"
#include "Packet.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class GCAddVampireCorpse;
//
// When a slayer newly enters a zone through login, a portal or a teleport, or
// when a slayer moves within a zone,(1) the PCs in the area that already hold
// information about this slayer (that is, that can see it) get the GCMove packet
// broadcast to them. But,(2) the PCs in the area that see this slayer
// for the first time get the GCAddVampireCorpse packet broadcast to them. Also,(3)
// this slayer receives, inside GCAddVampireCorpse, the information about the
// slayers within its newly opened field of view.
//
//----------------------------------------------------------------------

class GCAddVampireCorpse : public Packet {
public:
    // constructor
    GCAddVampireCorpse() {
        m_TreasureCount = 0;
    }
    GCAddVampireCorpse(const PCVampireInfo3& vampireInfo) : m_VampireInfo(vampireInfo), m_TreasureCount(0) {}
    ~GCAddVampireCorpse(){};


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_VAMPIRE_CORPSE;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return m_VampireInfo.getSize() + szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "GCAddVampireCorpse";
    }

    // get packet's debug string
    string toString() const;


public:
    // get vampire info
    PCVampireInfo3& getVampireInfo() {
        return m_VampireInfo;
    }
    const PCVampireInfo3& getVampireInfo() const {
        return m_VampireInfo;
    }
    void setVampireInfo(const PCVampireInfo3& vampireInfo) {
        m_VampireInfo = vampireInfo;
    }

    // get/set Treasure Count
    BYTE getTreasureCount() const {
        return m_TreasureCount;
    }
    void setTreasureCount(BYTE Count) {
        m_TreasureCount = Count;
    }

private:
    // Vampire's appearance information
    PCVampireInfo3 m_VampireInfo;

    BYTE m_TreasureCount;
};


//--------------------------------------------------------------------------------
//
// class GCAddVampireCorpseFactory;
//
// Factory for GCAddVampireCorpse
//
//--------------------------------------------------------------------------------

class GCAddVampireCorpseFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_VAMPIRE_CORPSE;
    static constexpr std::string_view kName = "GCAddVampireCorpse";
    static constexpr PacketSize_t kMaxSize{PCVampireInfo3::getMaxSize() + szBYTE};

    // create packet
    Packet* createPacket() override {
        return new GCAddVampireCorpse();
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
