//--------------------------------------------------------------------------------
//
// Filename    : GCShopSold.h
// Description : One player bought goods from a shop NPC, and
//               another player was also talking to the same shop NPC,
//               that player's item list has to be resynchronised.
//               This packet is the one for that.
//
//--------------------------------------------------------------------------------

#ifndef __GC_SHOP_SOLD_H__
#define __GC_SHOP_SOLD_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//--------------------------------------------------------------------------------
//
// class GCShopSold;
//
//--------------------------------------------------------------------------------

class GCShopSold : public Packet {
public:
    GCShopSold();
    virtual ~GCShopSold();

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SHOP_SOLD;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID + szShopVersion + szShopRackType + szBYTE;
    }

    // get packet name
    string getPacketName() const {
        return "GCShopSold";
    }

    // get packet's debug string
    string toString() const;


public:
    // get/set NPC's object id
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t creatureID) {
        m_ObjectID = creatureID;
    }

    // get/set shop version
    ShopVersion_t getShopVersion(void) const {
        return m_Version;
    }
    void setShopVersion(const ShopVersion_t ver) {
        m_Version = ver;
    }

    // get/set rack type
    ShopRackType_t getShopType(void) const {
        return m_RackType;
    }
    void setShopType(ShopRackType_t type) {
        m_RackType = type;
    }

    // get/set rack index
    BYTE getShopIndex(void) const {
        return m_RackIndex;
    }
    void setShopIndex(BYTE index) {
        m_RackIndex = index;
    }

private:
    // NPC's object id
    ObjectID_t m_ObjectID = 0;

    // Shop version
    ShopVersion_t m_Version = 0;

    // Display rack kind
    ShopRackType_t m_RackType = 0;

    // Display rack index
    BYTE m_RackIndex = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCShopSoldFactory;
//
// Factory for GCShopSold
//
//////////////////////////////////////////////////////////////////////

class GCShopSoldFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SHOP_SOLD;
    static constexpr std::string_view kName = "GCShopSold";
    static constexpr PacketSize_t kMaxSize{szObjectID + szShopVersion + szShopRackType + szBYTE};

    // create packet
    Packet* createPacket() override {
        return new GCShopSold();
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
    // *OPTIMIZATION HINT*
    // Define and return const static GCShopSoldPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
