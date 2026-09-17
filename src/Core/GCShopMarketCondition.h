//--------------------------------------------------------------------------------
//
// Filename    : GCShopMarketCondition.h
// Description : Packet that tells the player the server-side shop version.
//
//--------------------------------------------------------------------------------

#ifndef __GC_SHOP_MARKET_CONDITION_H__
#define __GC_SHOP_MARKET_CONDITION_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//--------------------------------------------------------------------------------
//
// class GCShopMarketCondition;
//
//--------------------------------------------------------------------------------

class GCShopMarketCondition : public Packet {
public:
    GCShopMarketCondition();
    virtual ~GCShopMarketCondition();

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SHOP_MARKET_CONDITION;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID + szMarketCond * 2;
    }

    // get packet name
    string getPacketName() const {
        return "GCShopMarketCondition";
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

    // get/set market condition buy
    MarketCond_t getMarketCondBuy(void) const {
        return m_MarketCondBuy;
    }
    void setMarketCondBuy(MarketCond_t cond) {
        m_MarketCondBuy = cond;
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
    MarketCond_t m_MarketCondBuy = 0;
    MarketCond_t m_MarketCondSell = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCShopMarketConditionFactory;
//
// Factory for GCShopMarketCondition
//
//////////////////////////////////////////////////////////////////////

class GCShopMarketConditionFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SHOP_MARKET_CONDITION;
    static constexpr std::string_view kName = "GCShopMarketCondition";
    static constexpr PacketSize_t kMaxSize{szObjectID + szMarketCond * 2};

    // create packet
    Packet* createPacket() override {
        return new GCShopMarketCondition();
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
    // Define and return const static GCShopMarketConditionPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
