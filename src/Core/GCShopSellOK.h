//--------------------------------------------------------------------------------
//
// Filename    : GCShopSellOK.h
// Description : When a player asks a shop NPC to buy goods and it
//               passes, this packet flies to the player.
//               The client takes this packet, updates the shop version,
//               checks the item information by object ID, and with the price
//               updates the player's money.
//
//--------------------------------------------------------------------------------

#ifndef __GC_SHOP_SELL_OK_H__
#define __GC_SHOP_SELL_OK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//--------------------------------------------------------------------------------
//
// class GCShopSellOK;
//
//--------------------------------------------------------------------------------

class GCShopSellOK : public Packet {
public:
    GCShopSellOK();
    virtual ~GCShopSellOK();

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SHOP_SELL_OK;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID + szShopVersion + szObjectID + szPrice;
    }

    // get packet name
    string getPacketName() const {
        return "GCShopSellOK";
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

    // get/set item object id
    ObjectID_t getItemObjectID() const {
        return m_ItemObjectID;
    }
    void setItemObjectID(ObjectID_t id) {
        m_ItemObjectID = id;
    }

    // get/set price
    Price_t getPrice() const {
        return m_Price;
    }
    void setPrice(Price_t price) {
        m_Price = price;
    }

private:
    // NPC's object id
    ObjectID_t m_ObjectID = 0;

    // Shop version
    ShopVersion_t m_Version = 0;

    // Item information
    ObjectID_t m_ItemObjectID = 0;

    // Price
    Price_t m_Price = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCShopSellOKFactory;
//
// Factory for GCShopSellOK
//
//////////////////////////////////////////////////////////////////////

class GCShopSellOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SHOP_SELL_OK;
    static constexpr std::string_view kName = "GCShopSellOK";
    static constexpr PacketSize_t kMaxSize{szObjectID + szShopVersion + szObjectID + szPrice};

    // create packet
    Packet* createPacket() override {
        return new GCShopSellOK();
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
