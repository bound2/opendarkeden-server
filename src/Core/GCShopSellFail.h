//--------------------------------------------------------------------------------
//
// Filename    : GCShopSellFail.h
// Description : The player tried to sell goods to an NPC and, for some
//               reason it failed, so this packet is sent to the player.
//
//--------------------------------------------------------------------------------

#ifndef __GC_SHOP_SELL_FAIL_H__
#define __GC_SHOP_SELL_FAIL_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//--------------------------------------------------------------------------------
//
// class GCShopSellFail;
//
//--------------------------------------------------------------------------------

class GCShopSellFail : public Packet {
public:
    GCShopSellFail();
    virtual ~GCShopSellFail();

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SHOP_SELL_FAIL;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }

    // get packet name
    string getPacketName() const {
        return "GCShopSellFail";
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


private:
    // NPC's object id
    ObjectID_t m_ObjectID = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCShopSellFailFactory;
//
// Factory for GCShopSellFail
//
//////////////////////////////////////////////////////////////////////

class GCShopSellFailFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SHOP_SELL_FAIL;
    static constexpr std::string_view kName = "GCShopSellFail";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    // create packet
    Packet* createPacket() override {
        return new GCShopSellFail();
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
    // Define and return const static GCShopSellFailPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
