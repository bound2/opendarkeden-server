//--------------------------------------------------------------------------------
//
// Filename    : GCDeleteObject.h
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __GC_DELETE_OBJECT_H__
#define __GC_DELETE_OBJECT_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//--------------------------------------------------------------------------------
//
// class GCDeleteObject;
//
// Sent when a particular object in the zone leaves the field of view; when the
// client receives it, it must use the OID inside to find the object and delete it from its zone.
//
// The following are concrete examples of when this packet is sent.
//
//(1) a PC logged out
//(2) a creature picks up an item
//(3) a particular effect disappeared
//(4) a corpse disappeared
//
// *CAUTION*
//
//(3) a particular effect disappeared.. the time at which an effect disappears is sent
// when the effect is created, so it does no harm to delete it. -_-;
//
//--------------------------------------------------------------------------------

class GCDeleteObject : public Packet {
public:
    // constructor
    GCDeleteObject() {}
    GCDeleteObject(ObjectID_t objectID) : m_ObjectID(objectID) {}
    ~GCDeleteObject(){};


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_DELETE_OBJECT;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }

    // get packet name
    string getPacketName() const {
        return "GCDeleteObject";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set object id
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t objectID) {
        m_ObjectID = objectID;
    }

private:
    // object id
    ObjectID_t m_ObjectID;
};


//////////////////////////////////////////////////////////////////////
//
// class GCDeleteObjectFactory;
//
// Factory for GCDeleteObject
//
//////////////////////////////////////////////////////////////////////

class GCDeleteObjectFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_DELETE_OBJECT;
    static constexpr std::string_view kName = "GCDeleteObject";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    // create packet
    Packet* createPacket() override {
        return new GCDeleteObject();
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
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
