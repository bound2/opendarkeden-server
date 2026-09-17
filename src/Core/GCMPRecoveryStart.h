//////////////////////////////////////////////////////////////////////
//
// Filename    : GCMPRecoveryStart.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_MP_RECOVERY_START_H__
#define __GC_MP_RECOVERY_START_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCMPRecoveryStart;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCMPRecoveryStart : public Packet {
public:
    // constructor
    GCMPRecoveryStart();

    // destructor
    ~GCMPRecoveryStart();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_MP_RECOVERY_START;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szBYTE + szMP + szMP;
    }

    // get packet's name
    string getPacketName() const {
        return "GCMPRecoveryStart";
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
    MP_t getPeriod() const {
        return m_Period;
    }
    void setPeriod(MP_t Period) {
        m_Period = Period;
    }

    // get / set Quantity
    MP_t getQuantity() const {
        return m_Quantity;
    }
    void setQuantity(MP_t Quantity) {
        m_Quantity = Quantity;
    }

private:
    // One turn
    BYTE m_Delay = 0;

    // How many times
    MP_t m_Period = 0;

    // How much
    MP_t m_Quantity = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCMPRecoveryStartFactory;
//
// Factory for GCMPRecoveryStart
//
//////////////////////////////////////////////////////////////////////

class GCMPRecoveryStartFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MP_RECOVERY_START;
    static constexpr std::string_view kName = "GCMPRecoveryStart";
    static constexpr PacketSize_t kMaxSize{szBYTE + szMP + szMP};

    // constructor
    GCMPRecoveryStartFactory() {}

    // destructor
    virtual ~GCMPRecoveryStartFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCMPRecoveryStart();
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
