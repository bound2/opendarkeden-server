//////////////////////////////////////////////////////////////////////
//
// Filename    : GCHPRecoveryEndToSelf.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_HP_RECOVERY_END_TO_SELF_H__
#define __GC_HP_RECOVERY_END_TO_SELF_H__

// include files
#include "EffectInfo.h"
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCHPRecoveryEndToSelf;
//
////////////////////////////////////////////////////////////////////

class GCHPRecoveryEndToSelf : public Packet {
public:
    GCHPRecoveryEndToSelf();

    virtual ~GCHPRecoveryEndToSelf();

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_HP_RECOVERY_END_TO_SELF;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCHPRecoveryEndToSelfPacketSize.
    PacketSize_t getPacketSize() const {
        return szHP;
    }

    // get packet's name
    string getPacketName() const {
        return "GCHPRecoveryEndToSelf";
    }

    // get packet's debug string
    string toString() const;

public:
    // get /set CurrentHP
    HP_t getCurrentHP() const {
        return m_CurrentHP;
    }
    void setCurrentHP(HP_t CurrentHP) {
        m_CurrentHP = CurrentHP;
    }

private:
    // Current HP
    HP_t m_CurrentHP;
};


//////////////////////////////////////////////////////////////////////
//
// class GCHPRecoveryEndToSelfFactory;
//
// Factory for GCHPRecoveryEndToSelf
//
//////////////////////////////////////////////////////////////////////

class GCHPRecoveryEndToSelfFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_HP_RECOVERY_END_TO_SELF;
    static constexpr std::string_view kName = "GCHPRecoveryEndToSelf";
    static constexpr PacketSize_t kMaxSize{szHP};

    // create packet
    Packet* createPacket() override {
        return new GCHPRecoveryEndToSelf();
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
    // Define and return const static GCHPRecoveryEndToSelfPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
