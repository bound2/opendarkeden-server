//////////////////////////////////////////////////////////////////////
//
// Filename    : GCGetOffMotorCycleOK.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_GET_OFF_MOTORCYCLE_OK_H__
#define __GC_GET_OFF_MOTORCYCLE_OK_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCGetOffMotorCycleOK;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////
class GCGetOffMotorCycleOK : public Packet {
public:
    // constructor
    GCGetOffMotorCycleOK();

    // destructor
    ~GCGetOffMotorCycleOK();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_GET_OFF_MOTORCYCLE_OK;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet's name
    string getPacketName() const {
        return "GCGetOffMotorCycleOK";
    }

    // get packet's debug string
    string toString() const;

private:
};


//////////////////////////////////////////////////////////////////////
//
// class GCGetOffMotorCycleOKFactory;
//
// Factory for GCGetOffMotorCycleOK
//
//////////////////////////////////////////////////////////////////////
class GCGetOffMotorCycleOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_GET_OFF_MOTORCYCLE_OK;
    static constexpr std::string_view kName = "GCGetOffMotorCycleOK";
    static constexpr PacketSize_t kMaxSize{0};

    // constructor
    GCGetOffMotorCycleOKFactory() {}

    // destructor
    virtual ~GCGetOffMotorCycleOKFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCGetOffMotorCycleOK();
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
