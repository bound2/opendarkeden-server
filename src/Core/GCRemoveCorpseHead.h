//--------------------------------------------------------------------------------
//
// Filename    : GCRemoveCorpseHead.h
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __GC_REMOVE_CORPSE_HEAD_H__
#define __GC_REMOVE_CORPSE_HEAD_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//--------------------------------------------------------------------------------
//
// class GCRemoveCorpseHead;
//
// Sent when a particular object in the zone leaves the field of view; on receiving it the client
// has to find the matching object by the OID inside the packet and delete it from the client's zone.
//
// The following are concrete examples of when this packet is sent.
//
//(1) When the PC logs out
//(2) When a creature picks the item up
//(3) When a particular effect disappears
//(4) When the corpse disappears
//
// *CAUTION*
//
//(3) When a particular effect disappears.. the time it disappears is sent when the effect is created,
// so it would do no harm to delete it. -_-;
//
//--------------------------------------------------------------------------------

class GCRemoveCorpseHead : public Packet {
public:
    // constructor
    GCRemoveCorpseHead() {}

    GCRemoveCorpseHead(ObjectID_t objectID) : m_ObjectID(objectID) {}
    ~GCRemoveCorpseHead(){};


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_REMOVE_CORPSE_HEAD;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }

    // get packet name
    string getPacketName() const {
        return "GCRemoveCorpseHead";
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
    ObjectID_t m_ObjectID = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCRemoveCorpseHeadFactory;
//
// Factory for GCRemoveCorpseHead
//
//////////////////////////////////////////////////////////////////////

class GCRemoveCorpseHeadFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_REMOVE_CORPSE_HEAD;
    static constexpr std::string_view kName = "GCRemoveCorpseHead";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    // create packet
    Packet* createPacket() override {
        return new GCRemoveCorpseHead();
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
