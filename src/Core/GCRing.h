//////////////////////////////////////////////////////////////////////
//
// Filename    : GCRing.h
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_RING_H__
#define __GC_RING_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class GCRing;
//
// Packet used when the game server tells the client that a particular user
// has moved. It carries (CreatureID, X, Y, DIR).
//
//////////////////////////////////////////////////////////////////////

class GCRing : public Packet {
public:
    // constructor
    GCRing();

    // destructor
    ~GCRing();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_RING;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szPhoneNumber + szSlotID + de::wire::stringWireSize(m_Name);
    }

    // get packet's name
    string getPacketName() const {
        return "GCRing";
    }

    // get packet's debug string
    string toString() const;

    // get/set PhoneNumber
    PhoneNumber_t getPhoneNumber() const {
        return m_PhoneNumber;
    }
    void setPhoneNumber(PhoneNumber_t PhoneNumber) {
        m_PhoneNumber = PhoneNumber;
    }

    // get/set SlotID
    SlotID_t getSlotID() const {
        return m_SlotID;
    }
    void setSlotID(SlotID_t SlotID) {
        m_SlotID = SlotID;
    }

    // get / set Name
    string getName() const {
        return m_Name;
    }
    void setName(const string& Name) {
        m_Name = Name;
    }

private:
    // PhoneNumber
    PhoneNumber_t m_PhoneNumber;

    // SlotID
    SlotID_t m_SlotID;

    // Name of the one who called.
    string m_Name;
};


//////////////////////////////////////////////////////////////////////
//
// class GCRingFactory;
//
// Factory for GCRing
//
//////////////////////////////////////////////////////////////////////

class GCRingFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_RING;
    static constexpr std::string_view kName = "GCRing";
    static constexpr PacketSize_t kMaxSize{szPhoneNumber + szSlotID + szBYTE + 20};

    // constructor
    GCRingFactory() {}

    // destructor
    virtual ~GCRingFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCRing();
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
