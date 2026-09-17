//////////////////////////////////////////////////////////////////////
//
// Filename    : GCRemoveFromGear.h
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_REMOVE_FROM_GEAR_H__
#define __GC_REMOVE_FROM_GEAR_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCRemoveFromGear;
//
// Packet used when the game server tells the client that a particular user
// has moved. It carries (CreatureID, X, Y, DIR).
//
//////////////////////////////////////////////////////////////////////

class GCRemoveFromGear : public Packet {
public:
    // constructor
    GCRemoveFromGear();

    // destructor
    ~GCRemoveFromGear();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_REMOVE_FROM_GEAR;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szSlotID;
    }

    // get packet's name
    string getPacketName() const {
        return "GCRemoveFromGear";
    }

    // get packet's debug string
    string toString() const;

    // get/set SlotID
    SlotID_t getSlotID() const {
        return m_SlotID;
    }
    void setSlotID(SlotID_t SlotID) {
        m_SlotID = SlotID;
    }

private:
    // SlotID
    SlotID_t m_SlotID = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCRemoveFromGearFactory;
//
// Factory for GCRemoveFromGear
//
//////////////////////////////////////////////////////////////////////

class GCRemoveFromGearFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_REMOVE_FROM_GEAR;
    static constexpr std::string_view kName = "GCRemoveFromGear";
    static constexpr PacketSize_t kMaxSize{szSlotID};

    // constructor
    GCRemoveFromGearFactory() {}

    // destructor
    virtual ~GCRemoveFromGearFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCRemoveFromGear();
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
