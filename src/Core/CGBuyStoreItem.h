////////////////////////////////////////////////////////////////////////////////
// Filename    : CGBuyStoreItem.h
// Description :
// Sent when a player looks at a shop NPC's display window and wants to buy
// an item. The server checks that the player has enough money and enough
// room in the inventory, then hands the item over to the player.
////////////////////////////////////////////////////////////////////////////////

#ifndef __CG_BUY_STORE_ITEM_H__
#define __CG_BUY_STORE_ITEM_H__

#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
//
// class CGBuyStoreItem;
//
////////////////////////////////////////////////////////////////////////////////

class CGBuyStoreItem : public Packet {
public:
    CGBuyStoreItem(){};
    ~CGBuyStoreItem(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_BUY_STORE_ITEM;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szObjectID + szBYTE;
    }
    string getPacketName() const {
        return "CGBuyStoreItem";
    }
    string toString() const;

public:
    ObjectID_t getOwnerObjectID() {
        return m_OwnerObjectID;
    }
    void setOwnerObjectID(ObjectID_t ObjectID) {
        m_OwnerObjectID = ObjectID;
    }

    ObjectID_t getItemObjectID() {
        return m_ItemObjectID;
    }
    void setItemObjectID(ObjectID_t ObjectID) {
        m_ItemObjectID = ObjectID;
    }

    BYTE getIndex(void) const {
        return m_Index;
    }
    void setIndex(BYTE index) {
        m_Index = index;
    }

private:
    ObjectID_t m_OwnerObjectID = 0;
    ObjectID_t m_ItemObjectID = 0;
    BYTE m_Index = 0;
};


////////////////////////////////////////////////////////////////////////////////
//
// class CGBuyStoreItemFactory;
//
////////////////////////////////////////////////////////////////////////////////

class CGBuyStoreItemFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_BUY_STORE_ITEM;
    static constexpr std::string_view kName = "CGBuyStoreItem";
    static constexpr PacketSize_t kMaxSize{szObjectID + szObjectID + szBYTE};

    Packet* createPacket() override {
        return new CGBuyStoreItem();
    }
    string getPacketName() const override {
        return string(kName);
    }
    PacketID_t getPacketID() const override {
        return kPacketID;
    }
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


////////////////////////////////////////////////////////////////////////////////
//
// class CGBuyStoreItemHandler;
//
////////////////////////////////////////////////////////////////////////////////

class CGBuyStoreItemHandler {
public:
    static void execute(CGBuyStoreItem* pPacket, Player* player);
};

#endif
