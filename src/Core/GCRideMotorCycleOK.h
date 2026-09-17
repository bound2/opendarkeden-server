//////////////////////////////////////////////////////////////////////
//
// Filename    : GCRideMotorCycleOK.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_RIDE_MOTORCYCLE_OK_H__
#define __GC_RIDE_MOTORCYCLE_OK_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCRideMotorCycleOK;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////
class GCRideMotorCycleOK : public Packet {
public:
    // constructor
    GCRideMotorCycleOK();

    // destructor
    ~GCRideMotorCycleOK();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_RIDE_MOTORCYCLE_OK;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }

    // get packet's name
    string getPacketName() const {
        return "GCRideMotorCycleOK";
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

private:
    // ObjectID
    ObjectID_t m_ObjectID = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCRideMotorCycleOKFactory;
//
// Factory for GCRideMotorCycleOK
//
//////////////////////////////////////////////////////////////////////
class GCRideMotorCycleOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_RIDE_MOTORCYCLE_OK;
    static constexpr std::string_view kName = "GCRideMotorCycleOK";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    // constructor
    GCRideMotorCycleOKFactory() {}

    // destructor
    virtual ~GCRideMotorCycleOKFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCRideMotorCycleOK();
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
#endif
