//////////////////////////////////////////////////////////////////////
//
// Filename    : CGPhoneDisconnect.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_PHONE_DISCONNECT_H__
#define __CG_PHONE_DISCONNECT_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class CGPhoneDisconnect;
//
//////////////////////////////////////////////////////////////////////

class CGPhoneDisconnect : public Packet {
public:
    // constructor
    CGPhoneDisconnect();

    // destructor
    ~CGPhoneDisconnect();

public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_PHONE_DISCONNECT;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static CGPhoneDisconnectPacketSize.
    PacketSize_t getPacketSize() const {
        return szSlotID;
    }

    // get packet name
    string getPacketName() const {
        return "CGPhoneDisconnect";
    }

    // get packet's debug string
    string toString() const;

public:
    // get / set phoneNumber
    SlotID_t getSlotID() const {
        return m_SlotID;
    }
    void setSlotID(SlotID_t SlotID) {
        m_SlotID = SlotID;
    }

private:
    // SlotID
    SlotID_t m_SlotID;
};


//////////////////////////////////////////////////////////////////////
//
// class CGPhoneDisconnectFactory;
//
// Factory for CGPhoneDisconnect
//
//////////////////////////////////////////////////////////////////////

class CGPhoneDisconnectFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_PHONE_DISCONNECT;
    static constexpr std::string_view kName = "CGPhoneDisconnect";
    static constexpr PacketSize_t kMaxSize{szSlotID};

    // create packet
    Packet* createPacket() override {
        return new CGPhoneDisconnect();
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
    // Define and return const static CGPhoneDisconnectPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGPhoneDisconnectHandler;
//
//////////////////////////////////////////////////////////////////////

class CGPhoneDisconnectHandler {
public:
    // execute packet's handler
    static void execute(CGPhoneDisconnect* pPacket, Player* player);
};

#endif
