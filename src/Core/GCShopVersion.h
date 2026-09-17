//--------------------------------------------------------------------------------
//
// Filename    : GCShopVersion.h
// Description : Packet that tells the player the server-side shop version.
//
//--------------------------------------------------------------------------------

#ifndef __GC_SHOP_VERSION_H__
#define __GC_SHOP_VERSION_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//--------------------------------------------------------------------------------
//
// class GCShopVersion;
//
//--------------------------------------------------------------------------------

class GCShopVersion : public Packet {
public:
    GCShopVersion();
    virtual ~GCShopVersion();

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SHOP_VERSION;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID + szShopVersion * 3 + szMarketCond;
    }

    // get packet name
    string getPacketName() const {
        return "GCShopVersion";
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
    ShopVersion_t getVersion(ShopRackType_t type) const {
        if (type >= SHOP_RACK_TYPE_MAX)
            throw InvalidProtocolException("GCShopVersion::getVersion() : Out of Bound!");
        return m_Version[type];
    }

    void setVersion(ShopRackType_t type, ShopVersion_t ver) {
        if (type >= SHOP_RACK_TYPE_MAX)
            throw InvalidProtocolException("GCShopVersion::setVersion() : Out of Bound!");
        m_Version[type] = ver;
    }

    // get/set market condition sell
    MarketCond_t getMarketCondSell(void) const {
        return m_MarketCondSell;
    }
    void setMarketCondSell(MarketCond_t cond) {
        m_MarketCondSell = cond;
    }

private:
    // NPC's object id
    ObjectID_t m_ObjectID = 0;

    // shop version
    ShopVersion_t m_Version[SHOP_RACK_TYPE_MAX] = {};

    MarketCond_t m_MarketCondSell = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCShopVersionFactory;
//
// Factory for GCShopVersion
//
//////////////////////////////////////////////////////////////////////

class GCShopVersionFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SHOP_VERSION;
    static constexpr std::string_view kName = "GCShopVersion";
    static constexpr PacketSize_t kMaxSize{szObjectID + szShopVersion * 3 + szMarketCond};

    // create packet
    Packet* createPacket() override {
        return new GCShopVersion();
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
    // Define and return const static GCShopVersionPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
