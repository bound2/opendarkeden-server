//////////////////////////////////////////////////////////////////////
//
// Filename    : GCGetOffMotorCycleFailed.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_GET_OFF_MOTORCYCLE_FAILED_H__
#define __GC_GET_OFF_MOTORCYCLE_FAILED_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCGetOffMotorCycleFailed;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////
class GCGetOffMotorCycleFailed : public Packet {
public:
    // constructor
    GCGetOffMotorCycleFailed();

    // destructor
    ~GCGetOffMotorCycleFailed();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_GET_OFF_MOTORCYCLE_FAILED;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet's name
    string getPacketName() const {
        return "GCGetOffMotorCycleFailed";
    }

    // get packet's debug string
    string toString() const;

private:
};


//////////////////////////////////////////////////////////////////////
//
// class GCGetOffMotorCycleFailedFactory;
//
// Factory for GCGetOffMotorCycleFailed
//
//////////////////////////////////////////////////////////////////////
class GCGetOffMotorCycleFailedFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_GET_OFF_MOTORCYCLE_FAILED;
    static constexpr std::string_view kName = "GCGetOffMotorCycleFailed";
    static constexpr PacketSize_t kMaxSize{0};

    // constructor
    GCGetOffMotorCycleFailedFactory() {}

    // destructor
    virtual ~GCGetOffMotorCycleFailedFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCGetOffMotorCycleFailed();
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
