//////////////////////////////////////////////////////////////////////
//
// Filename    : GCBloodDrainOK1.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_BLOOD_DRAIN_OK_1_H__
#define __GC_BLOOD_DRAIN_OK_1_H__

// include files
#include "Exception.h"
#include "ModifyInfo.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCBloodDrainOK1;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCBloodDrainOK1 : public ModifyInfo {
public:
    // constructor
    GCBloodDrainOK1();

    // destructor
    ~GCBloodDrainOK1();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_BLOOD_DRAIN_OK_1;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szObjectID + ModifyInfo::getPacketSize();
    }

    // get packet's name
    string getPacketName() const {
        return "GCBloodDrainOK1";
    }

    // get packet's debug string
    string toString() const;

    // get / set CEffectID
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
// class GCBloodDrainOK1Factory;
//
// Factory for GCBloodDrainOK1
//
//////////////////////////////////////////////////////////////////////

class GCBloodDrainOK1Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_BLOOD_DRAIN_OK_1;
    static constexpr std::string_view kName = "GCBloodDrainOK1";
    static constexpr PacketSize_t kMaxSize{szObjectID + ModifyInfo::getPacketMaxSize()};

    // constructor
    GCBloodDrainOK1Factory() {}

    // destructor
    virtual ~GCBloodDrainOK1Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCBloodDrainOK1();
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
