//////////////////////////////////////////////////////////////////////
//
// Filename    : CGDialUp.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_DIAL_UP_H__
#define __CG_DIAL_UP_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class CGDialUp;
//
//////////////////////////////////////////////////////////////////////

class CGDialUp : public Packet {
public:
    // constructor
    CGDialUp();

    // destructor
    ~CGDialUp();

public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_DIAL_UP;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static CGDialUpPacketSize.
    PacketSize_t getPacketSize() const {
        return szPhoneNumber;
    }

    // get packet name
    string getPacketName() const {
        return "CGDialUp";
    }

    // get packet's debug string
    string toString() const;

public:
    // get / set phoneNumber
    PhoneNumber_t getPhoneNumber() const {
        return m_PhoneNumber;
    }
    void setPhoneNumber(PhoneNumber_t PhoneNumber) {
        m_PhoneNumber = PhoneNumber;
    }

private:
    // SlotID
    PhoneNumber_t m_PhoneNumber;
};


//////////////////////////////////////////////////////////////////////
//
// class CGDialUpFactory;
//
// Factory for CGDialUp
//
//////////////////////////////////////////////////////////////////////

class CGDialUpFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_DIAL_UP;
    static constexpr std::string_view kName = "CGDialUp";
    static constexpr PacketSize_t kMaxSize{szPhoneNumber};

    // create packet
    Packet* createPacket() override {
        return new CGDialUp();
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
    // Define and return const static CGDialUpPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGDialUpHandler;
//
//////////////////////////////////////////////////////////////////////

class CGDialUpHandler {
public:
    // execute packet's handler
    static void execute(CGDialUp* pPacket, Player* player);
};

#endif
