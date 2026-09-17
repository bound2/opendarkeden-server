//////////////////////////////////////////////////////////////////////
//
// Filename    : GCHPRecoveryStartToSelf.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_HP_RECOVERY_START_TO_SELF_H__
#define __GC_HP_RECOVERY_START_TO_SELF_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCHPRecoveryStartToSelf;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCHPRecoveryStartToSelf : public Packet {
public:
    // constructor
    GCHPRecoveryStartToSelf();

    // destructor
    ~GCHPRecoveryStartToSelf();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_HP_RECOVERY_START_TO_SELF;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szBYTE + szHP + szHP;
    }

    // get packet's name
    string getPacketName() const {
        return "GCHPRecoveryStartToSelf";
    }

    // get packet's debug string
    string toString() const;

    // get / set Delay
    BYTE getDelay() const {
        return m_Delay;
    }
    void setDelay(BYTE Delay) {
        m_Delay = Delay;
    }

    // get / set Period
    HP_t getPeriod() const {
        return m_Period;
    }
    void setPeriod(HP_t Period) {
        m_Period = Period;
    }

    // get / set Quantity
    HP_t getQuantity() const {
        return m_Quantity;
    }
    void setQuantity(HP_t Quantity) {
        m_Quantity = Quantity;
    }

private:
    // One turn
    BYTE m_Delay = 0;

    // How many times
    HP_t m_Period = 0;

    // How much
    HP_t m_Quantity = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCHPRecoveryStartToSelfFactory;
//
// Factory for GCHPRecoveryStartToSelf
//
//////////////////////////////////////////////////////////////////////

class GCHPRecoveryStartToSelfFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_HP_RECOVERY_START_TO_SELF;
    static constexpr std::string_view kName = "GCHPRecoveryStartToSelf";
    static constexpr PacketSize_t kMaxSize{szBYTE + szHP + szHP};

    // constructor
    GCHPRecoveryStartToSelfFactory() {}

    // destructor
    virtual ~GCHPRecoveryStartToSelfFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCHPRecoveryStartToSelf();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get Packet Max Size
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
