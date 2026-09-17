//////////////////////////////////////////////////////////////////////
//
// Filename    : GCHPRecoveryEndToOthers.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_HP_RECOVERY_END_TO_OTHERS_H__
#define __GC_HP_RECOVERY_END_TO_OTHERS_H__

// include files
#include "EffectInfo.h"
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCHPRecoveryEndToOthers;
//
////////////////////////////////////////////////////////////////////

class GCHPRecoveryEndToOthers : public Packet {
public:
    GCHPRecoveryEndToOthers();

    virtual ~GCHPRecoveryEndToOthers();

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_HP_RECOVERY_END_TO_OTHERS;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCHPRecoveryEndToOthersPacketSize.
    PacketSize_t getPacketSize() const {
        return szObjectID + szHP;
    }

    // get packet's name
    string getPacketName() const {
        return "GCHPRecoveryEndToOthers";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set creature ID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t creatureID) {
        m_ObjectID = creatureID;
    }

    // get /set CurrentHP
    HP_t getCurrentHP() const {
        return m_CurrentHP;
    }
    void setCurrentHP(HP_t CurrentHP) {
        m_CurrentHP = CurrentHP;
    }

private:
    // Used to tell objects apart by an id that is unique within the zone.
    ObjectID_t m_ObjectID;

    // Current HP
    HP_t m_CurrentHP;
};


//////////////////////////////////////////////////////////////////////
//
// class GCHPRecoveryEndToOthersFactory;
//
// Factory for GCHPRecoveryEndToOthers
//
//////////////////////////////////////////////////////////////////////

class GCHPRecoveryEndToOthersFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_HP_RECOVERY_END_TO_OTHERS;
    static constexpr std::string_view kName = "GCHPRecoveryEndToOthers";
    static constexpr PacketSize_t kMaxSize{szObjectID + szHP};

    // create packet
    Packet* createPacket() override {
        return new GCHPRecoveryEndToOthers();
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
    // Define and return const static GCHPRecoveryEndToOthersPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
