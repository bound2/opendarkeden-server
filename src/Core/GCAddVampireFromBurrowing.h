//----------------------------------------------------------------------
//
// Filename    : GCAddVampireFromBurrowing.h
// Written By  : Reiot
//
//----------------------------------------------------------------------

#ifndef __GC_ADD_VAMPIRE_FROM_BURROWING_H__
#define __GC_ADD_VAMPIRE_FROM_BURROWING_H__

// include files
#include "EffectInfo.h"
#include "PCVampireInfo3.h"
#include "Packet.h"
#include "PacketFactory.h"

//----------------------------------------------------------------------
//
// class GCAddVampireFromBurrowing;
//
// When a slayer newly enters a zone through login, a portal or a
// teleport, or
// when a slayer moves within a zone,(1) the PCs in the area that
// already hold information about
// this slayer (that is, that can see it) get the
// GCMove packet
// broadcast to them. But,(2) the PCs in the area that see this slayer
// for the first time get the GCAddVampireFromBurrowing packet broadcast to them. Also,(3) this slayer receives, inside
// GCAddVampireFromBurrowing, the information about the slayers within its newly opened field of view.
//
//----------------------------------------------------------------------

class GCAddVampireFromBurrowing : public Packet {
public:
    // constructor
    GCAddVampireFromBurrowing() : m_pEffectInfo(NULL) {}
    GCAddVampireFromBurrowing(const PCVampireInfo3& vampireInfo) : m_VampireInfo(vampireInfo), m_pEffectInfo(NULL) {}

    virtual ~GCAddVampireFromBurrowing() noexcept;


public:
    // Read data from the input stream (buffer) and initialise
    // the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_VAMPIRE_FROM_BURROWING;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        // A packet carrying no effect record puts an empty list on the wire.
        EffectInfo noEffects;
        const EffectInfo& effects = (m_pEffectInfo != NULL) ? *m_pEffectInfo : noEffects;

        return m_VampireInfo.getSize() + effects.getSize();
    }

    // get packet's name
    string getPacketName() const {
        return "GCAddVampireFromBurrowing";
    }

    // get packet's debug string
    string toString() const;


public:
    // get/set vampire info
    PCVampireInfo3& getVampireInfo() {
        return m_VampireInfo;
    }
    const PCVampireInfo3& getVampireInfo() const {
        return m_VampireInfo;
    }
    void setVampireInfo(const PCVampireInfo3& vampireInfo) {
        m_VampireInfo = vampireInfo;
    }

    // get /set Effect Info
    EffectInfo* getEffectInfo() const {
        return m_pEffectInfo;
    }
    void setEffectInfo(EffectInfo* pEffectInfo) {
        m_pEffectInfo = pEffectInfo;
    }


private:
    // Vampire's appearance information
    PCVampireInfo3 m_VampireInfo;

    // Effect information
    EffectInfo* m_pEffectInfo;
};


//--------------------------------------------------------------------------------
//
// class GCAddVampireFromBurrowingFactory;
//
// Factory for GCAddVampireFromBurrowing
//
//--------------------------------------------------------------------------------

class GCAddVampireFromBurrowingFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_VAMPIRE_FROM_BURROWING;
    static constexpr std::string_view kName = "GCAddVampireFromBurrowing";
    static constexpr PacketSize_t kMaxSize{PCVampireInfo3::getMaxSize() + EffectInfo::getMaxSize()};

    // create packet
    Packet* createPacket() override {
        return new GCAddVampireFromBurrowing();
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
