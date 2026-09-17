//////////////////////////////////////////////////////////////////////
//
// Filename    : GCTakeOff.h
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_TAKE_OFF_H__
#define __GC_TAKE_OFF_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCTakeOff;
//
// Packet used when the game server tells the client that a particular user
// has moved. It carries (CreatureID, X, Y, DIR).
//
//////////////////////////////////////////////////////////////////////

class GCTakeOff : public Packet {
public:
    // constructor
    GCTakeOff();

    // destructor
    ~GCTakeOff();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_TAKE_OFF;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szObjectID + szSlotID;
    }

    // get packet's name
    string getPacketName() const {
        return "GCTakeOff";
    }

    // get packet's debug string
    string toString() const;

    // get / set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    // get/set SlotID
    SlotID_t getSlotID() const {
        return m_SlotID;
    }
    void setSlotID(SlotID_t SlotID) {
        m_SlotID = SlotID;
    }

private:
    // ObjectID
    ObjectID_t m_ObjectID = 0;

    // SlotID
    SlotID_t m_SlotID = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCTakeOffFactory;
//
// Factory for GCTakeOff
//
//////////////////////////////////////////////////////////////////////

class GCTakeOffFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_TAKE_OFF;
    static constexpr std::string_view kName = "GCTakeOff";
    static constexpr PacketSize_t kMaxSize{szObjectID + szSlotID};

    // constructor
    GCTakeOffFactory() {}

    // destructor
    virtual ~GCTakeOffFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCTakeOff();
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
