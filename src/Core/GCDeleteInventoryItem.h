//--------------------------------------------------------------------------------
//
// Filename    : GCDeleteInventoryItem.h
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __GC_DELETE_INVENTORY_ITEM_H__
#define __GC_DELETE_INVENTORY_ITEM_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//--------------------------------------------------------------------------------
//
// class GCDeleteInventoryItem;
//
//--------------------------------------------------------------------------------

class GCDeleteInventoryItem : public Packet {
public:
    // constructor
    GCDeleteInventoryItem() {}
    GCDeleteInventoryItem(ObjectID_t objectID) : m_ObjectID(objectID) {}
    ~GCDeleteInventoryItem(){};


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_DELETE_INVENTORY_ITEM;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }

    // get packet name
    string getPacketName() const {
        return "GCDeleteInventoryItem";
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
// class GCDeleteInventoryItemFactory;
//
// Factory for GCDeleteInventoryItem
//
//////////////////////////////////////////////////////////////////////

class GCDeleteInventoryItemFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_DELETE_INVENTORY_ITEM;
    static constexpr std::string_view kName = "GCDeleteInventoryItem";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    // create packet
    Packet* createPacket() override {
        return new GCDeleteInventoryItem();
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
